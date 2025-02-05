/*
console.h
控制台头文件
Copyright W24 Studio 
*/

#ifndef CONSOLE_H
#define CONSOLE_H
#include <window.h>
typedef struct CONSOLE
{
    window_t *window;
    char consbuf[80][25];
    uint32_t colorbuf[80][25];
    int curx,cury;
}console_t;
console_t *open_console(void);
void close_console(console_t *console);
void console_refresh(console_t *console);
void console_newline(console_t *console);
void console_movcur(console_t *console,int x,int y);
void console_setchr(console_t *console,int x,int y,char c);
int console_putchar(console_t *console,char c);
int console_putchar_norefresh(console_t *console,char c);
int console_putchar_color(console_t *console,char c,uint32_t color);
int console_putchar_color_norefresh(console_t *console,char c,uint32_t color);
void console_putstr(console_t *console,char *s);
void console_cleanscreen(console_t *console);
void console_putstr_color(console_t *console,char *s,uint32_t color);
char *console_input(console_t *console,int len);
int console_getkey(console_t *console);
void cmd_run(console_t *console,char *cmdline);

void cmd_mem(console_t *console);
void cmd_count(console_t *console);
void cmd_dir(console_t *console);
void cmd_langmode(console_t *console,int lmode);
void cmd_print(console_t *console,char *filename);
void cmd_bootinfo(console_t *console);
void cmd_finfo(console_t *console,char *filename);
int cmd_runapp(console_t *console,char* cmdline);
void cmd_autofill(console_t *console,char *cmdline);
#endif