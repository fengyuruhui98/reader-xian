#ifndef _ISO_BLOCK_C_
#define _ISO_BLOCK_C_
//start of file
#include "global.h"



//mifpro_icmd_func_type *mifpro_icmd_call_back = NULL;

uint8_t bgCID;
uint8_t bgPCB;
uint8_t bgCIDFlag;
uint16_t wgFWT = 5;//default 4.8ms
uint8_t bgSFGI;
uint8_t bgWTX;

#define CID_FLAG     0x08       //=0x08: 有CID
#define MAX_RF_BUF   60         //48-->64 

/*=======================================================================================
函数:
功能:
入口参数:
inbuf:含PCB等
=========================================================================================*/
//#define _DEBUG_ISO_BLOCK_TRANSCEVE_
uint16_t iso_block_transceve(uint8_t *inbuf,uint16_t inbytes,uint8_t *outbuf,uint16_t *outbytes,uint16_t timeout)
{
	
uint16_t i;
uint8_t rece_bits;
uint8_t rbytes,ptr=0,sbytes;
//uint16_t i;

#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
	printk("\niso_block_transceve-->inbuf[%d]:",inbytes);
	for(i=0;i<inbytes;i++) {
		if((i%16) == 0) printk("\n");
		printk(" %02x",(uint8_t)inbuf[i]);
	}
#endif

if(bgIsoType == 1){
  return 1;
  }
rc_clr_crypt();
rc_set_bitframe(0);
rc_idle();
rc_flush_fifo();
rc_clr_time_out();
//
//
rc_clr_irq();
rc_write_byte(REG_RC500_INTERRUPT_EN,(BIT_RXI|BIT_TIMERI)|0x80);
iso_block_set_time_out((timeout+4)/5);
rc_write_byte(REG_RC500_COMMAND,RC500_CMD_TRANSCEIVE);
if(inbytes > 64) sbytes = 64;
else sbytes = inbytes;	
rc_write_data_n(inbuf,sbytes);
ptr += sbytes;
inbytes -= sbytes;
while(inbytes){
  rbytes = rc_read_byte(REG_RC500_FIFO_LENGTH);
  if(rbytes < 32){
     if(inbytes > 32) sbytes = 32;
     else sbytes = inbytes;	
  	 rc_write_data_n(&inbuf[ptr],sbytes);
  	 ptr += sbytes;
  	 inbytes -= sbytes;
  	 }	
	}
//
/*if(mifpro_icmd_call_back != NULL) {
	mifpro_icmd_call_back();
	mifpro_icmd_call_back = NULL;
}*/
//
ptr = 0;
for(i=0;(uint16_t)i<(uint16_t)40000;i++){
  if(rc_read_byte(REG_RC500_PRIMARY_STATUS)&0x08) break;
  if(rc_is_in_receive()){	
	   rbytes = rc_read_byte(REG_RC500_FIFO_LENGTH);
     if(rbytes>=24){
  	     rc_read_data_n(&outbuf[ptr],rbytes);
  	     ptr += rbytes;
  	     }
  	  }
  else{
      delay_us(5);
  	  }	     
  //watchdog();	
  }
rc_clr_time_out();  
//
if(!(rc_read_byte(REG_RC500_INTERRUPT_RQ)&BIT_RXI)) return (uint8_t)-1;
if(rc_read_byte(REG_RC500_ERROR_FLAG)&0x0f) return (uint8_t)-2;

rece_bits = rc_rece_bits();
if(rece_bits%8) return (uint8_t)-3;

*outbytes = rece_bits/8;
rc_read_data_n(&outbuf[ptr],*outbytes);
*outbytes = *outbytes+ptr;
 
#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
	printk("\niso_block_transceve-->outbuf[%d]:",*outbytes);
	for(i=0;i<*outbytes;i++) {
		if((i%16) == 0) printk("\n");
		printk(" %02x",(uint8_t)outbuf[i]);
	}
#endif
 
return 0;
 
}


