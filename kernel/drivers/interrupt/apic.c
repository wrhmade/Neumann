/*
apic.c
高级可编程中断处理器
Copyright W24 Studio 
*/

#include <acpi.h>
#include <krnlcons.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <cpu.h>
#include <int.h>

typedef union PointerCast {
        void *ptr;
        uintptr_t val;
} PointerCast;

int x2apic_mode;

PointerCast lapic_ptr;
PointerCast ioapic_ptr;

void disable_pic(void)
{
    io_out8(0x21, 0xff);
    io_out8(0xa1, 0xff);
}

void ioapic_write(uint32_t reg, uint32_t value)
{
    *(volatile uint32_t *)ioapic_ptr.ptr = reg;
    PointerCast reg_ptr;
    reg_ptr.val = ioapic_ptr.val + 0x10;
    *(volatile uint32_t *)reg_ptr.ptr = value;
}

uint32_t ioapic_read(uint32_t reg)
{
    *(volatile uint32_t *)ioapic_ptr.ptr = reg;
    PointerCast reg_ptr;
    reg_ptr.val = ioapic_ptr.val + 0x10;
    return *(volatile uint32_t *)reg_ptr.ptr;
}

void ioapic_add(ioapic_routing *routing)
{
    uint32_t ioredtbl = (uint32_t)(0x10 + (uint32_t)(routing->irq * 2));
    uint64_t redirect = routing->vector;
    redirect |= lapic_id() << 56;
    ioapic_write(ioredtbl, (uint32_t)redirect);
    ioapic_write(ioredtbl + 1, (uint32_t)(redirect >> 32));
}

void lapic_write(uint32_t reg, uint32_t value)
{
    if (x2apic_mode) {
        io_wrmsr(0x800 + (reg >> 4), value);
        return;
    }
    PointerCast reg_ptr;
    reg_ptr.val = lapic_ptr.val + reg;
    *(volatile uint32_t *)reg_ptr.ptr = value;
}

uint32_t lapic_read(uint32_t reg)
{
    if (x2apic_mode) return io_rdmsr(0x800 + (reg >> 4));
    PointerCast reg_ptr;
    reg_ptr.val = lapic_ptr.val + reg;
    return *(volatile uint32_t *)reg_ptr.ptr;
}

uint64_t lapic_id()
{
    return lapic_read(LAPIC_REG_ID);
}

void local_apic_init()
{
    char s[50];
    x2apic_mode = cpu_x2apic();
    if (x2apic_mode)
        sprintf(s,"APIC: LAPIC = x2APIC\n");
    else
        sprintf(s,"APIC: LAPIC = xAPIC\n");
    krnlcons_putstr(s);
    lapic_write(LAPIC_REG_SPURIOUS, 0xff | 1 << 8);
    lapic_write(LAPIC_REG_TIMER, IRQ0);
    lapic_write(LAPIC_REG_TIMER_DIV, 11);
    lapic_write(LAPIC_REG_TIMER_INITCNT, ~((uint32_t)0));
    uint64_t start = nano_time();
    while (nano_time() - start < 1000000);

    uint64_t lapic_timer              = (~(uint32_t)0) - lapic_read(LAPIC_REG_TIMER_CURCNT);
    uint64_t calibrated_timer_initial = (uint64_t)((uint64_t)(lapic_timer * 1000) / 250);

    lapic_write(LAPIC_REG_TIMER, lapic_read(LAPIC_REG_TIMER) | 1 << 17);
    lapic_write(LAPIC_REG_TIMER_INITCNT, calibrated_timer_initial);
}

void io_apic_init()
{
    ioapic_routing *ioapic_router[] = {
        &(ioapic_routing) {IRQ0,  0 }, // Timer
        &(ioapic_routing) {IRQ1,  1 }, // Keyboard
        &(ioapic_routing) {IRQ5,  5 }, // Sound Card
        &(ioapic_routing) {IRQ6,  6 }, // Floppy
        &(ioapic_routing) {IRQ12, 12}, // Mouse
        &(ioapic_routing) {IRQ14, 14}, // IDE0
        &(ioapic_routing) {IRQ15, 15}, // IDE1
        0,
    };

    ioapic_routing **routing = ioapic_router;

    while (*routing != 0) {
        ioapic_add(*routing);
        routing++;
    }
}

void send_eoi()
{
    lapic_write(0xb0, 0);
}

void lapic_timer_stop()
{
    lapic_write(LAPIC_REG_TIMER_INITCNT, 0);
    lapic_write(LAPIC_REG_TIMER, (1 << 16));
}

void send_ipi(uint32_t apic_id, uint32_t command)
{
    if (x2apic_mode) {
        lapic_write(APIC_ICR_LOW, (((uint64_t)apic_id) << 32) | command);
    } else {
        lapic_write(APIC_ICR_HIGH, apic_id << 24);
        lapic_write(APIC_ICR_LOW, command);
    }
}

void apic_init(MADT *madt)
{
    char s[50];
    lapic_ptr.ptr = (void *)madt->local_apic_address;
    sprintf(s,"APIC: LAPIC Base address %p\n",lapic_ptr.ptr);
    krnlcons_putstr(s);

    uint8_t *entries_base = (uint8_t *)&madt->entries;
    size_t current        = 0;

    while (current < madt->h.Length - sizeof(MADT)) {
        MadtHeader *header = (MadtHeader *)(entries_base + current);
        if (header->entry_type == MADT_APIC_IO) {
            MadtIOApic *ioapic = (MadtIOApic *)(entries_base + current);
            ioapic_ptr.ptr     = (void *)ioapic->address;
		    sprintf(s,"APIC: IOAPIC Found at %p\n",ioapic_ptr.ptr);
            krnlcons_putstr(s);
        }
        current += header->length;
    }
    disable_pic();
    local_apic_init();
    io_apic_init();
}

int get_apic_mode()
{
    return x2apic_mode;
}