#ifndef JPEG_H
#define JPEG_H
#include <image.h>
int info_JPEG(struct DLL_STRPICENV *env,int *info,int size,unsigned char *fp);
int decode0_JPEG(struct DLL_STRPICENV *env,int size,unsigned char *fp,int b_type,unsigned char *buf,int skip);
#endif