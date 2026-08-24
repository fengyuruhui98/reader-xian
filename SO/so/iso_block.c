//iso_block.c
//编制:邓建华
//时间:20090512

#ifndef _ISO_BLOCK_C_
#define _ISO_BLOCK_C_
//start of file

//mifpro_icmd_func_type *mifpro_icmd_call_back = NULL;

UBYTE bgCID;
UBYTE bgPCB;
UBYTE bgCIDFlag;
UWORD wgFWT = 5;//default 4.8ms
//UWORD wgFWT = 30;//default 4.8ms
UBYTE bgSFGI;
UBYTE bgWTX;
UWORD bgFSCI;	//2014/12/30 17:31:12
unsigned long lgSFGT;		//2015/1/16 14:40:11

#define CID_FLAG     0x08       //=0x08: 有CID
#define MAX_RF_BUF   60         //48-->64

/*=======================================================================================
函数:
功能:
入口参数:
inbuf:含PCB等
=========================================================================================*/
//#define _DEBUG_ISO_BLOCK_TRANSCEVE_
UWORD iso_block_transceve(UBYTE *inbuf,UWORD inbytes,UBYTE *outbuf,UWORD *outbytes,UWORD timeout)
{
//UBYTE  rece_bits;
UWORD rece_bits;
UWORD ptr;
UBYTE obuf[500],sendbytes;
UWORD obytes;
UWORD uIRQ,uFIFOLen;
UBYTE	uRcvByte, uPrimary;	
UBYTE gCollPos;
UWORD i;

#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
	printk("\ntimeout %d iso_block_transceve-->inbuf[%d]:",timeout, inbytes);
	for(i=0;i<inbytes;i++) {
		if((i%16) == 0) printk("\n");
		printk(" %02x",(UBYTE)inbuf[i]);
	}
#endif

	if(bgIsoType == 0){
		rc_crc_enable();
		rc_crc_sel_14443a();
		rc_oddpari_enable();
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

	//2014/12/30 13:06根据IC卡片接收能力发送，一次发送32个字节
	if(inbytes <= 60)
	{
		rc_write_data_n(inbuf,inbytes);
		//
		rc_clr_irq();
		//2018/6/8 12:14:32原设置寄存器方法
		rc_write_byte(REG_RC500_INTERRUPT_EN,(BIT_RXI|BIT_TIMERI)|0x80);
		//2019/8/28 12:26:50删除此指令BIT_IDLEI
		//rc_write_byte(REG_RC500_INTERRUPT_EN,(BIT_HIALERTI | BIT_TXI | BIT_RXI | BIT_TIMERI )|0x80);
		iso_block_set_time_out((timeout+4)/5);
		rc_write_byte(REG_RC500_COMMAND,RC500_CMD_TRANSCEIVE);
	}else
	{
		rc_write_byte(REG_RC500_COMMAND,RC500_CMD_TRANSCEIVE);
		
		rc_clr_irq();
		rc_write_data_n(inbuf, 48);//
		rc_write_byte(REG_RC500_INTERRUPT_EN,(BIT_HIALERTI | BIT_TXI | BIT_RXI | BIT_TIMERI)|0x80);
		iso_block_set_time_out((timeout+4)/5);
		sendbytes = 48;
label_send_process:
		while(1){
			uFIFOLen = rc_read_byte(REG_RC500_FIFO_LENGTH);
			if(uFIFOLen < 32)
				break;
			//if((rc_read_byte(REG_RC500_INTERRUPT_RQ)&0x02) != 0x02)	break;
		};
		if((inbytes - sendbytes) > 32)
		{
			rc_write_data_n(&inbuf[sendbytes], 32);
			
			sendbytes += 32;
			goto label_send_process;
		}else
		{
			rc_write_data_n(&inbuf[sendbytes], inbytes - sendbytes);
		}
	}
	
	//发送数据是否完成
	do_gettimeofday(&rfStartTV);
	while(1)
	{
	  	if((rc_read_byte(REG_RC500_INTERRUPT_RQ)&0x10) == 0x10)	break;
		//if(0 == rc_read_byte(REG_RC500_FIFO_LENGTH)) break;
		//if((rc_read_byte(REG_RC500_PRIMARY_STATUS) & 0x70) > 0x30) break;
		//2016/11/17 15:08:09
		do_gettimeofday(&rfEndTV);
		if( ((rfEndTV.tv_sec - rfStartTV.tv_sec) > 0) || ((rfEndTV.tv_usec - rfStartTV.tv_usec) > timeout * 1000) )
		{
			rc_clr_time_out();
			//printk("  send %02x\n", timeout);
			return (UWORD)-10;
		}
	}
	
	
	//sh_us_delay(1000);
	//
	rece_bits = 0;
	ptr = 0;
	do_gettimeofday(&rfStartTV);
	while(1) 
	{
		uIRQ = rc_read_byte(REG_RC500_INTERRUPT_RQ);
		//接收是否超时
		if(uIRQ & 0x20)
		{
			rc_clr_time_out();
			#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
				printk("\ntimeout iso_block_transceve-->INTERRUPT_RQ=%02x",(UBYTE)uIRQ);
			#endif
	    	return 	(UWORD)-1;
		}
		//HiAlert是否置1
		if(uIRQ & 0x02)
		{
			uFIFOLen = rc_read_byte(REG_RC500_FIFO_LENGTH);
			if(uFIFOLen > 0)
			{
				rece_bits = rc_rece_bits();
				obytes = rece_bits / 8;
				rc_read_data_n(&obuf[ptr],obytes);
				//#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
				//	printk("\niso_block_transceve-->rece bytes[%d],ptr[%d]",obytes,ptr);
				//	for(i=0;i<obytes;i++) {
				//		if((i%16) == 0) printk("\n");
				//		printk(" %02x",(UBYTE)obuf[ptr+i]);
				//	}
				//#endif
				ptr	= ptr + obytes;
			}
			#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
				printk("\nHiAlert iso_block_transceve-->INTERRUPT_RQ=%02x fifolen %d",(UBYTE)uIRQ, uFIFOLen);
			#endif
		}
		//if( !rc_is_in_receive() )
		//	continue;
		//printk("\nREG_RC500_PRIMARY_STATUS %02x", rc_read_byte(REG_RC500_PRIMARY_STATUS));
		//接收数据是否结束
		if(uIRQ & 0x08) 
		{
			uPrimary = rc_read_byte(REG_RC500_PRIMARY_STATUS);
			if( ((uPrimary & 0x70) == 0x70) || ((uPrimary & 0x70) == 0x60) )
			{
				#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
					printk("\nconinute iso_block_transceve-->INTERRUPT_RQ=%02x",(UBYTE)uIRQ);
				#endif
				continue;
			}
			rc_clr_time_out();
			#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
				printk("\nfinished iso_block_transceve-->INTERRUPT_RQ=%02x -->primary_status=%02x",(UBYTE)uIRQ, uPrimary);
			#endif
			break;
		}
		//do_gettimeofday(&rfEndTV);
		//if( ((rfEndTV.tv_sec - rfStartTV.tv_sec) > 0) || ((rfEndTV.tv_usec - rfStartTV.tv_usec) > timeout * 1000) )
		//{
		//	rc_clr_time_out();
		//	printk("iso_block_transceve timeout %d\n", timeout);
		//	return (UWORD)-1;
		//}
		//usleep(1*1000);
	}


	gCollPos = 0x00;
	uRcvByte = rc_read_byte(REG_RC500_ERROR_FLAG);

	#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
		printk("\niso_block_transceve-->ERROR_FLAG=%02x",(UBYTE)uRcvByte);
	#endif

	uRcvByte = uRcvByte & 0x1F; 
	if(uRcvByte)
	{
		if(uRcvByte & BIT_COLL_ERR) 
		{
			gCollPos = rc_read_byte(REG_RC500_COLL_POS); // read collision position
			return  (UWORD)-2;//STATUS_COLLISION_ERROR;              //Collision Error
		}
		if(uRcvByte & BIT_PARITY_ERR) 
			return  (UWORD)-3;//STATUS_PARITY_ERROR;                 //Parity Error
	
		if(uRcvByte & BIT_FIFO_OVERFLOW)
		{
			rc_flush_fifo();
			return	(UWORD)-4;//STATUS_BUFFER_OVERFLOW;              //BufferOverflow Error
		}
		if(uRcvByte & BIT_FRAMING_ERR) 
		{
#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
			printk("\niso_block_transceve-->ERROR_FLAG=%02x",(UBYTE)uRcvByte);
#endif
			return  (UWORD)-5;//STATUS_FRAMING_ERROR;     
		}
		
		if(uRcvByte & BIT_CRC_ERR) 
			return  (UWORD)-6;//STATUS_CRC_ERROR;
	}
	 
#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
	printk("\niso_block_transceve-->ptr=%d",ptr);
#endif

	rece_bits = rc_rece_bits();
	if(rece_bits % 8) 
	{
#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
		printk("\niso_block_transceve-->Err:STATUS_BITCOUNT_ERROR,rece_bits=%d",(UBYTE)rece_bits);
#endif
		*outbytes=0;
		return  (UWORD)-7;//STATUS_BITCOUNT_ERROR;
	}

	obytes = rece_bits/8;
	if(obytes > 0)
	{			
		rc_read_data_n(&obuf[ptr],obytes);
		ptr	= ptr + obytes;	
	}

	if(ptr == 0)
	{
#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
		printk("\niso_block_transceve-->Err:STATUS_BITCOUNT_ZERO,ptr==0");
#endif
		*outbytes=0;
		return (UWORD)-8;
	}

	*outbytes = ptr;
	memcpy(outbuf, obuf, *outbytes);

#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
	printk("\niso_block_transceve-->outbuf[%d]:",*outbytes);
	for(i=0; i < *outbytes; i++) {
		if((i%16) == 0) printk("\n");
		printk(" %02x",(UBYTE)outbuf[i]);
	}
	printk("\n");
#endif

	return 0;
}

//2014/12/30 17:49:23
////UWORD iso_block_transceve(UBYTE *inbuf,UWORD inbytes,UBYTE *outbuf,UWORD *outbytes,UWORD timeout)
////{
////	
////UWORD i;
////UBYTE rece_bits;
////UBYTE rbytes,ptr=0,sbytes;
//////UWORD i;
////
////#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
////	printk("\niso_block_transceve-->inbuf[%d]:",inbytes);
////	for(i=0;i<inbytes;i++) {
////		if((i%16) == 0) printk("\n");
////		printk(" %02x",(UBYTE)inbuf[i]);
////	}
////#endif
////
////if(bgIsoType == 1){
////  return 1;
////  }
////rc_clr_crypt();
////rc_set_bitframe(0);
////rc_idle();
////rc_flush_fifo();
////rc_clr_time_out();
//////
//////
////rc_clr_irq();
////rc_write_byte(REG_RC500_INTERRUPT_EN,(BIT_RXI|BIT_TIMERI)|0x80);
////iso_block_set_time_out((timeout+4)/5);
////rc_write_byte(REG_RC500_COMMAND,RC500_CMD_TRANSCEIVE);
////if(inbytes > 64) sbytes = 64;
////else sbytes = inbytes;	
////rc_write_data_n(inbuf,sbytes);
////ptr += sbytes;
////inbytes -= sbytes;
////while(inbytes){
////  rbytes = rc_read_byte(REG_RC500_FIFO_LENGTH);
////  if(rbytes < 32){
////     if(inbytes > 32) sbytes = 32;
////     else sbytes = inbytes;	
////  	 rc_write_data_n(&inbuf[ptr],sbytes);
////  	 ptr += sbytes;
////  	 inbytes -= sbytes;
////  	 }	
////	}
//////
/////*if(mifpro_icmd_call_back != NULL) {
////	mifpro_icmd_call_back();
////	mifpro_icmd_call_back = NULL;
////}*/
//////
////ptr = 0;
////for(i=0;(UWORD)i<(UWORD)40000;i++){
////  if(rc_read_byte(REG_RC500_PRIMARY_STATUS)&0x08) break;
////  if(rc_is_in_receive()){	
////	   rbytes = rc_read_byte(REG_RC500_FIFO_LENGTH);
////     if(rbytes>=24){
////  	     rc_read_data_n(&outbuf[ptr],rbytes);
////  	     ptr += rbytes;
////  	     }
////  	  }
////  else{
////      delay_us(5);
////  	  }	     
////  //watchdog();	
////  }
////rc_clr_time_out();  
//////
////if(!(rc_read_byte(REG_RC500_INTERRUPT_RQ)&BIT_RXI)) return (UBYTE)-1;
////if(rc_read_byte(REG_RC500_ERROR_FLAG)&0x0f) return (UBYTE)-2;
////
////rece_bits = rc_rece_bits();
////if(rece_bits%8) return (UBYTE)-3;
////
////*outbytes = rece_bits/8;
////rc_read_data_n(&outbuf[ptr],*outbytes);
////*outbytes = *outbytes+ptr;
//// 
////#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
////	printk("\niso_block_transceve-->outbuf[%d]:",*outbytes);
////	for(i=0;i<*outbytes;i++) {
////		if((i%16) == 0) printk("\n");
////		printk(" %02x",(UBYTE)outbuf[i]);
////	}
////#endif
//// 
////return 0;
//// 
////}

/*
//UWORD iso_block_transceve(UBYTE *inbuf,UWORD inbytes,UBYTE *outbuf,UWORD *outbytes,UWORD timeout)
//{
////UBYTE  rece_bits;
//UWORD rece_bits;
//UWORD ptr;
//UBYTE rbytes;
//UWORD i;
//
//#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
//printk("\niso_block_transceve-->inbuf[%d]:",inbytes);
//for(i=0;i<inbytes;i++) {
//	if((i%16) == 0) printk("\n");
//	printk(" %02x",(UBYTE)inbuf[i]);
//}
//#endif
//
//if(bgIsoType == 0){
////  rc_crc_enable();
////  rc_crc_sel_14443a();
////  rc_oddpari_enable();
//  }
//else{
//	rc_crc_enable();
//  rc_crc_sel_14443b();
//  rc_pari_disable();
//	}   
//rc_clr_crypt();
//rc_set_bitframe(0);
//rc_idle();
//rc_flush_fifo();
//rc_clr_time_out();
////
//rc_write_data_n(inbuf,inbytes);
////
//rc_clr_irq();
//rc_write_byte(REG_RC500_INTERRUPT_EN,(BIT_RXI|BIT_TIMERI)|0x80);
//iso_block_set_time_out((timeout+4)/5);
//rc_write_byte(REG_RC500_COMMAND,RC500_CMD_TRANSCEIVE);
////
//for(i=0;(UWORD)i<(UWORD)6000;i++){
//  if(rc_read_byte(REG_RC500_PRIMARY_STATUS)&0x08) break;
//  if(rc_is_in_receive()){	
//	   rbytes = rc_read_byte(REG_RC500_FIFO_LENGTH);
//     if(rbytes>=8){
//  	     rc_read_data_n(&outbuf[ptr],rbytes);
//  	     ptr += rbytes;
//  	     }
//  	  }
//  else{
//      delay_us(50);
//  	  }	     
//  //watchdog();	
//}
//rc_clr_time_out();  
////
//if(!(rc_read_byte(REG_RC500_INTERRUPT_RQ)&BIT_RXI)) return (UWORD)-1;
//if(rc_read_byte(REG_RC500_ERROR_FLAG)&0x0f) return (UWORD)-2;
//
//rece_bits = rc_rece_bits();
//if(rece_bits%8) return (UWORD)-3;
//
//  //*outbytes = rece_bits/8;
//rc_read_data_n(&outbuf[ptr],*outbytes);
//*outbytes = *outbytes+ptr;
//
//#ifdef _DEBUG_ISO_BLOCK_TRANSCEVE_
//printk("\niso_block_transceve-->outbuf[%d]:",*outbytes);
//for(i=0;i<*outbytes;i++) {
//	if((i%16) == 0) printk("\n");
//	printk(" %02x",(UBYTE)outbuf[i]);
//}
//#endif
// 
//return 0;
//}
*/

/*=============================================================================================
函数：iso_block_set_time_out
功能：设置超时
入口参数:
cnt_5ms: 以5ms为单位的数值
===============================================================================================*/
void iso_block_set_time_out(UBYTE cnt_5ms)
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
UWORD _mifpro_ats(UBYTE cid,UBYTE *obuf,UWORD *obytes)
{
UBYTE inbuf[16],inbytes;
UWORD ret;
UBYTE fwi,i, j, fsci;

	wgFWT=5;
	bgSFGI=0;
	bgCIDFlag=0;//默认不支持cid 2021/5/6 14:38
	bgCID = 0;
	bgPCB = 1;
	bgFSCI = 48;
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
			printk("%02X ",(UBYTE)obuf[i]);	
		}
	}
	else{
	  printk("\nErr:ats,ret=%d",ret);
	} 	
