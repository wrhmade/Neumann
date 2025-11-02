/*
power.c
电源管理
Copyright W24 Studio 
*/

#include <power.h>
#include <acpi.h>
#include <krnlcons.h>
#include <nasmfunc.h>
#include <timer.h>
#include <task.h>
#include <window.h>
#include <stdio.h>
#include <sheet.h>
#include <graphic.h>

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

void scheduled_poweroff_taskmain(int time)
{
    int remaining_time=time;
    task_t *task=task_now();
    int h=0,m=0,s=0;
    for(;;)
    {
        if(remaining_time==0)
        {
            poweroff();
        }
        h=remaining_time/3600;
        m=remaining_time%3600/60;
        s=remaining_time-h*3600-m*60;
        char str[1000];
        if(task->langmode==1 || task->langmode==2)
        {
            sprintf(str,"电脑将在%02d:%02d:%02d后关闭，请注意保存你的数据，谨防丢失",h,m,s);
        }
        else
        {
            sprintf(str,"The computer will shut down after %02d:%02d:%02d. Please save your data carefully to prevent loss",h,m,s);
        }
        putstr_ascii_sheet(task->window->sheet,task->window->xsize/2-strlen(str)*8/2,(task->window->ysize-18)/2-4+18,0x000000,0xFFFFFF,str);
        sheet_refresh(task->window->sheet,0,0,task->window->xsize-1,task->window->ysize-1);
        sleep(1000000000);
        remaining_time--;
    }
}

void scheduled_poweroff(int time)
{
    if(time==0)
    {
        poweroff();
    }
    else
    {
        task_t *task=create_kernel_task(scheduled_poweroff_taskmain);
        task->tss.esp-=4;
        *((int *)(task->tss.esp+4))=time;
        name_task(task,"Scheduled Poweroff");
        task->langmode=task_now()->langmode;

        window_t *window=create_window(
                task_now()->langmode==0?"Scheduled Poweroff":((task_now()->langmode==1 || task_now()->langmode==2)?"定时关闭":"Scheduled Poweroff"),
            800,100,-1,0);

        window_settask(window,task);
        task_run(task);
    }

}