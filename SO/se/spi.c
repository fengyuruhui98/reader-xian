
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
#define SPI_DEBUG


static void pabort(const char *s)
{
	perror(s);
	abort();
}
uint8_t bgRfNow=0;

static const char *spi0 = "/dev/spidev0.0";//kkkkkkkkkkkkkkk
static const char *spi1 = "/dev/spidev1.0";//kkkkkkkkkkkkkkk for 3018 2.0. SE
static const char *spi2 = "/dev/spidev2.0";//kkkkkkkkkkkkkkk for 3018 2.0.
static const char *spi2_nss = "/sys/class/leds/ecspi3_cs0/brightness";
static	int fd_spi0=0,fd_spi1=0, fd_spi2=0, fd_spi2_nss=0;

static uint8_t mode = SPI_MODE_0 ;//| SPI_CS_HIGH;//     SPI_MODE_2(SPI_CPOL|0)  ;
static uint8_t bits = 8;
static uint32_t msb = 0;
//static uint32_t speed = 200*1000;
uint32_t speed = 200*1000;
//static uint16_t delay;

struct spi_ioc_transfer xfer[1];
/* 256bytes max FSD/FSC, plus 1 bytes header, plus 10 bytes reserve */
#define SENDBUF_LEN     (256+1+10)
#define RECVBUF_LEN     SENDBUF_LEN
static uint8_t snd_buf[SENDBUF_LEN];
static uint8_t rcv_buf[RECVBUF_LEN];

static const char *ledon ="1";
static const char *ledoff ="0";


void spi_set_speed(uint32_t sp)
{
	speed = sp;
}

/*=====================================================================================
函数：spi_init
功能：
=======================================================================================*/


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

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return ret;
}

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
	if(bgRfNow){
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

	if (!len)	return -1;

  snd_buf[0] = (reg<<1) | 0x80;
	//snd_buf[0] = reg;
	if (len > 1)
		memset(&snd_buf[1], reg<<1 , len-1);
	snd_buf[len] = 0;

	/* prepare spi buffer */
	xfer[0].tx_buf = (__u64) snd_buf;
	xfer[0].rx_buf = (__u64) rcv_buf;
	xfer[0].len = len + 1;

	ret = (spi_get_chn(), SPI_IOC_MESSAGE(1), xfer);
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

	ret = spidev_read(reg, 1, value);
	if (ret < 0)
		return ret;
	DEBUGP("%s reg = 0x%02x, val = 0x%02x\n", __FUNCTION__, reg, *value);
	return 1;
}

int spidev_reg_write(unsigned char reg, unsigned char value)
{
	int ret;

	ret = spidev_write(reg, 1, &value);
        if (ret < 0)
		return ret;

	DEBUGP("%s reg = 0x%02x, val = 0x%02x\n", __FUNCTION__, reg, value);
	return 1;
}

int spidev_fifo_read(unsigned char reg, unsigned char len, unsigned char *buf)
{
	int ret;

	ret = spidev_read(2, len, buf);//REG_FIFO_DATA
	if (ret < 0)
		return ret;

	DEBUGP("%s len=%u, val=%s\n", __FUNCTION__, len,rfid_hexdump(buf, len));

	return len;
}

