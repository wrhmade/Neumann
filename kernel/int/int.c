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


#define TIMER_INT_SERIAL_NOOUTPUT 1//发生时钟中断时不输出串口信息

static isr_t interrupt_handlers[256];

void isr_handler(registers_t regs)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    asm_cli();

    if(regs.int_no==7)//NM中断
    {
        fpu_handler(7);
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
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO; 
    char s[200];
    if(regs.int_no!=0x20 || !TIMER_INT_SERIAL_NOOUTPUT)//不是时钟中断
    {
      sprintf(s,"IRQ Reversed:0x%x\n",regs.int_no);
      serial_putstr(s);  
    }
    


    if (regs.int_no >= 0x28) io_out8(0xA0, 0x20); // 给从片发EOI
    io_out8(0x20, 0x20); // 给主片发EOI

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

void irq_mask_clear(unsigned char irq) {
  unsigned short port;
  unsigned char value;

  if (irq < 8) {
    port = PIC0_IMR;
  } else {
    port = PIC1_IMR;
    irq -= 8;
  }
  value = io_in8(port) & ~(1 << irq);
  io_out8(port, value);
}
void irq_mask_set(unsigned char irq) 
{
  unsigned short port;
  unsigned char value;

  if (irq < 8) {
    port = PIC0_IMR;
  } else {
    port = PIC1_IMR;
    irq -= 8;
  }
  value = io_in8(port) | (1 << irq);
  io_out8(port, value);
}