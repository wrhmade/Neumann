/*
ime.c
中文输入法
Copyright W24 Studio 
*/

#include <ime.h>
#include <mm.h>
#include <fifo.h>
#include <task.h>
#include <sheet.h>
#include <graphic.h>
#include <binfo.h>
#include <macro.h>
#include <message.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <keyboard.h>
#include <syscall.h>
#include <stdio.h>

extern int shift_pressing,ctrl_pressing;

ime_status_t *status;
uint32_t key_buf[128];
fifo_t key_fifo;
task_t *ime_task;
sheet_t *ime_sheet;
uint32_t *ime_shtbuf;
extern shtctl_t *global_shtctl;
extern sheet_t *keywin;

static int quotation_mark=0;
static int double_quotation_mark=0;

int double_byte;//全角模式
int choosing=0,select_page,select_index;
int n;
int current_mb_id;

chinese_t *chr_result;

char input[7];//最多7个字符决定一个汉字

char chooses[5][3];//候选

int input_index=0;
int mb_lines;

ime_mb_t ime_mbctl[IME_MB_MAX];
int mb_num=0;

int chooses_table_num;
ime_ct_t chooses_table[400];

void log(char *s);
void log_cn(char *s);

static void refresh();
static int ime_setup_mb(int id);

static void make_choose_table()
{
    chooses_table_num=0;
    for(int i=0;i<400;i++)
    {
        for(int j=0;j<5;j++)
        {
            chooses_table[i].chr[j].byte1=0;
            chooses_table[i].chr[j].byte2=0;
        }
        chooses_table[i].n=0;
    }

    int remaining=n;
    int k=0;
    int l=0;
    while(remaining>0)
    {
        if(remaining>=5)
        {
            chooses_table[k].n=5;
            for(int i=0;i<5;i++)
            {
                chooses_table[k].chr[i].byte1=chr_result[l].byte1;
                chooses_table[k].chr[i].byte2=chr_result[l].byte2;
                l++;
                remaining--;
            }
        }
        else
        {
            chooses_table[k].n=remaining;
            int m=remaining;
            for(int i=0;i<m;i++)
            {
                chooses_table[k].chr[i].byte1=chr_result[l].byte1;
                chooses_table[k].chr[i].byte2=chr_result[l].byte2;
                l++;
                remaining--;
            }
        }
        k++;
    }
    chooses_table_num=k;
}

static void ime_switch()
{
    int next_id=current_mb_id+1;
    if(ime_mbctl[next_id].flag==0)
    {
        next_id=0;
    }
    for(int i=0;i<390;i++)
    {
        chr_result[i].byte1=0;
        chr_result[i].byte2=0;
    }
    current_mb_id=next_id;
    choosing=0;
    input_index=0;
    memset(input,0,sizeof(input));
    ime_setup_mb(next_id);
    refresh();
}

