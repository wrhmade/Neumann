/*
int.c
中断处理程序
Copyright W24 Studio 
*/

#include <nasmfunc.h>
#include <int.h>
#include <graphic.h>
#include <binfo.h>
#include <stdio.h>
#include <macro.h>
#include <fault.h>
#include <com.h>
#include <message.h>
#include <task.h>
#include <fpu.h>
#include <com.h>
#include <acpi.h>
#include <io.h>
#include <limits.h>

#define TIMER_INT_SERIAL_NOOUTPUT 1//发生时钟中断时不输出串口信息

static isr_t interrupt_handlers[256];

void isr_handler(registers_t regs)
{
    asm_cli();

    if(regs.int_no==7)//NM中断
    {
        fpu_handler(7);
        return;
    }

    if(regs.cs%4==3)
    {
      char s[200];
      sprintf(s,"An application stopped due to an illegal operation. EIP=0x%08X",regs.eip);
      error_message(s,"异常操作");
      task_exit(-1);
    }
    else
    {
      fault_process(regs);
    }

    while (1);
}

void irq_handler(registers_t regs)
{
    char s[200];
    if(regs.int_no!=0x20 || !TIMER_INT_SERIAL_NOOUTPUT)//不是时钟中断
    {
      sprintf(s,"IRQ Reversed:0x%x\n",regs.int_no);
      serial_putstr(s);  
    }

    send_eoi();

    if (interrupt_handlers[regs.int_no])
    {
        isr_t handler = interrupt_handlers[regs.int_no]; // 有自定义处理程序，调用之
        handler(&regs); // 传入寄存器
    }
}

void register_interrupt_handler(uint8_t n, isr_t handler)
{
    char s[200];
    sprintf(s,"IRQ Regsitered:0x%x\n",n);
    serial_putstr(s);  
    interrupt_handlers[n] = handler;
}