/*
uint16_t iso_block_transceve(uint8_t *inbuf,uint16_t inbytes,uint8_t *outbuf,uint16_t *outbytes,uint16_t timeout)
{
//uint8_t  rece_bits;
uint16_t rece_bits;
uint16_t ptr;
uint8_t rbytes;
uint16_t i;

#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
printk("\niso_block_transceve-->inbuf[%d]:",inbytes);
for(i=0;i<inbytes;i++) {
	if((i%16) == 0) printk("\n");
	printk(" %02x",(uint8_t)inbuf[i]);
}
#endif

if(bgIsoType == 0){
//  rc_crc_enable();
//  rc_crc_sel_14443a();
//  rc_oddpari_enable();
  }
else{
	rc_crc_enable();
  rc_crc_sel_14443b();
  rc_pari_disable();
	}   
rc_clr_crypt();
rc_set_bitframe(0);
rc_idle();
rc_flush_fifo();
rc_clr_time_out();
//
rc_write_data_n(inbuf,inbytes);
//
rc_clr_irq();
rc_write_byte(REG_RC500_INTERRUPT_EN,(BIT_RXI|BIT_TIMERI)|0x80);
iso_block_set_time_out((timeout+4)/5);
rc_write_byte(REG_RC500_COMMAND,RC500_CMD_TRANSCEIVE);
//
for(i=0;(uint16_t)i<(uint16_t)6000;i++){
  if(rc_read_byte(REG_RC500_PRIMARY_STATUS)&0x08) break;
  if(rc_is_in_receive()){	
	   rbytes = rc_read_byte(REG_RC500_FIFO_LENGTH);
     if(rbytes>=8){
  	     rc_read_data_n(&outbuf[ptr],rbytes);
  	     ptr += rbytes;
  	     }
  	  }
  else{
      delay_us(50);
  	  }	     
  //watchdog();	
}
rc_clr_time_out();  
//
if(!(rc_read_byte(REG_RC500_INTERRUPT_RQ)&BIT_RXI)) return (uint16_t)-1;
if(rc_read_byte(REG_RC500_ERROR_FLAG)&0x0f) return (uint16_t)-2;

rece_bits = rc_rece_bits();
if(rece_bits%8) return (uint16_t)-3;

*outbytes = rece_bits/8;
rc_read_data_n(&outbuf[ptr],*outbytes);
*outbytes = *outbytes+ptr;

#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
printk("\niso_block_transceve-->outbuf[%d]:",*outbytes);
for(i=0;i<*outbytes;i++) {
	if((i%16) == 0) printk("\n");
	printk(" %02x",(uint8_t)outbuf[i]);
}
#endif
 
return 0;
}
*/

/*=============================================================================================
函数：iso_block_set_time_out
功能：设置超时
入口参数:
cnt_5ms: 以5ms为单位的数值
===============================================================================================*/
void iso_block_set_time_out(uint8_t cnt_5ms)
{
//
rc_write_byte(REG_RC500_TIMER_CLOCK,0x10);	  //(65536)/13.56M = 4833us
rc_write_byte(REG_RC500_TIMER_RELOAD,cnt_5ms);
//
return;
}


/*==================================================================================================
函数:
功能:
====================================================================================================*/
void pcb_reverse(void)
{
if(bgPCB) bgPCB=0;
else bgPCB=1;
return;			
}	

/*==================================================================================================
函数:mifpro_ats
功能:
====================================================================================================*/
//#define _DEBUG_MIFPRO_ATS_
uint16_t mifpro_ats(uint8_t cid,uint8_t *obuf,uint16_t *obytes)
{
uint8_t inbuf[16],inbytes;
uint16_t ret;
uint8_t fwi,i;

wgFWT=5;
bgSFGI=0;
bgCIDFlag=1;
bgCID = 0;
bgPCB = 1;
//
cid = cid&0x0f;
inbuf[0] = RATS_BLOCK;
//inbuf[1] = 0x00|cid;   //fifo:16bytes
//inbuf[1] = 0x10|cid;   //fifo:24bytes
//inbuf[1] = 0x40|cid;   //fifo:48bytes
//inbuf[1] = 0x50|cid;   //fifo:64bytes
//inbuf[1] = 0x60|cid;   //fifo:96bytes
//inbuf[1] = 0x70|cid;   //fifo:128bytes
inbuf[1] = 0x80|cid;   //fifo:256bytes
inbytes = 2;
ret = iso_block_transceve(inbuf,inbytes,obuf,obytes,ATS_TIME_OUT);

#ifdef _DEBUG_MIFPRO_ATS_
if(ret==0){
	printk("\nats:%d bytes",*obytes);
	for(i=0;i<*obytes;i++){
		if(i%16 == 0) printk("\n");
		printk("%02X ",(uint8_t)obuf[i]);	
		}
	}
else{
  printk("\nErr:ats,ret=%d",ret);
  } 	
#endif

if(ret!=0) return ret;
if((obuf[1]&0x70) == 0) return 0;
i = 2;
if(obuf[1]&0x10) i++;	
bgSFGI = obuf[i]&0x0f;
fwi = (uint8_t)(obuf[i]&0xf0)/16;
if(obuf[1]&0x20) i++;
if(obuf[1]&0x40){	
  if(obuf[i]&0x02) bgCIDFlag=1;
  bgCID = cid;
  }
  
wgFWT = 1;i=0;
while(i<fwi){
	wgFWT *= 2;
	i++;
	}
wgFWT = (wgFWT*3)/10;	

#ifdef _DEBUG_MIFPRO_ATS_
printk("\nSFGI=%d,FWT=%d,CID_FLAG=%d",bgSFGI,wgFWT,bgCIDFlag);
#endif

return 0;	
}	






