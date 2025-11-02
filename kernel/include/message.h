/*
message.h
弹窗消息头文件
Copyright W24 Studio 
*/

#ifndef MESSAGE_H
#define MESSAGE_H

#include <sheet.h>
#include <task.h>

#define MTMAN_MAX 300

typedef struct MESSGAE_TIP_MAN
{
    sheet_t *message_tip_sht;
    uint32_t *message_tip_buf;
    task_t *task;
    int flag;
}mtman_t;

void error_message(char *content,char *title);
void warn_message(char *content,char *title);
void info_message(char *content,char *title);

void message_tip_show(const char *title,const char *content);
#endif