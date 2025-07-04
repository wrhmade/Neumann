/*
nullzero.c
空设备 & 零设备
Copyright W24 Studio 
*/

#include <nullzero.h>
#include <vdisk.h>
#include <string.h>
#include <stdint.h>

void null_read(int drive, uint8_t *buffer, uint32_t number, uint32_t lba)
{
    buffer[0]=-1;
}

void null_write(int drive, uint8_t *buffer, uint32_t number, uint32_t lba)
{

}

void zero_read(int drive, uint8_t *buffer, uint32_t number, uint32_t lba)
{
    for(int i=0;i<number;i++)
    {
        buffer[i]=0;
    }

}

void zero_write(int drive, uint8_t *buffer, uint32_t number, uint32_t lba)
{
    
}

void init_nullzero()
{
    vdisk null,zero;
    strcpy(null.DriveName,"null");
    null.flag=1;
    null.Read=null_read;
    null.Write=null_write;
    null.sector_size=1;
    null.size=1;

    strcpy(zero.DriveName,"zero");
    zero.flag=1;
    zero.Read=zero_read;
    zero.Write=zero_write;
    zero.sector_size=1;
    zero.size=1;

    register_vdisk(null);
    register_vdisk(zero);
}