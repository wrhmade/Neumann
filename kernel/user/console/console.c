/*
console.c
控制台
Copyright W24 Studio 
*/

#include <console.h>
#include <window.h>
#include <task.h>
#include <graphic.h>
#include <sheet.h>
#include <fifo.h>
#include <mm.h>
#include <string.h>
#include <binfo.h>
#include <macro.h>
#include <stdint.h>

void console_main();

console_t *open_console(void)
{
    console_t *console=(console_t *)malloc(sizeof(console_t));
    window_t *window=create_window("Console",80*8+2,25*16+18+1,-1);
    show_window(window);
    move_window(window,120,120);
    task_t *task=create_kernel_task(console_main);
    window_settask(window,task);
    task_run(task);
    console->window=window;
    window->console=console;
    console->curx=console->cury=0;
    window->isconsole=1;
    int i,j;
    for(i=0;i<80;i++)
    {
        for(j=0;j<25;j++)
        {
            console->consbuf[i][j]=0;
        }
    }
    return window;
}

void close_console(console_t *console)
{
    close_window(console->window);
    task_remove(console->window->task);
    free(console->consbuf);
    free(console);
}

void console_refresh(console_t *console)
{
    char s[2];
    int i,j;
    boxfill(console->window->sheet->buf,console->window->xsize,1,18,console->window->xsize-2,console->window->ysize-2,0x000000);
    for(i=0;i<80;i++)
    {
        for(j=0;j<25;j++)
        {
            boxfill(console->window->sheet->buf,console->window->xsize,i*8+1,j*16+18,i*8+1+7,j*16+18+15,0x000000);
            if(console->consbuf[i][j]!='\0')
            {
                s[0]=console->consbuf[i][j];
                putstr_ascii(console->window->sheet->buf,console->window->xsize,i*8+1,j*16+18,0xFFFFFF,s);
            }
        }
    }
    for(i=console->curx*8+1;i<=console->curx*8+8;i++)
    {
        for(j=console->cury*16+18;j<=console->cury*16+33;j++)
        {
            console->window->sheet->buf[j*642+i]=0xFFFFFF-console->window->sheet->buf[j*642+i];
        }
    }
    sheet_refresh(console->window->sheet,0,0,console->window->xsize-1,console->window->ysize-1);
}

void console_newline(console_t *console)
{
    int i,j;
    if(console->cury>=24)
    {
        for(i=0;i<80;i++)
        {
            for(j=1;j<25;j++)
            {
                console->consbuf[i][j-1]=console->consbuf[i][j];
            }
        }
        for(i=0;i<80;i++)
        {
            console->consbuf[i][24]='\0';
        }
        console->cury=24;
    }
    else
    {
        console->cury++;
    }
    console->curx=0;
    console_refresh(console);
}

void console_movcur(console_t *console,int x,int y)
{
    console->curx=x;
    console->cury=y;
    console_refresh(console);
}

void console_setchr(console_t *console,int x,int y,char c)
{
    console->consbuf[x][y]=c;
    console_refresh(console);
}

static int console_putchar_sub(console_t *console,char c,int refresh)
{
    int a=0;
    if(c=='\n')
    {
        console_newline(console);
        a=1;
    }
    else if(c=='\t')
    {
        console_movcur(console,(console->curx + 4) & ~(4 - 1),console->cury);
    }
     else if(c=='\r')
    {
        console_movcur(console,0,console->cury);
    }
    else if(c=='\b')
    {
        console_movcur(console,console->curx-1,console->cury);
        console_setchr(console,console->curx,console->cury,0);
    }
    else
    {
        console->consbuf[console->curx++][console->cury]=c;
        if(console->curx>=80)
        {
            console_newline(console);
            a=1;
        }
    }
    if(refresh)console_refresh(console);
    return a;
}

int console_putchar(console_t *console,char c)
{
    return console_putchar_sub(console,c,1);
}

int console_putchar_norefresh(console_t *console,char c)
{
    return console_putchar_sub(console,c,0);
}

void console_putstr(console_t *console,char *s)
{
    int i;
    for(i=0;s[i]!=0;i++)
    {
        console_putchar_norefresh(console,s[i]);
    }
    console_refresh(console);
}

void console_cleanscreen(console_t *console)
{
    int i,j;
    for(i=0;i<80;i++)
    {
        for(j=0;j<25;j++)
        {
            console->consbuf[i][j]=0;
        }
    }
    console_movcur(console,0,0);
    console_refresh(console);
}

char *console_input(console_t *console,int len)
{
    task_t *task=console->window->task;
    char *str=(char *)malloc(sizeof(char)*(len+1));
    int i,index=0,line=0;
    
    for(;;)
    {
        if(fifo_status(&task->fifo)>0)
        {
            i=fifo_get(&task->fifo)-256;
            if(i=='\n')
            {
                str[index]=0;
                console_putchar(console,'\n');
                break;
            }
            else if(i=='\b')
            {
                if(index)
                {
                    if(console->curx==0 && line>0)
                    {
                        console_movcur(console,79,console->cury-1);
                        console_setchr(console,console->curx,console->cury,0);
                        line--;
                    }
                    else
                    {
                        console_putchar(console,'\b');
                    }
                    index--;
                }
            }
            else if(i>=20 && i<=255)
            {
                if(index<=len)
                {
                    str[index]=i;
                    if(console_putchar(console,i))
                    {
                        line++;
                    }
                    index++;
                }
            }
        }
    }
    return str;
}

void console_main()
{
    task_t *task=task_now();
    boxfill(task->window->sheet->buf,task->window->xsize,1,18,task->window->xsize-2,task->window->ysize-2,0x000000);
    sheet_refresh(task->window->sheet,0,0,task->window->xsize-1,task->window->ysize-1);
    console_refresh(task->window->console);

    console_putstr(task->window->console,"Neumann Console\n\tCopyright(c) 2023-2024 W24 Studio & 71GN Deep Space\n\n");

    char *sp;

	for(;;)
	{
        console_putstr(task->window->console,"[Command]");

        sp=console_input(task->window->console,80);
        cmd_run(task->window->console,sp);
        free(sp);
	}
}

void cmd_run(console_t *console,char *cmdline)
{
    task_t *task=task_now();
    if(strcmp(cmdline,"mem")==0)
    {
        cmd_mem(console);
    }
    else if(strcmp(cmdline,"exit")==0)
    {
        close_console(console);
    }
    else if(strcmp(cmdline,"cls")==0)
    {
        console_cleanscreen(console);
    }
    else if(strcmp(cmdline,"")==0)
    {
        
    }
    else 
    {
        console_putstr(console,"Invalid command.\n");
    }
}

void cmd_mem(console_t *console)
{
    char s[60];
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    sprintf(s,"Memory Total:%uMB\n",binfo->memtotal/1024/1024);
    console_putstr(console,s);
    sprintf(s,"Free:%uMB\n",free_space_total()/1024/1024);
    console_putstr(console,s);
    sprintf(s,"Used:%uMB\n",(binfo->memtotal-free_space_total())/1024/1024);
    console_putstr(console,s);
}