int spidev_fifo_write(unsigned char reg,  unsigned char len, const unsigned char *buf)
{
	int ret;

	ret = spidev_write(2, len, buf);
        if (ret < 0)
		return ret;

	DEBUGP("%s len=%u, data=%s\n", __FUNCTION__, len,rfid_hexdump(buf, len));

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


/**
* 功 能：同步数据传输
* 入口参数 ：
* TxBuf -> 发送数据首地址
* len -> 交换数据的长度
* 出口参数：
* RxBuf -> 接收数据缓冲区
* 返回值：0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Transfer(const uint8_t *TxBuf, uint8_t *RxBuf, int len)
{
int ret;


		xfer[0].tx_buf = (unsigned long) TxBuf,
		xfer[0].rx_buf = (unsigned long) RxBuf,
		xfer[0].len =len,


	ret = ioctl(fd_spi2, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 1)
	{
		pabort("can't send spi message");
		return -1;
	}else
	{
		#ifdef SPI_DEBUG
		int i;
		printf("\nsend spi message Succeed");
		printf("\nSPI Send [Len:%d]: ", len);
		for (i = 0; i < len; i++)
		{
			if (i % 8 == 0)
				printf("\n\t");
			printf("0x%02X ", TxBuf[i]);
		}
		printf("\n");
	
	
		printf("SPI Receive [len:%d]:", xfer[0].len);
		for (i = 0; i < xfer[0].len; i++)
		{
			if (i % 8 == 0)
				printf("\n\t");
			printf("0x%02X ", RxBuf[i]);
		}
		printf("\n");
		#endif
	}
	return ret;
}


/**
* 功 能：发送数据
* 入口参数 ：
* TxBuf -> 发送数据首地址
＊len -> 发送与长度
＊返回值：0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Write(uint8_t *TxBuf, int len)
{
int ret;

	ret = write(fd_spi2, TxBuf, len);
	if (ret < 0)
		pabort("SPI Write error\n");
	else
	{
	#ifdef SPI_DEBUG
		int i;
		printf("\nSPI Write [Len:%d]: return %d ", len, ret);
		for (i = 0; i < len; i++)
		{
			if (i % 8 == 0)
				printf("\n\t");
			printf("0x%02X ", TxBuf[i]);
		}
		printf("\n");
		
		
	#endif
	}


	return ret;
}


/**
* 功 能：接收数据
* 出口参数：
* RxBuf -> 接收数据缓冲区
* rtn -> 接收到的长度
* 返回值：>=0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Read(uint8_t *RxBuf, int len)
{
int ret;

	ret = read(fd_spi2, RxBuf, len);
	if (ret < 0)
		pabort("SPI Read error\n");
	else
	{
	#ifdef SPI_DEBUG
		int i;
		printf("SPI Read [len:%d]: return %d\n", len, ret);
		for (i = 0; i < len; i++)
		{
		if (i % 8 == 0)
		printf("\n\t");
		printf("0x%02X ", RxBuf[i]);
		}
		printf("\n");
	#endif
	}


	return ret;
}


/**
* 功 能：打开设备 并初始化设备
* 入口参数 ：
* 出口参数：
* 返回值：0 表示已打开 0XF1 表示SPI已打开 其它出错
* 开发人员：Lzy 2013－5－22
*/
int SPI_Open(void)
{
int fd;
int ret = 0;



	fd_spi2 = open(spi2, O_RDWR);
	if (fd_spi2 < 0)
		pabort("can't open device");
	else
		printf("SPI - Open Succeed. Start Init SPI...n");
	
	fd_spi2_nss = open(spi2_nss, O_RDWR);
	if(fd_spi2_nss < 0)
		pabort("can't open spi2 nss device");
	else
		printf("SPI2 Nss open succeed");
	return fd_spi2;
}


/**
* 功 能：关闭SPI模块
*/
int SPI_Close(void)
{


	close(fd_spi2);
	
	return 0;
}


/**
* 功 能：自发自收测试程序
* 接收到的数据与发送的数据如果不一样 ，则失败
* 说明：
* 在硬件上需要把输入与输出引脚短跑
* 开发人员：Lzy 2013－5－22
*/
int SPI_LookBackTest(void)
{
int ret, i;
const int BufSize = 16;
uint8_t tx[BufSize], rx[BufSize];


	bzero(rx, sizeof(rx));
	for (i = 0; i < BufSize; i++)
	tx[i] = i;
	
	
	pabort("nSPI - LookBack Mode Test...n");
	ret = SPI_Transfer(tx, rx, BufSize);
	if (ret > 1)
	{
	ret = memcmp(tx, rx, BufSize);
	if (ret != 0)
	{
	pabort("LookBack Mode Test errorn");
	//pabort("error");
	}
	else
	pabort("SPI - LookBack Mode OKn");
	}
	
	return ret;
}


int spi_set_CS_low()
{
int ret = 0;
unsigned short fm_se_mode;
	
	/*
	 * spi mode
	 */
	
	if( fd_spi2_nss <= 0)
		return 0;
	write(fd_spi2_nss, ledon, strlen(ledon));
	return 0;	
}
int spi_set_CS_high()
{
int ret = 0;
unsigned short fm_se_mode;
	
	
	if( fd_spi2_nss <= 0)
		return 0;
	write(fd_spi2_nss, ledoff, strlen(ledoff));
	return 0;	
}
//end of file
#endif