/*
sheet.h 
图层管理头文件
Copyright W24 Studio 
*/

#ifndef SHEET_H
#define SHEET_H

//==================================信息定义==================================

#define MAX_SHEETS 65536
#define SHEET_USE 1
#include <stdint.h>
#include <window.h>
typedef struct SHTCTL shtctl_t;

//图层信息结构体
typedef struct SHEET
{
    uint32_t *buf; // 图层上所描画内容的地址
    int bxsize, bysize; // 图层大小(bxsize*bysize)
    int vx0, vy0;       // 图层在画面上的位置坐标(vx0*vy0)
    int color_inv;      // 色号
    int height;         // 表示该图层在第几层(图层层号)
    int flags;          // 存放图层的设定值
    shtctl_t *ctl;
    window_t *window; //对应的窗口
    uint32_t movable;//可被鼠标拖移
    task_t *task;
}sheet_t;

//图层控制结构体
typedef struct SHTCTL
{
    uint32_t *vram, *map;        // 代表着VRAM的地址
    int xsize, ysize;                 // 代表着画面大小(xsize*ysize)
    int top;                          // 代表最上层的高度
    sheet_t *sheets[MAX_SHEETS]; // 记忆地址变量(对sheet0中存放的图层进行升序排序，并且保存 256*8字节)
    sheet_t sheets0[MAX_SHEETS]; // 存放所有的图层信息(256*32字节)
}shtctl_t;
sheet_t *sheet_alloc(shtctl_t *ctl);
void sheet_setbuf(sheet_t *sht, unsigned int *buf, int xs, int ys, int col_inv);
void sheet_refreshmap(shtctl_t *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
void sheet_refreshsub(shtctl_t *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_updown(sheet_t *sht, int height);
void sheet_refresh(sheet_t *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(sheet_t *sht, int vx0, int vy0);
void sheet_free(sheet_t *sht);
shtctl_t *shtctl_init(unsigned int *vram, int xs, int ys);
void putstr_ascii_sheet(sheet_t *sht, int x, int y, uint32_t c, uint32_t b, char *s);
#endif