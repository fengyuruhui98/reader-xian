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



extern const char *rfid_hexdump(const void *data, unsigned int len);
#define DEBUGP(x, args ...) fprintf(stderr, "%s(%d):%s: " x, __FILE__, __LINE__, __FUNCTION__, ## args)
#define DEBUGPC(x, args ...) fprintf(stderr, x, ## args)

#define printk(x, args ...) printf(x, ## args)
#define debug_printf(x, args ...) printf(x, ## args)

#define uword_to_buf(inword,outbuf)   {*((uint8_t *)(outbuf))=*((uint8_t *)&inword+INT_HIGH);*((uint8_t*)(outbuf)+1)=*((uint8_t *)&inword+INT_LOW);}
#define buf_to_uword(inbuf,outword)   {*((uint8_t *)&outword+INT_HIGH)=*((uint8_t *)(inbuf)+0);*((uint8_t *)&outword+INT_LOW)=*((uint8_t *)(inbuf)+1);}
#define udword_to_buf(indword,outbuf) {*((uint8_t *)(outbuf))=(uint8_t)*((uint8_t *)&indword+LONG_HIGH3);*((uint8_t*)(outbuf)+1)=(uint8_t)*((uint8_t *)&indword+LONG_HIGH2);*((uint8_t *)(outbuf)+2)=*((uint8_t *)&indword+LONG_HIGH1);*((uint8_t *)(outbuf)+3)=*((uint8_t *)&indword+LONG_HIGH0);}
#define buf_to_udword(inbuf,outdword) {*((uint8_t *)&outdword+LONG_HIGH3)=*((uint8_t *)inbuf+0);*((uint8_t *)&outdword+LONG_HIGH2)=*((uint8_t *)(inbuf)+1);*((uint8_t *)&outdword+LONG_HIGH1)=*((uint8_t *)(inbuf)+2);*((uint8_t *)&outdword+LONG_HIGH0)=*((uint8_t *)(inbuf)+3);}


#define udword_to_buf3(indword,outbuf) {*((uint8_t*)(outbuf))=(uint8_t)*((uint8_t *)&indword+LONG_HIGH2);*((uint8_t *)(outbuf)+1)=*((uint8_t *)&indword+LONG_HIGH1);*((uint8_t *)(outbuf)+2)=*((uint8_t *)&indword+LONG_HIGH0);}
#define buf3_to_udword(inbuf,outdword) {*((uint8_t *)&outdword+LONG_HIGH3)=0;*((uint8_t *)&outdword+LONG_HIGH2)=*((uint8_t *)(inbuf));*((uint8_t *)&outdword+LONG_HIGH1)=*((uint8_t *)(inbuf)+1);*((uint8_t *)&outdword+LONG_HIGH0)=*((uint8_t *)(inbuf)+2);}


#define udword_to_buf_reverse(indword,outbuf) {*((uint8_t *)(outbuf))=(uint8_t)*((uint8_t *)&indword+LONG_HIGH0);*((uint8_t*)(outbuf)+1)=(uint8_t)*((uint8_t *)&indword+LONG_HIGH1);*((uint8_t *)(outbuf)+2)=*((uint8_t *)&indword+LONG_HIGH2);*((uint8_t *)(outbuf)+3)=*((uint8_t *)&indword+LONG_HIGH3);}
#define buf_to_udword_reverse(inbuf,outdword) {*((uint8_t *)&outdword+LONG_HIGH0)=*((uint8_t *)inbuf+0);*((uint8_t *)&outdword+LONG_HIGH1)=*((uint8_t *)(inbuf)+1);*((uint8_t *)&outdword+LONG_HIGH2)=*((uint8_t *)(inbuf)+2);*((uint8_t *)&outdword+LONG_HIGH3)=*((uint8_t *)(inbuf)+3);}
#define buf_to_uword_reverse(inbuf,outword)   {*((uint8_t *)&outword+INT_LOW)=*((uint8_t *)(inbuf)+0);*((uint8_t *)&outword+INT_HIGH)=*((uint8_t *)(inbuf)+1);}
#define uword_to_buf_reverse(inword,outbuf)   {*((uint8_t *)(outbuf))=*((uint8_t *)&inword+INT_LOW);*((uint8_t*)(outbuf)+1)=*((uint8_t *)&inword+INT_HIGH);}

void delay_us(int us);
void delay_ms(int cnt);
void ms_delay(int us);
void us_delay(int cnt);

UBYTE InitModule(void);
void extendsPrintf(unsigned char *msg, unsigned char *inbuf, int in_len);

#include "spi.h"
#include "rc_base.h"
#include "rc_op.h"
#include "mcmh.h"
#include "mifare.h"
#include "iso_block.h"

#include ".//linux2440lib_sam.h"
#include ".//linux2440lib_uart.h"
#include ".//linux2440lib_sam_prot.h"
#include ".//linux2440lib_sam_int.h"
#include ".//linux2440lib_cbuf.h"
#include ".//linux2440lib_timer.h"

#include "spi.c"
#include "rc_base.c"
#include "rc_op.c"
#include "mcmh.c"
#include "mifare.c"
#include "iso_block.c"

#include ".//linux2440lib_sam.c"
#include ".//linux2440lib_uart.c"
#include ".//linux2440lib_sam_prot.c"
#include ".//linux2440lib_sam_int.c"
#include ".//linux2440lib_cbuf.c"
#include ".//linux2440lib_timer.c"

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