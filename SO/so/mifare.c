//mifare.c

#ifndef _MIFARE_C_
#define _MIFARE_C_
//start of file

//值操作缓冲区
//UBYTE  bgEm681ValueOperateMode;
//UBYTE  bgEm681ValueBlock;
//UDWORD dwgEm681Value;

//#define _mifare_debug_

/*=======================================================================
函数名：mcml_request:
功  能：
=========================================================================*/
UBYTE mcml_request(UBYTE req_code,UBYTE *atq)
{
UBYTE i, j;	
	
	//for(i=0;i<3;i++){	
	if((j = rc_request(req_code,atq)) == 0) return 0;	
	//    }
	//return (UBYTE)-1;
	return j;
}


/*=========================================================================
函数名：mcml_anticoll
功  能：防碰撞检测
入口参数: SNR:5字节卡序列号指针，最后一字节为校验字节
出口参数:
  0：正确
 其余：错误
==========================================================================*/
UBYTE mcml_anticoll(UBYTE *snr)
{
UBYTE ret;

#ifdef DEBUG_CLOCK

clock_t start;
	//
	start = clock();
	printf("enter ANTICOLL clock %d\n" ,start);
#endif

	ret = rc_anticoll(PICC_ANTICOLL1,0,snr);       
	return ret;
}

UBYTE mcml_anticoll2(UBYTE *snr)
{
UBYTE ret;

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
UBYTE mcml_select(UBYTE *snr,UBYTE *status)
{
UBYTE ret,buf[2];	

#ifdef	DEBUG_CLOCK
clock_t start;
	//
	start = clock();
	printf("enter select clock %d\n" ,start);
#endif
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
#ifdef	DEBUG_CLOCK
	start = clock();
	printf("select %d end clock_t %d\n", ret, start);
#endif
	return ret;
}

UBYTE mcml_select2(UBYTE *snr,UBYTE *status)
{
UBYTE ret,buf[2];	
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
UBYTE mcml_load_key(UBYTE keyset,UBYTE keyab, UBYTE sectno,UBYTE *buf)
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
UBYTE mcml_authentication(UBYTE keyset,UBYTE keyab,UBYTE sectno)
{
UBYTE ret;
	
	keyset = keyset;
	if(keyab == KEYA) keyab = PICC_AUTHENT1A;
	else keyab = PICC_AUTHENT1B;
	
	ret = rc_auth(keyab,sectno);
	//
	return (UBYTE)ret;
}

UBYTE mcml_authentication2(UBYTE keyset,UBYTE keyab,UBYTE sectno)
{
UBYTE ret;
	
	keyset = keyset;
	if(keyab == KEYA) keyab = PICC_AUTHENT1A;
	else keyab = PICC_AUTHENT1B;
	
	ret = rc_auth2(keyab,sectno);
	//
	return (UBYTE)ret;
}


/*==========================================================================
函数:mcml_read
功能:读一个块
============================================================================*/
UBYTE mcml_read(UBYTE block,UBYTE *outbuf)
{
	
	if(rc_read(block,outbuf) == 0) return 0;
	return (UBYTE)-1;
}

UBYTE mcml_read_4bytes(UBYTE block,UBYTE *outbuf)
{
//#ifdef _mifare_debug_
printk("\nmcml_read_4bytes,block[%d]....",block);
//#endif	
	if(rc_read_4bytes(block,outbuf) == 0) return 0;
	return (UBYTE)-1;
}


/*==========================================================================
函数：mcml_write
功能：写一个块
============================================================================*/
UBYTE mcml_write(UBYTE block,UBYTE *inbuf)
{
	if(rc_write(block,inbuf) == 0) return 0;
	return (UBYTE)-1;
}

UBYTE mcml_write_4bytes(UBYTE block,UBYTE *inbuf)
{
#ifdef _mifare_debug_
printk("\nmcml_write_4bytes,block[%d]....",block);
#endif	
	if(rc_write_4bytes(block,inbuf) == 0) return 0;
	return (UBYTE)-1;
}

/*===========================================================================
函数：mcml_increment
功能：增加一个值
=============================================================================*/
UBYTE mcml_increment(UBYTE addr, UDWORD value)
{
UBYTE buf[4];

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
UBYTE mcml_decrement(UBYTE addr,UDWORD value)
{
UBYTE buf[4];
	
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
UBYTE mcml_transfer(UBYTE addr)
{
//UBYTE value[4];
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
UBYTE mcml_restore(UBYTE addr)
{
//bgEm681ValueOperateMode = PICC_RESTORE;
//bgEm681ValueBlock = addr;
//return 0;	
UBYTE value[4];

	memset(value,0,4);
	return rc_value_op0(PICC_RESTORE,addr,value);
}


/*==================================================================================================
函数：mcml_pwr_on	2015/1/13 22:01:51
功能：
====================================================================================================*/
void mcml_pwr_on(void)
{
	rc_power_on();
	return;
}

/*==================================================================================================
函数：mcml_pwr_off
功能：
====================================================================================================*/
void mcml_pwr_off(void)
{
#ifdef	DEBUG_CLOCK
	clock_t start;
	
	start = clock();
	printf("mcml_pwr_off :%d\n\n", start);
#endif
	rc_power_off();
	return;
}

/*==================================================================================================
函数：mcml_halt
功能：
====================================================================================================*/
UBYTE mcml_halt(void)
{
#ifdef	DEBUG_CLOCK
clock_t start;

start = clock();
	printf("mcml_halt:%d\n\n", start);
#endif
	return rc_halta();
}

//end of file
#endif

