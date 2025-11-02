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

float fabsf(float x)
{
    return (x < 0.0f) ? -x : x;
}

long double fabsl(long double x)
{
    return (x < 0.0L) ? -x : x;
}

double fabs(double x)
{
    return (x < 0.0) ? -x : x;
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


/* sin运算 */
double sin(double x)
{
	__asm__ __volatile__("fldl %0 \n"
                         "fsin \n"
                         "fstpl %0\n"
                         : "+m"(x));
	return x;
}

/* cos运算 */
double cos(double x)
{
	__asm__ __volatile__("fldl %0 \n"
                         "fcos \n"
                         "fstpl %0\n"
                         : "+m"(x));
	return x;
}

/* tan运算 */
double tan(double x)
{
	__asm__ __volatile__("fldl %0 \n"
                         "fptan \n"
                         "fstpl %0\n"
                         "fstpl %0\n"
                         : "+m"(x));
	return x;
}

/* sqrt运算 */
double sqrt(double x)
{
	__asm__ __volatile__("fldl %0 \n"
						 "fsqrt \n"
						 "fstpl %0\n"
						 : "+m"(x));
	return x;
}

/*
 * @attention Rainy101112
 * These functions have been create by AI, may cause some bugs.
 */

double sinh(double x)
{
    double e_x = exp(x); 
    double e_neg_x = exp(-x);
    return (e_x - e_neg_x) / 2.0;
}

double cosh(double x)
{
    double e_x = exp(x);
    double e_neg_x = exp(-x);
    return (e_x + e_neg_x) / 2.0;
}

double tanh(double x)
{
    double e_x = exp(x);
    double e_neg_x = exp(-x);
    return (e_x - e_neg_x) / (e_x + e_neg_x);
}

double asin(double x)
{
    if (x < -1 || x > 1) {
        return NAN;
    }
    double y = x;
    double delta;
    const double tolerance = 1e-7;
    const double pi = 3.14159265358979323846;
    const double pi_over_2 = pi / 2;
    while (1) {
        double y1 = (1 - y * y) > 0 ? sqrt(1 - y * y) : 0;
        delta = (x - y / y1) / (1 + y * y / (y1 * y1));
        y += delta;

        if (fabs(delta) < tolerance) {
            break;
        }
    }
    return y + (x < 0 ? -pi_over_2 : pi_over_2);
}

double acos(double x)
{
    double sqrtPart = sqrt(1 - x * x);
    double atanPart;
    if (x >= 0) {
        atanPart = atan(sqrtPart / x);
    } else {
        atanPart = atan(sqrtPart / -x);
        atanPart = M_PI - atanPart;
    }
    return M_PI_2 - atanPart;
}

double atan(double x)
{
    double term = x;
    double sum = 0.0;
    int n = 1;
    while (fabs(term) > 1e-7) {
        sum += term;
        term = -term * x * x / (2 * n * (2 * n + 1));
        n += 1;
    }
    return sum;
}

double atan2(double y, double x)
{
    double result;
    if (x == 0) {
        if (y > 0) {
            result = M_PI_2;
        } else if (y < 0) {
            result = -M_PI_2;
        } else {
            return NAN;
        }
    } else if (x > 0) {
        result = atan(y / x);
    } else {
        if (y >= 0) {
            result = atan(y / x) + M_PI;
        } else {
            result = atan(y / x) - M_PI;
        }
    }
    return result;
}

double exp(double x)
{
    double sum = 1.0;
    double term = 1.0;
    int i = 1;
    do {
        term *= x / i;
        sum += term;
        i++;
    } while (term > 1e-7);
    return sum;
}

double log(double x)
{
    if (x <= 0) {
        return NAN;
    }
    double sum = 0.0;
    double term = 1.0;
    int i = 1;
    while (term > 1e-15) {
        sum += term;
        term *= (x - 1.0) / i;
        i++;
    }
    return sum;
}

double log10(double x)
{
    return log(x) / log(10);
}

double floor(double x)
{
    return x>=0?(int)x:(int)(x-1);
}

double ceil(double x)
{
    return x>=0?(int)(x+1):(int)x;
}

double fmod(double x, double y)
{
    return y==0?NAN:x-((int)(x/y))*y;
}