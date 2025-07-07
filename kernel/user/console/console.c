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
#include <binfo.h>
#include <macro.h>
#include <ini.h>
#include <lpt.h>
#include <math.h>
#include <message.h>
#include <ELF.h>
#include <pci.h>
#include <theme.h>
#include <desktop.h>
#include <disk.h>
#include <vfs.h>
#include <vfile.h>
#include <fullscreen.h>
#include <timer.h>
#include <power.h>
void console_main();

static char commands[60][15]={"mem","exit","cls","newconsole","count","bootinfo","messtest","print",
			"ls","mkdir","dzero","msdemo","lspci","echo","mkfile","rdfile","del","langmode","finfo","pcinfo",
            "themeset","cd","fullscreen","poweroff","reboot"};

#define COMMAND_NOW 22 //命令数量

#define DEFAULT_COLOR 0xAAAAAA

console_t *open_console(void)
{
    console_t *console=(console_t *)kmalloc(sizeof(console_t));
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
    console->running_app=NULL;
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
    if(console->running_app!=NULL)
    {
        task_remove(console->running_app);
    }
    close_window(console->window);
    task_remove(console->window->task);
    kfree(console->consbuf);
    kfree(console);
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
                putstr_ascii(console->window->sheet->buf,console->window->xsize,j*8+1,i*16+18,console->colorbuf[j][i],s);
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
                console->colorbuf[i][j-1]=console->colorbuf[i][j];
            }
        }
        for(i=0;i<80;i++)
        {
            console->consbuf[i][24]='\0';
            console->colorbuf[i][24]=DEFAULT_COLOR;
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

void console_setcolor(console_t *console,int x,int y,uint32_t color)
{
    console->colorbuf[x][y]=color;
    console_refresh(console);
}

static int console_putchar_sub(console_t *console,char c,int refresh,uint32_t color)
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
        if(console->curx>0)
        {
            console_movcur(console,console->curx-1,console->cury);
            console_setchr(console,console->curx,console->cury,0);
        }
    }
    else
    {
        console->colorbuf[console->curx][console->cury]=color;
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

int console_putchar_color(console_t *console,char c,uint32_t color)
{
    return console_putchar_sub(console,c,1,color);
}

int console_putchar_color_norefresh(console_t *console,char c,uint32_t color)
{
    return console_putchar_sub(console,c,0,color);
}

int console_putchar(console_t *console,char c)
{
    return console_putchar_sub(console,c,1,DEFAULT_COLOR);
}

int console_putchar_norefresh(console_t *console,char c)
{
    return console_putchar_sub(console,c,0,DEFAULT_COLOR);
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

void console_putstr_color(console_t *console,char *s,uint32_t color)
{
    int i;
    for(i=0;s[i]!=0;i++)
    {
        console_putchar_color_norefresh(console,s[i],color);
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
    char *str=(char *)kmalloc(sizeof(char)*(len+1));
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



int console_getkey(console_t *console)
{
    task_t *task=console->window->task;
    int i;

    for(;;)
    {
        if(fifo_status(&task->fifo)>0)
        {
            i=fifo_get(&task->fifo)-256;
            return i;
        }
    }
}

static int cmd_parse(char *cmd_str, char **argv, char token)
{
    int arg_idx = 0;
    while (arg_idx < MAX_ARG_NR) {
        argv[arg_idx] = NULL;
        arg_idx++;
    } // 开局先把上一个argv抹掉
    char *next = cmd_str; // 下一个字符
    int argc = 0; // 这就是要返回的argc了
    while (*next) { // 循环到结束为止
        if (*next != '"') {
            while (*next == token) *next++; // 多个token就只保留第一个，windows cmd就是这么处理的
            if (*next == 0) break; // 如果跳过完token之后结束了，那就直接退出
            argv[argc] = next; // 将首指针赋值过去，从这里开始就是当前参数
            while (*next && *next != token) next++; // 跳到下一个token
        } else {
            next++; // 跳过引号
            argv[argc] = next; // 这里开始就是当前参数
            while (*next && *next != '"') next++; // 跳到引号
        }
        if (*next) { // 如果这里有token字符
            *next++ = 0; // 将当前token字符设为0（结束符），next后移一个
        }
        if (argc > MAX_ARG_NR) return -1; // 参数太多，超过上限了
        argc++; // argc增一，如果最后一个字符是空格时不提前退出，argc会错误地被多加1
    }
    return argc;
}


static void console_put_prompt()
{
    char s[300];
    task_t *task=task_now();
    sprintf(s,"[%s]",task->work_dir);
    console_putstr(task->window->console,s);
}

void console_main()
{
    task_t *task=task_now();
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    
    task->langmode=(binfo->hzk16==NULL)?((binfo->hzk16f==NULL)?0:2):1;
    boxfill(task->window->sheet->buf,task->window->xsize,1,18,task->window->xsize-2,task->window->ysize-2,0x000000);
    sheet_refresh(task->window->sheet,0,0,task->window->xsize-1,task->window->ysize-1);
    console_refresh(task->window->console);

    console_putstr_color(task->window->console,"Neumann Console\n\tCopyright(c) 2023-2025 W24 Studio & 71GN Deep Space\n\n",0xFFFFFF);
        //console_putstr(task->window->console,"2");
    char value[20];
    int exist;
    //cmd_run(task->window->console,"nsh.bin");

    console_putstr_color(task->window->console,"Build-in shell\n",0x00FF00);

    char *sp;

    //cmd_run(task->window->console,"myapp.bin");

    
	for(;;)
	{
        memset(task->window->console->cmdline,0,CMDLINE_MAXLEN);
        
        
        console_put_prompt();
        sp=console_input(task->window->console,CMDLINE_MAXLEN);
        cmd_run(task->window->console,sp);
        kfree(sp);
	}
}

static void get_execuable_file(char *cmdline,char *filename)
{
    char *p=cmdline,*q=filename;
    while(*p!=0 && *p!=' ')
    {
        *q=*p;
        p++;
        q++;
    }
    *q=0;//结束
}

void cmd_run(console_t *console,char *cmdline)
{
    task_t *task=task_now();
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    //console_putstr(console,cmdline);
    if(strcmp(cmdline,"mem")==0)
    {
        cmd_mem(console);
    }
    else if(strcmp(cmdline,"fdtest")==0)
    {
        char buffer[512];
        for(int i=0;i<512;i++)
        {
            buffer[i]='A';
        }
        my_disk_write_device(DEVICE_FD,0,1,buffer);
    }
    else if(strcmp(cmdline,"fullscreen")==0)
    {
        fullscreen_show();
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
    else if(strcmp(cmdline,"bootinfo")==0)
    {
        cmd_bootinfo(console);
    }
    else if(strcmp(cmdline,"messtest")==0)
    {
        warn_message("This is a test message.","Warning");
    }
    else if(strncmp(cmdline,"print ",6)==0)
    {
        cmd_print(console,cmdline+6);
    }
    else if(strncmp(cmdline,"cd ",3)==0)
    {
        cmd_cd(console,cmdline+3);
    }
    else if(strcmp(cmdline,"ls")==0)
    {
        cmd_ls(console);
    }
    else if(strcmp(cmdline,"dzero")==0)
    {
        int a=114514/(1919810-1919810);
    }
    else if(strcmp(cmdline,"msdemo")==0)
    {
        start_msdemo();
    }
    else if(strcmp(cmdline,"poweroff")==0)
    {
        poweroff();
    }
    else if(strcmp(cmdline,"reboot")==0)
    {
        reboot();
    }
    else if(strcmp(cmdline,"lspci")==0)
    {
        cmd_lspci(console);
    }
    else if(strcmp(cmdline,"pcinfo")==0)
    {
        print_pcinfo(console);
    }
    else if(strncmp(cmdline,"echo ",5)==0)
    {
        console_putstr(console,cmdline+5);
        console_putchar(console,'\n');
    }
    else if(strncmp(cmdline,"mkdir ",6)==0)
    {
        cmd_mkdir(console,cmdline+6);
    }
    else if(strncmp(cmdline,"themeset ",9)==0)
    {
        if(vfs_open(cmdline+9)==0)
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
            desktop_reload(cmdline+9);
        }
    }
    else if(strncmp(cmdline,"mkfile ",7)==0)
    {
        vfs_mkfile(cmdline+7);
    }
    else if(strncmp(cmdline,"rdfile ",7)==0)
    {
        vfs_node_t node;
        if(cmdline[7]!='/')
        {
            char *abspath=rel2abs(cmdline+7);
            node=vfs_open(abspath);
            kfree(abspath);
        }
        else
        {
            node=vfs_open(cmdline+7);
        }
        if(node==0)
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
            if(node->type==file_dir)
            {
                if(task->langmode==0)
                {
                    console_putstr(console,"This is a directory.\n");
                }
                else if(task->langmode==1 || task->langmode==2)
                {
                    console_putstr(console,"这是一个目录.\n");
                } 
            }
            else
            {
                int status;
                char *buf = (char *) kmalloc(node->size + 5);
                vfs_read(node,buf,0,node->size);
                console_putstr(console,buf);
                console_putchar(console,'\n');
                kfree(buf);
            }
        }
    }
    else if(strncmp(cmdline,"del ",4)==0)
    {
        vfs_node_t node;
        node=vfs_open(cmdline+4);
        if(node==0)
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
        else
        {
            vfs_free(node);
        }
    }
    else if(strncmp(cmdline,"langmode ",9)==0)
    {
        cmd_langmode(console,cmdline[9]-'0');
    }
    else if(strncmp(cmdline,"finfo ",6)==0)
    {
        cmd_finfo(console,cmdline+6);
    }
    else if(strcmp(cmdline,"")==0)
    {
        
    }
    else 
    {
        char main[60];
        int exist=0;
        get_execuable_file(cmdline,main);
        int ret = try_to_run_external(main, &exist,cmdline, task_now()->window);
        if (!exist) {
            if(task->langmode==0)
            {
                console_putstr(console,"Invalid command.\n");
            }
            else if(task->langmode==1 || task->langmode==2)
            {
                console_putstr(console,"此命令无效.\n");
            }
            cmd_autofill(console,cmdline);
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

void cmd_ls(console_t *console)
{
    file_ls(console,task_now()->work_dir);
}

void cmd_mkdir(console_t *console,char *dirname)
{
    task_t *task=task_now();
    char *dirname_back=strdup(dirname);
    if(file_mkdir(dirname_back)==-1)
    {
        if(task->langmode==1 || task->langmode==2)
        {
            console_putstr(console,"目录创建错误.\n");
        }
        else
        {
            console_putstr(console,"Directory create error.\n");
        }
    }
    kfree(dirname_back);
}

void cmd_cd(console_t *console,char *dirname)
{
    task_t *task=task_now();
    // if(dirname[0]!='/')
    // {
    //     strcpy(dirname,rel2abs(dirname));
    // }
    int status=file_cd(dirname);
    char s[200];
    if(status==-1)
    {
        if(task->langmode==1 || task->langmode==2)
        {
            console_putstr(console,"找不到目录.\n");
        }
        else
        {
            console_putstr(console,"No such directory.\n");
        }
    }
    else if(status==-2)
    {
        if(task->langmode==1 || task->langmode==2)
        {
            sprintf(s,"%s不是目录.\n",dirname);
        }
        else
        {
            sprintf(s,"\"%s\" is not a directory.\n",dirname);
        }
        console_putstr(console,s);
    }
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
    vfs_node_t file;
    task_t *task=task_now();
    char s[40];
    file=vfs_open(filename);
    if(file==0)
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
    char *buf=(char *)kmalloc(sizeof(char)*(file->size+5));

    if(task->langmode==1 || task->langmode==2)
    {
        console_putstr(console,"请在打印机放纸,并按回车键.......");
    }
    else
    {
        console_putstr(console,"Please place paper on the printer and press Enter...");
    }
    console_input(console,10);
    vfs_read(file,buf,0,file->size);

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
        info_message("打印完成.","打印");
    }
    else
    {
        info_message("Printing completed.","Print");
    }
    close_window(window);
    kfree(buf);

}

void cmd_bootinfo(console_t *console)
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    task_t *task=task_now();
    char s[60];

    if(task->langmode==1 || task->langmode==2)
    {
        console_putstr(console,"启动信息:\n");
        sprintf(s,"分辨率:%dx%d\n",binfo->scrnx,binfo->scrny);
        console_putstr(console,s);
        sprintf(s,"宽高比:%d:%d\n",binfo->scrnx/(get_GCD(binfo->scrnx,binfo->scrny)),binfo->scrny/(get_GCD(binfo->scrnx,binfo->scrny)));
        console_putstr(console,s);
        sprintf(s,"色深:%d位\n",binfo->vmode);
        console_putstr(console,s);
        sprintf(s,"显存起始地址:%8p\n",binfo->vram);
        console_putstr(console,s);
    }
    else
    {
        console_putstr(console,"Boot Infomation:\n");
        sprintf(s,"Display resolution:%dx%d\n",binfo->scrnx,binfo->scrny);
        console_putstr(console,s);
        sprintf(s,"Aspect ratio:%d:%d\n",binfo->scrnx/(get_GCD(binfo->scrnx,binfo->scrny)),binfo->scrny/(get_GCD(binfo->scrnx,binfo->scrny)));
        console_putstr(console,s);
        sprintf(s,"Color Depth:%d位\n",binfo->vmode);
        console_putstr(console,s);
        sprintf(s,"Starting address of graphics memory:%8p\n",binfo->vram);
        console_putstr(console,s);
    }
    
}

static char* get_ftype(char *ext1,int langmode)
{
    char *type=(char *)kmalloc(sizeof(char)*50);
    char *ext=(char *)kmalloc(sizeof(char)*4);
    strncpy(ext,ext1,3);
    if(strcmp(ext,"BIN")==0)
    {
        if(langmode==1 || langmode==2)
        {
            strcpy(type,"二进制文件");
        }
        else
        {
            strcpy(type,"Binary File");
        }
    }
    else if(strcmp(ext,"TXT")==0)
    {
        if(langmode==1 || langmode==2)
        {
            strcpy(type,"文本文件");
        }
        else
        {
            strcpy(type,"Text File");
        }
    }
    else if(strcmp(ext,"INI")==0)
    {
        if(langmode==1 || langmode==2)
        {
            strcpy(type,"配置文件");
        }
        else
        {
            strcpy(type,"Configure File");
        }
    }
    else if(strcmp(ext,"BMP")==0)
    {
        if(langmode==1 || langmode==2)
        {
            strcpy(type,"位图文件");
        }
        else
        {
            strcpy(type,"Bitmap File");
        }
    }
    else if(strcmp(ext,"JPG")==0)
    {
        if(langmode==1 || langmode==2)
        {
            strcpy(type,"JEPG图片文件");
        }
        else
        {
            strcpy(type,"JPEG Image File");
        }
    }
    else if(strcmp(ext,"PRG")==0)
    {
        if(langmode==1 || langmode==2)
        {
            strcpy(type,"Neumann旧版可执行文件");
        }
        else
        {
            strcpy(type,"Neumann Old Executable File");
        }
    }
    else
    {
        if(langmode==1 || langmode==2)
        {
            strcpy(type,"未知");
        }
        else
        {
            strcpy(type,"Unknown");
        }
    }
    kfree(ext);
    return type;
}

static int isexecutable(vfs_node_t file,char *result)
{
    char *buf=(char *)kmalloc(file->size);
    task_t *task=task_now();
    vfs_read(file,buf,0,file->size);
    if(elf32Validate((Elf32_Ehdr *)buf))
    {
        if(result!=NULL)
        {
            if(task->langmode==1 || task->langmode==2)
            {
                strcpy(result,"可执行文件(ELF格式)");
            }
            else
            {
                strcpy(result,"Executable File (ELF Format)");
            }
        }
        kfree(buf);
        return 1;
    }
    else if(file->size>=36 && strncmp(buf + 4, "WPRG", 4) == 0 && buf[0] == 0x00)
    {
        if(result!=NULL)
        {
            if(task->langmode==1 || task->langmode==2)
            {
                strcpy(result,"可执行文件(Neumann旧版格式)");
            }
            else
            {
                strcpy(result,"Executable File (Neumann Old Format)");
            }
        }
        kfree(buf);
        return 2;    
    }
    else
    {
        if(result!=NULL)
        {

            if(task->langmode==1 || task->langmode==2)
            {
                strcpy(result,"不可执行文件");
            }
            else
            {
                strcpy(result,"Unexecutable File");
            }
        }
        kfree(buf);
        return 0;
    }
}

void cmd_finfo(console_t *console,char *filename)
{
    vfs_node_t file;
    task_t *task=task_now();
    file=vfs_open(filename);
    if(file==0)
    {
        if(task->langmode==1 || task->langmode==2)
        {
            console_putstr(console,"找不到这个文件\n");
        }
        else
        {
            console_putstr(console,"File not found.\n");
        }
        return;
    }
    char s[100];
    if(task->langmode==1 || task->langmode==2)
    {
        sprintf(s,"文件名:%s\n",file->name);
    }
    else
    {
        sprintf(s,"File Name:%s\n",file->name);
    }
    console_putstr(console,s);
    if(task->langmode==1 || task->langmode==2)
    {
        sprintf(s,"文件大小:%uKB\n",file->size/1024);
    }
    else
    {
        sprintf(s,"File Size:%uKB\n",file->size/1024);
    }
    console_putstr(console,s);
    char ext[200];
    int flag=0;
    int j=0;
    for(int i=0;i<strlen(file->name);i++)
    {
        if(file->name[i]=='.')
        {
            flag=1;
        }
        if(flag)
        {
            ext[j++]=file->name[i];
        }
    }
    ext[j]=0;
    char *type=get_ftype(ext,task->langmode);
    if(task->langmode==1 || task->langmode==2)
    {
        sprintf(s,"文件类型:%s\n",type);
    }
    else
    {
        sprintf(s,"File Type:%s\n",type);
    }
    console_putstr(console,s);
    kfree(type);
    char *result=(char *)kmalloc(sizeof(char)*50);
    isexecutable(file,result);
    if(task->langmode==1 || task->langmode==2)
    {
        sprintf(s,"可执行:%s\n",result);
    }
    else
    {
        sprintf(s,"Executable:%s\n",result);
    }
    console_putstr(console,s);
    kfree(result);
}


//自动预测
void cmd_autofill(console_t *console,char *cmdline)
{
    int i,first_print=0,printed=0;
    task_t *task=task_now();
    for(i=0;i<COMMAND_NOW;i++)
    {
        if(strncmp(cmdline,commands[i],strlen(cmdline))==0)
        {
            if(first_print==0)
            {
                first_print=1;
                if(task->langmode==1 || task->langmode==2)
                {
                    console_putstr_color(console,"预测命令:",0xFFFFFF);
                }
                else
                {
                    console_putstr_color(console,"Prediction command:",0xFFFFFF);
                }
            }
            console_putstr_color(console,commands[i],0x00FFFF);
            console_putchar(console,' ');
            printed=1;
        }
    }
    if(printed)
    {
        console_putstr(console,"\n");
    }
}
