/*
sheet.c 
图层管理
Copyright W24 Studio 
*/

#include <sheet.h>
#include <stdint.h>
#include <stddef.h>
#include <mm.h>
#include <graphic.h>
#include <string.h>
#include <task.h>
#include <dbuffer.h>

void putstr_ascii_sheet(sheet_t *sht, int x, int y, uint32_t c, uint32_t b, char *s)
{
    task_t *task=task_now();
    int l=strlen(s);
    boxfill(sht->buf, sht->bxsize, x, y, x + l * 8 - 1, y + 15, b);
    if (task->langmode != 0 && task->langbyte != 0) 
    {
        putstr_ascii(sht->buf, sht->bxsize, x, y, c, s);
        sheet_refresh(sht, x - 8, y, x + l * 8, y + 16);
    }
    else
    {
        putstr_ascii(sht->buf, sht->bxsize, x, y, c, s);
        sheet_refresh(sht, x, y, x + l * 8, y + 16);
    }
    
    
}

// 初始化图层控制结构体函数
shtctl_t *shtctl_init(unsigned int *vram, int xs, int ys)
{
    shtctl_t *ctl;
    int i;

    // 分配记忆图层控制变量的内存空间
    // sizeof(struct  Shtctl)：C语言会计算出该变量型所需的字节数
    ctl = (shtctl_t *)kmalloc(sizeof(shtctl_t));
    if (ctl == 0)
    {
        goto error;
    }
    ctl->map = (unsigned int *)kmalloc(xs * ys * sizeof(uint32_t));
    if (ctl->map == 0)
    {
        kfree(ctl);
        goto error;
    }
    ctl->vram = vram;
    ctl->xsize = xs;
    ctl->ysize = ys;
    ctl->top = -1; // 没有透明图层

    // 把sheets0[i].flag的所有标志全设为0
    for (i = 0; i < MAX_SHEETS; i++)
    {
        ctl->sheets0[i].flags = 0; // 把标志全设为0
        ctl->sheets0[i].ctl = ctl; // 记录所属
    }

error:
    return ctl;
}

// 申请还未被使用的图层
sheet_t *sheet_alloc(shtctl_t *ctl)
{
    sheet_t *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        // 如果sheets0[i].flags的值为0
        if (ctl->sheets0[i].flags == 0)
        {
            // 寻找在sheets0[]中未使用的图层
            sht = &ctl->sheets0[i];

            // 找到在sheets0[]中未使用的图层就把标志设置为正在使用
            sht->flags = SHEET_USE; // 把标志设置为正在使用

            // 表示图层的高度还没有设置
            sht->height = -1; // 隐藏图层

            sht->window = NULL;//标记为非窗口

            sht->movable = 0;//标记为不可拖移
            // 返回其地址
            return sht;
        }
    }
    return 0; // 没有可使用的图层
}

// 设定图层的缓冲区大小和透明色的函数
void sheet_setbuf(sheet_t *sht, unsigned int *buf, int xs, int ys, int col_inv)
{
    sht->buf = buf;
    sht->bxsize = xs;
    sht->bysize = ys;
    if(col_inv==-1)
    {
        sht->have_color_inv=0;
    }
    else
    {
        sht->have_color_inv=1;
        sht->color_inv=col_inv;
    }

    for(int y=0;y<ys;y++)
    {
        for(int x=0;x<xs;x++)
        {
            buf[y*sht->bxsize+x]|=0xFF000000;
        }
    }

    return;
}

// 刷新绘制新图层函数改版2
void sheet_refreshmap(shtctl_t *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned int *buf, sid, *map = ctl->map;
    sheet_t *sht;
    if (vx0 < 0)
    {
        vx0 = 0;
    }
    if (vy0 < 0)
    {
        vy0 = 0;
    }
    if (vx1 > ctl->xsize)
    {
        vx1 = ctl->xsize;
    }
    if (vy1 > ctl->ysize)
    {
        vy1 = ctl->ysize;
    }
    for (h = h0; h <= ctl->top; h++)
    {
        sht = ctl->sheets[h];
        sid = sht - ctl->sheets0; // 计算当前图层的编号
        buf = sht->buf;
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0)
        {
            bx0 = 0;
        }
        if (by0 < 0)
        {
            by0 = 0;
        }
        if (bx1 > sht->bxsize)
        {
            bx1 = sht->bxsize;
        }
        if (by1 > sht->bysize)
        {
            by1 = sht->bysize;
        }
        for (by = by0; by < by1; by++)
        {
            vy = sht->vy0 + by;
            for (bx = bx0; bx < bx1; bx++)
            {
                vx = sht->vx0 + bx;
                if (sht->have_color_inv==0 || buf[by * sht->bxsize + bx] != sht->color_inv)
                {
                    map[vy * ctl->xsize + vx] = sid;
                }
            }
        }
    }
    return;
}

