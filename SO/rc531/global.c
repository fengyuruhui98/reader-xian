#ifndef _GLOBAL_C_
#define _GLOBAL_C_
//start of file

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <linux/types.h>



const char *
rfid_hexdump(const void *data, unsigned int len)
{
	static char string[1024];
	unsigned char *d = (unsigned char *) data;
	unsigned int i;//, left;

	string[0] = '\0';
	//left = sizeof(string);
	for (i = 0; len--; i += 3) {
		if (i >= sizeof(string) -4)
			break;
		snprintf(string+i, 4, " %02x", *d++);
	}
	return string;
}


void delay_ms(int cnt)//???????????
{
	usleep(1000*cnt);
}

void delay_us(int cnt)//???????????
{
	usleep(cnt);
}

void ms_delay(int cnt)//???????????
{
	usleep(1000*cnt);
}

void us_delay(int cnt)//???????????
{
	usleep(cnt); 
}

#endif