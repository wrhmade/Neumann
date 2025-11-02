/*
hpet.c
高级配置和电源管理接口驱动
Copyright W24 Studio 
*/

#include <acpi.h>
#include <krnlcons.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <int.h>

HpetInfo *hpet_addr;
static uint32_t hpetPeriod = 0;

void timer_callback(registers_t *regs);

uint64_t nano_time(void)
{
    if (hpet_addr == 0) return 0;
    uint64_t mcv = hpet_addr->mainCounterValue;
    return mcv * hpetPeriod;
}

void hpet_init(Hpet *hpet)
{
    char s[100];
    hpet_addr = (HpetInfo *)(hpet->base_address.address);
    sprintf(s,"HPET: HPET Base address mapped to virtual address %p\n", hpet_addr);
    krnlcons_putstr(s);

    uint32_t counterClockPeriod = hpet_addr->generalCapabilities >> 32;
    hpetPeriod                  = counterClockPeriod / 1000000;
    hpet_addr->mainCounterValue = 0;

    sprintf(s,"HPET: HPET Main counter is initialized to 0\n");
    krnlcons_putstr(s);
    sprintf(s,"HPET: HPET Counter Clock Period = %u (ns)\n", counterClockPeriod);
    krnlcons_putstr(s);
    sprintf(s,"HPET: HPET Timer Period = %u (us)\n", hpetPeriod);
    krnlcons_putstr(s);
 
    hpet_addr->generalConfiguration |= 1;
    register_interrupt_handler(IRQ0, &timer_callback);
    sprintf(s,"HPET: HPET General Configuration Register set to %p\n", hpet_addr->generalConfiguration);
    krnlcons_putstr(s);
}
