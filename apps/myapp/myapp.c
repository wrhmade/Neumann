#include <stdio.h>
#include <napi/file/file.h>
#include <napi/consio/cons.h>
#include <stdlib.h>

#define bool2str(n) n?"True":"False"

int main(int argc,char **argv)
{
    if(argc==1)
    {
        printf("No file name.\n");
    }
    else
    {
        int fd=napi_open_file(argv[1],NAPI_FILE_READ);
        if(fd==-1)
        {
            printf("error 1\n");
            return -1;
        }
        napi_fstat_t stat;
        napi_file_status(fd,&stat);
    
        char *buffer=malloc(stat.size);
        printf("Buffer Base:0x%08X\n",buffer);
        int ret=napi_read_file(fd,buffer,stat.size);
        if(ret==-1)
        {
            napi_puts("error\n");
        }
        else
        {
            napi_puts(buffer);
            napi_putc('\n');
        }
    }
    return 0;
}
