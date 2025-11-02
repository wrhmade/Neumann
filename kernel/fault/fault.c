/*
fault.c
异常处理程序
Copyright W24 Studio 
*/

#include <fault.h>
#include <int.h>
#include <binfo.h>
#include <graphic.h>
#include <macro.h>
#include <nasmfunc.h>
#include <stdio.h>
#include <string.h>
#include <buzzer.h>
#include <com.h>
#include <krnlcons.h>
#include <ELF.h>

sym_info_t get_symbol_info(void *kernel_file_address, Elf32_Addr symbol_address);

void fault_process(registers_t regs)
{ 
    krnlcons_display();

    krnlcons_change_backcolor(0x0000AA);
	//不设置为黑屏
	krnlcons_change_forecolor(0xffffff);
    krnlcons_cleanscreen();
    krnlcons_putstr("Your system is currently experiencing some issues, which are preventing you from continuing to use it. If this is your first time encountering this issue, please force a restart. If you frequently encounter this issue, consider whether there is a problem with your computer or system.\n");
    krnlcons_putstr("Here are some technical information that can help you solve this error:\n");
    
    

    krnlcons_putstr("System Fault\n");


    char type[200];
	switch(regs.int_no)
	{
		case 0:
			strcpy(type,"#DE - Division Error");
			break;
		case 1:
			strcpy(type,"#DB - Debug Exceptions");
			break;
		case 2:
			strcpy(type,"NMI");
			break;
		case 3:
			strcpy(type,"#BP - Breakpoint");
			break;
		case 4:
			strcpy(type,"#OF - Overflow");
			break;
		case 5:
			strcpy(type,"#BR - Bound Check");
			break;
		case 6:
			strcpy(type,"#UD - Invalid Opcode");
			break;
		case 7:
			strcpy(type,"#NM - Coprocessor Not Available");
			break;
		case 8:
			strcpy(type,"#DF - Double Fault");
			break;
		case 9:
			strcpy(type,"CSO - Coprocesser Segment Overrun");
			break;
		case 10:
			strcpy(type,"#TS - INvalid TSS");
			break;
		case 11:
			strcpy(type,"#NP - Segment Not Present");
			break;
		case 12:
			strcpy(type,"#SS - Stack Exception");
			break;
		case 13:
			strcpy(type,"#GP - General Protection Exception");
			break;
		case 14:
			strcpy(type,"#PF - Page Fault");
			break;
		case 16:
			strcpy(type,"#MF - Coprocessor Error");
			break;
		case 17:
			strcpy(type,"#AC");
			break;
		case 18:
			strcpy(type,"#MC");
			break;
		case 19:
			strcpy(type,"#XM");
			break;
		case 20:
			strcpy(type,"#VE");
			break;
		default:
			strcpy(type,"OTH");
			break;
	}

    char s[500];
    sprintf(s,"INT %02XH\n%s\n",regs.int_no,type);


    krnlcons_putstr(s);
    serial_putstr(s);
    write_serial('\n');

    sprintf(s,"CS  0x%08X DS  0x%08X SS  0x%08X EIP 0x%08X\n",regs.cs,regs.ds,regs.ss,regs.eip);
    krnlcons_putstr(s);
    serial_putstr(s);
    sprintf(s,"EAX 0x%08X EBX 0x%08X ECX 0x%08X EDX 0x%08X\n",regs.eax,regs.ebx,regs.ecx,regs.edx);
    krnlcons_putstr(s);
    serial_putstr(s);
    sprintf(s,"EDI 0x%08X ESI 0x%08X ESP 0x%08X EBP 0x%08X\n",regs.edi,regs.esi,regs.esp,regs.ebp);
    krnlcons_putstr(s);
    serial_putstr(s);
    sprintf(s,"EFLAGS 0x%08X USER ESP 0x%08X ERROR CODE 0x%08X\n",regs.eflags,regs.user_esp,regs.err_code);
    krnlcons_putstr(s);
    serial_putstr(s);

    if(regs.int_no==14)//如果是Page Fault
    {
        uint32_t faulting_address;
        asm volatile("mov %%cr2, %0" : "=r" (faulting_address)); //

        int present = !(regs.err_code & 0x1); // 页不存在
        int rw = regs.err_code & 0x2; // 只读页被写入
        int us = regs.err_code & 0x4; // 用户态写入内核页
        int reserved = regs.err_code & 0x8; // 写入CPU保留位
        int id = regs.err_code & 0x10; // 由取指引起

        if (present) {
            sprintf(s,"*** Page Fault:Present *** at 0x%08X",faulting_address);
        } else if (rw) {
            sprintf(s,"*** Page Fault:Read-Only *** at 0x%08X",faulting_address);
        } else if (us) {
            sprintf(s,"*** Page Fault:User-Mode *** at 0x%08X",faulting_address);
        } else if (reserved) {
            sprintf(s,"*** Page Fault:Reversed *** at 0x%08X",faulting_address);
        } else if (id) {
            sprintf(s,"*** Page Fault:Decode Address *** at 0x%08X",faulting_address);
        }
        krnlcons_putstr(s);
    }

    serial_putstr("\nSystem Halted.");
	print_stack_trace();
    for(;;)
    {
        
        asm_hlt();
    }
}

void print_stack_trace(void)
{
    char s[50];
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    union RbpNode {
            uintptr_t inner;
            union RbpNode *next;
    } *ebp;

    uintptr_t eip;
    __asm__ volatile("movl %%ebp, %0" : "=r"(ebp));
    __asm__ volatile("call 1f\n1: popl %0" : "=r"(eip));

	sprintf(s,"\nCall Trace:\n <TASK>\n");
    krnlcons_putstr(s);
    serial_putstr(s);

    for (int i = 0; i < 16 && eip && (uintptr_t)ebp > 0x1000; ++i) {
        sym_info_t sym_info = get_symbol_info(binfo->kernel_elf_base, eip);
        if (!sym_info.name) {
            sprintf(s,"  [<%p>] %s\n", eip, "unknown");
			krnlcons_putstr(s);
			serial_putstr(s);
        } else {
            sprintf(s,"  [<%p>] `%s`+%p\n", eip, sym_info.name, eip - sym_info.addr);
			krnlcons_putstr(s);
			serial_putstr(s);
        }
        eip = *(uintptr_t *)(ebp + 1);
        ebp = ebp->next;
    }
    sprintf(s," </TASK>\n");
	krnlcons_putstr(s);
	serial_putstr(s);
}