/*==================================================================================================
函数:mifpro_deselect
功能:
====================================================================================================*/
//#define _DEBUG_MIFPRO_DESELECT_
uint16_t mifpro_deselect(uint8_t *outbuf)
{
uint16_t ret;
uint8_t inbuf[4];
uint16_t inbytes,outbytes;
#ifdef _DEBUG_MIFPRO_DESELECT_
uint16_t i;
#endif

inbuf[0] = 0xc2;
inbytes = 1;
if(bgCIDFlag){
	 inbuf[0] |= CID_FLAG;
	 inbuf[1] = bgCID;
	 inbytes++;
	 }
 
ret = iso_block_transceve(inbuf,inbytes,outbuf,&outbytes,DESELECT_TIME_OUT);
if(ret){
   #ifdef _DEBUG_MIFPRO_DESELECT_
   printk("\nErr:mifpro_deselect,ret=%d",(uint16_t)ret);
   #endif
	 return ret;
   }

#ifdef _DEBUG_MIFPRO_DESELECT_
printk("\nOK:mifpro_deselect,outbytes=%d",(uint16_t)outbytes);
for(i=0;i<outbytes;i++){
	 if(i%16 == 0) printk("\n");
	 printk("%02X ",(uint8_t)outbuf[i]);
	 }
#endif
return outbytes;	
}	



/*==================================================================================================
函数:mifpro_deselect
功能:
====================================================================================================*/
//#define _DEBUG_MIFPRO_WTX_
uint16_t mifpro_wtx(uint8_t *outbuf)
{
uint16_t ret;
uint8_t inbuf[4];
uint16_t inbytes,outbytes;
#ifdef _DEBUG_MIFPRO_WTX_
uint16_t i;
#endif

inbuf[0] = 0xf2;
inbytes = 1;
if(bgCIDFlag){
	 inbuf[0] |= CID_FLAG;
	 inbuf[1] = bgCID;
	 inbytes++;
	 }
inbuf[inbytes++] = bgWTX;
// 
ret = iso_block_transceve(inbuf,inbytes,outbuf,&outbytes,wgFWT*bgWTX);
if(ret){
   #ifdef _DEBUG_MIFPRO_WTX_
   printk("\nErr:mifpro_wtx,ret=%d",(uint16_t)ret);
   #endif
	 return ret;
   }

#ifdef _DEBUG_MIFPRO_WTX_
printk("\nOK:mifpro_wtx,outbytes=%d",(uint16_t)outbytes);
for(i=0;i<outbytes;i++){
	 if(i%16 == 0) printk("\n");
	 printk("%02X ",(uint8_t)outbuf[i]);
	 }
#endif
return outbytes;	
}	



/*==============================================================================
函数：mifpro_noack
功能：
================================================================================*/
//#define _DEBUG_MIFPRO_NOACK_
uint16_t mifpro_noack(uint8_t *outbuf)
{
uint16_t outbytes,inbytes;
uint16_t ret;
uint8_t inbuf[3];
#ifdef _DEBUG_MIFPRO_NOACK_
uint16_t i;
#endif

inbuf[0] = (bgPCB&0x01)|0xb2;
inbytes = 1;
if(bgCIDFlag){
	 inbuf[0] |= CID_FLAG; 
   inbuf[1] = bgCID;
   inbytes++;
   }
ret = iso_block_transceve(inbuf,inbytes,outbuf,&outbytes,wgFWT);
if(ret){
   #ifdef _DEBUG_MIFPRO_NOACK_
   printk("\nErr:mifpro_noack,ret=%d",(uint16_t)ret);
   #endif
	 return ret;
   }

#ifdef _DEBUG_MIFPRO_NOACK_
printk("\nOK:mifpro_noack,outbytes=%d",(uint16_t)outbytes);
for(i=0;i<outbytes;i++){
	 if(i%16 == 0) printk("\n");
	 printk("%02X ",(uint8_t)outbuf[i]);
	 }
#endif
return outbytes;
}	 

