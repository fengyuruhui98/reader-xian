//sam_int.c

#ifndef _SAM_INT_C_
#define _SAM_INT_C_
//start of file

int bpgApduExpectLen = 0;

UBYTE bgSamResetFlag=0;

//#define _TEST_SAM_
//#define	_DEBUG_  
#define _NEW_SAM_BOARD_EXTSAMV12_
//
//#define _DEBUG_sam_atr0_
typedef struct
{
	UBYTE  etu;
	UBYTE  wait_etu;
	UDWORD timeout;
} SAM_CONFIG;

SAM_CONFIG  gSamConfig[MAX_SAM_INDEX];

#define SAM_CMD_VER       0
#define SAM_CMD_ATR       1
#define SAM_CMD_APDU      2
#define SAM_CMD_PPS		  3
#define SAM_CMD_APDU_EXT  4

/*===================================================================================================
函数：sam_set
功能：
=====================================================================================================*/
void sam_set(UBYTE index,UBYTE etu,UBYTE wait_etu)
{
	index = index%MAX_SAM_INDEX;
	gSamConfig[index].etu = etu;
	gSamConfig[index].wait_etu = wait_etu;
#ifdef	_DEBUG_	
	printf("sam index %d\n", index);
	printf("sam etu %02x\n", etu);
	printf("sam wait etu %d\n", wait_etu);
#endif
	//20140414
	//if(bgSamResetFlag)	return;
	//
	//  mcu_powerctrl_clr();usleep(200*1000);
	//  mcu_powerctrl_set();usleep(200*1000);
	//  mcu1_reset();usleep(500*1000);
	//  //
	//  bgSamResetFlag = 1;
	
	return;	
}	


/*===================================================================================================
函数：
功能：
=====================================================================================================*/
//void mcu1_select(void)
//{
//char sbuf[1000],rbuf[1000];
//int sbytes,rbytes;
////
//sbuf[0] = 0xff;
//sbuf[1] = 0xff;
//sbuf[2] = 0xff;
//sbuf[3] = SAM_PIN_COMSEL;
//sbuf[4] = 1;
//sbytes = 5;
////
//lib_cmd(sbuf,sbytes,rbuf,&rbytes);
////
//return;			
//}	
//
///*===================================================================================================
//函数：
//功能：
//=====================================================================================================*/
//void mcu2_select(void)
//{
//char sbuf[1000],rbuf[1000];
//int sbytes,rbytes;
////
//sbuf[0] = 0xff;
//sbuf[1] = 0xff;
//sbuf[2] = 0xff;
//sbuf[3] = SAM_PIN_COMSEL;
//sbuf[4] = 0;
//sbytes = 5;
////
//lib_cmd(sbuf,sbytes,rbuf,&rbytes);
////
//return;			
//}

/*===================================================================================================
函数：
功能：
=====================================================================================================*/
//void mcu1_reset(void)
//{
//char sbuf[1000],rbuf[1000];
//int sbytes,rbytes;
////
//sbuf[0] = 0xff;
//sbuf[1] = 0xff;
//sbuf[2] = 0xff;
//sbuf[3] = SAM_PIN_RST0;
//sbuf[4] = 0;
//sbytes = 5;
////
//lib_cmd(sbuf,sbytes,rbuf,&rbytes);
////
//usleep(100*1000);//100ms
////
//sbuf[0] = 0xff;
//sbuf[1] = 0xff;
//sbuf[2] = 0xff;
//sbuf[3] = SAM_PIN_RST0;
//sbuf[4] = 1;
//sbytes = 5;
////
//lib_cmd(sbuf,sbytes,rbuf,&rbytes);
//return;			
//}	

/*===================================================================================================
函数：
功能：
=====================================================================================================*/
//void mcu2_reset(void)
//{
//char sbuf[1000],rbuf[1000];
//int sbytes,rbytes;
////
//sbuf[0] = 0xff;
//sbuf[1] = 0xff;
//sbuf[2] = 0xff;
//sbuf[3] = SAM_PIN_RST1;
//sbuf[4] = 0;
//sbytes = 5;
////
//lib_cmd(sbuf,sbytes,rbuf,&rbytes);
////
//usleep(100*1000);//100ms
////
//sbuf[0] = 0xff;
//sbuf[1] = 0xff;
//sbuf[2] = 0xff;
//sbuf[3] = SAM_PIN_RST1;
//sbuf[4] = 1;
//sbytes = 5;
////
//lib_cmd(sbuf,sbytes,rbuf,&rbytes);
////
//return;			
//}

