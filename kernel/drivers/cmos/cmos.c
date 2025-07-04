/*
cmos.c
CMOS读取
Copyright W24 Studio 
*/

#include <cmos.h>
#include <stdint.h>
#include <io.h>

#pragma GCC optimize("00") //硬件处理不开优化

static uint8_t read_cmos(uint8_t p)
{
    uint8_t data;
    io_out8(CMOS_INDEX, p);
    data = io_in8(CMOS_DATA);
    io_out8(CMOS_INDEX, 0x80);
    return data;
}

void get_current_time(current_time_t *ctime)
{
    ctime->year = bcd2hex(read_cmos(CMOS_CUR_CEN)) * 100 + bcd2hex(read_cmos(CMOS_CUR_YEAR));
    ctime->month = bcd2hex(read_cmos(CMOS_CUR_MON));
    ctime->day = bcd2hex(read_cmos(CMOS_CUR_DAY));
    ctime->hour = bcd2hex(read_cmos(CMOS_CUR_HOUR));
    ctime->min = bcd2hex(read_cmos(CMOS_CUR_MIN));
    ctime->sec = bcd2hex(read_cmos(CMOS_CUR_SEC));
}
