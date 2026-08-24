//rc_base.c

#ifndef _RC_BASE_C_
#define _RC_BASE_C_
//start of file
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include<string.h>

#include "global.h"


//变量定义
uint8_t gRc500Key[6];
uint8_t gThisCardSnr[13];
uint8_t gNhhFlag=0;
uint8_t bgRfNow=0;
uint8_t bgRcPageNow[2]={0,0};

/*=============================================================================================
函数：
功能：
===============================================================================================*/
void sh_us_delay(int cnt)
{
	int i;
	//for( i = 0; i < 25*cnt; i++);
	for( i = 0; i < 15*cnt; i++);
	
}

void rc_delay(void)
{
	usleep(200);
	//???sh_us_delay(1);
	return;	
}	


/*=============================================================================================
函数：rc522_write_byte
功能：
===============================================================================================*/
/*
void rc_write_byte(uint8_t addr,uint8_t inbyte)
{
	uint8_t page,reg_in_page;	
	//
	addr &= 0x3f;	
	page = (addr>>3); reg_in_page = (addr&0x07);
	//
	if(page != bgRcPageNow[bgRfNow]){
	   //rc_cs_clr();
	   //spi_send_byte(REG_RC500_PAGE);
	   //spi_send_byte(page|0x80);
	   //rc_cs_set();
     spidev_reg_write(REG_RC500_PAGE,page|0x80);
	   bgRcPageNow[bgRfNow] = page;   
	   }
//	rc_cs_clr();
//	spi_send_byte(reg_in_page<<1);
//	spi_send_byte(inbyte);
	spidev_reg_write(reg_in_page<<1,inbyte);
//	rc_cs_set();
	return;
}*/
void rc_write_byte(uint8_t addr,uint8_t inbyte)
{
	spidev_reg_write(addr,inbyte);
	return;
}
/*=============================================================================================
函数：rc522_read_byte
功能：
===============================================================================================*/
/*uint8_t rc_read_byte(uint8_t addr)
{
	uint8_t ch;	
	uint8_t page,reg_in_page;	
	
	addr &= 0x3f;	
	page = (addr>>3); reg_in_page = (addr&0x07);
	
	if(page != bgRcPageNow[bgRfNow]){
	   //rc_cs_clr();
	   //spi_send_byte(REG_RC500_PAGE);
	   //spi_send_byte(page|0x80);
	   //rc_cs_set();
     spidev_reg_write(REG_RC500_PAGE,page|0x80);
	   bgRcPageNow[bgRfNow] = page;   
	   }
	//rc_cs_clr();
	//spi_send_byte((reg_in_page<<1)|0x80);
	//ch = spi_rece_byte();
	spidev_reg_read((reg_in_page<<1)|0x80, &ch);
	//rc_cs_set();

return ch;
}*/

uint8_t rc_read_byte(uint8_t addr)
{
uint8_t ch;	
/*
rc_cs_clr();
spi_send_byte((addr<<1)|0x80);
ch = spi_rece_byte();
rc_cs_set();
*/
spidev_reg_read(addr, &ch);
return ch;
}



/*=============================================================================================
函数：rc_write_bytes
功能：
===============================================================================================*/
void rc_write_bytes(uint8_t addr,uint8_t *inbuf,uint8_t inbytes)
{
//	uint8_t i;
		
//	for(i=0;i<inbytes;i++){
//	   rc_write_byte(addr,inbuf[i]);
//	   }
	spidev_fifo_write(addr, inbytes, inbuf);
	return;
}	

/*=============================================================================================
函数：rc_read_bytes
功能：
===============================================================================================*/
void rc_read_bytes(uint8_t addr,uint8_t *outbuf,uint8_t outbytes)
{
/*uint8_t i;	
//
for(i=0;i<outbytes;i++){
	outbuf[i] = rc_read_byte(addr);
	}
//*/
spidev_fifo_read(addr,outbytes, outbuf);
return;

}