/*===================================================================================================
函数：
功能：
=====================================================================================================*/
//void mcu_powerctrl_clr(void)
//{
//char sbuf[1000],rbuf[1000];
//int sbytes,rbytes;
////
//sbuf[0] = 0xff;
//sbuf[1] = 0xff;
//sbuf[2] = 0xff;
//sbuf[3] = SAM_PIN_PWRCTL;
//sbuf[4] = 0;
//sbytes = 5;
////
//lib_cmd(sbuf,sbytes,rbuf,&rbytes);
////
//return;			
//}

/*===================================================================================================
函数：
功能：
=====================================================================================================*/
//void mcu_powerctrl_set(void)
//{
//char sbuf[1000],rbuf[1000];
//int sbytes,rbytes;
////
//sbuf[0] = 0xff;
//sbuf[1] = 0xff;
//sbuf[2] = 0xff;
//sbuf[3] = SAM_PIN_PWRCTL;
//sbuf[4] = 1;
//sbytes = 5;
////
//lib_cmd(sbuf,sbytes,rbuf,&rbytes);
////
//return;			
//}

/*===================================================================================================
函数：sam_atr
功能：
=====================================================================================================*/
#ifdef _DEBUG_
#define _DEBUG_sam_atr0_
#endif

int sam_atr0(UBYTE index,UBYTE *obuf,UBYTE *obytes)
{
UBYTE inbuf[256];
UBYTE inbytes;
UBYTE outbuf[256];
UBYTE outbytes;
#ifdef _DEBUG_sam_atr0_
UBYTE i;
#endif

	if(index >= MAX_SAM_INDEX) return -3;

#ifdef _DEBUG_sam_atr0_
	printf("\x0d\x0a sam_atr0,index=%d \x0d\x0a",index);
#endif

	 
//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
	if((index/4) != 0){
	   mcu2_select();
	   }
	else{
	   mcu1_select();
	   }	
#endif

	inbuf[0] = SAM_CMD_ATR;
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
	inbuf[1] = index%4;
#else
	inbuf[1] = index;
#endif

	inbuf[2] = gSamConfig[index].etu;
	inbytes = 3;   
	sam_prot_rece_reset();
	sam_prot_start_send(inbuf,inbytes);
	//超时等待1000ms
	timer_set(TIMER_CMD_PROCESS_INDEX,1000);
	//timer_set(TIMER_CMD_PROCESS_INDEX,3000); //20230617
	while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
		sam_prot_send_process();
		sam_prot_rece_process();
		if(!sam_prot_rece_ready()){
		//mdelay(1);
		continue;	
	}
	sam_prot_rece_get(outbuf,&outbytes);
	sam_prot_rece_reset();
	
	if(outbytes < 4) return -1;
	*obytes = outbytes-3;
	memcpy(obuf,&outbuf[3],*obytes);
	#ifdef _DEBUG_sam_atr0_
		printf("\x0d\x0a sam sam_atr0:");
		for(i=0;i<*obytes;i++) {
			if((i%16) == 0) printf("\x0d\x0a ");
			printf(" %02x",obuf[i]);
		}
		printf("\x0d\x0a ");
	#endif	
	
		return 0;
	}	
	
#ifdef _DEBUG_sam_atr0_
	printf("over time return \n");
#endif	
	
	return -2;	
}


