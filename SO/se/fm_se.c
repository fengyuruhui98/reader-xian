#ifndef _FM_SE_C_
#define _FM_SE_C_
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG_FUDAN	1

int fm_se_encode(unsigned char *cmdbuf, unsigned char *data, unsigned short inlen, unsigned char *outbuf)
{
unsigned char buf[2000], bcc = 0;
int	i, j;
int sendlen;

	i = 6;
	if( inlen > 255 )
	{
		sendlen = 4 + 3 + inlen;
		buf[5] = 0;
		buf[6] = inlen >> 8;
		buf[7] = (unsigned char)inlen;
		i += 3;
	}else
	{
		sendlen = 4 + 1 + inlen;
		buf[5] = inlen;
		//i += 1;
	}
	//ÆðÊ¼
	buf[0] = sendlen >> 8;
	//ÐòºÅ
	buf[1] = (unsigned char)sendlen;
	
	//CLA INS P1 P2
	memcpy(&buf[2], cmdbuf, 4);
	
	//data
	if(inlen > 0)
		memcpy(&buf[i], data, inlen);
	i += inlen;
	
	for( j = 0; j < 7; j++)
	{
		bcc ^= buf[j];
	}
	for(j = 0; j < inlen; j++)
	{
		bcc ^= data[j];
	}
	//
	buf[i++] = ~bcc;
	
#ifdef DEBUG_FUDAN
	printf("FM SE send: ");
	for(j = 0; j < i; j++)
	{
		printf("%02X", buf[j]);
	}	
	printf("\n");
#endif
	memcpy(outbuf, buf, i);
	
	return i;
}

int fm_se_decode(unsigned char *inbuf, unsigned char *outbuf, unsigned short inlen)
{
unsigned char buf[2000], bcc = 0;
int	i, j;
int resvLen;

#ifdef DEBUG_FUDAN
	printf("FM SE Resv: ");
	for(j = 0; j < inlen; j++)
	{
		printf("%02X", inbuf[j]);
	}
	printf("\n");
#endif

	resvLen = (inbuf[0] << 8) + inbuf[1];
	//
	if(resvLen != inlen - 2)
		return 0;
	
	for( j = 0; j < inlen; j++)
	{
		bcc ^= inbuf[j];
	}
	//
	bcc = ~bcc;
	if(inbuf[inlen] != bcc)
		return 0;
	
	memcpy(outbuf, &inbuf[2], resvLen);
	return resvLen;
}


int fm_se_comm(char *cmdbuf, char *inbuf, int inlen, char *outbuf, unsigned short *outlen)
{
unsigned char buf[300], resbuf[300], buff[100];
int len, lreadlen, retryResv;
unsigned char blnTimeout, fStat, command[2];
long rtn;
int i, ret;
unsigned short msgLen;

	/*
	 * spi mode
	 */
	ret = spi_set_CS_low();
	if (ret)
		printf("can't set spi mode NSS low\n");
	buff[0] = 0x02;
	ret = SPI_Write(buff, 1);
	if(ret < 0)
		printf("can't write command\n");
//	
	usleep(4);
	msgLen = fm_se_encode(cmdbuf, inbuf, inlen, resbuf);
//	memcpy(&buff[1], resbuf, msgLen);
//	ret = SPI_Transfer(buff, buf, msgLen);
	ret = SPI_Write(resbuf, msgLen);
	if(ret < 0)
	{
		printf("can't write C_APDU\n");
	}
	ret = spi_set_CS_high();
	if (ret)
		printf("can't set spi mode NSS High\n");

	usleep(5);
	ret = spi_set_CS_low();
	if (ret)
		printf("can't set spi mode NSS low\n");
	buff[0] = 0x05;
	ret = SPI_Write(buff, 1);
	if(ret < 0)
		printf("can't write status\n");
	while(1)
	{
		usleep(1000);
		ret = SPI_Read(buff, 1);
		//ret = SPI_Transfer(buff, buf, 1);
		if(ret < 0)
			printf("can't read status response\n");
		printf("read status response %02x\n", buf[0]);
		if(buf[0] == 0 )
			break;
	}	
	ret = spi_set_CS_high();
	if (ret)
		printf("can't set spi mode NSS High\n");

	ret = spi_set_CS_low();
	if (ret)
		printf("can't set spi mode NSS low\n");

	buff[0] = 0x03;
	ret = SPI_Write(buff, 1);
	if(ret < 0)
		printf("can't write getdata\n");
	
	ret = SPI_Read(buff, 2);
	if (ret< 0)
		printf("can't read getdata length\n");
	
	lreadlen = (buff[0] << 8) + buff[1];
	ret = SPI_Read(&buff[2], lreadlen);
	if(ret < 0)
		printf("can't read get data\n");
		
	*outlen = fm_se_decode(buff, outbuf, msgLen);
		
}


int fm_se_get_version()
{
unsigned char cmd[6];
unsigned char data[100];
unsigned char i;
unsigned short len;
int ret;

	memcpy(cmd, "\xB0\x02\x00\x00", 4);
	
	printf("\nget se version\n");	
	ret = fm_se_comm(cmd, NULL, 0, data, &len);
		
	return ret;
}


int fm_se_getRandom()
{
unsigned char cmd[6];
unsigned char data[100];
unsigned char i;
unsigned short len;
int ret;

	memcpy(cmd, "\x00\x84\x00\x00", 4);
	data[0] = 8;
	
	printf("\nget se random\n");	
	ret = fm_se_comm(cmd, data, 1, data, &len);
	
	return ret;
}		

#endif