/*=============================================================================================
函数：rc_set_time_out
功能：设置超时
入口参数:
cnt_150us: 以150us为单位的数值
===============================================================================================*/
void rc_set_time_out(uint16_t cnt_150us)
{
	//
	rc_write_byte(REG_RC500_TIMER_CLOCK,0x0b);	  //2048/13.56M = 151us
	rc_write_byte(REG_RC500_TIMER_CONTROL,0x06);   //2017/9/18 19:42:12数据传输完毕后自动运行  
	rc_write_byte(REG_RC500_TIMER_RELOAD,cnt_150us);
	//
	return;
}


/*=============================================================================================
函数：rc_clr_bits
功能：
===============================================================================================*/
void  rc_clr_bits(uint8_t addr,uint8_t mask)
{
	uint8_t tch;
	//
	tch = rc_read_byte(addr);
	//if((tch&mask)==0) return;//???
	rc_write_byte(addr,tch&(uint8_t)(~mask));
	//
	return;	
}	

/*=============================================================================================
函数：rc_set_bits
功能：
===============================================================================================*/
void  rc_set_bits(uint8_t addr,uint8_t mask)
{
	uint8_t tch;
	//
	tch = rc_read_byte(addr);
	//if((tch&mask)>0) return;//???
	rc_write_byte(addr,tch|mask);
	//
	return;		
}	


/*=============================================================================================
函数：
功能：
===============================================================================================*/
void rf_select(uint8_t no)
{
	if(no) bgRfNow = 1;
	else bgRfNow = 0;
	return;		
}
	
/*=============================================================================================
函数：
功能：
===============================================================================================*/
//static const char *rf1rst = "/sys/bus/platform/devices/leds-gpio/leds/rf1rst/brightness";
//static const char *rf2rst = "/sys/bus/platform/devices/leds-gpio/leds/rf2rst/brightness";
static const char *rf1rst = "/sys/class/leds/rf1rst/brightness";
static const char *rf2rst = "/sys/class/leds/rf2rst/brightness";
static const char *up ="1";
static const char *down ="0";
 

int set_rcio_act(const char *rcio,const char *act)  
{  
    FILE *fp;  
    if ((fp = fopen(rcio, "rb+")) == NULL)   
    {  
        printf("Cannot open %s.\n",rcio);  
        return -1;  
    }  
    fwrite(act, sizeof(char), strlen(act), fp); 
    printf("reset pin : %s.\n",act);        
    fclose(fp);  
    return 1;  
}  

void rc0_rst_set()
{
	set_rcio_act(rf1rst,up);
}

void rc0_rst_clr()
{
	set_rcio_act(rf1rst,down);
}

void rc1_rst_set()
{
	set_rcio_act(rf2rst,up);
}

void rc1_rst_clr()
{
	set_rcio_act(rf2rst,down);
}

void rc_rst_set(void)
{
	if(bgRfNow){ rc1_rst_set();}
	else{ rc0_rst_set();}		
	rc_delay();	
	return;
}

void rc_rst_clr(void)
{
	if(bgRfNow){ rc1_rst_clr();}
	else{ rc0_rst_clr();}
	rc_delay();	
	return;	
}	


/*=============================================================================================
函数：
功能：
===============================================================================================*/
uint8_t rc_irq(void)
{
/*	int ret;
	//	
	if(bgRfNow){ ret = rc1_irq();}
	else{ ret = rc0_irq();	}
	//
	if(ret) return 1;
*/	return 0;		
	
}	 

/*=============================================================================================
函数：
功能：
===============================================================================================
void rc_cs_set(void)
{
	if(bgRfNow){ rc1_cs_set();}
	else{ rc0_cs_set();}
	rc_delay();	
	return;			
}	*/

/*=============================================================================================
函数：
功能：
===============================================================================================
void rc_cs_clr(void)
{
	if(bgRfNow){ rc1_cs_clr();}
	else{ rc0_cs_clr();}
	rc_delay();	
	return;			
}	*/



//end of file
#endif