/*===================================================================================================
函数：sam_apdu
功能：
=====================================================================================================*/
#ifdef _DEBUG_
#define _SAM_APDU0_
#endif
int  sam_apdu0(UBYTE index,UBYTE *ibuf,UBYTE ibytes,UBYTE *obuf,UBYTE *obytes)
{
UBYTE inbuf[256];
UBYTE inbytes;
UBYTE outbuf[256];
UBYTE outbytes;
#ifdef _SAM_APDU0_
UBYTE i;
#endif

#ifdef _SAM_APDU0_
	printf("\x0d\x0a sam_apdu0,index=%d",index);
	printf("\x0d\x0a sam apdu0 command:");
	for(i=0;i<ibytes;i++) {
		if((i%16) == 0) printf("\x0d\x0a ");
		printf(" %02x",ibuf[i]);
	}
	printf("\x0d\x0a ");
#endif
	
	if(index >= MAX_SAM_INDEX) return -3;
	if(ibytes < 5) return -3;	
	if(ibytes > 251) return -3;	
		 
	gSamConfig[index].timeout = 1000;
	 
//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
	if((index/4) != 0){
		mcu2_select();
	}
	else{
		mcu1_select();
	}	
#endif

	inbuf[0] = SAM_CMD_APDU;
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
	inbuf[1] = index%4;
#else
	inbuf[1] = index;
#endif
	inbuf[2] = gSamConfig[index].etu;
	inbuf[3] = gSamConfig[index].wait_etu;
	memcpy(&inbuf[4],ibuf,ibytes);
	inbytes = 4+ibytes;   
	sam_prot_rece_reset();
	sam_prot_start_send(inbuf,inbytes);
	//超时等待
	timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
	while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
		sam_prot_send_process();
		sam_prot_rece_process();
		if(!sam_prot_rece_ready()){
			//if(bgEnUCOS) OSTimeDly(1);
			continue;	
		}
		sam_prot_rece_get(outbuf,&outbytes);
		sam_prot_rece_reset();
		if(outbytes < 5) return -1;
		*obytes = outbytes-4;
		memcpy(obuf,&outbuf[4],*obytes);
  
	#ifdef _SAM_APDU0_
		printf("\x0d\x0a sam apdu0 response:");
		for(i=0;i<*obytes;i++) {
			if((i%16) == 0) printf("\x0d\x0a ");
			printf(" %02x",obuf[i]);
		}
		printf("\x0d\x0a ");
	#endif	
  
		return 0;
	}	

	return -2;	
}
/*===================================================================================================
函数：sam_apdu1
功能：2014/1/14 11:00:44
=====================================================================================================*/
#ifdef _DEBUG_
#define _SAM_APDU1_		1
#endif
//#define _SAM_APDU1_		1//20230617 释放#define _SAM_APDU1_		1
int  sam_apdu1(UBYTE index,UBYTE expectlen, UBYTE *ibuf,UBYTE ibytes,UBYTE *obuf,UBYTE *obytes)
{	
UBYTE inbuf[256];
UBYTE inbytes;
UBYTE outbuf[256] ,buff[100];
UWORD outbytes;

fd_set readfd;
struct timeval timeout;
struct timeval tv1,tv2;
struct timezone tz1,tz2;
long ret, lngreadlen, fStat, j;
unsigned long tv1_usec;
unsigned char blnTimeout, blnlrc, lrc;
UBYTE i;

//printf("\n sam apdu1……");
#ifdef _SAM_APDU1_
	printf("\x0d\x0a sam_apdu1,index=%d",index);
	printf("\x0d\x0a sam apdu1 command:");
	for(i=0;i<ibytes;i++) 
	{
		if((i%16) == 0) printf("\x0d\x0a ");
		printf(" %02x",ibuf[i]);
	}
//		    gettimeofday(&tv1,&tz1);
//		    tv1_usec = tv1.tv_usec;
//		    printf("lib B %d  %d\n", tv1.tv_sec, tv1.tv_usec/1000);
#endif

	
	if(index >= MAX_SAM_INDEX) return -3;
	if(ibytes < 5) return -3;	
	if(ibytes > 251) return -3;	
	 
	gSamConfig[index].timeout = 1000;
	 
	//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
	if((index/4) != 0)
	{
   		mcu2_select();
	}
	else
	{
   		mcu1_select();
   	}	
#endif

	inbuf[0] = SAM_CMD_APDU;
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
	inbuf[1] = index%4;
#else
	inbuf[1] = index;
#endif
	inbuf[2] = gSamConfig[index].etu;
	inbuf[3] = gSamConfig[index].wait_etu;
	if(expectlen > 0)
	{
		inbuf[0] = SAM_CMD_APDU_EXT;
		inbuf[4] = expectlen;
		memcpy(&inbuf[5], ibuf, ibytes);
		inbytes = 5 + ibytes;
	}else
	{
		memcpy(&inbuf[4],ibuf,ibytes);
		inbytes = 4+ibytes; 
	}
	//
	sam_prot_gen_block(inbuf, inbytes, outbuf, &outbytes);
	tcflush(gUartHandle[SAM_UART_INDEX], TCIOFLUSH);
	ret = write(gUartHandle[SAM_UART_INDEX], outbuf, outbytes);
#ifdef _SAM_APDU1_
	printf("\x0d\x0a sam_apdu1 write return %d fd %d \n", ret, gUartHandle[SAM_UART_INDEX]);
	printf("\x0d\x0a sam apdu1 command:");
	for(i=0;i<outbytes;i++) 
	{
		printf(" %02x",outbuf[i]);
	}
	printf("\x0d\x0a");
#endif
	//receive ACK
	lngreadlen = 0;
	blnlrc = 0;
	do
	{
		FD_ZERO(&readfd);
		FD_SET(gUartHandle[SAM_UART_INDEX], &readfd);
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000000;
		ret = select(gUartHandle[SAM_UART_INDEX] + 1, &readfd, NULL, NULL, &timeout);
		if(ret > 0)
		{
			fStat = read(gUartHandle[SAM_UART_INDEX], buff, 1);
			if(fStat > 0)
			{
				#ifdef _SAM_APDU1_
					printf("\x0d\x0a ack= %02x\n", buff[0]);
				#endif
				if(buff[0] != ACK)
					return -3;
				break;
			}
		}else
		{
			#ifdef _SAM_APDU1_
				printf("\x0d\x0a overtime \n");
			#endif
			return -2;
		}
	}while(1);
	//read response
	do
	{
		FD_ZERO(&readfd);
		FD_SET(gUartHandle[SAM_UART_INDEX], &readfd);
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000000;
		
		ret = select(gUartHandle[SAM_UART_INDEX] + 1, &readfd, NULL, NULL, &timeout);
		if(ret > 0)
		{
			gettimeofday(&tv1,&tz1);
			blnTimeout = 0xff;
			do
			{
				fStat = read(gUartHandle[SAM_UART_INDEX], buff, 1);
				if(fStat > 0)
				{
					//outbuf[lngreadlen] = buff[0];
					memcpy(&outbuf[lngreadlen], buff, fStat);
					//2018/4/26 8:03
					if(outbuf[0] != 0x02)
						continue;
					lngreadlen += fStat;
					#ifdef _SAM_APDU1_
						for(i = 0; i < fStat; i++)
							printf("%02x ", buff[i]);
						printf("\n");
					#endif
					if(lngreadlen > 3)
					{
						if(outbuf[lngreadlen - 1] == 0x03)
						{
							j = 0;
							for(i = lngreadlen - 2; i > 0; i--)
							{
								if(outbuf[i] != 0x10)
									break;
								else
									j++;
							}
							if(j % 2 == 0)
							{
								ret = samDeleteDLE(outbuf, &lrc, lngreadlen);
							#ifdef _SAM_APDU1_
								printf("\x0d\x0a delete dle %d lrc %02x\n", ret, lrc);
							#endif
								if(ret > 0)
								{
									blnlrc = 0xff;
									break;
								}
							}
						}
					}
				}
//				gettimeofday(&tv2,&tz2);
//				if ((((tv2.tv_sec-tv1.tv_sec)*1000000)+tv2.tv_usec-tv1.tv_usec)>=1000000)
//				{
//					blnTimeout = 0;
//					printf("\n");
//					return -2;
//				}
				FD_ZERO(&readfd);
				FD_SET(gUartHandle[SAM_UART_INDEX], &readfd);
				timeout.tv_sec = 0;
				timeout.tv_usec = 50000;
				ret = select(gUartHandle[SAM_UART_INDEX] + 1, &readfd, NULL, NULL, &timeout);
				if(ret > 0)
				{
					continue;
				}else if(ret == 0)
				{
					blnTimeout = 0;
					lngreadlen = 0;
				}else
				{
					blnTimeout = 0;
					lngreadlen = 0;
				}
			}while(blnTimeout);
		}else
		{
			return -2;
		}
		//read last byte
		if(blnlrc)
			break;
	}while(1);

	blnlrc = 0xff;
	do
	{
		FD_ZERO(&readfd);
		FD_SET(gUartHandle[SAM_UART_INDEX], &readfd);
		timeout.tv_sec = 0;
		timeout.tv_usec = 50000;
		ret = select(gUartHandle[SAM_UART_INDEX] + 1, &readfd, NULL, NULL, &timeout);
		if(ret > 0)
		{
			fStat = read(gUartHandle[SAM_UART_INDEX], buff, 1);
			if(fStat > 0)
			{
				#ifdef _SAM_APDU1_
					printf("lrc %02x\n", buff[0]);
				#endif
				if((buff[0]==0x10)  && blnlrc)
				{
					blnlrc = 0;
					continue;
				}
					
				if(buff[0] != lrc)
					return -3;
				break;
			}
		}else
		{
			return -2;
		}
	}while(1);
	//
	outbuf[0] = 0x06;
	write(gUartHandle[SAM_UART_INDEX], outbuf, 1);
	if(outbuf[1] < 5) return -4;
	*obytes = outbuf[1] - 4;
	memcpy(obuf,&outbuf[6], *obytes);


//		    gettimeofday(&tv2,&tz2);
//		    j = (tv2.tv_usec - tv1_usec)/1000;
//		    printf("lib A %ds %duc %ds  %dms tv1 %d\n", tv2.tv_sec, tv2.tv_usec,(tv2.tv_sec-tv1.tv_sec), j, tv1_usec);
	return 0;
}

