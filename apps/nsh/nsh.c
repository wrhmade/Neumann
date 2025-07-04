#include <napi.h>
#include "nsh.h"

void main()
{
    shell_main();
    napi_exit(0);
}