#endif

	if(ret!=0) return ret;
	//2014/12/30 12:49 pei
	//TL  T0 (TA TB TC) T1 ~ Tk
	//T0 b8 b7 b6 b5 b4 b3 b2 b1 
	//   0  TC TB TA ---FSCI----
	fsci = obuf[1] & 0xF;
	switch(fsci)
	{
	case 0:
		bgFSCI = 16;
		break;
	case 1:
		bgFSCI = 24;
		break;
	case 2:
		bgFSCI = 32;
		break;
	case 3:
		bgFSCI = 40;
		break;
	case 4:
		bgFSCI = 48;
		break;
	case 5:
		bgFSCI = 64;
		break;
	case 6:
		bgFSCI = 96;
		break;
	case 7:
		bgFSCI = 128;
		break;
	case 8:
		bgFSCI = 256;
		break;
	default:
		bgFSCI = 256;
		break;
	}
	//check TA TB TC
	if((obuf[1]&0x70) == 0) return 0;
	//from TA TB TC
	i = 2;
	if(obuf[1]&0x10) i++;	
	//TB  b8 b7  b6 b5 b4 b3 b2 b1 
	//    ---FWI-----  ----SFGI---
	bgSFGI = obuf[i]&0x0f;
	if(bgSFGI == 15)
		bgSFGI = 0;
	
	lgSFGT = 1;
	j = 0;
	while(j < bgSFGI){
		lgSFGT *= 2;
		j++;
	}
	lgSFGT = lgSFGT * 302;		//us
	//
	fwi = (UBYTE)(obuf[i]&0xf0)/16;
	//
	if(obuf[1]&0x20) i++;
	//TC b8 b7 b6 b5 b4 b3 b2 b1
	//   ------000000----- 1   0
	if(obuf[1]&0x40){	
		if(obuf[i]&0x02) {
			bgCIDFlag=1;
			bgCID = cid;
		}
	}
	//
	wgFWT = 1;i=0;
	while(i<fwi){
		wgFWT *= 2;
		i++;
	}
	wgFWT = (wgFWT*3)/10;	
	if( wgFWT < 10)
		wgFWT = 10;
	do_gettimeofday(&atsTV);

