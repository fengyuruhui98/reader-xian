#ifndef _GLOBAL_H_
#define _GLOBAL_H_
//start of file

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
#include <time.h>

#include <errno.h>

#include "linux2440lib.h"

#define UBYTE unsigned char
#define UWORD unsigned short
#define UDWORD unsigned long


#define INT_LOW   0
#define INT_HIGH  1
#define LONG_HIGH3  3
#define LONG_HIGH2  2
#define LONG_HIGH1  1
#define LONG_HIGH0  0

//---------------------------------------------------------------
#define BIT00_MASK  0x00000001
#define BIT01_MASK  0x00000002
#define BIT02_MASK  0x00000004
#define BIT03_MASK  0x00000008
#define BIT04_MASK  0x00000010
#define BIT05_MASK  0x00000020
#define BIT06_MASK  0x00000040
#define BIT07_MASK  0x00000080
#define BIT08_MASK  0x00000100
#define BIT09_MASK  0x00000200
#define BIT10_MASK  0x00000400
#define BIT11_MASK  0x00000800
#define BIT12_MASK  0x00001000
#define BIT13_MASK  0x00002000
#define BIT14_MASK  0x00004000
#define BIT15_MASK  0x00008000
#define BIT16_MASK  0x00010000
#define BIT17_MASK  0x00020000
#define BIT18_MASK  0x00040000
#define BIT19_MASK  0x00080000
#define BIT20_MASK  0x00100000
#define BIT21_MASK  0x00200000
#define BIT22_MASK  0x00400000
#define BIT23_MASK  0x00800000
#define BIT24_MASK  0x01000000
#define BIT25_MASK  0x02000000
#define BIT26_MASK  0x04000000
#define BIT27_MASK  0x08000000
#define BIT28_MASK  0x10000000
#define BIT29_MASK  0x20000000
#define BIT30_MASK  0x40000000
#define BIT31_MASK  0x80000000
#define BIT0_MASK   BIT00_MASK
#define BIT1_MASK   BIT01_MASK
#define BIT2_MASK   BIT02_MASK
#define BIT3_MASK   BIT03_MASK
#define BIT4_MASK   BIT04_MASK
#define BIT5_MASK   BIT05_MASK
#define BIT6_MASK   BIT06_MASK
#define BIT7_MASK   BIT07_MASK
#define BIT8_MASK   BIT08_MASK
#define BIT9_MASK   BIT09_MASK




void delay_us(int us);
void delay_ms(int cnt);
void ms_delay(int us);
void us_delay(int cnt);

UBYTE InitModule(void);
void extendsPrintf(unsigned char *msg, unsigned char *inbuf, int in_len);


#include "cos.c"

#include "cos.h"

uint8_t bgRfNow=0;

/*=============================================================================
º¯Êý£ºdelay_ms
¹¦ÄÜ£º
============================================================================
void delay_us(UWORD delay_us)
{
	//udelay(delay_us);
//2013/10/23 10:17:08
	sh_us_delay(delay_us);
return;	
}	
void delay_ms(UWORD cnt)
{
	//mdelay(cnt);
	sh_ms_delay(cnt);
return;	
}	===*/


//end of file
#endif