/*
ini.c
INI配置文件解析
Copyright W24 Studio 
*/

/*
INI配置文件格式如下：
[Section 1]
Key1=value1
Key2=value2

[Section 2]
Key1=value1
Key2=value2
*/

#include <ini.h>
#include <stddef.h>
#include <string.h>
#include <mm.h>
#include <console.h>
#include <task.h>
#include <syscall.h>

char* read_line(const char* str, int line_number)
{
    int current_line = 1;
    char *next,*current;
    char* line = NULL;
    size_t len;
    current=str;
    while(1)
    {
        next=strchr(current,'\n');
        if(next==NULL)
        {
            break;
        }
        if(current_line==line_number)
        {
            len = next - current;
            line=(char *)kmalloc(sizeof(char)*(len+1));
            strncpy(line,current,len);
            line[len]=0;
            break;
        }
        current_line++;
        current=next+1;
    }
    
    
    return line;

}
                                                                        
int countLines(char* str)
{
    int lines = 0,i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            lines++;
        }
    }
    return lines;
}

int read_ini_buf(char *buf,char* section,char* key,char *value)
{
    int lines=countLines(buf),i,found_section;
    char *line,*section_tmp,*key_tmp;
    section_tmp=(char *)kmalloc(sizeof(char)*(strlen(section)+3));
    key_tmp=(char *)kmalloc(sizeof(char)*(strlen(key)+2));
    sprintf(section_tmp,"[%s]",section);
    sprintf(key_tmp,"%s=",key);
    //console_putstr(task_now()->window->console,section_tmp);
    for(i=1;i<=lines;i++)
    {
        line=read_line(buf,i);
        if(line[0]==';')//注释
        {
            kfree(line);
            continue;
        }
        if(strcmp(line,section_tmp)==0)
        {
            found_section=1;
        }
        else
        {
            if(line[0]=='[')found_section=0;
        }
        if(found_section)
        {
            if(strncmp(line,key_tmp,strlen(key_tmp))==0)
            {
                //console_putstr(task_now()->window->console,line+strlen(key_tmp));
                strcpy(value,line+strlen(key_tmp));
                kfree(section_tmp);
                kfree(key_tmp);
                return 0;
            }
        }
        kfree(line);
    }
    kfree(section_tmp);
    kfree(key_tmp);
    //console_putstr(task_now()->window->console,"Not Found");
    return -1;
}

int read_ini(char *filename,char* section,char* key,char *value)
{
    int retvalue;
    char *buf;
    vfs_node_t node;
    node=vfs_open(filename);
    if(node==0)
    {
        //value=NULL;
        return -1;
    }
    buf=(char *)kmalloc(node->size+5);
    vfs_read(node,buf,0,node->size);
    retvalue=read_ini_buf(buf,section,key,value);
    kfree(buf);

    return retvalue;
}