/*===============================================================================
函数：
功能：
=================================================================================*/
//#define _DEBUG_MIFPRO_ACK_
uint16_t mifpro_ack(uint8_t *outbuf)
{
uint16_t outbytes,inbytes;
uint16_t ret;
uint8_t inbuf[3];
#ifdef _DEBUG_MIFPRO_ACK_
uint16_t i;
#endif

pcb_reverse();
inbuf[0] = (bgPCB&0x01)|0xa2;
inbytes = 1;
if(bgCIDFlag){
	 inbuf[0] |= CID_FLAG; 
   inbuf[1] = bgCID;
   inbytes++;
   }
ret = iso_block_transceve(inbuf,inbytes,outbuf,&outbytes,wgFWT);
if(ret){
   #ifdef _DEBUG_MIFPRO_ACK_
   printk("\nErr:mifpro_ack,ret=%d",(uint16_t)ret);
   #endif
	 return (uint16_t)ret;
   }

#ifdef _DEBUG_MIFPRO_ACK_
printk("\nOK:mifpro_ack,outbytes=%d",(uint16_t)outbytes);
for(i=0;i<outbytes;i++){
	 if(i%16 == 0) printk("\n");
	 printk("%02X ",(uint8_t)outbuf[i]);
	 }
#endif
return outbytes;
}	 



/*===============================================================================
函数：mifpro_icmd_nochain
功能：
=================================================================================*/
//#define _DEBUG_MIFPRO_ICMD_NOCHAIN_
uint16_t mifpro_icmd_nochain(uint16_t len,uint8_t *inbuf,uint8_t *outbuf)
{
uint16_t outbytes,inbytes;
uint8_t buf[257];
uint16_t ret;
#ifdef _DEBUG_MIFPRO_ICMD_NOCHAIN_
uint16_t i;
#endif

#ifdef _DEBUG_MIFPRO_ICMD_NOCHAIN_
printk("\nmifpro_icmd_nochain,cmd len=%d",len);
for(i=0;i<len;i++){
   if(i%16 == 0) printk("\n");
   printk("%02X ",(uint8_t)inbuf[i]);	
   }  
#endif

if((uint16_t)len > (uint16_t)MAX_RF_BUF) return (uint16_t)-1;

pcb_reverse();
buf[0] = (bgPCB&0x01)|0x02;
inbytes = 1;
if(bgCIDFlag){
   buf[0] |= CID_FLAG;
   buf[1] = bgCID;
   inbytes++;
   }
memcpy(&buf[inbytes],inbuf,len);
inbytes += len;

ret = iso_block_transceve(buf,inbytes,outbuf,&outbytes,wgFWT);
if(ret){
   #ifdef _DEBUG_MIFPRO_ICMD_NOCHAIN_
   printk("\nErr:mifpro_icmd_nochain,ret=%d",(uint16_t)ret);
   #endif
	 return (uint16_t)ret;
   }

#ifdef _DEBUG_MIFPRO_ICMD_NOCHAIN_
printk("\nOK:mifpro_icmd_nochain,outbytes=%d",(uint16_t)outbytes);
for(i=0;i<outbytes;i++){
	 if(i%16 == 0) printk("\n");
	 printk("%02X ",(uint8_t)outbuf[i]);
	 }
#endif
return outbytes;
}	

