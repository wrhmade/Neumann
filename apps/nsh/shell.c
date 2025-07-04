#include <napi.h>
#include "nsh.h"

void shell_main()
{
    char cmdline[81];
    for(;;)
    {
        cmdline[0]=0;
        napi_putstr_color("[Default] ",2);
        napi_putstr_color("[/]\n",1);
        napi_putstr_color("# ",7);

        napi_input(cmdline,80);

        if(cmdline[0]==0)continue;
    }
}