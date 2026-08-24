#include <stdlib.h>
#include <sys/time.h>

#include "xdr_file_manage.h"
#include "bin_file_manage.h"
#include "hh_cpu_operation.h"
#include "xa_error_code.h"
#include "xa_sam.h"
#include "linux2440lib.h"


extern unsigned char cpu_19_data[64];

/*
function: select the file
*/
char CPU_select_file(char *sfi, char len, unsigned char *out_buf, unsigned char *out_len)
{
unsigned char buf[300], cpubuf[300];
unsigned short cpulen;
int ret, i;

	//select file
	memcpy(out_buf, "\xf0\x80", 2);
	memcpy(buf, "\x00\xa4\x00\x00\x02", 5);
	if(len != 2)
		buf[2] = 0x04;
	buf[4] = len;
	memcpy(&buf[5], sfi, len);
#ifdef DEBUG_PRINT
	PRINTK("select:");
	for(i = 0;i < 5 + len; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(buf, 5 + len, cpubuf, &cpulen);
#ifdef DEBUG_PRINT
	PRINTK("return %02x ", ret);
	for(i = 0; i < cpulen; i++) PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
#endif
	if(ret != 0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\xf0\x81", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		memcpy(out_buf, &cpubuf[cpulen - 2], 2);
		return CE_INVADLIDCARD;
	}
	if((cpulen - 2) > 0) 
		memcpy(out_buf, &cpubuf[0], cpulen - 2);
	if(out_len != NULL)
		*out_len = cpulen - 2;
#ifdef DEBUG_PRINT
	PRINTK("select file %02x%02x:len %02x ", sfi[0], sfi[1], cpulen);
	for(i = 0;i < cpulen; i++) PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
#endif
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif

	return 0;
}
/*
function:external authorization
	1 whether extern authorization or not
	2 psam index
	3 factor
	4 out buffer
*/
char CPU_externauth(char extern_auth_type, char sam_index, unsigned char *cpu_factor, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], factor[20], des[60], deslen;
unsigned char cpubuf[100], cpurandom[8];
unsigned short	cpulen;
	
	if(extern_auth_type == 0)
		return 0;
	//get cpu random
	memcpy(out_buf, "\xf0\x10", 2);
	memset(buf, 0x00, 40);
	memcpy(buf, "\x00\x84\x00\x00\x08", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("get cpu card random failure\n");
#endif
		return CE_READ;
	}
#ifdef DEBUG_2_PRINT
	PRINTK("get random :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf0\x11", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	memcpy(out_buf, "\xf0\x12", 2);
	//memcpy(factor, cpu_05_data, 8);
	memcpy(factor, cpu_factor, 8);
	if((ret = cpu_cal_dcmk(sam_index, NULL, factor, 8, 0, cpubuf, 8, des, &deslen)) != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("cpu cal mac return %d\n", ret);
#endif
		return CE_INVADLIDCARD;
	}
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	//external auth
	memset(buf, 0x00, 50);
	memcpy(buf, "\x00\x82\x00\x01\x08", 5);
	memcpy(&buf[5], des, 8);
#ifdef DEBUG_2_PRINT
	PRINTK("external auth :");
	for(i = 0; i < 13; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf0\x13", 2);
	if(mifpro_apdu(buf, 5 + 8, cpubuf, &cpulen) != 0)
		return CE_READ;
#ifdef DEBUG_2_PRINT
	PRINTK("external auth %02x %02x \n", cpubuf[0], cpubuf[1]);
#endif
	memcpy(out_buf, "\xf0\x14", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))
		return CE_INVADLIDCARD;
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	return 0;
}
/*
function:
	初始化符合应用消费交易
parameter:
	1、密钥索引号（1）
	2、交易金额（4）高字节在前\
	3、终端机编号（6）SAM卡号
return :
*/
char CPU_init_for_capp(char key_index, int transvalue, char *device_id, unsigned char *user_sn, unsigned char *city, unsigned char city_len, char *out_buf)
{
int ret;
unsigned char buf[80];
unsigned char cpubuf[300];
unsigned char i, timebuf[10];
unsigned short cpulen;

#ifdef	DEBUG_PRINT
struct timeval tv1,tv2;
struct timezone tz1,tz2;
unsigned long tv1_usec;
	
	gettimeofday(&tv1,&tz1);
	tv1_usec = tv1.tv_usec;
	PRINTK("initcapp  %d  %d\n", tv1.tv_sec, tv1.tv_usec/1000);
#endif
	/*
	memcpy(buf, "\x80\xfa\x00\x01\x08", 5);
	memcpy(&buf[5], ch_cpu20_logic_id, 8);
	if((ret = sam_apdu(xa_metro_psam_index, buf, 5 + 8, cpubuf, &cpulen, 0, 0)) != 0)
	{
		PRINTK("capp init consume key return %d \n", ret);
		return CE_INVADLIDCARD;
	}
	PRINTK("consume sub-key:\n");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))
		return CE_INVADLIDCARD;
	*/
	//initialize for capp
	memcpy(buf,"\x80\x50\x03\x02\x0B", 5);
	buf[3] = tpCPU.EDorEP;
	//key
	buf[5] = key_index;
	LongToByte(transvalue, &buf[6]);
	memcpy(&buf[10], device_id, 6);
	buf[16] = 0xf;
#ifdef DEBUG_PRINT
	PRINTK("capp init data:");
	for(i = 0; i < 17; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(buf, 17, cpubuf, &cpulen);
	if(ret != 0)
	{
#ifdef DEBUG_PRINT
		PRINTK("cpu initial capp failure\n");
#endif
		return CE_READ;
	}
	//if((cpubuf[ret - 2] != 0x90) || (cpubuf[ret - 1] != 0x00))
#ifdef DEBUG_PRINT
	PRINTK("capp init return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if((cpubuf[cpulen - 2] == 0x94) && (cpubuf[cpulen - 1] == 0x01))
	{
		return CE_ENOUGH_BALANCE;
	}
	if(cpulen != 17)
	{
		return CE_INVADLIDCARD;
	}
#ifdef DEBUG_TIME
	memcpy(timebuf, "\xf0\xf0\x00", 3);
	ReaderResponse(csc_comm, ERR_OK, 0xF0, timebuf, 2);
#endif	
	//old balnce(4) sn(2) overdraft(3) key version(1) al(1) random(4)
	memcpy(capp_init, cpubuf, 15);

	//cal maC1:─交易类型标识/交易金额/终端机编号/交易日期（终端）/交易时间（终端）
	memset(buf, 0x00, 80);
	//random
	memcpy(&buf[0], &capp_init[11], 4);
	//transaction sn
	memcpy(&buf[4], &capp_init[4], 2);
	//transaction amount
	LongToByte(transvalue, &buf[6]);
	//transaction type-for ed is 0x09 and for ep is 0x0a;
	buf[10] = tpCPU.capp_type ;
	//date time bcd
	memcpy(&buf[11], tpCPU.time_bcd, 7);
	//key version &
	buf[18] = capp_init[9];
	buf[19] = capp_init[10];
	//logic id
	memcpy(&buf[20], user_sn, 8);
	//city code
	memcpy(&buf[28], city, city_len);
	memcpy(out_buf, buf, 0x24);

#ifdef	DEBUG_PRINT	
	gettimeofday(&tv2,&tz2);
	PRINTK("capp %ds %duc %ds  %dms tv1 %d\n", tv2.tv_sec, tv2.tv_usec/1000,(tv2.tv_sec-tv1.tv_sec), (tv1_usec - tv2.tv_usec)/1000, tv1_usec);
#endif
	/*test
	if((ret = cpu_cal_mac1(xa_metro_psam_index, buf, 0x24, cpubuf)) != 0)
	{
		PRINTK("cpu cal mac1 return %d\n", ret);
		return CE_INVADLIDCARD;
	}
	//psam sn & mac1
	memcpy(mac1, cpubuf, 8);
#ifdef DEBUG_TIME
	memcpy(timebuf, "\xf0\xf1\x00", 3);
	ReaderResponse(csc_comm, ERR_OK, 0xF0, timebuf, 2);
#endif	
	*/
	return 0;
}
/*************************************************************
function:UPDATE CAPP DATA CACHE
parameter:
	1、thread
	2、SFI
	3 record index
	4 length
	5 
*************************************************************/
char CPU_update_capp(char thread_id, unsigned char SFI_index, unsigned char rec_index, unsigned char len, unsigned char *rec_buf, unsigned char cycleflag)
{
int ret, i;
unsigned char	buf[300], cpubuf[300];
unsigned short cpulen;
unsigned char 	des[50], callen, cpurandom[8], factor[20];

	//cal mac by using the thread
	if(thread_id)
	{
		sem_init(&g_samreturn, 0, 0);
		ch_mac_sel = tpCPU.thread_mac1;
		sem_post(&g_samcalwait);
	}
	//line procted mac
/*	memset(buf, 0x00, 40);
	memcpy(buf, "\x00\x84\x00\x00\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		PRINTK("get cpu card random failure\n");
		return CE_INVADLIDCARD;
	}
#ifdef DEBUG_PRINT
	PRINTK("get random :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
	memset(cpurandom, 0x00, 8);
	memcpy(cpurandom, cpubuf, 4);
*/
	//update record
	memset(buf, 0x00, 300);
	memcpy(buf, cpurandom, 8);
	memcpy(&buf[8], "\x80\xdc", 2);
	buf[10] = rec_index;
	buf[11] = (SFI_index << 3);
	if(cycleflag)
	{
		buf[8] = 0;
		buf[10] = 0;
		buf[11] = (SFI_index << 3) | 0x03;
	}
	//buf[12] = len + 4;
	buf[12] = len;
	memcpy(&buf[13], rec_buf, len);
	if(((5 + len) % 8) != 0)
		buf[13 + len] = 0x80;
	callen = ((5 + 8 + len) - (5 + 8 + len) % 8) / 8 + 1;
#ifdef DEBUG_PRINT
	PRINTK("capp update before:%d ", callen);
	for(i = 0; i < callen * 8 ; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
/*	memset(factor, 0x00, 8);
	memcpy(factor, ch_cpu20_logic_id, 8);
	memcpy(&factor[8], "\x21\x50\x80", 3);
	if((ret = cpu_cal_protect_mac(factor, 16, "\x00\x00", buf, callen * 8, des)) != 0)
	{
		PRINTK("line mac return %d\n", ret);
		return CE_INVADLIDCARD;
	}
	memcpy(&buf[8 + 5 + len], des, 4);
#ifdef DEBUG_PRINT
	PRINTK("capp line protected:");
	for(i = 8; i < 5 + 12 + len; i++) PRINTK("%02x ", buf[i]);	
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))

	PRINTK("\n");
#endif
	*/
	ret = mifpro_apdu(&buf[8], 5 + len , cpubuf, &cpulen);
	if(ret != 0)
	{
#ifdef DEBUG_PRINT
		PRINTK("capp update failure\n");
#endif		
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("capp update return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif	
	if((cpubuf[cpulen - 2]!= 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	return 0;
}
/*************************************************************
function:debit CAPP purchase
	更新缓存到ic卡
parameter:
	1、SFI
	2、
*************************************************************/
char CPU_debit_for_capppurchase(unsigned char *rollback, cpu_proc_callback proc, unsigned char *out_buf)
{
int ret;
unsigned char buf[80];
unsigned char cpubuf[80];
unsigned short cpulen;
unsigned char mac2[4],i;
	
	/*if((ret = cpu_cal_mac1(xa_metro_psam_index, ch_cpu_mac_data, 0x24, cpubuf)) != 0)
	{
		PRINTK("cpu cal mac1 return %d\n", ret);
		return CE_INVADLIDCARD;
	}
	//psam sn & mac1
	memcpy(mac1, cpubuf, 8);
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, buf, 2);
#endif*/
	//wait calculate mac1 return
#ifdef DEBUG_PRINT
	struct timeval tv1,tv2;
	struct timezone tz1,tz2;
	unsigned long tv1_usec;
	gettimeofday(&tv1,&tz1);
	PRINTK("debit  %d  %d\n", tv1.tv_sec, tv1.tv_usec/1000);
#endif

	sem_wait(&g_samreturn);

#ifdef DEBUG_PRINT
	gettimeofday(&tv2,&tz2);
	PRINTK("debit  %d  %d\n", tv2.tv_sec, tv2.tv_usec/1000);
	PRINTK("wait sam calculate mac return %02x\n", mac_ret);
#endif

	if(mac_ret != 0)
		return CE_WRITE;

	memcpy(buf,"\x80\x54\x01\x00\x0F",5);
	//psam sn
	memcpy(&buf[5], mac1, 4);
	memcpy(tpCPU.sam_sn, mac1, 4);
	//date time bcd
	memcpy(&buf[9], tpCPU.time_bcd, 7);
	//mac1
	memcpy(&buf[16], &mac1[4], 4);
#ifdef DEBUG_PRINT
	PRINTK("debit data:");
	for(i = 0; i < 20; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(buf, 20, cpubuf, &cpulen);
#ifdef DEBUG_PRINT
	PRINTK("cpu debit return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif	
	if(ret != 0)
	{
#ifdef DEBUG_2_PRINT
		PRINTK("debit failure ret %02x\n", ret);
#endif
		if(proc != NULL)
		{
			(*proc)(0xff);
		}
		return CE_WRITE;
	}
	//ee write 
	*rollback = 0;
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	memcpy(tpCPU.tac, cpubuf, 4);
	memcpy(mac2, &cpubuf[4], 4);
	//using thread to check the mac2
	memcpy(ch_cpu_mac_data, mac2, 4);
	ch_mac_sel = tpCPU.thread_mac2;
	sem_post(&g_samcalwait);
	//blnCalMAC2 = 0xff;
	/*credit sam for purchase
	memcpy(buf, mac2, 4);
	if(cpu_cal_mac2(buf, 4) != 0)
		return CE_INVADLIDCARD;
	*/
	return 0;
}
/*
function:initilize the credit
*/
char CPU_init_for_credit(int transvalue, unsigned char *deviceid, unsigned char *out_buf)
{
int ret;
unsigned char buf[80];
unsigned char cpubuf[50];
unsigned short cpulen;
unsigned char i, timebuf[10];
	
	//verify pin
	memcpy(out_buf, "\xf1\x00", 2);
	memcpy(buf, "\x00\x20\x00\x00\x02\x12\x34", 7);
	if((ret = mifpro_apdu(buf, 7, cpubuf, &cpulen)) != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("verify pin return %d\n", ret);
#endif
		return CE_WRITE;
	}
	memcpy(out_buf, "\xf1\x01", 2);
#ifdef DEBUG_PRINT
	PRINTK("len: %02x%02x", cpulen);
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
	
#ifdef DEBUG_PRINT
	PRINTK(" pin ret: %02x%02x\n", cpubuf[cpulen -2], cpubuf[cpulen - 1]);
#endif
	//load initial
	memset(buf, 0x00, 40);
	memcpy(buf, "\x80\x50\x00\x02\x0b", 5);
	buf[5] = 0x01;
	LongToByte(transvalue, &buf[6]);
	//
	memcpy(&buf[10], deviceid, 6);
	
	buf[16] = 0x10;
#ifdef DEBUG_PRINT
	PRINTK("load initial:");
	for(i = 0; i < 16; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf1\x02", 2);
	if((ret = mifpro_apdu(buf, 16, cpubuf, &cpulen)) != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("load command return %d\n", ret);
#endif
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("load initial return:len %02x-%02x%02x \n", cpulen, cpubuf[0], cpubuf[1]);
#endif
	memcpy(out_buf, "\xf1\x03", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
	
#ifdef DEBUG_PRINT
	PRINTK("load return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif	

	memcpy(capp_init, cpubuf, 16);
	return 0;
}


void output_binary_trace(char *pStrTraceTitle, unsigned char* pBytesOutput, unsigned int dataLength)
{
char buffer[256];
int i_loop;
	
	sprintf(buffer, "%s\nPointer Address:0x%08X DataLength:%d", pStrTraceTitle, (unsigned int)pBytesOutput, dataLength);
	//PRINTK(buffer);
	for(i_loop = 0; i_loop<dataLength; i_loop++)
	{
		if(i_loop%16)
		{
			if(!(i_loop%4))
			{
				PRINTK(" ", i_loop/16);
			}
		}
		else
		{
			PRINTK("\n%02d: ", i_loop/16);
		}
		PRINTK("%02X", *pBytesOutput++);
	}
	PRINTK("\n");
}

//8	用户卡取认证码	用户卡	80CA000009		TTTTTTTTTTTTTTTTTT	TTTTTTTTTTTTTTTTTT为认证码	
//9	PSAM送认证码	SAM卡	80CA000009		TTTTTTTTTTTTTTTTTT			TTTTTTTTTTTTTTTTTT为8中返回的认证码	
char city_auth(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], factor[20], des[60], deslen, key[2];
unsigned char cpubuf[300], cpurandom[8], samlen;
unsigned short	cpulen;
	
	//get cpu auth code
	memcpy(out_buf, "\xf5\x10", 2);
	memset(buf, 0x00, 40);
	memcpy(buf, "\x80\xca\x00\x00\x09", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("get cpu city-card auth code failure\n");
#endif
		return CE_INVADLIDCARD;
	}
#ifdef DEBUG_PRINT
	PRINTK("get auth-code :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf5\x11", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	memcpy(factor, cpubuf, 9);
	
	memcpy(out_buf, "\xf5\x13", 2);
	memset(buf, 0x00, 40);
	memcpy(buf, "\x80\xca\x00\x00\x09", 5);
	memcpy(&buf[5], cpubuf, 9);
	ret = sam_apdu(tpCPU.sz_psam_index, buf, 14, cpubuf, &samlen, 0, 0);
	if(ret != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("city-sam auth code failure\n");
#endif
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("city-sam auth-code :");
	for(i = 0; i < samlen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf5\x13", 2);
	if((cpubuf[samlen - 2] != 0x90) || (cpubuf[samlen - 1] != 0x00))
		return CE_INVADLIDCARD;
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif

	return CE_OK;
}

/*
function:
	初始化符合应用消费交易
parameter:
	1、密钥索引号（1）
	2、交易金额（4）高字节在前\
	3、终端机编号（6）SAM卡号
return :
*/
char CPU_init_for_purchase(char key_index, int transvalue, char *device_id, unsigned char *user_sn, unsigned char *city, unsigned char *out_buf)
{
int ret;
unsigned char buf[80];
unsigned char cpubuf[300];
unsigned short cpulen;
unsigned char i, timebuf[10];
char chCode;
	
	//initialize for purchase
	memcpy(buf, "\x80\x50\x01\x02\x0B", 5);
	//key
	buf[5] = key_index;
	LongToByte(transvalue, &buf[6]);
	memcpy(&buf[10], device_id, 6);
	buf[16] = 0xf;
#ifdef DEBUG_PRINT
	PRINTK("purchase init data:");
	for(i = 0; i < 17; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(buf, 17, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("purchase init return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if((cpubuf[cpulen - 2] != 0x90)||(cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
#ifdef DEBUG_TIME
	memcpy(timebuf, "\xf0\xf0\x00", 3);
	ReaderResponse(csc_comm, ERR_OK, 0xF0, timebuf, 2);
#endif	
	//old balnce(4) sn(2) overdraft(3) key version(1) al(1) random(4)
	memcpy(capp_init, cpubuf, 15);

	//cal maC1:─交易类型标识/交易金额/终端机编号/交易日期（终端）/交易时间（终端）
	memset(buf, 0x00, 80);
	//random
	memcpy(&buf[0], &capp_init[11], 4);
	//transaction sn
	memcpy(&buf[4], &capp_init[4], 2);
	//transaction amount
	LongToByte(transvalue, &buf[6]);
	//transaction type
	buf[10] = 0x06;
	//date time bcd
	memcpy(&buf[11], tpCPU.time_bcd, 7);
	//key version &
	buf[18] = capp_init[9];
	buf[19] = capp_init[10];
	//logic id
	memcpy(&buf[20], user_sn, 8);
	memcpy(ch_cpu_mac_data, buf, 0x1c);
	
	//calculate mac1
	if((ret = cpu_cal_mac1(tpCPU.sz_psam_index, buf, 0x1c, cpubuf)) != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("cpu cal mac1 return %d\n", ret);
#endif
		return CE_INVADLIDCARD;
	}
	//psam sn & mac1
	memcpy(mac1, cpubuf, 8);
	memcpy(tpCPU.sam_sn, cpubuf, 4);
#ifdef DEBUG_TIME
	memcpy(timebuf, "\xf0\xf1\x00", 3);
	ReaderResponse(csc_comm, ERR_OK, 0xF0, timebuf, 2);
#endif	
	//
	return 0;
}
/*************************************************************
function:debit for purchase
	更新缓存到ic卡
parameter:
	1、SFI
	2、
*************************************************************/
char CPU_debit_for_purchase(unsigned char *rollback, cpu_proc_callback proc, unsigned char *out_buf)
{
int ret;
unsigned char buf[80];
unsigned char cpubuf[80];
unsigned short cpulen;
unsigned char mac2[4],i;
	
	/*if((ret = cpu_cal_mac1(ch_cpu_mac_data, 0x24, cpubuf)) != 0)
	{
		PRINTK("cpu cal mac1 return %d\n", ret);
		return CE_INVADLIDCARD;
	}
	//psam sn & mac1
	memcpy(mac1, cpubuf, 8);
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, buf, 2);
#endif*/
	//
	memcpy(buf,"\x80\x54\x01\x00\x0F",5);
	//psam sn
	memcpy(&buf[5], mac1, 4);
	//date time bcd
	memcpy(&buf[9], tpCPU.time_bcd, 7);
	//mac1
	memcpy(&buf[16], &mac1[4], 4);
#ifdef DEBUG_PRINT
	PRINTK("debit data:");
	for(i = 0; i < 20; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(buf, 20, cpubuf, &cpulen);
	if(ret != 0)
	{
		//bakup the phyical id
		//memcpy(ch_cput_phyical_id_bak, ch_cput_phyical_id, 8);
#ifdef	DEBUG_PRINT
		PRINTK("debit failure %02x %02x\n", cpubuf[0], cpubuf[1]);
#endif
		if(proc != NULL)
		{
			(*proc)(0xff);
		}
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("cpu debit return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	*rollback = 0;
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	memcpy(tpCPU.tac, cpubuf, 4);
	memcpy(mac2, &cpubuf[4], 4);
	//using thread to check the mac2
	memcpy(ch_cpu_mac_data, mac2, 4);
	//blnCalMAC2 = 0xff;
	//credit sam for purchase
	memcpy(buf, mac2, 4);
	if(cpu_cal_mac2(tpCPU.sz_psam_index, buf, 4, buf) != 0)
		return 0;
	//
	return 0;
}

/*

*/
char CPU_VerifyPIN(unsigned char *pin, unsigned char len, unsigned char *out_buf)
{
int ret;
unsigned char buf[40];
unsigned char cpubuf[100], Le;
unsigned short cpulen;

	memcpy(out_buf, "\xf1\x00", 2);
	memcpy(buf, "\x00\x20\x00\x00", 4);
	buf[4] = len;
	memcpy(&buf[5], pin, len);
	if((ret = mifpro_apdu(buf, 5 + len, cpubuf, &cpulen)) != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("verify pin return %d\n", ret);
#endif
		return CE_WRITE;
	}
	memcpy(out_buf, "\xf1\x01", 2);
#ifdef DEBUG_PRINT
	PRINTK("verify pin :%02x status %02x %02x\n", cpulen, cpubuf[0], cpubuf[1]);
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
	
	return 0;
}

#define	READER_2440

unsigned char mifpro_apdu(unsigned char *inbuf, unsigned char inbytes, unsigned char *outbuf, unsigned short *outbytes)
{
unsigned char ret;

#ifdef	READER_2440
unsigned char 	bak, bak2, bak3;
unsigned char len;
	ret = mifpro_icmd(inbuf, inbytes, outbuf, &len);
#else
unsigned short len;
	ret = mifpro_icmd(inbuf, inbytes, outbuf, &len);
#endif	
	
	if(ret == 0)
	{
		if(len == 0)
			*outbytes = 256;
		else if(len == 1)
			*outbytes = 257;
		else 
		{
			*outbytes = len;
		}
	}
	return ret;	
}
