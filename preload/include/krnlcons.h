/*
krnlcons.h
内核控制台界面头文件
Copyright W24 Studio 
*/

#ifndef KRNLCONS_H
#define KRNLCONS_H
void krnlcons_display();
void krnlcons_cleanscreen();
void krnlcons_showcur();
void krnlcons_putchar_color(char c,int cc,int cb);
void krnlcons_putchar(char c);
void krnlcons_putchar_color_nomove(char c,int cc,int cb);
void krnlcons_putchar_nomove(char c);
void krnlcons_putstr(char *s);
void krnlcons_putstr_color(char *s,int cc,int cb);
void krnlcons_change_backcolor(int c);
void krnlcons_change_forecolor(int c);
#endif