#ifdef _DEBUG_MIFPRO_ATS_
	printk("\nSFGI=%d,FWT=%d,CID_FLAG=%d\n",bgSFGI, wgFWT, bgCIDFlag);
	printk("\nlgSFGT=%d s= %d us=%d\n", lgSFGT, atsTV.tv_sec, atsTV.tv_usec);
#endif
	//2015/8/4 10:30:58 PEI
	//2020/10/14 14:49:57 杭州充值无返回，超时时间修改为300ms
	if( wgFWT > 300)
		wgFWT = 300;

	return 0;	
}	






/*==================================================================================================
函数:mifpro_deselect
功能:
====================================================================================================*/
//#define _DEBUG_MIFPRO_DESELECT_
UWORD _mifpro_deselect(UBYTE *outbuf)
{
UWORD ret;
UBYTE inbuf[4];
UWORD inbytes,outbytes;
#ifdef _DEBUG_MIFPRO_DESELECT_
UWORD i;
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
		printk("\nErr:mifpro_deselect,ret=%d",(UWORD)ret);
		#endif
		return ret;
	}

#ifdef _DEBUG_MIFPRO_DESELECT_
	printk("\nOK:mifpro_deselect,outbytes=%d",(UWORD)outbytes);
	for(i=0;i<outbytes;i++){
		 if(i%16 == 0) printk("\n");
		 printk("%02X ",(UBYTE)outbuf[i]);
	}
