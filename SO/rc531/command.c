#ifndef _COMMAND_C_
#define _COMMAND_C_
//start of file
#include "global.h"




//#define _command_debug_

uint8_t bpgSamResetFlag = 0;

/*=====================================================================================
函数:dev0_cmd_process
功能:
=======================================================================================*/
void dev0_cmd_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outbuf,uint8_t *outbytes)
{
uint16_t addr,bytes;
uint8_t ret;
//uint16_t tint;

////2014/4/17 14:52:29 初始化SAM卡板 start
//if(!bpgSamResetFlag) {
//	mcu_powerctrl_clr();mdelay(200);  
//	mcu_powerctrl_set();mdelay(200);
//	mcu1_reset();mdelay(200);
//	mcu2_reset();mdelay(200);
//	bpgSamResetFlag = 1;
//}
////2014/4/17 14:52:40 end	
	

//mifare卡片命令组处理	
#ifdef CMD_MIF_START_CMD
if((inbuf[0] >= CMD_MIF_START_CMD) && (inbuf[0] <= CMD_MIF_END_CMD)){
	 cmd_mifare_process(inbuf,inbytes,outbuf,outbytes);
	 goto label_exit;
	 }
#endif


switch((uint8_t)inbuf[0]){
case CMD_REPORT_VER:
     if(inbytes != 1){
     	  if(inbuf[1] != 0) goto label_err;
         }
     strcpy((char *)outbuf,"HHJT2440_K2.6.32.2_H1.0_S1.2_20131128");
     *outbytes = strlen((char *)outbuf);
	   goto label_exit;
//---------------------------------------------------
/*----------------------------------------------------
------------------------------------------------------*/
#ifdef  CMD_NJAFC_CLASS
case (uint8_t)CMD_NJAFC_CLASS:
	 if(inbytes < 2) goto label_err;
   cmd_njafc_process(&inbuf[1],inbytes-1,outbuf,outbytes);
	 goto label_exit;
#endif	


/*----------------------------------------------------
------------------------------------------------------*/
#ifdef  CMD_MIFPRO_CLASS
case (uint8_t)CMD_MIFPRO_CLASS:
	 if(inbytes < 2) goto label_err;
   cmd_mifpro_process(&inbuf[1],inbytes-1,outbuf,outbytes);
	 goto label_exit;
#endif	

/*----------------------------------------------------
//73 02 LED 00: LED亮
//73 02 LED 01: LED灭
//LED:  0x01:红色，0x02:绿色， 0x03:红绿同时 

//73 03 0 TOH TOL: WATCHDOG START
//73 03 1 TOH TOL: WATCHDOG STOP
//73 04 : WATCHDOG TICK  
------------------------------------------------------*/
#ifdef  LED_CLASS
case (uint8_t)LED_CLASS:
   //watchdog
   /*
   if(inbuf[1] == 0x03) //watchdog start/stop
   {
   		if(inbytes != 5) goto label_err;
   		buf_to_uint16_t(&inbuf[3],tint);
   		watchdog_init(inbuf[2],tint);
   		goto label_ok;
   }
   if(inbuf[1] == 0x04) //watchdog tick
   {
   		if(inbytes != 2) goto label_err;
   		watchdog();
   		goto label_ok;
   }
   */
   
   //led
	 if(inbytes != 4) goto label_err;   	
	 if(inbuf[1] != 0x02) goto label_err;
	 switch(inbuf[2]){
	 	  case 0x01:
	 	  	   if(inbuf[3]) rled(LED_OFF);
	 	  	   else rled(LED_ON);
	 	  	   break;		
	 	  case 0x02:
	 	  	   if(inbuf[3]) gled(LED_OFF);
	 	  	   else gled(LED_ON);
	 	  	   break;		
	 	  case 0x03:
	 	  	   if(inbuf[3]) {
	 	  	   	 rled(LED_OFF);
	 	  	   	 gled(LED_OFF);
	 	  	   	 }
	 	  	   else{
	 	  	   	 rled(LED_ON);
	 	  	   	 gled(LED_ON);
	 	  	     }
	 	  	   break;		
	 	  default:
	 	  	   goto label_err;			
	 }	 
   goto label_ok; 	
#endif


/*----------------------------------------------------
------------------------------------------------------*/
#ifdef DEBUG_CLASS
case (uint8_t)DEBUG_CLASS:
	    if((uint8_t)inbuf[1] == (uint8_t)0xff){
	    	if(inbytes < 3) goto label_err;
	    	cmd_debug_process(&inbuf[2],inbytes-2,outbuf,outbytes);
	    	goto label_exit;	
	    	}
	  goto label_err;  
#endif
/*----------------------------------------------------
------------------------------------------------------*/
default:
    goto label_exit;
}

label_err:
outbuf[0] = 0xff;
*outbytes = 1;
goto label_exit;

label_ok:
outbuf[0] = 0x00;
*outbytes = 1;
goto label_exit;

label_exit:
return;
}

