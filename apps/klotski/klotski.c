//klotski.c
//数字华容道简单逻辑
//Copyright W24 Studio 2025
#include "klotski.h"
#include <stdlib.h>

kmap_t *create_map(int xsize,int ysize)
{
    int size=xsize*ysize;
    kmap_t *kmap=malloc(sizeof(kmap_t));
    kmap->xsize=xsize;
    kmap->ysize=ysize;
    kmap->map=malloc(sizeof(int)*size);
    int n=1;
    for(int i=0;i<ysize;i++)
    {
        for(int j=0;j<xsize;j++)
        {
            MAP_POS(kmap,j,i)=(i==ysize-1 && j==xsize-1)?-1:n++;
        }
    }
    return kmap;
}

void free_map(kmap_t *kmap)
{
    free(kmap->map);
    free(kmap);
}

int map_find_number(kmap_t *kmap,int n,int *x,int *y)
{
    if(x==NULL || y==NULL)return -1;
    for(int i=0;i<kmap->ysize;i++)
    {
        for(int j=0;j<kmap->xsize;j++)
        {
            if(MAP_POS(kmap,j,i)==n)
            {
                *x=j;
                *y=i;
                return 0;
            }
        }
    }
    return -1;
}

int map_move_number(kmap_t *kmap,int n)
{
    int x,y,tx=-1,ty=-1,ttx,tty;
    int dx[4]={-1,1,0,0};
    int dy[4]={0,0,1,-1};
    map_find_number(kmap,-1,&x,&y);

    for(int i=0;i<4;i++)
    {
        ttx=x+dx[i];
        tty=y+dy[i];
        if(ttx>=0 && ttx<=kmap->xsize-1)
        {
            if(tty>=0 && tty<=kmap->ysize-1)
            {
                if(MAP_POS(kmap,ttx,tty)==n)
                {
                    tx=ttx;
                    ty=tty;
                }
            }
        }
    }

    if(tx==-1 || ty==-1)
    {
        return -1;
    }
    else
    {
        int t;
        t=MAP_POS(kmap,tx,ty);
        MAP_POS(kmap,tx,ty)=MAP_POS(kmap,x,y);
        MAP_POS(kmap,x,y)=t;
    }
}

static void disrupt_map_sub(kmap_t *kmap)
{
    int x,y;
    map_find_number(kmap,-1,&x,&y);
    int dx[4]={0,0,1,-1};
    int dy[4]={1,-1,0,0};
    int ttx[4],tty[4],ttn=0;
    for(int i=0;i<4;i++)
    {
        if(x+dx[i]>=0 && x+dx[i]<=kmap->xsize-1)
        {
            if(y+dy[i]>=0 && y+dy[i]<=kmap->ysize-1)
            {
                ttx[ttn]=x+dx[i];
                tty[ttn]=y+dy[i];
                ttn++;
            }
        }
    }
    int n=randint(0,ttn-1);
    map_move_number(kmap,MAP_POS(kmap,ttx[n],tty[n]));
}

void disrupt_map(kmap_t *kmap,int times)
{
    for(int i=0;i<times;i++)
    {
        disrupt_map_sub(kmap);
    }
}

int map_check(kmap_t *kmap)
{
    int n=1;
    for(int i=0;i<kmap->ysize;i++)
    {
        for(int j=0;j<kmap->xsize;j++)
        {
            if(MAP_POS(kmap,j,i)==n)
            {
                n++;
            }
            else
            {   return j==kmap->xsize-1 && i==kmap->ysize-1 && MAP_POS(kmap,j,i)==-1;
            }
        }
    }
    return 1;
}
