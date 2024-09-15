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
#include <hd.h>
#include <fat16.h>
#include <binfo.h>
#include <macro.h>
#include <ini.h>
#include <lpt.h>
void console_main();


console_t *open_console(void)
{
    console_t *console=(console_t *)malloc(sizeof(console_t));
    window_t *window;
    task_t *task=task_now();
    if(task->langmode==0)
    {
        window=create_window("Console",80*8+2,25*16+18+1,-1,1);
    }
    else if(task->langmode==1 || task->langmode==2)
    {
        window=create_window("控制台",80*8+2,25*16+18+1,-1,1);
    }
    show_window(window);
    move_window(window,120,120);
    task_t *console_task=create_kernel_task(console_main);
    window_settask(window,console_task);
    task_run(console_task);
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
    for(i=0;i<25;i++)
    {
        for(j=0;j<80;j++)
        {
            boxfill(console->window->sheet->buf,console->window->xsize,j*8+1,i*16+18,j*8+1+7,i*16+18+15,0x000000);
            if(console->consbuf[j][i]!='\0')
            {
                s[0]=console->consbuf[j][i];
                putstr_ascii(console->window->sheet->buf,console->window->xsize,j*8+1,i*16+18,0xFFFFFF,s);
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
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    
    task->langmode=(binfo->hzk16==NULL)?((binfo->hzk16f==NULL)?0:2):1;
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
    else if(strcmp(cmdline,"newconsole")==0)
    {
        open_console();
    }
    else if(strcmp(cmdline,"count")==0)
    {
        cmd_count(console);
    }
    else if(strncmp(cmdline,"print ",6)==0)
    {
        cmd_print(console,cmdline+6);
    }
    else if(strcmp(cmdline,"dir")==0)
    {
        cmd_dir(console);
    }
    else if(strcmp(cmdline,"dzero")==0)
    {
        int a=114514/(1919810-1919810);
    }
    else if(strncmp(cmdline,"echo ",5)==0)
    {
        console_putstr(console,cmdline+5);
        console_putchar(console,'\n');
    }
    else if(strncmp(cmdline,"mkfile ",7)==0)
    {
        fat16_create_file(NULL,cmdline+7);
    }
    else if(strncmp(cmdline,"rdfile ",7)==0)
    {
        fileinfo_t finfo;
        if(fat16_open_file(&finfo,cmdline+7)==-1)
        {
            if(task->langmode==0)
            {
                console_putstr(console,"No such file.\n");
            }
            else if(task->langmode==1 || task->langmode==2)
            {
                console_putstr(console,"没有这样的文件.\n");
            } 
        }
        else
        {
            int status;
            char *buf = (char *) malloc(finfo.size + 5);
            status = fat16_read_file(&finfo, buf);
            console_putstr(console,buf);
            console_putchar(console,'\n');
            free(buf);
        }
    }
    else if(strcmp(cmdline,"wrfile")==0)
    {
        fileinfo_t finfo;
        if(fat16_open_file(&finfo,"114514.txt")==-1)
        {
            if(task->langmode==0)
            {
                console_putstr(console,"No such file.\n");
            }
            else if(task->langmode==1 || task->langmode==2)
            {
                console_putstr(console,"没有这样的文件.\n");
            } 
        }
        else
        {
            int status;
            char *buf = (char *) malloc(512);
            strcpy(buf,"1145141919810");
            status = fat16_write_file(&finfo, buf, strlen(buf));
            free(buf);
        }
    }
    else if(strncmp(cmdline,"del ",4)==0)
    {
        if(fat16_delete_file(cmdline+4)!=0)
        {
            if(task->langmode==0)
            {
                console_putstr(console,"No such file or delete error.\n");
            }
            else if(task->langmode==1 || task->langmode==2)
            {
                console_putstr(console,"没有这样的文件或删除时出错.\n");
            } 
        }
    }
    else if(strncmp(cmdline,"langmode ",9)==0)
    {
        cmd_langmode(console,cmdline[9]-'0');
    }
    else if(strcmp(cmdline,"")==0)
    {
        
    }
    else 
    {
        if(task->langmode==0)
        {
            console_putstr(console,"Invalid command.\n");
        }
        else if(task->langmode==1 || task->langmode==2)
        {
            console_putstr(console,"此命令无效.\n");
        }
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

void cmd_count(console_t *console)
{
    char s[60];
    int tick=0;
    for(;;)
    {
        sprintf(s,"%010u\r",tick);
        console_putstr(console,s);
        tick++;
    }
}

void cmd_dir(console_t *console)
{
    int entries,i,total=0,total_kb=0;;
    fileinfo_t *root_dir = read_dir_entries(&entries);
    task_t *task=task_now();
    if(task->langmode==0)
    {
        console_putstr(console,"Files on disk:\n");
    }
    else if(task->langmode==1 || task->langmode==2)
    {
        console_putstr(console,"磁盘上的文件:\n");
    }
    char s[50],s2[10];
    for (i = 0; i < entries; i++)
    {
        if(root_dir[i].name[0]!=0xe5)
        {
            strncpy(s2,root_dir[i].name,8);
            sprintf(s,"%s.%s\t%dKB\t",s2,root_dir[i].ext,root_dir[i].size/1024);
            console_putstr(console,s);
            
            console_putchar(console,'\n');
            total++;
            total_kb+=root_dir[i].size/1024;
        }
    }
    if(task->langmode==0)
    {
        sprintf(s,"\n\t%d file(s) total\n\t%d KB total\n",total,total_kb);
    }
    else if(task->langmode==1 || task->langmode==2)
    {
        sprintf(s,"\n\t一共有%d个文件\n\t总共%dKB\n",total,total_kb);
    }
    console_putstr(console,s);
    free(root_dir);
    console_putchar(console,'\n');
}

void cmd_langmode(console_t *console,int lmode)
{
    task_t *task=task_now();
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    if(lmode!=0 && lmode!=1 && lmode!=2)
    {
        if(task->langmode==0)
        {
            console_putstr(console,"Language Mode Error.\n");
        }
        else if(task->langmode==1 || task->langmode==2)
        {
            console_putstr(console,"语言模式出错.\n");
        }
    }
    else
    {
        if(lmode==1 && binfo->hzk16==NULL)
        {
            console_putstr(console,"HZK16.bin not loaded!Please check if this file exists or check neumann.ini.\n");
        }
        else if(lmode==2 && binfo->hzk16f==NULL)
        {
            console_putstr(console,"HZK16F.bin not loaded!Please check if this file exists or check neumann.ini\n");
        }
        else
        {
            task->langmode=lmode;
        }
    }

    if(task->langmode==0)
    {
        console_putstr(console,"Now you are in ASCII English mode.\n");
    }
    else if(task->langmode==1)
    {
        console_putstr(console,"你目前处于GB2312简体中文模式下.\n");
    }
    else if(task->langmode==2)
    {
        console_putstr(console,"你目前处于GB2312繁体中文模式下.\n");
    }
}

void cmd_print(console_t *console,char *filename)
{
    fileinfo_t finfo;
    task_t *task=task_now();
    char s[40];
    if(fat16_open_file(&finfo,filename)!=0)
    {
        if(task->langmode==1 || task->langmode==2)
        {
            console_putstr(console,"找不到文件.\n");
        }
        else
        {
            console_putstr(console,"No such file.\n");
        }
        return;
    }
    char *buf=(char *)malloc(sizeof(char)*(finfo.size+5));

    // if(task->langmode==1 || task->langmode==2)
    // {
    //     console_putstr(console,"请在打印机放纸,并按回车键.......");
    // }
    // else
    // {
    //     console_putstr(console,"Please place paper on the printer and press Enter...");
    // }
    // console_input(console,10);
    fat16_read_file(&finfo,buf);

    if(task->langmode==1 || task->langmode==2)
    {
        console_putstr(console,"正在打印.......\n");
    }
    else
    {
        console_putstr(console,"Printing...\n");
    }
    int i=0;
    window_t *window;
    if(task->langmode==1 || task->langmode==2)
    {
        window=create_window("打印",102,34,0,0);
    }
    else
    {
        window=create_window("Print",102,34,0,0);
    }
    
    while(buf[i])
    {
        lpt_put(buf[i]);
        boxfill(window->sheet->buf,window->xsize,1,18,window->xsize-2,window->ysize-2,0xC6C6C6);
        boxfill(window->sheet->buf,window->xsize,1,18,(int)((i+1)*100/strlen(buf))+1,window->ysize-2,0xFF0000);
        sprintf(s,"%d%%",(int)((i+1)*100/strlen(buf)));
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-strlen(s)*8/2,18,0x000000,s);
        sheet_refresh(window->sheet,0,0,window->xsize-1,window->ysize-1);
        i++;
    }
    if(task->langmode==1 || task->langmode==2)
    {
        console_putstr(console,"完成\n");
    }
    else
    {
        console_putstr(console,"Done\n");
    }
    close_window(window);
    free(buf);

}