/*=====================================================================================
函数:cmd_debug_process
功能:
=======================================================================================*/
void cmd_debug_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outbuf,uint8_t *outbytes)
{
uint16_t tuint16_t,i;
	
switch((uint8_t)inbuf[0]){	
case DEBUG_DELAY:
	   if(inbytes != 3) goto label_err;
	   *((char *)&tuint16_t+INT_HIGH) = inbuf[1];
	   *((char *)&tuint16_t+INT_LOW) = inbuf[2];
	   delay_ms(tuint16_t);
	   goto label_ok;	
#ifdef 	DEBUG_RC_RD_BYTE   
case DEBUG_RC_RD_BYTE:
	   if(inbytes != 2) goto label_rc_rd_all;
	   outbuf[1] = rc_read_byte(inbuf[1]);
	   outbuf[0] = 0;
	   *outbytes = 2;
	   return;	
	   
	   label_rc_rd_all:
	   if(inbytes != 1) goto label_err;
	   for(i=0;i<64;i++) outbuf[i+1] = rc_read_byte(i);
	   outbuf[0] = 0;
	   *outbytes = 65;	
	   return;
#endif
#ifdef 	DEBUG_MIFPRO_ICMD_CALL_BACK   
case DEBUG_MIFPRO_ICMD_CALL_BACK:
	   if(inbytes != 5) goto label_err;
 	   //mifpro_icmd_func_call_back = (mifpro_icmd_func_type *)&inbuf[1];
	   goto label_ok;
#endif      
default:	   
	   break;
  }
goto label_err;
//
label_err:
outbuf[0] = 0xff;
*outbytes = 1;
return;
//
label_ok:
outbuf[0] = 0x00;
*outbytes = 1;
return;
}


