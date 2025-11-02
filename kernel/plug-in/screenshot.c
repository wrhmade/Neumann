/*
screenshot.c
截图
Copyright W24 Studio 
*/

#include <binfo.h>
#include <graphic.h>
#include <macro.h>
#include <mm.h>
#include <string.h>
#include <stdio.h>
#include <vfs.h>
#include <cmos.h>
#include <console.h>
#include <image.h>
#include <message.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#define STBIW_MALLOC(sz)        kmalloc(sz)
#define STBIW_REALLOC(p,newsz)  krealloc(p,newsz)
#define STBIW_FREE(p)           kfree(p)
#include <stb_image_write.h>

static void bgra_to_rgba(struct BGRA *bgra, struct RGBA *rgba, int len)
{
    for(int i = 0; i < len; i++)
    {
        rgba[i].r = bgra[i].r;
        rgba[i].g = bgra[i].g;
        rgba[i].b = bgra[i].b;
        rgba[i].a = bgra[i].a;
    }
}

int screenshot()
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    int out_len;

    struct RGBA *img=(struct RGBA *)kmalloc(sizeof(struct RGBA)*binfo->scrnx*binfo->scrny);

    bgra_to_rgba((struct BGRA *)(binfo->vram),img,binfo->scrnx*binfo->scrny);
    for(int i=0;i<binfo->scrnx*binfo->scrny;i++)
    {
        img[i].a=0xFF;
    }


    unsigned char *data=stbi_write_png_to_mem((const unsigned char *)img,binfo->scrnx*4,binfo->scrnx,binfo->scrny,4,&out_len);
    char path[1000];
    char s[1000];
    kfree(img);

    current_time_t ctime;
    get_current_time(&ctime);
    sprintf(path,"/screenshot_%d-%02d-%02d_%02d-%02d-%02d.png",ctime.year,ctime.month,ctime.day,ctime.hour,ctime.min,ctime.sec);

    vfs_node_t node;
    if(vfs_mkfile(path)!=0)
    {
        return -1;
    }
    node=vfs_open(path);
    vfs_write(node,data,0,out_len);
    STBIW_FREE(data);

    sprintf(s,"你的屏幕截图已保存在%s.",path);

    message_tip_show("截图",s);
    return 0;
}