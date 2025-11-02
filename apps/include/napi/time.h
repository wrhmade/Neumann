#ifndef NAPI_TIME_H
#define NAPI_TIME_H
typedef struct {
    int year, month, day, hour, min, sec;
} napi_time_t;

void napi_getcurrenttime(napi_time_t *time);
#endif