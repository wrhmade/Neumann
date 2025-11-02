#include <time.h>
#include <stdlib.h>
#include <napi/time.h>

// 判断是否为闰年
static int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取月份的天数
static int get_days_in_month(int year, int month) {
    static const int days_per_month[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days_per_month[month - 1];
}

// 验证日期时间是否有效
static int is_valid_datetime(const napi_time_t *t) {
    if (t->year < 1970 || t->year > 2100 ||
        t->month < 1 || t->month > 12 ||
        t->day < 1 || t->day > get_days_in_month(t->year, t->month) ||
        t->hour < 0 || t->hour > 23 ||
        t->min < 0 || t->min > 59 ||
        t->sec < 0 || t->sec > 59) {
        return 0;
    }
    return 1;
}

// 手动计算时间戳（从1970-01-01 00:00:00开始的秒数）
static long long manual_datetime_to_timestamp(const napi_time_t *t) {
    if (!is_valid_datetime(t)) {
        return -1;
    }

    long long total_days = 0;
    long long total_seconds = 0;
    
    // 计算从1970年到目标年份之前的总天数
    for (int y = 1970; y < t->year; y++) {
        total_days += is_leap_year(y) ? 366 : 365;
    }
    
    // 计算目标年份中，目标月份之前的总天数
    for (int m = 1; m < t->month; m++) {
        total_days += get_days_in_month(t->year, m);
    }
    
    // 加上目标月份的天数（减去1，因为从1日开始）
    total_days += t->day - 1;
    
    // 计算总秒数
    total_seconds = total_days * 24 * 3600;  // 天数转秒数
    total_seconds += t->hour * 3600;         // 小时转秒数
    total_seconds += t->min * 60;            // 分钟转秒数
    total_seconds += t->sec;                 // 秒数
    
    return total_seconds;
}

time_t time(time_t *timer)
{
    napi_time_t current_time;
    napi_getcurrenttime(&current_time);
    int ret=manual_datetime_to_timestamp(&current_time);
    if(ret==-1)
    {
        if(timer!=NULL)
        {
            *timer=(time_t)-1;
        }
        return -1;
    }
    if(timer!=NULL)
    {
        *timer=(time_t)ret;
    }
    return ret;
}
