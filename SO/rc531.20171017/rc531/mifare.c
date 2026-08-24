#ifndef _MIFARE_C_
#define _MIFARE_C_
//start of file

#include "global.h"




//值操作缓冲区
//uint8_t  bgEm681ValueOperateMode;
//uint8_t  bgEm681ValueBlock;
//UDWORD dwgEm681Value;

//#define _mifare_debug_

/*=======================================================================
函数名：mcml_request:
功  能：
=========================================================================*/
uint8_t mcml_request(uint8_t req_code,uint8_t *atq)
{
uint8_t i;	
for(i=0;i<3;i++){	
   if(rc_request(req_code,atq) == 0) return 0;	
    }
return (uint8_t)-1;
}


/*=========================================================================
函数名：mcml_anticoll
功  能：防碰撞检测
入口参数: SNR:5字节卡序列号指针，最后一字节为校验字节
出口参数:
  0：正确
 其余：错误
==========================================================================*/
uint8_t mcml_anticoll(uint8_t *snr)
{
uint8_t ret;
ret = rc_anticoll(PICC_ANTICOLL1,0,snr);       
return ret;
}

uint8_t mcml_anticoll2(uint8_t *snr)
{
uint8_t ret;
ret = rc_anticoll(PICC_ANTICOLL2,0,snr);       
return ret;
}


/*==========================================================================
函数名：mcml_select
功  能：选择指定序列号的卡
入口参数:
	SNR: 卡序列号字符串指针
	cardsize:返回的卡容量标识指针
出口参数:
 0:正确
-1:错误
=============================================================================*/
uint8_t mcml_select(uint8_t *snr,uint8_t *status)
{
uint8_t ret,buf[2];	
//
ret = rc_select(PICC_ANTICOLL1,snr,buf);
status[0] = buf[0];
//if(ret==0){
	gThisCardSnr[0] = snr[0];
	gThisCardSnr[1] = snr[1];
	gThisCardSnr[2] = snr[2];
	gThisCardSnr[3] = snr[3];	
	gThisCardSnr[4] = snr[4];	
//	}
//
return ret;
}

uint8_t mcml_select2(uint8_t *snr,uint8_t *status)
{
uint8_t ret,buf[2];	
//
ret = rc_select(PICC_ANTICOLL2,snr,buf);
status[0] = buf[0];
//
return ret;
}


/*===========================================================================
函数名：mcml_load_key
功  能：
=============================================================================*/
uint8_t mcml_load_key(uint8_t keyset,uint8_t keyab, uint8_t sectno,uint8_t *buf)
{
keyab = keyab;
keyset=keyset;
sectno=sectno;
gRc500Key[0] = buf[0];
gRc500Key[1] = buf[1];
gRc500Key[2] = buf[2];
gRc500Key[3] = buf[3];
gRc500Key[4] = buf[4];
gRc500Key[5] = buf[5];
return 0;
}


/*==========================================================================
函数:mcml_authentication:
功能:认证
============================================================================*/
uint8_t mcml_authentication(uint8_t keyset,uint8_t keyab,uint8_t sectno)
{
uint8_t ret;
	
keyset = keyset;
if(keyab == KEYA) keyab = PICC_AUTHENT1A;
else keyab = PICC_AUTHENT1B;

ret = rc_auth(keyab,sectno);
//
return (uint8_t)ret;
}

uint8_t mcml_authentication2(uint8_t keyset,uint8_t keyab,uint8_t sectno)
{
uint8_t ret;
	
keyset = keyset;
if(keyab == KEYA) keyab = PICC_AUTHENT1A;
else keyab = PICC_AUTHENT1B;

ret = rc_auth2(keyab,sectno);
//
return (uint8_t)ret;
}


/*==========================================================================
函数:mcml_read
功能:读一个块
============================================================================*/
uint8_t mcml_read(uint8_t block,uint8_t *outbuf)
{
	
if(rc_read(block,outbuf) == 0) return 0;
return (uint8_t)-1;
}

uint8_t mcml_read_4bytes(uint8_t block,uint8_t *outbuf)
{
#ifdef _mifare_debug_
printk("\nmcml_read_4bytes,block[%d]....",block);
#endif	
if(rc_read_4bytes(block,outbuf) == 0) return 0;
return (uint8_t)-1;
}


/*==========================================================================
函数：mcml_write
功能：写一个块
============================================================================*/
uint8_t mcml_write(uint8_t block,uint8_t *inbuf)
{
if(rc_write(block,inbuf) == 0) return 0;
return (uint8_t)-1;
}

uint8_t mcml_write_4bytes(uint8_t block,uint8_t *inbuf)
{
#ifdef _mifare_debug_
printk("\nmcml_write_4bytes,block[%d]....",block);
#endif	
if(rc_write_4bytes(block,inbuf) == 0) return 0;
return (uint8_t)-1;
}

/*===========================================================================
函数：mcml_increment
功能：增加一个值
=============================================================================*/
uint8_t mcml_increment(uint8_t addr, uint32_t value)
{
uint8_t buf[4];

//bgEm681ValueOperateMode = PICC_INCREMENT;
//dwgEm681Value = value;
//bgEm681ValueBlock = addr;
//return 0;	

udword_to_buf_reverse(value,buf);
return rc_value_op0(PICC_INCREMENT,addr,buf);
}

/*============================================================================
函数：mcml_decrement
功能：减少一个值
==============================================================================*/
uint8_t mcml_decrement(uint8_t addr,uint32_t value)
{
uint8_t buf[4];
	
//bgEm681ValueOperateMode = PICC_DECREMENT;
//dwgEm681Value = value;
//bgEm681ValueBlock = addr;
//return 0;	
udword_to_buf_reverse(value,buf);
return rc_value_op0(PICC_DECREMENT,addr,buf);
}

/*====================================================================================================
函数：mcml_transfer
功能：实际写操作发生
=====================================================================================================*/
uint8_t mcml_transfer(uint8_t addr)
{
//uint8_t value[4];
//
//switch(bgEm681ValueOperateMode){
//	case PICC_RESTORE:
//	case PICC_DECREMENT:
//	case PICC_INCREMENT:
//		   break;
//	default:
//		   return 1;	   		
//	}
//value[0] = *((char *)&dwgEm681Value+LONG_HIGH0);
//value[1] = *((char *)&dwgEm681Value+LONG_HIGH1);
//value[2] = *((char *)&dwgEm681Value+LONG_HIGH2);
//value[3] = *((char *)&dwgEm681Value+LONG_HIGH3);
////	
//return rc_value_op(bgEm681ValueOperateMode,bgEm681ValueBlock,value,addr);
return rc_transfer(addr);
}

/*==================================================================================================
函数：mcml_restore
功能：
====================================================================================================*/
uint8_t mcml_restore(uint8_t addr)
{
//bgEm681ValueOperateMode = PICC_RESTORE;
//bgEm681ValueBlock = addr;
//return 0;	
uint8_t value[4];

memset(value,0,4);
return rc_value_op0(PICC_RESTORE,addr,value);
}


/*==================================================================================================
函数：mcml_pwr_off
功能：
====================================================================================================*/
void mcml_pwr_off(void)
{
rc_power_off();
return;
}

/*==================================================================================================
函数：mcml_halt
功能：
====================================================================================================*/
uint8_t mcml_halt(void)
{
return rc_halta();
}

//end of file
#endif