/*=====================================================================================
函数:cmd_mifare_process
功能:
=======================================================================================*/
#ifdef CMD_MIF_START_CMD
void cmd_mifare_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outbuf,uint8_t *outbytes)
{
uint16_t i,bytes;
uint32_t value;
uint8_t ret;
	
switch((uint8_t)inbuf[0]){
//---------------------------------------------
#ifdef CMD_REQUEST
case CMD_REQUEST:
if((inbytes == 3)||(inbytes == 4)){
 *((char *)&bytes+INT_HIGH)  = inbuf[1];
 *((char *)&bytes+INT_LOW)   = inbuf[2];
 if(inbytes == 4) ret = inbuf[3];
 else ret = PICC_REQSTD; 	
 for(i=0;i<bytes;i++){
    if(mcml_request(ret,outbuf)==0){
        *outbytes = 2;
        return;
        }
    }
 goto label_ok;
 } 
goto label_err;
#endif	
//-----------------------------------------------
//防冲撞
//-----------------------------------------------
#ifdef CMD_ANTICOLL
case CMD_ANTICOLL:
switch(inbytes){
	case 1:
		  ret = mcml_anticoll(outbuf);
		  break;
  case 2:
  	  if(inbuf[1] == 0){
  	  	 ret = mcml_anticoll2(outbuf);  	  	  
	       break;
	       }
	     goto label_err;
	
	default:
		  goto label_err;
	}    
if(ret == 0){
    *outbytes = 5;
    return;
    }
goto label_err;
#endif
//-----------------------------------------------------
//选择卡
//------------------------------------------------------
#ifdef CMD_SELECT
case CMD_SELECT:
if(inbytes == 7){
   ret = mcml_select2(&inbuf[1],&outbuf[1]);
   if(ret != 0) goto label_err;
   outbuf[0] = 0;
   *outbytes = 2;
   return;
   }	
if(inbytes != 6) goto label_err;
ret = mcml_select(&inbuf[1],&outbuf[1]);
if(ret != 0) goto label_err;
outbuf[0] = 0;
*outbytes = 2;
return;
#endif
//------------------------------------------------------
/*---------------------------------------------------
设置密钥
-----------------------------------------------------*/
#ifdef CMD_LOAD_KEY
case CMD_LOAD_KEY:
if(inbytes != 10) goto label_err;
if(inbuf[2] == 0)  inbuf[2] = KEYA;
else   inbuf[2] = KEYB;
ret = mcml_load_key(inbuf[1],inbuf[2],inbuf[3],&inbuf[4]);
if(ret != 0) goto label_err;
goto label_ok;
#endif
/*----------------------------------------------------
密码比较
-----------------------------------------------------*/
#ifdef CMD_AUTHENTICATION
case CMD_AUTHENTICATION:
if(inbytes != 4) goto label_err;
if(inbuf[2] == 0)  inbuf[2] = KEYA;
else inbuf[2] = KEYB;
ret = mcml_authentication(inbuf[1],inbuf[2],inbuf[3]);
if(ret != 0){
	outbuf[0] = ret;
	*outbytes = 1;
	return;
	}
goto label_ok;
#endif
/*--------------------------------------------------------
读卡
---------------------------------------------------------*/
#ifdef CMD_RD_BLOCK
case CMD_RD_BLOCK:
#ifdef _command_debug_
printk("\nCMD_RD_BLOCK,inbytes[%d]",inbytes);
for(i=0;i<inbytes;i++) printk(" %02x",inbuf[i]);
#endif
if((inbytes != 2)&&(inbytes != 3)) goto label_err;
if(inbytes == 2) {
	ret = mcml_read(inbuf[1],outbuf);
	if(ret != 0) goto label_err;
	*outbytes = 16;
}
else {
	ret = mcml_read_4bytes(inbuf[1],outbuf);
	if(ret != 0) goto label_err;
	*outbytes = 4;	
}
return;
#endif

/*---------------------------------------------------------
写卡
----------------------------------------------------------*/
#ifdef CMD_WR_BLOCK
case CMD_WR_BLOCK:
#ifdef _command_debug_
printk("\nCMD_WR_BLOCK,inbytes[%d]",inbytes);
for(i=0;i<inbytes;i++) printk(" %02x",inbuf[i]);
#endif	
if((inbytes != 18)&&(inbytes != 6)) goto label_err;
if(inbytes == 18) {
	ret = mcml_write(inbuf[1],&inbuf[2]);
	if(ret != 0) goto label_err;
}
else {
	ret = mcml_write_4bytes(inbuf[1],&inbuf[2]);
	if(ret != 0) goto label_err;	
}
goto label_ok;
#endif

/*---------------------------------------------------------
停止卡
----------------------------------------------------------*/
#ifdef CMD_HALT
case CMD_HALT:
if(inbytes != 1) goto label_err;
ret = mcml_halt();
if(ret != 0) goto label_err;
goto label_ok;
#endif

/*---------------------------------------------------------
increment
----------------------------------------------------------*/
#ifdef CMD_INCREASE
case CMD_INCREASE:
if(inbytes != 6) goto label_err;
*((char *)&value+LONG_HIGH3) = inbuf[2];
*((char *)&value+LONG_HIGH2) = inbuf[3];
*((char *)&value+LONG_HIGH1) = inbuf[4];
*((char *)&value+LONG_HIGH0) = inbuf[5];
ret = mcml_increment(inbuf[1],value);
if(ret != 0) goto label_err;
goto label_ok;
#endif

/*-----------------------------------------------------------
decrement
------------------------------------------------------------*/
#ifdef CMD_DECREASE
case CMD_DECREASE:
if(inbytes != 6) goto label_err;
*((char *)&value+LONG_HIGH3) = inbuf[2];
*((char *)&value+LONG_HIGH2) = inbuf[3];
*((char *)&value+LONG_HIGH1) = inbuf[4];
*((char *)&value+LONG_HIGH0) = inbuf[5];
ret = mcml_decrement(inbuf[1],value);
if(ret != 0) goto label_err;
goto label_ok;
#endif

/*---------------------------------------------------------
MCM POWER OFF
----------------------------------------------------------*/
#ifdef CMD_PWR_OFF
case CMD_PWR_OFF:
if((inbytes != 1)&&(inbytes != 2)&&(inbytes != 3)) goto label_err;	
if(inbytes == 1){	//rf_pwr_off
  mcml_pwr_off();
  goto label_ok;
  }
else if(inbytes == 2) { //rf_select
	if((uint8_t)inbuf[1] >= 2) goto label_err;	
	rf_select(inbuf[1]);
	goto label_ok;
}
else { //rf_reset
	rc_init();
	goto label_ok;
}
#endif

/*-----------------------------------------------------------
restore
------------------------------------------------------------*/
#ifdef CMD_RESTORE
case CMD_RESTORE:
if(inbytes != 2) goto label_err;
ret = mcml_restore(inbuf[1]);
if(ret != 0) goto label_err;
goto label_ok;
#endif

/*-----------------------------------------------------------
transfer
-------------------------------------------------------------*/
#ifdef CMD_TRANSFER
case CMD_TRANSFER:
if(inbytes != 2) goto label_err;
ret = mcml_transfer(inbuf[1]);
if(ret != 0) goto label_err;
goto label_ok;
#endif
//------------------------------------------------------------
default:
	break;
  }	
//goto label_err;
//	
label_err:
outbuf[0] = 0xff;
*outbytes = 1;
return;
//
label_ok:
outbuf[0] = 0x00;
*outbytes = 1;
return;	
}	
#endif