/*===================================================================================================
函数：samDeleteDLE
功能：2014/1/14 11:00:34
=====================================================================================================*/
long samDeleteDLE(unsigned char *pbytData, unsigned char *lrc, int intLength)
{
unsigned char 	bytTempData[600];
char		inlen;
int 		intDeleteLen, i;
unsigned char 	chCheck;

	//check the message head
	if(pbytData[0] != 0x02) return 0;
	//delete the DLE
	memcpy(bytTempData, pbytData, intLength);
	intDeleteLen = intLength;
	for(i = 1; i < intLength; i++)
	{
		if(bytTempData[i] == 0x10)
		{
			memcpy(&bytTempData[i], &bytTempData[i + 1], intLength - 1 - i);
			intDeleteLen -= 1;
		}
	}
	//check the message length:
	inlen = intDeleteLen - 3;
	if((unsigned char)inlen != bytTempData[1])
	{//confirm the command is correct for response
		pbytData[6] = bytTempData[6];
		return -211;
	}
	//check the message CRC
	chCheck = 0;
	for(i = 0; i < intDeleteLen; i++)
	{
		chCheck ^= bytTempData[i];
	}
	*lrc = chCheck;
	memcpy(pbytData, bytTempData, intDeleteLen);
	
	return intDeleteLen;

}






