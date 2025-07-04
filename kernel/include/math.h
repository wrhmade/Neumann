/*
maths.h
数学运算头文件
Copyright W24 Studio 
*/

#ifndef MATH_H
#define MATH_H
#include <stdint.h>
#define abs(x) ((x) < 0 ? -(x) : (x))
double pow(double x, double y);
double ldexp(double x, int exp);
int get_GCD(int a,int b);
int get_LCM(int a,int b);
#endif