static void refresh()
{
    task_t *task=task_now();
    boxfill(ime_shtbuf,ime_sheet->bxsize,0,0,ime_sheet->bxsize-1,ime_sheet->bysize-1,0xFF808080);

    if(mb_num==0)
    {
        putstr_ascii(ime_shtbuf,ime_sheet->bxsize,0,0,0xFF000000,"未加载码表");
        return;
    }

    if(status->inputmode==0)
    {
        putstr_ascii(ime_shtbuf,ime_sheet->bxsize,0,0,0xFF000000,"英");
    }
    if(status->inputmode==1)
    {
        putstr_ascii(ime_shtbuf,ime_sheet->bxsize,0,0,0xFF000000,"中");
        putstr_ascii(ime_shtbuf,ime_sheet->bxsize,24+16+8,0,0xFF000000,ime_mbctl[current_mb_id].name);
    }
    task->langbyte=0;
    if(status->inputmode==1)
    {
        if(double_byte)
        {
            putstr_ascii(ime_shtbuf,ime_sheet->bxsize,24,0,0xFF000000,"全");
        }
        else
        {
            putstr_ascii(ime_shtbuf,ime_sheet->bxsize,24,0,0xFF000000,"半");
        }
    }
    task->langbyte=0;
    putstr_ascii(ime_shtbuf,ime_sheet->bxsize,24+16+80,0,0xFF000000,input);
    if(choosing)boxfill(ime_shtbuf,ime_sheet->bxsize,24+16+80+input_index*8,13,24+16+80+input_index*8+7,15,0xFF000000);

    int x=8*7+16+80+32,i;
    int number=1;
    char chr[3];

    make_choose_table();

    if(choosing)
    {
        for(i=0;i<5;i++)
        {
            chr[0]=number+'0';
            chr[1]=0;
            putstr_ascii(ime_shtbuf,ime_sheet->bxsize,x-8,0,0xFFFF0000,chr);
            chr[0]=chooses_table[select_page].chr[i].byte1;
            chr[1]=chooses_table[select_page].chr[i].byte2;
            chr[2]=0;
            if(select_index==i)
            {
                putstr_ascii(ime_shtbuf,ime_sheet->bxsize,x,0,0xFFFF0000,chr);
            }
            else
            {
                putstr_ascii(ime_shtbuf,ime_sheet->bxsize,x,0,0xFF000000,chr);
            }
            strcpy(chooses[i],chr);
            //log_cn(chr);
            x+=24;
            number+=1;
        }
    }

    sheet_refresh(ime_sheet,0,0,ime_sheet->bxsize-1,ime_sheet->bysize-1);
}

void ime_findinput()
{
    int i=0;
    char *p;
    for(i=0;i<390;i++)
    {
        chr_result[i].byte1=0;
        chr_result[i].byte2=0;
    }
    char substr[40];
    sprintf(substr,"\n%s ",input);
    p=strstr(status->mb,substr);
    i=0;
    n=0;
    
    if(p!=NULL)
    {
        p += strlen(substr)-1;
        p+=1;
        while(!(*(p-1)=='\n' && i!=0))
        {
            chr_result[i].byte1=*p;
            chr_result[i].byte2=*(p+1);
            i++;
            n++;
            p+=3;
        }
    }
}


void ime_put(char *s)
{
    int i;
    for(i=0;s[i]!=0;i++)
    {
        fifo_put(&keywin->window->task->fifo,(s[i]&0xff)+256);
    }
}

void ime_input(char c)
{
    int need_to_reset=1;
    if(c=='\n' && choosing==1)
    {
        ime_put(input);
        int i;
        for(i=0;i<7;i++)
        {
            input[i]=0;
        }
        input_index=0;
        choosing=0;
        select_index=0;
    }
    else if(c=='\b' && input_index>0)
    {
        input_index--;
        input[input_index]=0;
    }
    else if(c=='=')
    {
        if(select_page<chooses_table_num-1)
        {
            select_page++;
            select_index=0;
        }
        need_to_reset=0;
    }
    else if(c=='-')
    {
        if(select_page>0)
        {
            select_index=4;
            select_page--;
        }
        need_to_reset=0;
    }
    else if(c==']' && choosing)
    {
        
        if(select_index==4)
        {
            if(select_page<chooses_table_num-1)
            {
                select_page++;
                select_index=0;
            }
        }
        else
        {
            if(select_index<chooses_table[select_page].n-1)
            {
                select_index++;
            }
        }
        need_to_reset=0;
    }
    else if(c=='[' && choosing)
    {
        if(select_index==0)
        {
            if(select_page>0)
            {
                select_index=4;
                select_page--;
            }
        }
        else
        {
            select_index--;
        }
        need_to_reset=0;
    }
    else if(c==' ' && choosing)
    {
        int i;
        ime_put(chooses[select_index]);
        for(i=0;i<7;i++)
        {
            input[i]=0;
        }
        input_index=0;
        choosing=0;
        select_index=0;
    }
    else if(c>=0x20 && c<=0xFF && input_index<=5)
    {
        select_page=0;
        if(c>='1' && c<='5')
        {
            int number=c-'0'-1;
            int i;
            ime_put(chooses[number]);
            for(i=0;i<7;i++)
            {
                input[i]=0;
            }
            input_index=0;
            choosing=0;
            select_index=0;
        }
        else if(c=='0' || (c>='6' && c<='9'))
        {

        }
        else
        {
            input[input_index]=c;
            input_index++;
        }
    }
    
    if(input_index>0)
    {
        choosing=1;
    }
    else
    {
        choosing=0;
    }
    if(need_to_reset)
    {
        select_index=0;
        select_page=0;
    }
    ime_findinput();
    refresh();
}


