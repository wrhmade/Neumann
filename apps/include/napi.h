#ifndef NAPI_H
#define NAPI_H
void napi_exit(int returnvalue);
void napi_putstr(char *s);
void napi_putstr_color(char *s,int color);
char napi_getkey();
void napi_input(char *buffer,int len);
void napi_putchar(char c);
void napi_putchar_color(char c,int color);
int napi_makewin(char *title,int x,int y);
int napi_makewin_nobtn(char *title,int x,int y);
void napi_closewin(int win);
void napi_putstrwin(int win,int x,int y,int c,char *s);
void napi_getcmdline(char *cmdline);
#endif