/*===================================================================================================
函数：sam_apdu0_ext
功能：
=====================================================================================================*/
#ifdef _DEBUG_
//#define _SAM_APDU0_EXT_
#endif
int  sam_apdu0_ext(UBYTE index,UBYTE expectlen,UBYTE *ibuf,UBYTE ibytes,UBYTE *obuf,UBYTE *obytes)
{
UBYTE inbuf[256];
UBYTE inbytes;
UBYTE outbuf[256];
UBYTE outbytes;
#ifdef _SAM_APDU0_EXT_
UBYTE i;
#endif

#ifdef _SAM_APDU0_EXT_
printf("\nsam_apdu0_ext,index=%d,expectlen=%d",index,expectlen);
printf("\nsam_apdu0_ext command:");
for(i=0;i<ibytes;i++) {
	if((i%16) == 0) printf("\x0d\x0a ");
	printf(" %02x",ibuf[i]);
}
printf("\x0d\x0a ");
#endif
	
	if(index >= MAX_SAM_INDEX) return -3;
	if(ibytes < 5) return -3;	
	if(ibytes > 251) return -3;	
		 
	gSamConfig[index].timeout = 1000;
	 
//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
if((index/4) != 0){
   mcu2_select();
   }
else{
   mcu1_select();
   }	
#endif

	inbuf[0] = SAM_CMD_APDU_EXT;
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
inbuf[1] = index%4;
#else
inbuf[1] = index;
#endif
	inbuf[2] = gSamConfig[index].etu;
	inbuf[3] = gSamConfig[index].wait_etu;
	inbuf[4] = expectlen;
	memcpy(&inbuf[5],ibuf,ibytes);
	inbytes = 5+ibytes;
	sam_prot_rece_reset();
	sam_prot_start_send(inbuf,inbytes);
	//超时等待
	timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
	while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
		sam_prot_send_process();
		sam_prot_rece_process();
		if(!sam_prot_rece_ready()){
			//if(bgEnUCOS) OSTimeDly(1);
			continue;	
		}
		sam_prot_rece_get(outbuf,&outbytes);
		sam_prot_rece_reset();
		if(outbytes < 5) return -1;
		*obytes = outbytes-4;
		memcpy(obuf,&outbuf[4],*obytes);
  
  #ifdef _SAM_APDU0_EXT_
	printf("\x0d\x0a sam_apdu0_ext response:");
	for(i=0;i<*obytes;i++) {
		if((i%16) == 0) printf("\x0d\x0a ");
		printf(" %02x",obuf[i]);
	}
	printf("\x0d\x0a ");
	#endif	
  
		return 0;
	}	

	return -2;	
}

