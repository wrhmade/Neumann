#include "klotski.h"
#include <stdio.h>

int main()
{
    kmap_t *kmap=create_map(4,4);
    disrupt_map(kmap,50);
    int a;
    while(!map_check(kmap))
    {
        for(int i=0;i<kmap->ysize;i++)
        {
            for(int j=0;j<kmap->xsize;j++)
            {
                printf("%3d ",MAP_POS(kmap,j,i));
            }
            printf("\n");
        }
        scanf("%d",&a);
        map_move_number(kmap,a);
    }
    printf("Game over\n");
}