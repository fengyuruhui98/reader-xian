//sam_prot.c
//SAM卡模组协议
//编制：邓建华
//时间：20080226

#ifndef _SAM_PROT_C_
#define _SAM_PROT_C_
//start of file


UBYTE   bpgSamReceBlock[SAM_PROT_MAX_LEN+1];
UBYTE   bgSamReceLen;
UBYTE   bgSamRecePtr;
UBYTE   bgSamReceState,bgSamReceSubState;
UBYTE   bgSamLrc;

UBYTE  bpgSamSendBlock[SAM_PROT_MAX_LEN+1];
UBYTE  bgSamSendLen;
UBYTE  bgSamSendState;

//#define	_DEBUG_

#ifdef _DEBUG_
#define  _SAM_PROT_DEBUG_  
#endif

/*===============================================================================
函数：
功能：
=================================================================================*/
void sam_prot_rece_reset(void)
{
	bgSamReceState = SAM_PROT_WAIT_STX;
	bgSamReceSubState = SAM_PROT_SUB_STATE_WAIT_DLE;
	return;	
}	

/*===============================================================================
函数：
功能：
=================================================================================*/
void sam_prot_send_reset(void)
{
	bgSamSendState = SAM_SEND_STATE_OK;
	return;	
}	


/*============================================================================================
函数:sam_prot_gen_block
功能:
==============================================================================================*/
void sam_prot_gen_block(UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UWORD *outbytes)
{
UWORD ptr;
UBYTE lrc;
UBYTE i;
//
	ptr=0;
	outbuf[ptr++] = STX;lrc = STX;
	//
	switch((UBYTE)inbytes){
		case STX:
		case ETX:
		case DLE:
			outbuf[ptr++] = DLE;
			break;
		default:
			break;	   
	}
	outbuf[ptr++] = inbytes;lrc ^= inbytes;
	//
	for(i=0;(UBYTE)i<(UBYTE)inbytes;i++){
		switch((UBYTE)inbuf[i]){
			case STX:
			case ETX:
			case DLE:
				outbuf[ptr++] = DLE;
				break;
			default:
				break;	   
		}
		outbuf[ptr++] = inbuf[i];lrc ^= inbuf[i];
	}
	outbuf[ptr++] = ETX;lrc ^= ETX;
	//
	switch((UBYTE)lrc){
		case STX:
		case ETX:
		case DLE:
			outbuf[ptr++] = DLE;
			break;
		default:
			break;	   
	}
	outbuf[ptr++] = lrc;
	//
	*outbytes = ptr;
	//
	return;	
}

/*===============================================================================
函数：sam_push_byte
功能：
=================================================================================*/
void sam_push_byte(UBYTE inbyte)
{
	timer_clr(TIMER_SAM_RECE_INDEX);	
	//等待协议起始符	
	if((UBYTE)bgSamReceState == SAM_PROT_WAIT_STX){
		 if(inbyte != STX) return;
		 bgSamLrc = STX;	
		 bgSamReceState = SAM_PROT_WAIT_LEN;
		 bgSamReceSubState = SAM_PROT_SUB_STATE_WAIT_DLE;
		 timer_set(TIMER_SAM_RECE_INDEX,SAM_CHAR_INTERVAL);
		 return;
	}
	//等待协议终止符	 
	if((UBYTE)bgSamReceState == SAM_PROT_WAIT_ETX){
		 if(inbyte != ETX){
		 	 #ifdef _SAM_PROT_DEBUG_
		 	 	printf("\nE:ETX[%02X]",inbyte);
		 	 #endif 
		 	 goto label_err;
		 	 }
		 bgSamReceState = SAM_PROT_WAIT_LRC;
		 goto label_wait_dle;
	}
	//DLE处理
	if(((UBYTE)inbyte == (UBYTE)DLE) &&(bgSamReceSubState != SAM_PROT_SUB_STATE_WAIT_DATA)){
		 bgSamReceSubState = SAM_PROT_SUB_STATE_WAIT_DATA;
		 return;	
		 }
	//等待长度
	if((UBYTE)bgSamReceState == SAM_PROT_WAIT_LEN){
		 bgSamReceLen = inbyte;
		 if((UBYTE)inbyte > (UBYTE)SAM_PROT_MAX_LEN){
		 	 #ifdef _SAM_PROT_DEBUG_
		 	 	printf("\nE:len[%02X]",(UBYTE)inbyte);
		 	 #endif 	 	 
		 	 goto label_err;
		 	 }
		 bgSamRecePtr = 0;
		 if(bgSamReceLen != 0) bgSamReceState = SAM_PROT_WAIT_DATA;
		 else bgSamReceState = SAM_PROT_WAIT_LRC;	
		 goto label_wait_dle;
	}
	//等待数据
	if((UBYTE)bgSamReceState == SAM_PROT_WAIT_DATA){
		 bpgSamReceBlock[bgSamRecePtr++] = inbyte;
		 if(bgSamRecePtr != bgSamReceLen) goto label_wait_dle; 
		 //
		 bgSamReceState = SAM_PROT_WAIT_ETX;
		 goto label_wait_dle;
	}
	//等待LRC
	if((UBYTE)bgSamReceState == SAM_PROT_WAIT_LRC){
	   bgSamLrc ^= inbyte;	
		 if((UBYTE)bgSamLrc != 0){
		 	 #ifdef _SAM_PROT_DEBUG_
		 	 printf("E:CRC");
		 	 #endif 
		 	 goto label_err;
		 	 }
	   bgSamReceState = SAM_PROT_WAIT_ACK;
		 return;
	}
	
	return;

//等待DLE
label_wait_dle:
	bgSamLrc ^= inbyte;	
	bgSamReceSubState = SAM_PROT_SUB_STATE_WAIT_DLE;
	return;

//错误出口
label_err:
	bgSamReceState = SAM_PROT_ERR;
	return;	
}