/*===================================================================================
函数：mifpro_icmd_chain
功能：
=====================================================================================*/
//#define _DEBUG_MIFPRO_ICMD_CHAIN_
uint16_t mifpro_icmd_chain(uint16_t len,uint8_t *inbuf,uint8_t *outbuf)
{
uint16_t outbytes,inbytes;
uint8_t buf[257];
uint16_t ret;
#ifdef _DEBUG_MIFPRO_ICMD_CHAIN_
uint16_t i;
#endif

#ifdef _DEBUG_MIFPRO_ICMD_CHAIN_
printk("\nmifpro_icmd_chain,cmd len=%d",len);
for(i=0;i<len;i++){
   if(i%16 == 0) printk("\n");
   printk("%02X ",(uint8_t)inbuf[i]);	
   }  
#endif

if((uint16_t)len > (uint16_t)MAX_RF_BUF) return (uint16_t)-1;

pcb_reverse();
buf[0] = (bgPCB&0x01)|0x12;
inbytes = 1;
if(bgCIDFlag){
   buf[0] |= CID_FLAG;
   buf[1] = bgCID;
   inbytes++;
   }
memcpy(&buf[inbytes],inbuf,len);
inbytes += len;

ret = iso_block_transceve(buf,inbytes,outbuf,&outbytes,wgFWT);
if(ret){
   #ifdef _DEBUG_MIFPRO_ICMD_CHAIN_
   printk("\nErr:mifpro_icmd_chain,ret=%d",(uint16_t)ret);
   #endif
	 return (uint16_t)ret;
   }

#ifdef _DEBUG_MIFPRO_ICMD_CHAIN_
printk("\nOK:mifpro_icmd_chain,outbytes=%d",(uint16_t)outbytes);
for(i=0;i<outbytes;i++){
	 if(i%16 == 0) printk("\n");
	 printk("%02X ",(uint8_t)outbuf[i]);
	 }
#endif
return outbytes;
}	


