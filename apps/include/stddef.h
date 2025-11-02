/*
stddef.h 
一般变量别名 
Copyright W24 Studio 
*/
#ifndef STDDEF_H
#define STDDEF_H 
#define NULL ((void *)0)

typedef __INTPTR_TYPE__  ssize_t;
typedef __UINTPTR_TYPE__ size_t;
typedef __INTPTR_TYPE__				intptr_t;
typedef __UINTPTR_TYPE__			uintptr_t;

typedef char bool;
typedef short wchar_t;
#define SIZE_MAX 4294967295U
#endif
