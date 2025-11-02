//rand.c
//随机数生成
//Copyright W24 Studio 2025
#include "klotski.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

// --- 定义MT19937常数 ---
// 注意：这些常数是针对32位版本的MT19937
#define MT_N 624
#define MT_M 397
#define MT_MATRIX_A 0x9908b0dfUL   // 常数向量 a
#define MT_UPPER_MASK 0x80000000UL // 最高有效位 w-r 位
#define MT_LOWER_MASK 0x7fffffffUL // 最低有效位 r 位

// 状态向量结构体（可选，这里我们使用全局变量）
static uint32_t mt[MT_N]; // 状态数组
static int mti = MT_N + 1; // mti == N+1 表示 mt[N] 未初始化

// --- 函数声明 ---
void mt_seed(uint32_t seed);
uint32_t mt_rand(void);
void mt_init_by_array(uint32_t init_key[], int key_length);

// --- 初始化生成器 ---
// 使用一个32位整数作为种子
void mt_seed(uint32_t seed) {
    mt[0] = seed;
    for (mti = 1; mti < MT_N; mti++) {
        // 参见 Knuth TAOCP Vol2. 3rd Ed. P.106 用于乘数
        // 过去版本使用另一个乘数 (69069)，但那个效果不佳
        mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
    }
}

// --- 用数组初始化生成器 ---
// 使用一个键数组初始化，适用于需要更强随机种子的情况
void mt_init_by_array(uint32_t init_key[], int key_length) {
    int i, j, k;
    mt_seed(19650218UL);
    i = 1; j = 0;
    k = (MT_N > key_length ? MT_N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL)) + init_key[j] + j; // 非线性
        i++; j++;
        if (i >= MT_N) { mt[0] = mt[MT_N-1]; i = 1; }
        if (j >= key_length) j = 0;
    }
    for (k = MT_N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL)) - i; // 非线性
        i++;
        if (i >= MT_N) { mt[0] = mt[MT_N-1]; i = 1; }
    }
    mt[0] = 0x80000000UL; // MSB 是 1; 确保非初始零数组
}

// --- 生成随机数 ---
// 在 [0, 2^32-1] 范围内生成一个随机数
uint32_t mt_rand(void) {
    static uint32_t mag01[2] = {0x0UL, MT_MATRIX_A};
    // mag01[x] = x * MT_MATRIX_A，其中 x 是 0 或 1

    uint32_t y;

    // 如果所有数字都用完了，生成一组新的
    if (mti >= MT_N) {
        int kk;

        // 如果未初始化，使用默认种子
        if (mti == MT_N + 1) {
            mt_seed(5489UL); // 默认初始种子
        }

        // 生成 N 个数字
        for (kk = 0; kk < MT_N - MT_M; kk++) {
            y = (mt[kk] & MT_UPPER_MASK) | (mt[kk+1] & MT_LOWER_MASK);
            mt[kk] = mt[kk+MT_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < MT_N-1; kk++) {
            y = (mt[kk] & MT_UPPER_MASK) | (mt[kk+1] & MT_LOWER_MASK);
            mt[kk] = mt[kk+(MT_M-MT_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[MT_N-1] & MT_UPPER_MASK) | (mt[0] & MT_LOWER_MASK);
        mt[MT_N-1] = mt[MT_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    // 温度变换（Tempering）
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

int randint(int min,int max)
{
    mt_seed((uint32_t)time(0));
    return mt_rand()%(max-min+1)+min;
}