#endif
	return outbytes;	
}	



/*==================================================================================================
函数:mifpro_deselect
功能:
====================================================================================================*/
//#define _DEBUG_MIFPRO_WTX_
UWORD mifpro_wtx(UBYTE *outbuf)
{
UWORD ret;
UBYTE inbuf[4];
UWORD inbytes,outbytes;
#ifdef _DEBUG_MIFPRO_WTX_
UWORD i;
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
   printk("\nErr:mifpro_wtx,ret=%d",(UWORD)ret);
   #endif
	 return ret;
   }

#ifdef _DEBUG_MIFPRO_WTX_
printk("\nOK:mifpro_wtx,outbytes=%d",(UWORD)outbytes);
for(i=0;i<outbytes;i++){
	 if(i%16 == 0) printk("\n");
	 printk("%02X ",(UBYTE)outbuf[i]);
	 }
#endif
return outbytes;	
}	



/*==============================================================================
函数：mifpro_noack
功能：
================================================================================*/
//#define _DEBUG_MIFPRO_NOACK_
UWORD mifpro_noack(UBYTE *outbuf)
{
UWORD outbytes,inbytes;
UWORD ret;
UBYTE inbuf[3];
#ifdef _DEBUG_MIFPRO_NOACK_
UWORD i;
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
   		printk("\nErr:mifpro_noack,ret=%d",(UWORD)ret);
#endif
		return ret;
	}

