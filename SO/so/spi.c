
#ifndef _SPI_C_
#define _SPI_C_

//start of file
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <string.h>
#include "global.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
//#define SPI_DEBUG


static void pabort(const char *s)
{
	perror(s);
	abort();
}


static const char *spi0 = "/dev/spidev0.0";//kkkkkkkkkkkkkkk
#ifdef	READER_3018
//3018
	static const char *spi1 = "/dev/spidev2.0";//kkkkkkkkkkkkkkk
#else
//3030
	static const char *spi1 = "/dev/spidev1.0";//kkkkkkkkkkkkkkk
#endif
static	int fd_spi0=0,fd_spi1=0;

static uint8_t mode = SPI_MODE_0 ;//| SPI_CS_HIGH;//     SPI_MODE_2(SPI_CPOL|0)  ;
static uint8_t bits = 8;
static uint32_t msb = 0;
//uint32_t speed = 4 * 1000*1000;
uint32_t speed = 2 * 1000*1000;
static uint16_t delay;

struct spi_ioc_transfer xfer[1];
/* 256bytes max FSD/FSC, plus 1 bytes header, plus 10 bytes reserve */
#define SENDBUF_LEN     (256+1+10)
#define RECVBUF_LEN     SENDBUF_LEN
static char snd_buf[SENDBUF_LEN];
static char rcv_buf[RECVBUF_LEN];


unsigned char blnSPI = 0;

void spi_set_speed(uint32_t sp)
{
	speed = sp;
}

int spi_set_mode(int fd)
{
	int ret = 0;
	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/* MSB First */
	ret = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &msb);
	if (ret == -1)
		pabort("can't get spi WR_MSB");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

#ifdef	SPI_DEBUG
	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
#endif

	return ret;
}

/*=====================================================================================
函数：spi_init
功能：
=======================================================================================*/

/*--------------------------*/
void spi_init(void)
{

	fd_spi0 = open(spi0, O_RDWR);
	if (fd_spi0 < 0)
		pabort("can't open device");
	spi_set_mode(fd_spi0);	
		
	fd_spi1 = open(spi1, O_RDWR);
	if (fd_spi1 < 0)
		pabort("can't open device");
	spi_set_mode(fd_spi1);	
}

void spi_init_chn(void)//2017/9/19 10:25:13
{
	if(bgRfNow)
	{
		  if (fd_spi1 > 0) return;
			fd_spi1 = open(spi1, O_RDWR);
			if (fd_spi1 < 0)
				pabort("can't open device");
			spi_set_mode(fd_spi1);	
	}
	else {
		if (fd_spi0 > 0) return;
		fd_spi0 = open(spi0, O_RDWR);
		if (fd_spi0 < 0)
			pabort("can't open device");
		spi_set_mode(fd_spi0);	
	}
}



void spi_close()
{
	if (fd_spi0 > 0)
		close(fd_spi0);	
		
	if (fd_spi1 > 0)
		close(fd_spi1);	
}



/*=====================================================================================
函数：spi_send_byte
功能：
=======================================================================================*/
int spi_get_chn()
{
	if(bgRfNow) return fd_spi1;
	else return fd_spi0;
}


int spidev_read(unsigned char reg, unsigned char len,
		       unsigned char *buf)
{
int ret;
	
	if( !blnSPI)
	{
		spi_init();
		spi_set_mode(fd_spi0);
		spi_set_mode(fd_spi1);
		blnSPI = 0xff;
	}

	if (!len)	return -1;

	snd_buf[0] = (reg<<1) | 0x80;
	//snd_buf[0] = reg;
	if (len > 1)
		memset(&snd_buf[1], reg<<1 , len-1);
	snd_buf[len] = 0;

	/* prepare spi buffer */
	//memset(xfer, 0, sizeof xfer);
	xfer[0].tx_buf = (__u64) snd_buf;
	xfer[0].rx_buf = (__u64) rcv_buf;
	xfer[0].len = len + 1;

	ret = ioctl(spi_get_chn(), SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) {
		DEBUGPC("ERROR sending command\n");
		return ret;
	} else if (ret != (len + 1)) {
		DEBUGPC("ERROR sending command bad length\n");
		return -1;
	}

	memcpy(buf, &rcv_buf[1], len);

	return len;
}

int spidev_write(unsigned char reg, unsigned char len,
		       const unsigned char *buf)
{
int ret;

	if( !blnSPI)
	{
		spi_init();
		spi_set_mode(fd_spi0);
		spi_set_mode(fd_spi1);
		blnSPI = 0xff;
	}

	if (!len)
		return -1;

	snd_buf[0] = (reg << 1) & 0x7E;
  //snd_buf[0] = reg;
	memcpy(&snd_buf[1], buf, len);

	/* prepare spi buffer */
	xfer[0].tx_buf = (__u64) snd_buf;
	xfer[0].rx_buf = (__u64) NULL;
	xfer[0].len = len + 1;

	ret = ioctl(spi_get_chn(), SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) {
		DEBUGPC("ERROR sending command\n");
		return ret;
	}
	else if (ret != len+1)
		return -1;

	return len;
}



int spidev_reg_read(unsigned char reg, unsigned char *value)
{
int ret;

	if( !blnSPI)
	{
		spi_init();
		spi_set_mode(fd_spi0);
		spi_set_mode(fd_spi1);
		blnSPI = 0xff;
	}

	ret = spidev_read(reg, 1, value);
	if (ret < 0)
		return ret;
	//DEBUGP("%s reg = 0x%02x, val = 0x%02x\n", __FUNCTION__, reg, *value);
	//DEBUGP("reg = 0x%02x, val = 0x%02x\n",reg, *value);
	return 1;
}