/*===============================================================================
函数：sam_prot_send_process
功能：
=================================================================================*/
void sam_prot_send_process(void)
{
UBYTE outbuf[2*SAM_PROT_MAX_LEN+10];
UWORD outbytes;
UBYTE ch;

#ifdef _SAM_PROT_DEBUG_
UWORD i;
#endif
	
	switch(bgSamSendState){
	case SAM_SEND_STATE_OK:
		   return;
	case SAM_SEND_STATE_S0:
	case SAM_SEND_STATE_S1:
	case SAM_SEND_STATE_S2:
		   sam_prot_gen_block(bpgSamSendBlock,bgSamSendLen,outbuf,&outbytes);
		   uart_put_bytes(SAM_UART_INDEX,outbuf,outbytes,100);
		   
				#ifdef _SAM_PROT_DEBUG_
				printf("\x0d\x0a Sam Send:");
				for(i=0;i<outbytes;i++){
				if(i%16==0) printf("\x0d\x0a ");
				printf("%02X ",(UBYTE)outbuf[i]);
				}
				#endif
				
				goto label_ok; //2013/11/17 18:00:22
				
       //发送数据
       //
       bgSamSendState++;   //sn send
       break; 
  case SAM_SEND_STATE_S0_SEND:
  case SAM_SEND_STATE_S1_SEND:
  case SAM_SEND_STATE_S2_SEND:
  	   if(!uart_send_is_empty(SAM_UART_INDEX)) break;
  	   bgSamSendState++;   //sn wait
       timer_set(TIMER_SAM_SEND_INDEX,SAM_WAIT_ACK_TIME);
       break; 	     
  case SAM_SEND_STATE_S0_WAIT:
  case SAM_SEND_STATE_S1_WAIT:
  case SAM_SEND_STATE_S2_WAIT:     
       //
  	   if(!uart_rece_is_empty(SAM_UART_INDEX)){
  	   	 ch = uart_get_byte(SAM_UART_INDEX);
  	   	 if(ch == ACK){
  	   	 	 #ifdef _SAM_PROT_DEBUG_
           printf("\n 收到ACK[%lums]",timer_get(TIMER_SAM_SEND_INDEX));
           #endif
         	 goto label_ok;
  	   	 	 }
         if(ch == NAK){
         	 if(bgSamSendState == SAM_SEND_STATE_S2_WAIT) goto label_err;
         	 bgSamSendState++;   //重发
         	 #ifdef _SAM_PROT_DEBUG_
           printf("\n 收到NAK:重发");
           #endif
         	 break;	
         	 }
         #ifdef _SAM_PROT_DEBUG_
         printf("\n 收到其他应答数据[%02X]:不处理",(UBYTE)ch);
         #endif
         return;
         }
       if(timer_check(TIMER_SAM_SEND_INDEX)){    //超时
         if(bgSamSendState == SAM_SEND_STATE_S2_WAIT){
         	 #ifdef _SAM_PROT_DEBUG_
         	 printf("\x0d\x0a错误:发送等待应答超时[%lums]",timer_get(TIMER_SAM_SEND_INDEX));
         	 #endif
         	 goto label_err;
         	 }
         bgSamSendState++;   //重发
         #ifdef _SAM_PROT_DEBUG_
         printf("\x0d\x0a发送等待应答超时:重发[%lums]",timer_get(TIMER_SAM_SEND_INDEX));
         #endif
         break;	
       	 }
  	   break;
	default:
		   break;
	}
	return;

label_ok:
	bgSamSendState = SAM_SEND_STATE_OK;	
	return;	

label_err:
	bgSamSendState = SAM_SEND_STATE_ERR;	
	sam_prot_send_reset();
	return;	
}	


