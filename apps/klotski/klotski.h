//klotski.h
//数字华容道简单逻辑头文件
//Copyright W24 Studio 2025
#ifndef KLOTSKI_H
#define KLOTSKI_H
#define POSITION(x,xsize,y) y*xsize+x
#define MAP_POS(kmap,x,y) kmap->map[POSITION(x,kmap->xsize,y)]

//klotski.c
typedef struct KLOTSKI_MAP
{
    int *map;
    int xsize,ysize;
}kmap_t;

kmap_t *create_map(int xsize,int ysize);
void free_map(kmap_t *kmap);
int map_find_number(kmap_t *kmap,int n,int *x,int *y);
int map_move_number(kmap_t *kmap,int n);
void disrupt_map(kmap_t *kmap,int times);
int map_check(kmap_t *kmap);

//rand.c
int randint(int min,int max);
#endif