void ime_main()
{
    int i;
    refresh();
    for(;;)
    {
        if(mb_num==0)//一个码表都没加载
        {
            //收到什么就发送什么
            if(fifo_status(&key_fifo)>0)
            {
                i=fifo_get(&key_fifo);
                fifo_put(&keywin->window->task->fifo,i);
            }
        }
        else if(ctrl_pressing)
        {
            if(shift_pressing)
            {
                while(ctrl_pressing && shift_pressing);
                status->inputmode=!status->inputmode;
                for(i=0;i<7;i++)
                {
                    input[i]=0;
                }
                for(i=0;i<390;i++)
                {
                    chr_result[i].byte1=0;
                    chr_result[i].byte2=0;
                }
                choosing=0;
                select_index=0;
                select_page=0;
                input_index=0;
                refresh();
                continue;
            }
        }
        if(fifo_status(&key_fifo)>0)
        {
            i=fifo_get(&key_fifo);
            
            if(status->inputmode==0)
            {
                fifo_put(&keywin->window->task->fifo,i);
            }
            else if(i-256==0x0A)//换行
            {
                if(choosing)
                {
                    ime_input(i-256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,i);    
                }
            }
            else if(i-256=='\b')//退格
            {
                if(choosing)
                {
                    ime_input('\b');
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,i);
                }
            }
            else if(i-256==' ')//空格
            {
                if(ctrl_pressing)
                {
                    while(ctrl_pressing);
                    double_byte=!double_byte;
                    refresh();
                }
                else if(shift_pressing)
                {
                    while(shift_pressing);
                    ime_switch();
                }
                else
                {
                    if(choosing)
                    {
                        ime_input(' ');
                    }
                    else
                    {
                        fifo_put(&keywin->window->task->fifo,i);
                    }
                }
            }
            else if(i-256=='.')//句号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'.'+256);
                }
            }
            else if(i-256==',')//逗号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                    fifo_put(&keywin->window->task->fifo,0xac+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,','+256);
                }
            }
            else if(i-256==';')//分号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                    fifo_put(&keywin->window->task->fifo,0xbb+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,';'+256);
                }
            }
            else if(i-256=='\\')//顿号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                    fifo_put(&keywin->window->task->fifo,0xa2+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'\\'+256);
                }
            }
            else if(i-256=='_')//破折号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                    fifo_put(&keywin->window->task->fifo,0xaa+256);
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                    fifo_put(&keywin->window->task->fifo,0xaa+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'_'+256);
                }
            }
            else if(i-256=='!')//感叹号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'!'+256);
                }
            }
            else if(i-256=='?')//问号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                    fifo_put(&keywin->window->task->fifo,0xbf+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'?'+256);
                }
            }
            else if(i-256==':')//冒号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                    fifo_put(&keywin->window->task->fifo,0xba+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,':'+256);
                }
            }
            else if(i-256=='<')//左书名号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                    fifo_put(&keywin->window->task->fifo,0xb6+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'<'+256);
                }
            }
            else if(i-256=='>')//右书名号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                    fifo_put(&keywin->window->task->fifo,0xb7+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'>'+256);
                }
            }
            else if(i-256=='(')//左括号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                    fifo_put(&keywin->window->task->fifo,0xa8+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'('+256);
                }
            }
            else if(i-256==')')//右括号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                    fifo_put(&keywin->window->task->fifo,0xa9+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,')'+256);
                }
            }
            else if(i-256=='[')//左中括号
            {
                if(choosing)
                {
                    ime_input('[');
                }
                else
                {
                    if(double_byte)
                    {
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                    fifo_put(&keywin->window->task->fifo,(0xbe)+256);
                    }
                    else
                    {
                        fifo_put(&keywin->window->task->fifo,'['+256);
                    }
                }
            }
            else if(i-256==']')//右中括号
            {
                if(choosing)
                {
                    ime_input(']');
                }
                else
                {
                    if(double_byte)
                    {
                        fifo_put(&keywin->window->task->fifo,0xa1+256);
                        fifo_put(&keywin->window->task->fifo,0xbf+256);
                    }
                    else
                    {
                        fifo_put(&keywin->window->task->fifo,']'+256);
                    }
                }
            }
            else if(i-256=='{')//左大括号
            {
                fifo_put(&keywin->window->task->fifo,'{'+256);
            }
            else if(i-256=='}')//右大括号
            {
                fifo_put(&keywin->window->task->fifo,'}'+256);
            }
            else if(i-256=='-')//减号
            {
                 if(choosing)
                {
                    ime_input('-');
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'-'+256);
                }
            }
            else if(i-256=='`')//点号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa1+256);
                    fifo_put(&keywin->window->task->fifo,0xa4+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'`'+256);
                }
            }
            else if(i-256=='~')//波浪
            {
                fifo_put(&keywin->window->task->fifo,'~'+256);
            }
            else if(i-256=='=')//等于号
            {
                if(choosing)
                {
                    ime_input('=');
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'='+256);
                }
            }
            else if(i-256=='+')//加号
            {
                fifo_put(&keywin->window->task->fifo,'+'+256);
            }
            else if(i-256=='@')//艾特号
            {
                fifo_put(&keywin->window->task->fifo,'@'+256);
            }
            else if(i-256=='#')//井号
            {
                fifo_put(&keywin->window->task->fifo,'#'+256);
            }
            else if(i-256=='$')//人民币符号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,0xa3+256);
                    fifo_put(&keywin->window->task->fifo,0xa4+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'$'+256);
                }
            }
            else if(i-256=='%')//百分号
            {
                fifo_put(&keywin->window->task->fifo,'%'+256);
            }
            else if(i-256=='^')//省略号
            {
                if(double_byte)
                {
                    fifo_put(&keywin->window->task->fifo,'.'+256);
                    fifo_put(&keywin->window->task->fifo,'.'+256);
                    fifo_put(&keywin->window->task->fifo,'.'+256);
                    fifo_put(&keywin->window->task->fifo,'.'+256);
                    fifo_put(&keywin->window->task->fifo,'.'+256);
                    fifo_put(&keywin->window->task->fifo,'.'+256);
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'^'+256);
                }
            }
            else if(i-256=='&')//and号
            {
                fifo_put(&keywin->window->task->fifo,'&'+256);
            }
            else if(i-256=='*')//星号
            {
                fifo_put(&keywin->window->task->fifo,'*'+256);
            }
            else if(i-256=='\'')//引号
            {
                if(double_byte)
                {
                    if(quotation_mark)
                    {
                        fifo_put(&keywin->window->task->fifo,0xa1+256);
                        fifo_put(&keywin->window->task->fifo,0xaf+256);
                    }
                    else
                    {
                        fifo_put(&keywin->window->task->fifo,0xa1+256);
                        fifo_put(&keywin->window->task->fifo,(0xae)+256);
                    }
                    quotation_mark=!quotation_mark;
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'\''+256);
                }
            }
            else if(i-256=='\"')//双引号
            {
                if(double_byte)
                {
                    if(double_quotation_mark)
                    {
                        fifo_put(&keywin->window->task->fifo,0xa1+256);
                        fifo_put(&keywin->window->task->fifo,0xb1+256);
                    }
                    else
                    {
                        fifo_put(&keywin->window->task->fifo,0xa1+256);
                        fifo_put(&keywin->window->task->fifo,0xb0+256);
                    }
                    double_quotation_mark=!double_quotation_mark;
                }
                else
                {
                    fifo_put(&keywin->window->task->fifo,'\"'+256);
                }
            }
            else if(i-256>='0' && i-256<='9' && !choosing)//数字
            {
                fifo_put(&keywin->window->task->fifo,i);
            }
            else
            {
                ime_input(i-256);
            }
        }
        // if(ctrl_pressing)
        // {
        //     //log("Ctrl pressed.Waiting Space...");
        //     while(1)
        //     {
        //         if(fifo_status(&key_fifo))
        //         {
        //             i=fifo_get(&key_fifo);
        //             if(i-256==' ')
        //             {
        //                 status->inputmode=!status->inputmode;
        //                 refresh();
        //                 break;
        //             }
        //         }
        //     }
        // }
        
    };
}