#ifdef _DEBUG_MIFPRO_NOACK_
	printk("\nOK:mifpro_noack,outbytes=%d",(UWORD)outbytes);
	for(i=0;i<outbytes;i++){
		if(i%16 == 0) printk("\n");
		printk("%02X ",(UBYTE)outbuf[i]);
	}
#endif
	
	return outbytes;
}	 

/*===============================================================================
函数：
功能：
=================================================================================*/
//#define _DEBUG_MIFPRO_ACK_
UWORD mifpro_ack(UBYTE *outbuf)
{
UWORD outbytes,inbytes;
UWORD ret;
UBYTE inbuf[3];
#ifdef _DEBUG_MIFPRO_ACK_
UWORD i;
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
		printk("\nErr:mifpro_ack,ret=%d",(UWORD)ret);
#endif
		return (UWORD)ret;
	}

#ifdef _DEBUG_MIFPRO_ACK_
	printk("\nOK:mifpro_ack,outbytes=%d",(UWORD)outbytes);
	for(i=0;i<outbytes;i++){
		if(i%16 == 0) printk("\n");
		printk("%02X ",(UBYTE)outbuf[i]);
	}
#endif
	return outbytes;
}	 



/*===============================================================================
函数：mifpro_icmd_nochain
功能：
=================================================================================*/
//#define _DEBUG_MIFPRO_ICMD_NOCHAIN_
UWORD mifpro_icmd_nochain(UWORD len,UBYTE *inbuf,UBYTE *outbuf)
{
UWORD outbytes,inbytes;
UBYTE buf[500];
UWORD ret;
#ifdef _DEBUG_MIFPRO_ICMD_NOCHAIN_
UWORD i;
#endif
	
#ifdef _DEBUG_MIFPRO_ICMD_NOCHAIN_
	printk("\nmifpro_icmd_nochain,cmd len=%d",len);
	for(i=0;i<len;i++){
	   if(i%16 == 0) printk("\n");
	   printk("%02X ",(UBYTE)inbuf[i]);	
	}  
#endif


	//if((UWORD)len > (UWORD)MAX_RF_BUF) return (UWORD)-1;
	if((UWORD)len > (UWORD)bgFSCI) return (UWORD)-1;

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
		printk("\nErr:mifpro_icmd_nochain,ret=%d",(UWORD)ret);
#endif
		return (UWORD)ret;
	}



