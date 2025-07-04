/*
disk.c
包括硬盘、软盘等的磁盘操作
Copyright W24 Studio 
*/

#include <disk.h>
#include <hd.h>
#include <fdc.h>
#include <macro.h>
#include <binfo.h>

void my_disk_write_device(int device,int lba,int cnt,void *buffer)
{
    switch(device)
    {
        case DEVICE_HD:
            hd_write(lba,cnt,buffer);
            break;
        case DEVICE_FD:
            write_block(lba,buffer,cnt);
            break;
        default:
            return -1;
    }
}

void my_disk_read_device(int device,int lba,int cnt,void *buffer)
{
    switch(device)
    {
        case DEVICE_HD:
            hd_read(lba,cnt,buffer);
            break;
        case DEVICE_FD:
            read_block(lba,buffer,cnt);
            break;
        default:
            return -1;
    }
}

void my_disk_write(int lba,int cnt,void *buffer)
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    my_disk_write_device(binfo->boot_device,lba,cnt,buffer);
}

void my_disk_read(int lba,int cnt,void *buffer)
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    my_disk_read_device(binfo->boot_device,lba,cnt,buffer);
}