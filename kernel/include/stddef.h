/*
stddef.h 
一般变量别名 
Copyright W24 Studio 
*/
#ifndef STDDEF_H
#define STDDEF_H

#include <stdint.h>
typedef int8_t sbyte;
typedef uint8_t byte;
#define NULL ((void *)0)

typedef __INTPTR_TYPE__  ssize_t;
typedef __UINTPTR_TYPE__ size_t;
typedef __INTPTR_TYPE__				intptr_t;
typedef __UINTPTR_TYPE__			uintptr_t;

typedef int8_t bool;
typedef int16_t wchar_t;
#define true 1
#define false 0


#endif