#ifdef _DEBUG_MIFPRO_ICMD_NOCHAIN_
	printk("\nOK:mifpro_icmd_nochain,outbytes=%d",(UWORD)outbytes);
	for(i=0;i<outbytes;i++){
		 if(i%16 == 0) printk("\n");
		 printk("%02X ",(UBYTE)outbuf[i]);
	}
#endif
	return outbytes;
}	

/*===================================================================================
函数：mifpro_icmd_chain
功能：
=====================================================================================*/
//#define _DEBUG_MIFPRO_ICMD_CHAIN_
UWORD mifpro_icmd_chain(UWORD len,UBYTE *inbuf,UBYTE *outbuf)
{
UWORD outbytes,inbytes;
UBYTE buf[500];
UWORD ret;
#ifdef _DEBUG_MIFPRO_ICMD_CHAIN_
UWORD i;
#endif

#ifdef _DEBUG_MIFPRO_ICMD_CHAIN_
	printk("\nmifpro_icmd_chain,cmd len=%d",len);
	for(i=0;i<len;i++){
		if(i%16 == 0) printk("\n");
		printk("%02X ",(UBYTE)inbuf[i]);	
	}  
#endif

	//2014/12/30 13:03
	//if((UWORD)len > (UWORD)MAX_RF_BUF) return (UWORD)-1;
	if((UWORD)len > (UWORD)bgFSCI) return (UWORD)-1;
	
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
			printk("\nErr:mifpro_icmd_chain,ret=%d",(UWORD)ret);
		#endif
		//printk("   err exit\n");
		return (UWORD)ret;
	   }
	
	#ifdef _DEBUG_MIFPRO_ICMD_CHAIN_
		printk("\nOK:mifpro_icmd_chain,outbytes=%d",(UWORD)outbytes);
		for(i=0;i<outbytes;i++){
			 if(i%16 == 0) printk("\n");
			 printk("%02X ",(UBYTE)outbuf[i]);
			 }
	#endif
	//printk("      exit\n");
	return outbytes;
}	


