#ifndef IMAGE_H
#define IMAGE_H
struct DLL_STRPICENV{/*64KB*/
	int work[64*1024/4];
};
struct ARGB{
	unsigned char a,r,g,b;
};
#endif