void set_apdu_expect_len(int len)
{
	bpgApduExpectLen = len;
}

void clr_apdu_expect_len(void)
{
	bpgApduExpectLen = 0;
}


/*===================================================================================================
函数：
功能：
=====================================================================================================*/
#ifdef _DEBUG_
#define _DEBUG_sam_pps0_
#endif
int  sam_pps0(UBYTE index,UBYTE ta1,UBYTE *obuf,UBYTE *obytes)
{
UBYTE inbuf[256];
UBYTE inbytes;
UBYTE outbuf[256];
UBYTE outbytes;

#ifdef _DEBUG_sam_pps0_
UBYTE i; 	
#endif

	if(index >= MAX_SAM_INDEX) return -3;
		 
	gSamConfig[index].timeout = 1000;
	 
//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
if((index/4) != 0){
   mcu2_select();
   }
else{
   mcu1_select();
   }	
#endif

	inbuf[0] = SAM_CMD_PPS;
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
inbuf[1] = index%4;
#else
inbuf[1] = index;
#endif
	inbuf[2] = gSamConfig[index].etu; //SAM_ETU_372;
	inbuf[3] = ta1;
	inbytes = 4;
#ifdef _DEBUG_sam_pps0_
printf("\x0d\x0a pts input:");
for(i=0;i<inbytes;i++) printf(" %02x",inbuf[i]);
printf("\x0d\x0a ");
#endif   

	sam_prot_rece_reset();
	sam_prot_start_send(inbuf,inbytes);
	//超时等待
	timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
	while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
		sam_prot_send_process();
		sam_prot_rece_process();
		if(!sam_prot_rece_ready()){
			//if(bgEnUCOS) OSTimeDly(1);
			continue;	
		}
		sam_prot_rece_get(outbuf,&outbytes);
		sam_prot_rece_reset();
		//if(outbytes < 5) return -1; 
		*obytes = outbytes;
		memcpy(obuf,outbuf,*obytes);
	  #ifdef _DEBUG_sam_pps0_
	  printf("\x0d\x0a pts resp:");
	  for(i=0;i<outbytes;i++) printf(" %02x",outbuf[i]);
	  printf("\x0d\x0a ");
	  #endif	
	  return 0;
	}	

	return -2;		
}	

/*===================================================================================================
函数：取sam board version
功能：index 0-mcu1,1-mcu2
=====================================================================================================*/
int  sam_get_ver(UBYTE index,UBYTE *obuf,UBYTE *obytes)
{
UBYTE inbuf[256];
UBYTE inbytes;
UBYTE outbuf[256];
UBYTE outbytes;
	
	if(index >= 2) return -3;
		 
	gSamConfig[index].timeout = 1000;
		 
//根据mcu index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
if((index) != 0){
   mcu2_select();
   }
else{
   mcu1_select();
   }	
#endif

	inbuf[0] = SAM_CMD_VER;
	inbytes = 1;   
	sam_prot_rece_reset();
	sam_prot_start_send(inbuf,inbytes);
	//超时等待
	timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
	while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
		sam_prot_send_process();
		sam_prot_rece_process();
		if(!sam_prot_rece_ready()){
			//if(bgEnUCOS) OSTimeDly(1);
			continue;	
		}
		sam_prot_rece_get(outbuf,&outbytes);
		sam_prot_rece_reset();
		if(outbytes < 8) return -1;
		*obytes = outbytes;
		memcpy(obuf,outbuf,*obytes);	
		return 0;
	}	

	return -2;		
}


