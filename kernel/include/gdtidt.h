/*
gdtidt.h
GDT&IDT管理程序头文件
Copyright W24 Studio 
*/

#ifndef GDTIDT_H
#define GDTIDT_H
#include <stdint.h>
struct gdt_entry_struct {
    uint16_t limit_low; // BYTE 0~1
    uint16_t base_low; // BYTE 2~3
    uint8_t base_mid; // BYTE 4
    uint8_t access_right; // BYTE 5, P|DPL|S|TYPE (1|2|1|4)
    uint8_t limit_high; // BYTE 6, G|D/B|0|AVL|limit_high (1|1|1|1|4)
    uint8_t base_high; // BYTE 7
} __attribute__((packed));

struct gdt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

typedef struct gdt_entry_struct gdt_entry_t;

struct idt_entry_struct {
    uint16_t offset_low, selector; // offset_low里没有一坨，selector为对应的保护模式代码段
    uint8_t dw_count, access_right; // dw_count始终为0，access_right的值大多与硬件规程相关，只需要死记硬背，不需要进一步了解（
    uint16_t offset_high; // offset_high里也没有一坨
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

typedef struct idt_ptr_struct idt_ptr_t;

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();


void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint16_t ar);
void idt_set_gate(uint8_t num, uint32_t offset, uint16_t sel, uint8_t flags);

void init_gdtidt(void);
#endif