// 刷新绘制新图层函数改版
void sheet_refreshsub(shtctl_t *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned int *buf, *vram = ctl->vram, *map = ctl->map, sid;
    sheet_t *sht;

    // refresh范围超出边界时，进行修正
    if (vx0 < 0)
    {
        vx0 = 0;
    }
    if (vy0 < 0)
    {
        vy0 = 0;
    }
    if (vx1 > ctl->xsize)
    {
        vx1 = ctl->xsize;
    }
    if (vy1 > ctl->ysize)
    {
        vy1 = ctl->ysize;
    }
    for (h = h0; h <= h1; h++)
    {
        sht = ctl->sheets[h];
        buf = sht->buf;
        sid = sht - ctl->sheets0;
        // 计算vx0到vx1和vy0到vy1相对于sht的位置
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0)
        {
            bx0 = 0;
        }
        if (by0 < 0)
        {
            by0 = 0;
        }
        if (bx1 > sht->bxsize)
        {
            bx1 = sht->bxsize;
        }
        if (by1 > sht->bysize)
        {
            by1 = sht->bysize;
        }
        for (by = by0; by < by1; by++)
        {
            vy = sht->vy0 + by;
            for (bx = bx0; bx < bx1; bx++)
            {
                vx = sht->vx0 + bx;
                if (map[vy * ctl->xsize + vx] == sid)
                {
                    vram[vy * ctl->xsize + vx] = alphablend(buf[by * sht->bxsize + bx],vram[vy * ctl->xsize + vx],(buf[by * sht->bxsize + bx]>>24)&0xff);
                }
            }
        }
    }
    dbuffer_refresh();
    return;
}

// 设定图层高度的函数(设定该图层在图层管理序列ctl->sheets中的位置)
void sheet_updown(sheet_t *sht, int height)
{
    shtctl_t *ctl = sht->ctl;
    int h, old = sht->height; // 保存当前图层的高度

    // 如果请求的高度超过了最顶层，则限制在最顶层
    if (height > ctl->top + 1)
    {
        height = ctl->top + 1;
    }
    // 如果请求的高度低于最底层，则限制在最底层
    if (height < -1)
    {
        height = -1;
    }
    sht->height = height; // 更新图层的高度

    // 根据高度变化重新排序sheets[]数组
    if (old > height)
    { // 如果图层的高度降低了
        if (height >= 0)
        {
            // 如果图层没有完全被移除
            for (h = old; h > height; h--)
            {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
            // 更新显示映射，从height+1开始
            sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
            // 更新子图层的显示，从old开始
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
        }
        else
        { // 如果图层被完全移除
            if (ctl->top > old)
            {
                // 如果图层上方还有图层，则将它们下移
                for (h = old; h < ctl->top; h++)
                {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--; // 减少顶层图层计数
            // 更新显示映射
            sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
            // 更新子图层的显示，从0开始
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
        }
    }
    else if (old < height)
    { // 如果图层的高度增加了
        if (old >= 0)
        {
            // 如果图层之前已经存在
            for (h = old; h < height; h++)
            {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else
        { // 如果图层之前不存在
            // 将所有图层上移
            for (h = ctl->top; h >= height; h--)
            {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++; // 增加顶层图层计数
        }
        // 更新显示映射
        sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
        // 更新子图层的显示//
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
    }
    return;
}

// 设定图层的缓冲区大小和透明色的函数
void sheet_refresh(sheet_t *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0)
    { // 如果图层的高度大于等于0，则表示该图层是可见的，需要进行屏幕刷新
        sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
    }
    return;
}

// 移动图层函数(上下左右移动图层)
void sheet_slide(sheet_t *sht, int vx0, int vy0)
{
    shtctl_t *ctl = sht->ctl;              // 获取图层控制块的指针
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0; // 保存图层原来的位置
    sht->vx0 = vx0;                             // 更新图层的X坐标
    sht->vy0 = vy0;                             // 更新图层的Y坐标
    if (sht->height >= 0)
    {                                                                                                              // 如果图层高度大于等于0，即图层是可见的，则需要刷新屏幕
        sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);                  // 刷新旧位置的屏幕映射
        sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);                        // 刷新新位置的屏幕映射
        sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1); // 刷新旧位置的子图层
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);           // 刷新新位置的子图层
    }
    return;
}

// 释放已使用图层的内存函数
void sheet_free(sheet_t *sht)
{
    if (sht->height >= 0)
    {
        sheet_updown(sht, -1); // 如果图层高度大于等于0，即图层是可见的，则需要先将其移出屏幕
    }
    sht->flags = 0; // 清除图层的状态标志，标记为未使用
    return;
}