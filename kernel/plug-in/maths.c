/*
maths.c
数学运算
Copyright W24 Studio 
*/

#include <maths.h>

int pow(int a,int n)
//指数运算
{
    int s=1,i;
    for(i=0;i<n;i++)
    {
        s*=a;
    }
    return s;
}

int get_GCD(int a,int b)
//求最大公因数（辗转相除法）
{
	int t1=a;
	int t2=b;
	int tmp;
	while(t1%t2!=0)
	{
		tmp=t1%t2;
		t1=t2;
		t2=tmp;
	}
	return t2;
} 

int get_LCM(int a,int b)
//求最小公倍数（公式法）
{
	return a*b/get_GCD(a,b);
}