int spidev_reg_write(unsigned char reg, unsigned char value)
{
	int ret;

	if( !blnSPI)
	{
		spi_init();
		spi_set_mode(fd_spi0);
		spi_set_mode(fd_spi1);
		blnSPI = 0xff;
	}
	ret = spidev_write(reg, 1, &value);
        if (ret < 0)
		return ret;

	//DEBUGP("%s reg = 0x%02x, val = 0x%02x\n", __FUNCTION__, reg, value);
	//DEBUGP("reg = 0x%02x, val = 0x%02x\n",reg, *value);
	return 1;
}

int spidev_fifo_read(unsigned char reg, unsigned char len, unsigned char *buf)
{
	int ret;

	if( !blnSPI)
	{
		spi_init();
		spi_set_mode(fd_spi0);
		spi_set_mode(fd_spi1);
		blnSPI = 0xff;
	}
	ret = spidev_read(2, len, buf);//REG_FIFO_DATA
	if (ret < 0)
		return ret;

	//DEBUGP("%s len=%u, val=%s\n", __FUNCTION__, len,
	//       rfid_hexdump(buf, len));

	return len;
}

int spidev_fifo_write(unsigned char reg,  unsigned char len, const unsigned char *buf)
{
	int ret;

	if( !blnSPI)
	{
		spi_init();
		spi_set_mode(fd_spi0);
		spi_set_mode(fd_spi1);
		blnSPI = 0xff;
	}
	ret = spidev_write(2, len, buf);
	if (ret < 0)
		return ret;

	//DEBUGP("%s len=%u, data=%s\n", __FUNCTION__, len,
	//       rfid_hexdump(buf, len));

	return len;
}



/*
void spi_send_byte(uint8_t inbyte)
{
	int ret;
	ret = write(fd_531, &inbyte, 1);
	if (ret < 0){
	  perror("spi open error");
	}else{
#ifdef SPI_DEBUG
		printf("spi send:%.2X \n", inbyte);
		//puts("");
#endif
	}
	//return ret;
}
void spi_send_byte(uint8_t inbyte)
{
	int ret;

 	struct spi_ioc_transfer tr = {
		.tx_buf = &inbyte,
		.rx_buf = NULL,
		.len = 1,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd_531, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

#ifdef SPI_DEBUG
		printf("spi send:%.2X \n", inbyte);
		//puts("");
#endif
}*/
/*=====================================================================================
函数：spi_rece_byte
功能：
=======================================================================================*/

/*uint8_t spi_rece_byte(void)
{
	int ret;
	uint8_t val;
	ret = read(fd_531, &val, 1);
	if (ret < 0){
	  perror("spi open error");
	}else{
#ifdef SPI_DEBUG
		printf("spi rec :%.2X \n", val);
#endif
	}
 return val;
}

uint8_t spi_rece_byte(uint8_t inbyte)
{
	int ret;
	uint8_t val;

 	struct spi_ioc_transfer tr = {
		.tx_buf = NULL,
		.rx_buf = &val,
		.len = 1,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd_531, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

#ifdef SPI_DEBUG
		printf("spi rec:%.2X \n", val);
		//puts("");
#endif
 return val;
}*/
/*=====================================================================================
函数：
功能：
=======================================================================================*/

/**

int spi_send_bytes(int fd, uint8_t *tx, int len)
{
	int ret;
	ret = write(fd, tx, len);
	if (ret < 0){
	  perror("SPI_IOC_MESSAGE");
	}else{
#ifdef SPI_DEBUG
    int i;
		for (i = 0; i < len; i++)
		{
			if (!(ret % 8))	puts("");
			printf("%.2X ", tx[i]);
		}
		puts("");
#endif
	}
	return ret;
}
*/

/**

int spi_read_bytes(int fd, uint8_t *rx, int len)
{
	int ret;
	ret = read(fd, rx, len);
	if (ret < 0){
	  perror("SPI_IOC_MESSAGE");
	}else{
#ifdef SPI_DEBUG
    int i;
		for (i = 0; i < len; i++)
		{
			if (!(ret % 8))	puts("");
			printf("%.2X ", rx[i]);
		}
		puts("");
#endif
	}
return ret;
}

*/
/*=====================================================================================
函数：
功能：
======================================================================================
static void do_msg(int fd, int len)  
{  
 struct spi_ioc_transfer xfer[2];  
 unsigned char  buf[32], *bp;  
 int   status;  

 memset(xfer, 0, sizeof(xfer));  
 memset(buf, 0, sizeof(buf));  
 if (len > sizeof(buf))  
  len = sizeof(buf);  
 buf[0] = 0xaa;  
 xfer[0].tx_buf = (__u64) buf;  
 xfer[0].len = 1;  
 xfer[1].rx_buf = (__u64) buf;  
 xfer[1].len = len;  
 status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);  
 if (status < 0) {  
  perror("SPI_IOC_MESSAGE");  
  return;  
 }  
 printf("response(%2d, %2d): ", len, status);  
 for (bp = buf; len; len--)  
  printf(" %02x", *bp++);  
 printf("/n");  
}  


static void transfer(int fd, int len)
{
	int ret;
	uint8_t tx[transfer_len] = {};
	uint8_t rx[ARRAY_SIZE(tx)] = {};
  
  memset(tx, 0, sizeof tx);  
  memset(rx, 0, sizeof rx);  

 	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);
	}
	puts("");
}


=*/



//end of file
#endif