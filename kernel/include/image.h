#ifndef IMAGE_H
#define IMAGE_H
struct DLL_STRPICENV{/*64KB*/
	int work[64*1024/4];
};

struct ARGB{
	unsigned char a,r,g,b;
}__attribute__((packed));

struct BGRA{//小端序
	unsigned char b,g,r,a;
}__attribute__((packed));

struct RGBA{
	unsigned char r,g,b,a;
}__attribute__((packed));

#endif