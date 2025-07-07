/*
power.c
电源管理
Copyright W24 Studio 
*/

#include <power.h>
#include <acpi.h>
#include <krnlcons.h>

void poweroff()
{
    asm_cli();
    krnlcons_display();
    krnlcons_cleanscreen();
    krnlcons_putstr("Power off...");
    acpi_poweroff();
    krnlcons_cleanscreen();
    krnlcons_putstr("It is now safe to shut down the computer.");
    for(;;);
}

void reboot()
{
    asm_cli();
    krnlcons_display();
    krnlcons_cleanscreen();
    krnlcons_putstr("Rebooting...");
    acpi_reboot();
    for(;;);
}