#ifdef _TEST_SAM_
/*===================================================================================================
函数：sam_apdu_send
功能：
=====================================================================================================*/
#ifdef _DEBUG_
//#define _SAM_APDU_SEND_
#endif
int  sam_apdu_send(UBYTE index,UBYTE *ibuf,UBYTE ibytes)
{
UBYTE inbuf[256];
UBYTE inbytes;

#ifdef _SAM_APDU_SEND_
UBYTE i;
#endif

#ifdef _SAM_APDU_SEND_
	printf("\x0d\x0a sam_apdu_send,index=%d",index);
	printf("\x0d\x0a sam_apdu_send command:");
	for(i=0;i<ibytes;i++) {
		if((i%16) == 0) printf("\x0d\x0a ");
		printf(" %02x",ibuf[i]);
	}
	printf("\x0d\x0a ");
#endif

	if(index >= MAX_SAM_INDEX) return -3;
	if(ibytes < 5) return -3;	
	if(ibytes > 251) return -3;	
		 
	gSamConfig[index].timeout = 1000;
		 
//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
if((index/4) != 0){
   mcu2_select();
   }
else{
   mcu1_select();
   }
#endif	

	inbuf[0] = SAM_CMD_APDU;
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
inbuf[1] = index%4;
#else
inbuf[1] = index;
#endif
	inbuf[2] = gSamConfig[index].etu;
	inbuf[3] = gSamConfig[index].wait_etu;
	memcpy(&inbuf[4],ibuf,ibytes);
	inbytes = 4+ibytes;   
	sam_prot_rece_reset();
	sam_prot_start_send(inbuf,inbytes);
	
	while(bgSamSendState != SAM_SEND_STATE_OK){
		sam_prot_send_process();	
	}
	timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
	return 0;

/*
//超时等待
timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
	sam_prot_send_process();
  sam_prot_rece_process();
  if(!sam_prot_rece_ready()){
  	 //if(bgEnUCOS) OSTimeDly(1);
  	 continue;	
  	 }
  sam_prot_rece_get(outbuf,&outbytes);
  sam_prot_rece_reset();
  if(outbytes < 5) return -1;
  *obytes = outbytes-4;
  memcpy(obuf,&outbuf[4],*obytes);	
  return 0;
  }	

return -2;
*/	
}

/*===================================================================================================
函数：sam_apdu
功能：
=====================================================================================================*/
#ifdef _DEBUG_
//#define _SAM_APDU_RECEIVE_
#endif
int  sam_apdu_receive(UBYTE *obuf,UBYTE *obytes)
{
UBYTE outbuf[256];
UBYTE outbytes;

#ifdef _SAM_APDU_RECEIVE_
UBYTE i;
#endif

/*	
if(index >= MAX_SAM_INDEX) return -3;
if(ibytes < 5) return -3;	
if(ibytes > 251) return -3;	
	 
gSamConfig[index].timeout = 1000;
	 
//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
if((index/4) != 0){
   mcu2_select();
   }
else{
   mcu1_select();
   }	
#endif

inbuf[0] = SAM_CMD_APDU;
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
inbuf[1] = index%4;
#else
inbuf[1] = index;
#endif
inbuf[2] = gSamConfig[index].etu;
inbuf[3] = gSamConfig[index].wait_etu;
memcpy(&inbuf[4],ibuf,ibytes);
inbytes = 4+ibytes;   
sam_prot_rece_reset();
sam_prot_start_send(inbuf,inbytes);
//超时等待
timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
*/

	while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
		//sam_prot_send_process();
		sam_prot_rece_process();
		if(!sam_prot_rece_ready()){
			//if(bgEnUCOS) OSTimeDly(1);
			continue;	
		}
		sam_prot_rece_get(outbuf,&outbytes);
		sam_prot_rece_reset();
		if(outbytes < 5) return -1;
		
		*obytes = outbytes-4;
		memcpy(obuf,&outbuf[4],*obytes);
	  
		#ifdef _SAM_APDU_RECEIVE_
			printf("\x0d\x0a sam_apdu_receive response:");
			for(i=0;i<*obytes;i++) {
				if((i%16) == 0) printf("\x0d\x0a ");
				printf(" %02x",obuf[i]);
			}
			printf("\x0d\x0a ");
		#endif  
	  	
		return 0;
	}	
	
	return -2;	
}

