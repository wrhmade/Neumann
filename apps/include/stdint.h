/*
stdint.h 
一般变量别名 
Copyright W24 Studio 
*/
#ifndef STDINT_H
#define STDINT_H 
#ifndef __int8_t_defined  
# define __int8_t_defined  
typedef signed char             int8_t;   
typedef short int               int16_t;  
typedef int                     int32_t;  
# if __WORDSIZE == 64  
typedef long int                int64_t;  
# else  
__extension__  
typedef long long int           int64_t;  
# endif  
#endif  
  
  
typedef unsigned char           uint8_t;  
typedef unsigned short int      uint16_t;  
#ifndef __uint32_t_defined  
typedef unsigned int            uint32_t;  
# define __uint32_t_defined  
#endif  
#if __WORDSIZE == 64  
typedef unsigned long int       uint64_t;  
#else  
__extension__  
typedef unsigned long long int  uint64_t;  
#endif  

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned long	ulong;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#define SIZE_MAX 4294967295U
#endif