/*==================================================================================
函数：mifpro_icmd
功能：
====================================================================================*/
//#define _DEBUG_MIFPRO_ICMD_
//#define _NOACK_TIMEOUT_10MS_
uint8_t mifpro_icmd(uint8_t *ibuf,uint16_t ibytes,uint8_t *obuf,uint16_t *obytes)
{
uint16_t slen;
uint16_t  ret;
uint16_t rptr,sptr;
uint16_t i;
uint8_t outbuf[257];
uint16_t offset;
uint8_t last_pcb;

#ifdef _NOACK_TIMEOUT_10MS_
uint16_t tempFwt;
#endif
//2013/11/13 12:29:38


#ifdef _DEBUG_MIFPRO_ICMD_ 
	printk("\nmifpro_icmd,ibytes[%d]",(uint16_t)ibytes);
	for(i=0;i<ibytes;i++) {
		if((i%16) == 0) printk("\n");
		printk(" %02x",(uint8_t)ibuf[i]);
	}
#endif

sptr = 0;rptr = 0;
if(bgCIDFlag) offset=2;
else offset=1;	
	
label_send_loop:
if(ibytes > MAX_RF_BUF){
    slen = MAX_RF_BUF;
    ret = mifpro_icmd_chain(slen,&ibuf[sptr],&outbuf[rptr]); 
    last_pcb = bgPCB;
    }
else{
	  slen = ibytes;
	  ret = mifpro_icmd_nochain(slen,&ibuf[sptr],&outbuf[rptr]); 
	  }   


label_resp_process:
#ifdef _DEBUG_MIFPRO_ICMD_ 
printk("\nresp_process,ret=%x,offset=%d,rptr=%d",(uint16_t)ret,(uint16_t)offset,(uint16_t)rptr);
if((uint16_t)ret <= (uint16_t)0x100) {
	for(i=0;i<ret;i++) {
		if((i%16) == 0) printk("\n");
		printk(" %02x",(uint8_t)outbuf[rptr+i]);
	}
}
#endif	   
//if((uint8_t)ret > 128)  goto label_noack;
if((uint16_t)ret > (uint16_t)0x8000)  goto label_noack;	
if(ret < offset) goto label_abnormal;

//rece no chain
if((outbuf[rptr]&I_BLOCK_MASK) == I_BLOCK_NO_CHAIN){
	 #ifdef _DEBUG_MIFPRO_ICMD_
	 printk("\nRece no chain");
	 #endif 
   for(i=0;i<(ret-offset);i++) outbuf[rptr+i] = outbuf[rptr+i+offset];
   rptr += (ret-offset);
   if((uint16_t)rptr > 256) return (uint8_t)-3; 
   memcpy(obuf,outbuf,rptr);
   *obytes = rptr;	
		#ifdef _DEBUG_MIFPRO_ICMD_ 
		printk("\nmifpro_icmd,obytes[%d]",(uint16_t)rptr);
		for(i=0;i<rptr;i++) {
			if((i%16) == 0) printk("\n");
			printk(" %02x",(uint8_t)obuf[i]);
		}
		#endif
		
   return 0;
   }

//rece chain
if((outbuf[rptr]&I_BLOCK_MASK) == I_BLOCK_CHAIN){
	 #ifdef _DEBUG_MIFPRO_ICMD_
	 printk("\nRece chain");
	 #endif 
   for(i=0;i<(ret-offset);i++) outbuf[rptr+i] = outbuf[rptr+i+offset];
   rptr += (ret-offset);
   if((uint16_t)rptr > 256) return (uint8_t)-4; 
   //ack
   ret = mifpro_ack(&outbuf[rptr]);
   goto label_resp_process;
   }

//wtx 
if((uint8_t)(outbuf[rptr]&WTX_BLOCK_MASK) == (uint8_t)WTX_BLOCK){
   bgWTX = outbuf[rptr+offset]&0x3f;
   ret = mifpro_wtx(&outbuf[rptr]);
   goto label_resp_process;
   }  

   	
//ack,continue	
if((outbuf[rptr]&R_BLOCK_MASK) == R_BLOCK_ACK){
   if((last_pcb&0x01) != (outbuf[rptr]&0x01)){  
       #ifdef _DEBUG_MIFPRO_ICMD_
       printk("\nrece last ack,go to resend");
       #endif
       goto label_send_loop;
       }
   #ifdef _DEBUG_MIFPRO_ICMD_
   printk("\nrece ack,con to send");
   #endif
	 sptr += slen;
	 if(ibytes <= slen){
	    #ifdef _DEBUG_MIFPRO_ICMD_
	 	  printk("\nErr:abnormal,rece ack but no data to send, len=%d,slen=%d",ibytes,slen);
	    #endif	
	 	  return (uint8_t)-5;
	 	  }	 	
	 ibytes -= slen;
	 goto label_send_loop;	
	 }

//其他块
#ifdef _DEBUG_MIFPRO_ICMD_
printk("\nErr:unknown pcb %02X.",(uint8_t)outbuf[rptr]);
#endif
return (uint8_t)-6;   

label_abnormal: 
#ifdef _DEBUG_MIFPRO_ICMD_ 
printk("\nErr:abnormal,ret=%d",(uint8_t)ret);
#endif 
return (uint8_t)-7; 

label_noack:
#ifdef _DEBUG_MIFPRO_ICMD_ 
printk("\nErr:noack,ret=%d",(uint16_t)ret);
#endif 
#ifdef _NOACK_TIMEOUT_10MS_
tempFwt = wgFWT;
wgFWT = 10;
#endif
ret = mifpro_noack(&outbuf[rptr]);
#ifdef _NOACK_TIMEOUT_10MS_
wgFWT = tempFwt;
#endif
//if((uint8_t)ret > 128){
if((uint16_t)ret > (uint16_t)0x8000){	
  //ret = mifpro_noack(&outbuf[rptr]);
	//if((uint8_t)ret > 128){
        #ifdef _DEBUG_MIFPRO_ICMD_ 		
        printk("\nErr:noack 2times,timeout");
        #endif
		return (uint8_t)-8;
	//}
	}
goto label_resp_process;	
}

//#define _DEBUG_mifpro_pps_
uint8_t mifpro_pps(uint8_t pps1,uint8_t *ppss)
{
	uint8_t ps;
	uint8_t ret;

  #ifdef _DEBUG_mifpro_pps_ 		
  printk("\nmifpro_pps:pps1[%02x]",(uint8_t)pps1);
  #endif	
	
	ret = rc_pps(bgCID,pps1,&ps);
	if(ret) return ret;
  *ppss = ps;
  #ifdef _DEBUG_mifpro_pps_ 		
  printk("\nmifpro_pps:ppss[%02x]",(uint8_t)ps);
  #endif  
  return 0;	
}

//#define _DEBUG_mifpro_set_speed_
void mifpro_set_speed(uint8_t tx_speed,uint8_t rx_speed)
{
  #ifdef _DEBUG_mifpro_pps_ 		
  printk("\nmifpro_set_speed:tx_speed[%02x],rx_speed[%02x]",(uint8_t)tx_speed,(uint8_t)rx_speed);
  #endif
	rc_set_speed(tx_speed,rx_speed);
	return;
}

/*
void mifpro_icmd_call_back_set(mifpro_icmd_func_type *p)
{
	mifpro_icmd_call_back = p;
	return;	
}
*/

//end of file
#endif