/*===================================================================================================
函数：sam_apdu_send
功能：
=====================================================================================================*/
#ifdef _DEBUG_
//#define _SAM_APDU_SEND_EXT_
#endif
int  sam_apdu_send_ext(UBYTE index,UBYTE *ibuf,UBYTE ibytes,UBYTE expectlen)
{
UBYTE inbuf[256];
UBYTE inbytes;

#ifdef _SAM_APDU_SEND_EXT_
UBYTE i;
#endif

#ifdef _SAM_APDU_SEND_EXT_
printf("\x0d\x0a sam_apdu_send_ext,index=%d,expectlen=%d",index,expectlen);
printf("\x0d\x0a sam_apdu_send_ext command:");
for(i=0;i<ibytes;i++) {
	if((i%16) == 0) printf("\x0d\x0a ");
	printf(" %02x",ibuf[i]);
}
printf("\x0d\x0a ");
#endif

	
	if(index >= MAX_SAM_INDEX) return -3;
	if(ibytes < 5) return -3;	
	if(ibytes > 251) return -3;	
		 
	gSamConfig[index].timeout = 1000;
	 
//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
if((index/4) != 0){
   mcu2_select();
   }
else{
   mcu1_select();
   }
#endif	

	if(expectlen) {
		inbuf[0] = SAM_CMD_APDU_EXT;
		#ifndef _NEW_SAM_BOARD_EXTSAMV12_
			inbuf[1] = index%4;
		#else
			inbuf[1] = index;
		#endif
		inbuf[2] = gSamConfig[index].etu;
		inbuf[3] = gSamConfig[index].wait_etu;
		inbuf[4] = expectlen;
		memcpy(&inbuf[5],ibuf,ibytes);
		inbytes = 5+ibytes;
	}
	else {
		inbuf[0] = SAM_CMD_APDU;
		#ifndef _NEW_SAM_BOARD_EXTSAMV12_
			inbuf[1] = index%4;
		#else
			inbuf[1] = index;
		#endif
		inbuf[2] = gSamConfig[index].etu;
		inbuf[3] = gSamConfig[index].wait_etu;
		memcpy(&inbuf[4],ibuf,ibytes);
		inbytes = 4+ibytes;
	}	   
	sam_prot_rece_reset();
	sam_prot_start_send(inbuf,inbytes);
	
	while(bgSamSendState != SAM_SEND_STATE_OK){
		sam_prot_send_process();	
	}
	timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);

	return 0;

/*
//超时等待
timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
	sam_prot_send_process();
  sam_prot_rece_process();
  if(!sam_prot_rece_ready()){
  	 //if(bgEnUCOS) OSTimeDly(1);
  	 continue;	
  	 }
  sam_prot_rece_get(outbuf,&outbytes);
  sam_prot_rece_reset();
  if(outbytes < 5) return -1;
  *obytes = outbytes-4;
  memcpy(obuf,&outbuf[4],*obytes);	
  return 0;
  }	

return -2;
*/	
}

/*===================================================================================================
函数：sam_apdu_receive_ext
功能：
=====================================================================================================*/
#ifdef _DEBUG_
//#define _SAM_APDU_RECEIVE_EXT_
#endif
int  sam_apdu_receive_ext(UBYTE *obuf,UBYTE *obytes)
{
UBYTE outbuf[256];
UBYTE outbytes;

#ifdef _SAM_APDU_RECEIVE_EXT_
UBYTE i;
#endif

/*	
if(index >= MAX_SAM_INDEX) return -3;
if(ibytes < 5) return -3;	
if(ibytes > 251) return -3;	
	 
gSamConfig[index].timeout = 1000;
	 
//根据sam index选择CPU
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
if((index/4) != 0){
   mcu2_select();
   }
else{
   mcu1_select();
   }	
#endif

inbuf[0] = SAM_CMD_APDU;
#ifndef _NEW_SAM_BOARD_EXTSAMV12_
inbuf[1] = index%4;
#else
inbuf[1] = index;
#endif
inbuf[2] = gSamConfig[index].etu;
inbuf[3] = gSamConfig[index].wait_etu;
memcpy(&inbuf[4],ibuf,ibytes);
inbytes = 4+ibytes;   
sam_prot_rece_reset();
sam_prot_start_send(inbuf,inbytes);
//超时等待
timer_set(TIMER_CMD_PROCESS_INDEX,gSamConfig[index].timeout);
*/

	while(!timer_check(TIMER_CMD_PROCESS_INDEX)){
		//sam_prot_send_process();
		sam_prot_rece_process();
		if(!sam_prot_rece_ready()){
			//if(bgEnUCOS) OSTimeDly(1);
			continue;	
		}
		sam_prot_rece_get(outbuf,&outbytes);
		sam_prot_rece_reset();
		if(outbytes < 5) return -1;
		
		*obytes = outbytes-4;
		memcpy(obuf,&outbuf[4],*obytes);
	  
		#ifdef _SAM_APDU_RECEIVE_EXT_
			printf("\x0d\x0a sam_apdu_receive_ext response:");
			for(i=0;i<*obytes;i++) {
				if((i%16) == 0) printf("\x0d\x0a ");
				printf(" %02x",obuf[i]);
			}
			printf("\x0d\x0a ");
		#endif  
	  	
		return 0;
	}	
	
	return -2;	
}
#endif


//end of file
#endif 