void ime_init()
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;

    status=(ime_status_t *)kmalloc(sizeof(ime_status_t));
    status->enabled=1;
    status->inputmode=0;

    for(int i=0;i<IME_MB_MAX;i++)
    {
        ime_mbctl[i].flag=0;
    }
    

    chr_result=(chinese_t *)kmalloc(sizeof(chinese_t *)*390);


    ime_sheet=sheet_alloc(global_shtctl);
    ime_sheet->movable=1;//标记为可拖移
    ime_shtbuf=(uint32_t *)kmalloc(sizeof(uint32_t)*312*16);
    sheet_setbuf(ime_sheet,ime_shtbuf,312,16,-1);
    boxfill(ime_shtbuf,ime_sheet->bxsize,0,0,ime_sheet->bxsize-1,ime_sheet->bysize-1,0xFF808080);
    
    putstr_ascii(ime_shtbuf,ime_sheet->bxsize,0,0,0xFF000000,"正在初始化......");

    sheet_updown(ime_sheet,-1);
    
    double_byte=0;
    
    sheet_refresh(ime_sheet,0,0,ime_sheet->bxsize-1,ime_sheet->bysize-1);
    sheet_slide(ime_sheet,binfo->scrnx-ime_sheet->bxsize,binfo->scrny-ime_sheet->bysize-100);
    sheet_updown(ime_sheet,1);

    fifo_init(&key_fifo,128,key_buf);

    ime_load_mb("/resource/ime/pymb.dat","拼音");
    ime_load_mb("/resource/ime/wbmb.dat","五笔");

    ime_task=create_kernel_task(ime_main);
    ime_task->langmode=1;
    choosing=0;
    name_task(ime_task,"Neumann中文输入法");
    task_run(ime_task);
}