/*===============================================================================
函数：sam_prot_rece_process
功能：
=================================================================================*/
void sam_prot_rece_process(void)
{
UBYTE ch;
#ifdef _SAM_PROT_DEBUG_
int i;
#endif

//前面包未处理	
if(bgSamReceState == SAM_PROT_WAIT_PROCESS) return;   
//
switch(bgSamSendState){
  case SAM_SEND_STATE_S0_WAIT:      //发送程序等待应答时跳过
  case SAM_SEND_STATE_S1_WAIT:
  case SAM_SEND_STATE_S2_WAIT:
  	   return;
  default:
  	   break;	        
	}
//
label_rece_loop:
if(uart_rece_is_empty(SAM_UART_INDEX)){
	if(bgSamReceState == SAM_PROT_WAIT_STX) return;
  if(timer_check(TIMER_SAM_RECE_INDEX)){
  	#ifdef _SAM_PROT_DEBUG_
	  printf("\x0d\x0a字符间超时");
	  #endif	  
	  //NAK
	  uart_put_byte(SAM_UART_INDEX,NAK);
	  //
  	sam_prot_rece_reset();
  	return;
  	}
  return;
  }
//
ch = uart_get_byte(SAM_UART_INDEX);
sam_push_byte(ch);
//
if((UBYTE)bgSamReceState == (UBYTE)SAM_PROT_ERR){
	#ifdef _SAM_PROT_DEBUG_	  
	printf("\x0d\x0a数据接收错误");
	#endif
	//NAK
	uart_put_byte(SAM_UART_INDEX,NAK);
	//
	sam_prot_rece_reset();
	return;
	}
if((UBYTE)bgSamReceState == (UBYTE)SAM_PROT_WAIT_ACK){
	//ACK
	uart_put_byte(SAM_UART_INDEX,ACK);
	//
	bgSamReceState = SAM_PROT_WAIT_PROCESS;
	#ifdef _SAM_PROT_DEBUG_
  printf("\x0d\x0a Sam Rece Block:");
  for(i=0;i<bgSamReceLen;i++){
		if(i%16==0) printf("\x0d\x0a ");
		printf("%02X ",(UBYTE)bpgSamReceBlock[i]);
		}
  #endif
	return;
	}

	goto label_rece_loop;
}	


/*===============================================================================
函数：sam_prot_start_send
功能：
=================================================================================*/
void sam_prot_start_send(UBYTE *inbuf,UBYTE inbytes)
{
#ifdef _SAM_PROT_DEBUG_
int i;
#endif

if((UBYTE)inbytes > 	(UBYTE)SAM_PROT_MAX_LEN) return;
memcpy(bpgSamSendBlock,inbuf,inbytes);
bgSamSendLen = inbytes;
bgSamSendState = SAM_SEND_STATE_S0;
//
#ifdef _SAM_PROT_DEBUG_
printf("\x0d\x0a Sam Send Block:");
for(i=0;i<bgSamSendLen;i++){
		if(i%16==0) printf("\x0d\x0a ");
		printf("%02X ",(UBYTE)bpgSamSendBlock[i]);
		}
#endif
	//
	sam_prot_send_process();
	return;
}

/*===============================================================================
函数：sam_prot_rece_ready
功能：
=================================================================================*/
UBYTE sam_prot_rece_ready(void)
{
	if(bgSamReceState == SAM_PROT_WAIT_PROCESS) return 1;
	return 0;	
}

/*===============================================================================
函数：sam_prot_start_send
功能：
=================================================================================*/
void  sam_prot_rece_get(UBYTE *outbuf,UBYTE *outbytes)
{
	memcpy(outbuf,bpgSamReceBlock,bgSamReceLen);
	*outbytes = bgSamReceLen;
	return;
}

//end of file
#endif

