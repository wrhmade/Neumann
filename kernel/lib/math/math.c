/*
maths.c
数学运算
Copyright W24 Studio 
*/

#include <math.h>
#include <stdint.h>

double pow(double x, double y)
{
    if (y == 0) {
        return 1;
    }
    if (y < 0) {
        return 1 / pow(x, -y);
    }
    double result = x;
    for (int i = 1; i < y; i++) {
        result *= x;
    }
    return result;
}

double ldexp(double x, int exp)
{
    return x * pow(2.0, exp);
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

uint64_t div64(uint64_t dividend, uint64_t divisor)
//64位无符号整数除法
{
    uint64_t quotient = 0;
    uint64_t temp = 1;
    uint64_t current = divisor;

    // 将除数左移直到它大于或等于被除数
    while (dividend >= current) {
        current <<= 1;
        temp <<= 1;
    }

    // 进行除法运算
    while (temp > 1) {
        current >>= 1;
        temp >>= 1;

        if (dividend >= current) {
            dividend -= current;
            quotient += temp;
        }
    }

    return quotient;
}

//有符号
int64_t univdi3(int64_t dividend, int64_t divisor) {
    // 步骤1：处理符号
    int negative = 0;
    if (dividend < 0) {
        dividend = -dividend;
        negative = !negative;
    }
    if (divisor < 0) {
        divisor = -divisor;
        negative = !negative;
    }

    // 步骤2：调用 div64 函数
    uint64_t quotient = div64((uint64_t)dividend, (uint64_t)divisor);

    // 步骤3：处理结果符号
    if (negative) {
        quotient = -quotient;
    }

    // 步骤4：返回结果
    return (int64_t)quotient;
}


uint64_t umoddi3(uint64_t dividend, uint64_t divisor)
//计算余数
{
    // 步骤1：调用 div64 函数
    uint64_t quotient = div64(dividend, divisor);

    // 步骤2：计算余数
    uint64_t remainder = dividend - quotient * divisor;

    // 步骤3：返回余数
    return remainder;
}

uint64_t __udivdi3(uint64_t dividend, uint64_t divisor)
{
    return univdi3(dividend,divisor);
}

uint64_t __umoddi3(uint64_t dividend, uint64_t divisor)
{
    return umoddi3(dividend,divisor);
}