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

void fault_process(registers_t regs)
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    boxfill(binfo->vram,binfo->scrnx,0,0,binfo->scrnx-1,binfo->scrny-1,0x000000);
    
    putstr_ascii(binfo->vram,binfo->scrnx,0,0,0xFFFFFF,"System Fault");

    char s[200];
    sprintf(s,"ISR:%u",regs.int_no);


    putstr_ascii(binfo->vram,binfo->scrnx,0,16,0xFFFFFF,s);

    sprintf(s,"CS  %010u DS  %010u SS  %010u EIP %010u",regs.cs,regs.ds,regs.ss,regs.eip);
    putstr_ascii(binfo->vram,binfo->scrnx,0,32,0xFFFFFF,s);
    sprintf(s,"EAX %010u EBX %010u ECX %010u EDX %010u",regs.eax,regs.ebx,regs.ecx,regs.edx);
    putstr_ascii(binfo->vram,binfo->scrnx,0,48,0xFFFFFF,s);
    sprintf(s,"EDI %010u ESI %010u ESP %010u EBP %010u",regs.edi,regs.esi,regs.esp,regs.ebp);
    putstr_ascii(binfo->vram,binfo->scrnx,0,64,0xFFFFFF,s);
    sprintf(s,"EFLAGS %010u USER EIP %010u ERROR CODE %010u",regs.eflags,regs.user_esp,regs.err_code);
    putstr_ascii(binfo->vram,binfo->scrnx,0,80,0xFFFFFF,s);

    for(;;)
    {
        asm_hlt();
    }
}