/*=====================================================================================
函数:cmd_njafc_process
功能:
=======================================================================================*/
#ifdef CMD_NJAFC_CLASS
void cmd_njafc_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outbuf,uint8_t *outbytes)
{
uint8_t ret;
	
switch((uint8_t)inbuf[0]){
	#ifdef CMD_SET_CRYPT   
  case (uint8_t)CMD_SET_CRYPT:
  	    if(inbytes != 2) goto label_err;
  	    ret = rc_select_op_type(inbuf[1]);
  	    if(ret) goto label_err;
  	    goto label_ok;	
  #endif	    
  //
 
   //
   default:
  	   break;	    
   }
goto label_err;

//	
label_ok:
outbuf[0] = 0x00;
*outbytes = 1;
return;
//  
label_err:
outbuf[0] = 0xff;
*outbytes = 1;
return;
//
//label_send:
//return;  
}
#endif


/*=====================================================================================
函数:cmd_mifpro_process
功能:
=======================================================================================*/
#ifdef CMD_MIFPRO_CLASS 
void cmd_mifpro_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outbuf,uint8_t *outbytes)
{
uint8_t len,ret;
//uint8_t ibuf[65];
uint16_t tword;
uint16_t obytes;
	
switch((uint8_t)inbuf[0]){
case CMD_MIFPRO_ATS:
     if(inbytes != 2) goto label_err;
     bgCID = inbuf[1];
     ret = mifpro_ats(bgCID,outbuf,&obytes);
     if((char)ret == 0){
     	 *outbytes = obytes;
     	 goto label_send;
     	 }
     outbuf[0] = 0x01;
     outbuf[1] = ret;	 
     *outbytes = 2;
     goto label_send;
case CMD_MIFPRO_DESELECT:
     if(inbytes != 2) goto label_err;
     bgCID = inbuf[1];
     ret = mifpro_deselect(outbuf);
     if((char)ret > 0){
     	 *outbytes = ret;
     	 goto label_send;
     	 }
     goto label_err;	
case CMD_MIFPRO_ICMD_NOCHAIN:
     if(inbytes < 2) goto label_err;
     len = inbytes-1;
     ret = mifpro_icmd_nochain(len,&inbuf[1],&outbuf[1]);
     if((char)ret > 0){
     	 outbuf[0] = 0;
     	 *outbytes = ret+1;
     	 goto label_send;
     	 }
     outbuf[0] = 0xff;
     outbuf[1] = ret;
     *outbytes = 2;
     goto label_send;	
 case CMD_MIFPRO_ICMD_CHAIN:
     if(inbytes < 2) goto label_err;
     len = inbytes-1;
     ret = mifpro_icmd_chain(len,&inbuf[1],&outbuf[1]);
     if((char)ret > 0){          	
     	 outbuf[0] = 0;
     	 *outbytes = ret+1;
     	 goto label_send;
     	 }
     outbuf[0] = 0xff;
     outbuf[1] = ret;
     *outbytes = 2;
     goto label_send;	
case CMD_MIFPRO_WTX:
     if(inbytes != 1) goto label_err;
     ret = mifpro_wtx(outbuf);
     if((char)ret > 0){
     	  *outbytes = ret;
     	  goto label_send;
     	  }
     goto label_err;	
case CMD_MIFPRO_RBLOCK:
     if(inbytes != 1) goto label_err;
     ret = mifpro_ack(outbuf);
     if((char)ret > 0){
     	  *outbytes = ret;
     	  goto label_send;
     	  }
     goto label_err;	
case CMD_CRC_A:
     if(inbytes <= 1) goto label_err;
     inbytes -= 1;
     tword = rc_crc_a(&outbuf[1],inbytes);
     outbuf[0] = *((char *)&tword+INT_HIGH);
     outbuf[1] = *((char *)&tword+INT_LOW);
     *outbytes = 2;
     goto label_send;
case CMD_MIFPRO_ICMD:
     if(inbytes < 2) goto label_err;
     len = inbytes-1;
     ret = mifpro_icmd(&inbuf[1],len,&outbuf[1],&obytes);
     if((char)ret == 0){
     	 outbuf[0] = 0;
     	 *outbytes = obytes+1;
     	 goto label_send;
     	 }
     outbuf[0] = 0xff;
     outbuf[1] = ret;
     *outbytes = 2;
     goto label_send;	
case CMD_MIFPRO_NOACK:
     if(inbytes != 1) goto label_err;
     ret = mifpro_noack(outbuf);
     if((char)ret > 0){
     	  *outbytes = ret;
     	  goto label_send;
     	  }
     goto label_err;
//add start by xux,2013/8/26 15:24:32
case CMD_MIFPRO_PPS:
     if(inbytes != 2) goto label_err;
     ret = mifpro_pps(inbuf[1],&outbuf[1]);
     if(ret){
     	  outbuf[0] = 1;
     	  outbuf[1] = ret;
     	  }	
     else{
        outbuf[0] = 0;
        }	  
	   *outbytes = 2;
	   goto label_send;	   
case CMD_MIFPRO_SET_SPEED:
     if(inbytes != 3) goto label_err;
     mifpro_set_speed(inbuf[1],inbuf[2]);
     outbuf[0] = 0x00;
     *outbytes = 1;
     goto label_send;
//add end
default:
	break;
  }	
goto label_err;
//	
label_err:
outbuf[0] = 0xff;
*outbytes = 1;
return;
//

label_send:
return;
}	
#endif

#endif