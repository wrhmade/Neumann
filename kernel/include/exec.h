/*
exec.h
运行应用程序头文件
Copyright W24 Studio 
*/

#ifndef EXEC_H
#define EXEC_H
#include <sheet.h>
void app_kill(sheet_t *sht);
int sys_create_process(const char *app_name, const char *cmdline, const char *work_dir, window_t *window);
void app_entry(const char *app_name, const char *cmdline, const char *work_dir);
void exec_ldt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint16_t ar);
int try_to_run_external(char *name, int *exist,char *cmd_line, window_t *window);
void *sys_sbrk(int incr);
#endif