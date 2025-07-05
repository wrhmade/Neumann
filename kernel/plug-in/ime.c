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
int choosing=0,select_start,select_index;

chinese_t *py_result;

char input[7];//最多7个字符决定一个汉字

char chooses[5][3];//候选

int input_index=0;
int mb_lines;

void log(char *s);
void log_cn(char *s);


static void refresh()
{
    task_t *task=task_now();
    boxfill(ime_shtbuf,ime_sheet->bxsize,0,0,ime_sheet->bxsize-1,ime_sheet->bysize-1,0x808080);
    if(status->inputmode==0)
    {
        putstr_ascii(ime_shtbuf,ime_sheet->bxsize,0,0,0x000000,"英");
    }
    if(status->inputmode==1)
    {
        putstr_ascii(ime_shtbuf,ime_sheet->bxsize,0,0,0x000000,"中");
    }
    task->langbyte=0;
    if(status->inputmode==1)
    {
        if(double_byte)
        {
            putstr_ascii(ime_shtbuf,ime_sheet->bxsize,24,0,0x000000,"全");
        }
        else
        {
            putstr_ascii(ime_shtbuf,ime_sheet->bxsize,24,0,0x000000,"半");
        }
    }
    task->langbyte=0;
    putstr_ascii(ime_shtbuf,ime_sheet->bxsize,24+16,0,0x000000,input);
    if(choosing)boxfill(ime_shtbuf,ime_sheet->bxsize,24+16+input_index*8,13,24+16+input_index*8+7,15,0x000000);

    int x=8*7+16+32,i;
    int number=1;
    char chr[3];


    if(choosing)
    {
        for(i=0;i<5;i++)
        {
            chr[0]=number+'0';
            chr[1]=0;
            putstr_ascii(ime_shtbuf,ime_sheet->bxsize,x-8,0,0xFF0000,chr);
            chr[0]=py_result[i+select_start].byte1;
            chr[1]=py_result[i+select_start].byte2;
            chr[2]=0;
            if(select_index==i)
            {
                putstr_ascii(ime_shtbuf,ime_sheet->bxsize,x,0,0xFF0000,chr);
            }
            else
            {
                putstr_ascii(ime_shtbuf,ime_sheet->bxsize,x,0,0x000000,chr);
            }
            strcpy(chooses[i],chr);
            //log_cn(chr);
            x+=24;
            number+=1;
        }
    }

    sheet_refresh(ime_sheet,0,0,ime_sheet->bxsize-1,ime_sheet->bysize-1);
}

void ime_findpy()
{
    int i=0;
    char *line,*p,*q;
    char str[101];
    for(i=0;i<390;i++)
    {
        py_result[i].byte1=0;
        py_result[i].byte2=0;
    }
    char substr[40],chr[3];
    sprintf(substr,"\n%s ",input);
    p=strstr(status->mb,substr);
    i=0;

    
    if(p!=NULL)
    {
        p += strlen(substr)-1;
        p+=1;
        while(!(*(p-1)=='\n' && i!=0))
        {
            py_result[i].byte1=*p;
            py_result[i].byte2=*(p+1);
            i++;
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
        select_start+=5;
        select_index=0;
    }
    else if(c=='-')
    {
        if(select_start>0)
        {
            select_start-=5;
            select_index=0;
        }
    }
    else if(c==']' && choosing)
    {
        select_index++;
        if(select_index==5)
        {
            select_start+=5;
            select_index=0;
        }
    }
    else if(c=='[' && choosing)
    {
        if(select_index==0)
        {
            if(select_start>0)
            {
                select_index=4;
                select_start-=5;
            }
        }
        else
        {
            select_index--;
        }
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
        select_start=0;
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
    ime_findpy();
    refresh();
}


void ime_main()
{
    int i;
    refresh();
    for(;;)
    {
        if(ctrl_pressing)
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
                    py_result[i].byte1=0;
                    py_result[i].byte2=0;
                }
                choosing=0;
                select_index=0;
                select_start=0;
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
    

    vfs_node_t node=vfs_open("/resource/ime/pymb.dat");

    if(node==0)
    {
        status->enabled=0;
        error_message("无法加载拼音码表!","输入法错误");
        return;
    }
    
    status->mb=(char *)kmalloc(sizeof(char)*(node->size+5));
    vfs_read(node,status->mb,0,node->size);

    py_result=(chinese_t *)kmalloc(sizeof(chinese_t *)*390);


    ime_sheet=sheet_alloc(global_shtctl);
    ime_sheet->movable=1;//标记为可拖移
    ime_shtbuf=(uint32_t *)kmalloc(sizeof(uint32_t)*232*16);
    sheet_setbuf(ime_sheet,ime_shtbuf,232,16,-1);
    boxfill(ime_shtbuf,ime_sheet->bxsize,0,0,ime_sheet->bxsize-1,ime_sheet->bysize-1,0x808080);



    
    sheet_updown(ime_sheet,-1);
    
    double_byte=0;
    
    sheet_refresh(ime_sheet,0,0,ime_sheet->bxsize-1,ime_sheet->bysize-1);
    sheet_slide(ime_sheet,binfo->scrnx-ime_sheet->bxsize,binfo->scrny-ime_sheet->bysize-100);
    sheet_updown(ime_sheet,1);
    fifo_init(&key_fifo,128,key_buf);

    ime_task=create_kernel_task(ime_main);
    ime_task->langmode=1;
    choosing=0;
    task_run(ime_task);
}