/*==================================================================================
函数：mifpro_icmd
功能：
====================================================================================*/
//#define _DEBUG_MIFPRO_ICMD_
//#define _NOACK_TIMEOUT_10MS_
UBYTE _mifpro_icmd(UBYTE *ibuf,UWORD ibytes,UBYTE *obuf,UWORD *obytes)
{
UWORD slen;
UWORD  ret;
UWORD rptr,sptr;
UWORD i, noack; //2014/12/30 17:38:32
UBYTE outbuf[400];
UWORD offset;
UBYTE last_pcb;

#ifdef _NOACK_TIMEOUT_10MS_
	UWORD tempFwt;
#endif
//2013/11/13 12:29:38
//	do_gettimeofday(&apduTV);
//	//printk("\ns= %d us=%d\n", apduTV.tv_sec, apduTV.tv_usec);
//	while( ((apduTV.tv_sec - atsTV.tv_sec) < (lgSFGT / 1000000)) || (((apduTV.tv_sec - atsTV.tv_sec) == 0) && ((apduTV.tv_usec - atsTV.tv_usec) < lgSFGT)) )
//	{
//		do_gettimeofday(&apduTV);
//	}
	//printk("\nlgSFGT=%d s= %d us=%d\n", lgSFGT, apduTV.tv_sec, apduTV.tv_usec);
#ifdef _DEBUG_MIFPRO_ICMD_ 
	printk("\nmifpro_icmd,ibytes[%d]",(UWORD)ibytes);
	for(i=0;i<ibytes;i++) {
		if((i%16) == 0) printk("\n");
		printk(" %02x",(UBYTE)ibuf[i]);
	}
#endif

	noack = 0;
	sptr = 0;rptr = 0;
	if(bgCIDFlag) offset=2;
	else offset=1;	
		
label_send_loop:
//if(ibytes > MAX_RF_BUF){
//    slen = MAX_RF_BUF;
//    ret = mifpro_icmd_chain(slen,&ibuf[sptr],&outbuf[rptr]); 
//    last_pcb = bgPCB;
//    }
//else{
//	  slen = ibytes;
//	  ret = mifpro_icmd_nochain(slen,&ibuf[sptr],&outbuf[rptr]); 
//	  }   
//2014/12/30 13:24
	if(ibytes > bgFSCI){
	    slen = bgFSCI;
	    ret = mifpro_icmd_chain(slen,&ibuf[sptr],&outbuf[rptr]); 
	    last_pcb = bgPCB;
	}
	else{
		slen = ibytes;
		ret = mifpro_icmd_nochain(slen,&ibuf[sptr],&outbuf[rptr]);
	}   


label_resp_process:
#ifdef _DEBUG_MIFPRO_ICMD_ 
	printk("\nresp_process,ret=%d,offset=%d,rptr=%d",(int)ret,offset,(UWORD)rptr);
	if((UWORD)ret <= (UWORD)0x200) {
		for(i=0;i<ret;i++) {
			if((i%16) == 0) printk("\n");
			printk(" %02x",(UBYTE)outbuf[rptr+i]);
		}
	}
#endif	   
	//if((UBYTE)ret > 128)  goto label_noack;
	if((UWORD)ret > (UWORD)0x8000)  goto label_noack;	
	if(ret < offset) goto label_abnormal;
	
	//rece no chain
	if((outbuf[rptr]&I_BLOCK_MASK) == I_BLOCK_NO_CHAIN){
#ifdef _DEBUG_MIFPRO_ICMD_
		printk("\nRece no chain");
#endif 
		for(i=0;i<(ret-offset);i++) outbuf[rptr+i] = outbuf[rptr+i+offset];
		rptr += (ret-offset);
		if((UWORD)rptr > 300) return (UBYTE)-3; 
		
		memcpy(obuf,outbuf,rptr);
		*obytes = rptr;	
#ifdef _DEBUG_MIFPRO_ICMD_ 
		printk("\nmifpro_icmd,obytes[%d]",(UWORD)rptr);
		for(i=0;i<rptr;i++) {
			if((i%16) == 0) printk("\n");
			printk(" %02x",(UBYTE)obuf[i]);
		}
		printk("\n%s\n", __TIME__);
#endif
		
   		return 0;
	}

	//rece chain
	if((outbuf[rptr]&I_BLOCK_MASK) == I_BLOCK_CHAIN){
#ifdef _DEBUG_MIFPRO_ICMD_
		printk("\nRece chain rptr %d offset %d\n", rptr, offset);
#endif 
   		for(i=0;i<(ret-offset);i++) outbuf[rptr+i] = outbuf[rptr+i+offset];
   		rptr += (ret-offset);
   		if((UWORD)rptr > 256) return (UBYTE)-4; 
   		//ack
   		ret = mifpro_ack(&outbuf[rptr]);
   		goto label_resp_process;
   	}

	//wtx 
	if((UBYTE)(outbuf[rptr]&WTX_BLOCK_MASK) == (UBYTE)WTX_BLOCK){
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
			return (UBYTE)-5;
		}	 	
		ibytes -= slen;
	 	goto label_send_loop;	
	}

	//其他块