static int ime_setup_mb(int id)
{
    if(id<0 || id>IME_MB_MAX-1)
    {
        return -1;
    }
    if(status->mb)
    {
        kfree(status->mb);
    }
    status->mb=kmalloc(ime_mbctl[id].size);
    memset(status->mb,0,ime_mbctl[id].size);
    memcpy(status->mb,ime_mbctl[id].buffer,ime_mbctl[id].size);
    current_mb_id=id;
    return 0;
}

int ime_load_mb(const char *filename,const char *mb_name)
{
    int id=-1;
    for(int i=0;i<IME_MB_MAX;i++)
    {
        if(ime_mbctl[i].flag==0)
        {
            id=i;
            break;
        }
    }
    if(id==-1)
    {
        return -1;
    }
    
    vfs_node_t node=vfs_open(filename);
    if(node==0)
    {
        char message_content[1024];
        sprintf(message_content,"名为 %s 的码表文件(%s)无法加载",mb_name,filename);
        error_message(message_content,"错误");
        return -1;
    }
    ime_mbctl[id].buffer=(char *)kmalloc(sizeof(char)*(node->size+5));
    vfs_read(node,ime_mbctl[id].buffer,0,node->size);

    strcpy(ime_mbctl[id].name,mb_name);
    ime_mbctl[id].size=node->size;
    ime_mbctl[id].flag=1;
    mb_num++;
    if(mb_num==1)
    {
        ime_setup_mb(id);
    }
    return id;
}