#ifdef _DEBUG_MIFPRO_ICMD_
	printk("\nErr:unknown pcb %02X.",(UBYTE)outbuf[rptr]);
	printk("\n%s\n", __TIME__);
#endif
	return (UBYTE)-6;   

label_abnormal: 
#ifdef _DEBUG_MIFPRO_ICMD_ 
		printk("\nErr:abnormal,ret=%d",(UBYTE)ret);
		printk("\n%s\n", __TIME__);
#endif 
	return (UBYTE)-7; 

label_noack:
	noack++;
#ifdef _DEBUG_MIFPRO_ICMD_ 
	printk("\nErr:noack,ret=%d",(UWORD)ret);
#endif 
#ifdef _NOACK_TIMEOUT_10MS_
	tempFwt = wgFWT;
	wgFWT = 10;
#endif
	
	ret = mifpro_noack(&outbuf[rptr]);

#ifdef _NOACK_TIMEOUT_10MS_
	wgFWT = tempFwt;
#endif
	//if((UBYTE)ret > 128){
	if((UWORD)ret > (UWORD)0x8000){	
		//ret = mifpro_noack(&outbuf[rptr]);
		//if((UBYTE)ret > 128){
#ifdef _DEBUG_MIFPRO_ICMD_ 		
        printk("\nErr:noack 2times,timeout");
        printk("\n%s\n", __TIME__);
#endif
        //2014/12/30 17:44:28
		if(noack < 2)
		{
			goto label_noack;
		}
		return (UBYTE)-8;
		//}
	}
	goto label_resp_process;	
}

//#define _DEBUG_mifpro_pps_
UBYTE mifpro_pps(UBYTE pps1,UBYTE *ppss)
{
	UBYTE ps;
	UBYTE ret;

  #ifdef _DEBUG_mifpro_pps_ 		
  printk("\nmifpro_pps:pps1[%02x]",(UBYTE)pps1);
  #endif	
	
	ret = rc_pps(bgCID,pps1,&ps);
	if(ret) return ret;
  *ppss = ps;
  #ifdef _DEBUG_mifpro_pps_ 		
  printk("\nmifpro_pps:ppss[%02x]",(UBYTE)ps);
  #endif  
  return 0;	
}

//#define _DEBUG_mifpro_set_speed_
UBYTE mifpro_set_speed(UBYTE tx_speed,UBYTE rx_speed)
{
	#ifdef _DEBUG_mifpro_pps_ 		
		printk("\nmifpro_set_speed:tx_speed[%02x],rx_speed[%02x]",(UBYTE)tx_speed,(UBYTE)rx_speed);
	#endif
	rc_set_speed(tx_speed,rx_speed);
	return 0;
}

void do_gettimeofday(struct timeval *TV)
{
struct timezone TZ;

	gettimeofday(TV, &TZ);

	return ;
}
//end of file
#endif

