#include "xa_tong_operation.h"
//需去掉lib定义
#include "linux2440lib.h"
#include "xa_error_code.h"
#include "xdr_file_manage.h"
#include "bin_file_manage.h"
#include "xa_sam.h"
#include "serial.h"
#include "hh_cpu_operation.h"
#include "eeprom.h"
#include "xa_operation.h"
#include "time_tools.h"

//#define	DEBUG_ROLLBACK	1
//#define DEBUG_PRINT_ADD 1 //20230710
unsigned char szt_mac1[8], szt_mac2[4];
unsigned char cput_15_data[30];			//public application file
unsigned char cput_21_data[68];			//person file
unsigned char cput_01_data[4];			//electric purse
unsigned char cput_05_data[30];			//xian MF file 05
unsigned char cpu_19_data[64];
unsigned char cput_39_data[32];			//special card valid date
unsigned char cput_1A_data[31];			//
unsigned char cput_16_data[55];			//
unsigned char purchase_init[19];		//init capp purcahse return
unsigned char purchase_debit[8];		//debit capp purchase return

/*
*/
void sz_tong_ee_write(unsigned char sn_bak)
{
unsigned short addr;
	
	addr = EE_CITY_BACKUP;
	ee_write(addr, 1, &ch_sz_cput_rollback);
	addr += 1;
	ee_write(addr, 8, ch_cput_phyical_id);
	addr += 8;
	ee_write(addr, 1, &ch_tong_code_bak);
	addr += 1;

	ee_write(addr, XA_CPUT_15_LEN, cput_15_data);
	addr += XA_CPUT_15_LEN;
	ee_write(addr, XA_CPUT_1A_LEN, cput_1A_data);
	addr += XA_CPUT_1A_LEN;

	ee_write(addr, SZ_CPUT_05_LEN, cput_05_data);
	addr += SZ_CPUT_05_LEN;
	ee_write(addr, SZ_CPU_19_LEN, cpu_19_data);
	addr += SZ_CPU_19_LEN;

	ee_write(addr, sizeof(tpCPU), tpCPU.curtime);
	addr += sizeof(tpCPU);
	ee_write(addr, 19, capp_init);
	addr += 19;
}
/*
*/
void sz_tong_ee_read()
{
unsigned long addr;
	
#ifdef	DEBUG_PRINT
	PRINTK("need read bakup infor\n");
#endif
	addr = EE_CITY_BACKUP;
	ee_read(addr, 1, &ch_sz_cput_rollback);
	addr += 1;
	ee_read(addr, 8, ch_cput_phyical_id_bak);
	addr += 8;
	ee_read(addr, 1, &ch_tong_code_bak);
	addr += 1;

	ee_read(addr, XA_CPUT_15_LEN, cput_15_data);
	addr += XA_CPUT_15_LEN;
	ee_read(addr, XA_CPUT_1A_LEN, cput_1A_data);
	addr += XA_CPUT_1A_LEN;
	ee_read(addr, SZ_CPUT_05_LEN, cput_05_data);

	addr += SZ_CPUT_05_LEN;
	ee_read(addr, SZ_CPU_19_LEN, cpu_19_data);
	addr += SZ_CPU_19_LEN;

	ee_read(addr, sizeof(tpCPU), tpCPU.curtime);
	addr += sizeof(tpCPU);
	ee_read(addr, 19, capp_init);
	addr += 19;

}

/*
*/
void xa_tong_rollback_write(unsigned char *rec_buf, unsigned short rec_len)
{
unsigned short addr;
unsigned char i, j;
unsigned long protectSecond;

	//find the first NO rollBack
	for(i = 0; i < 10; i++)
	{
		if(memcmp(tpXACPUProtect[i].phyicalID, ch_cput_phyical_id, 8) == 0)
			break;
	}
	if(i >= 10)
	{
		for(i = 0; i < 10; i++)
		{
			if(tpXACPUProtect[i].rollBack == 0)
				break;
		}
	}
	//if list is full then rewrite the FIRST record
	if(i >= 10)
	{//find the MAX second of the early the data
		i = 0;
		protectSecond = tpXACPUProtect[i].usecond;
		for(j = 0; j < 10; j++)
		{
			if(protectSecond < tpXACPUProtect[j].usecond)
			{
				i = j;
				protectSecond = tpXACPUProtect[j].usecond;
			}
		}
	}
	//
	tpXACPUProtect[i].rollBack = ch_sz_cput_rollback;
	memcpy(tpXACPUProtect[i].phyicalID, ch_cput_phyical_id, 8);
	tpXACPUProtect[i].usecond = ~tpCPU.lowsecond;
	memcpy(tpXACPUProtect[i].time_bcd, tpCPU.time_bcd, 7);
	tpXACPUProtect[i].tranAmount = tpCPU.tranamount;
	tpXACPUProtect[i].balance = tpCPU.balance;
	memcpy(tpXACPUProtect[i].capp_init, capp_init, 19);
	memcpy(tpXACPUProtect[i].sam_sn, tpCPU.sam_sn, 4);
	//
	if(rec_buf != NULL)
		memcpy(tpXACPUProtect[i].rec_buf, rec_buf, rec_len);
	tpXACPUProtect[i].rec_len = rec_len;
	
	return ;
}
/*
function:external authorization
*/
char CPUT_gettransprove(unsigned char transtypeid, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], factor[20], des[60], deslen;
unsigned char cpubuf[100], cpurandom[8];
char chCode, purchase_init_bak[19];
unsigned short purchase_sn, purchase_sn_bak, cpulen;

	//extern auth
	memcpy(out_buf, "\xf0\x91", 2);
	//if((chCode = CPU_externauth(0, sam_index, cpu_factor, out_buf)) != 0)
	//	return chCode;
	
#ifdef DEBUG_PRINT
	PRINTK("get trans prove type:%02x\n", transtypeid);
#endif
	if(transtypeid == 0x05)
	{
		if(CPUT_VerifyPIN(0, "\x12\x34\x56", 3, out_buf) != 0)
			return CE_READ;
		//initilize for consume according to the sn to check whether transaction finished or not
		memcpy(purchase_init_bak, capp_init, 19);
		memcpy(out_buf, "\xf0\x92", 2);
		//initialize for purchase
		memcpy(buf, "\x80\x50\x01\x01\x0B", 5);
		buf[5] = 0x01;
		LongToByte(tpCPU.tranamount, &buf[6]);
		memcpy(&buf[10], ch_cput_psam_id, 6);
		buf[16] = 0x0f;
		ret = mifpro_apdu(buf, 17, cpubuf, &cpulen);
#ifdef DEBUG_PRINT
		PRINTK("initalize for purchase ret:%02x cpubuf %02x%02x\n", ret, cpubuf[0], cpubuf[1]);
#endif
		if((ret != 0) || (cpulen != 17))
		{
			memcpy(capp_init, purchase_init_bak, 19);
		}else
			memcpy(capp_init, cpubuf, 15);
	}else if(transtypeid == 0x0a)
	{
		memcpy(purchase_init_bak, capp_init, 19);
		if(CPU_init_for_capp(1, tpCPU.tranamount, ch_cput_psam_id, ch_cput_logic_id, "\x21\x50\x80", 3, ch_cpu_mac_data) != 0)
			memcpy(capp_init, purchase_init_bak, 19);
	}else if(transtypeid == 0x09)
	{
		memcpy(purchase_init_bak, capp_init, 19);
		memcpy(ch_cput_logic_id, &cput_15_data[12], 8);
		memcpy(buf, &cput_15_data[2], 2);
		buf[2] = 0xff;
		if(0 != CPU_init_for_capp(1, tpCPU.tranamount, ch_cput_psam_id, ch_cput_logic_id, buf, 3, ch_cpu_mac_data))
		{
			return CE_READ;
		}
	}else
	{
		//initilize for credit according to the sn to check whether transaction finished or not
		memcpy(purchase_init_bak, capp_init, 19);
		memcpy(out_buf, "\xf0\x92", 2);
		if(0 != CPU_init_for_credit(tpCPU.tranamount, ch_cput_isam_id, out_buf))
		{
			return CE_READ;
		}
	}
	
	ByteToShort(&purchase_sn_bak, &purchase_init_bak[4]);
	ByteToShort(&purchase_sn, &capp_init[4]);
	memcpy(out_buf, "\xf0\x94", 2);
	if(purchase_sn == (purchase_sn_bak + 1))
	{
		//get transaction prove
		memcpy(out_buf, "\xf0\xa0", 2);
		memset(buf, 0x00, 40);
		memcpy(buf, "\x80\x5a\x00\x00\x02", 5);
		buf[3] = transtypeid;
		memcpy(&buf[5], &capp_init[4], 2);
		ret = mifpro_apdu(buf, 7, cpubuf, &cpulen);
#ifdef DEBUG_PRINT
		PRINTK("get trans prove data:");
		for(i = 0; i < 7; i++) PRINTK("%02x", buf[i]);
		PRINTK("\n");
#endif
		if(ret != 0)
		{
			PRINTK("get transaction prove failure\n");
			return CE_READ;
		}
#ifdef DEBUG_PRINT
		PRINTK("get trans prove ret:");
		for(i = 0; i < cpulen; i++) PRINTK("%02x", cpubuf[i]);
		PRINTK("\n");
#endif
		memcpy(out_buf, "\xf0\xa1", 2);
		if((cpubuf[cpulen - 2] == 0x90) && (cpubuf[cpulen - 1] == 0x00))
		{
#ifdef DEBUG_TIME
			ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif
			memcpy(capp_init, purchase_init_bak, 19);
			memcpy(tpCPU.tac, &cpubuf[4], 4);
			return 0;
		}
	}

	return CE_INVADLIDCARD;
}


/*
function:read the file 15
parameter:
*/
char CPUT_GetFiles15(unsigned char *out_buf)
{
int ret;
unsigned char buf[40];
unsigned char cpubuf[80], Le;
unsigned short cpulen;
	
	//read file 15
	memcpy(out_buf, "\xf0\x05", 2);
	memcpy(buf, "\x00\xb0\x95\x00\x1e", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("mifpro_apdu return %d\n", ret);
#endif
		return CE_READ;
	}
	memcpy(out_buf, "\xf0\x06", 2);
#ifdef DEBUG_PRINT
	PRINTK("file 15 len :%02x, logicid: %02x %02x %02x %02x %02x %02x %02x %02x appindex:%02x app ver:%02x \n", cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3],
			cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7], cpubuf[8], cpubuf[9]);
	PRINTK("app sn: %02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
			 cpubuf[10], cpubuf[11], cpubuf[12], cpubuf[13], cpubuf[14], cpubuf[15], cpubuf[16], cpubuf[17], cpubuf[18], cpubuf[19]);
	PRINTK(" app startdate %02x%02x%02x%02x valid date %02x%02x%02x%02x fci %02x%02x\n", 
			cpubuf[20], cpubuf[21], cpubuf[22], cpubuf[23], cpubuf[24], cpubuf[25], cpubuf[26], cpubuf[27], cpubuf[28], cpubuf[29]);
#endif
	if(cpulen != 0x1e + 2)
		return CE_INVADLIDCARD;
	memcpy(ch_cput_logic_id, cpubuf, 8);
	memcpy(cput_15_data, cpubuf, XA_CPUT_15_LEN);
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	return 0;
}

/*
function:read file 16
parameter:
	*entrytime:
	*curtime:
	*mileclass:
*/
char CPUT_GetFiles21(unsigned char *out_buf)
{
int ret;
unsigned char buf[80], chCode;
unsigned char cpubuf[100], Le;
long 	i;
unsigned short cpulen;

	//select file 21
	memcpy(out_buf, "\x4b\xa3", 2);
	memcpy(buf,"\x00\x21", 2);
	if(0 != (chCode = CPU_select_file(buf, 2, out_buf, NULL)))
		return chCode;
	//read file 21 --68bytes
	memcpy(buf, "\x00\xb0\x00\x00\x44", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
	if((cpulen == 2) && (cpubuf[0] == 0x6c))
	{
		buf[4] = Le = cpubuf[1];
		ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
		if(ret != 0)
			return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("file21 len %02x person type:%02x expired date %02x%02x-%02x-%02x\n", 
		cpulen, cpubuf[0], cpubuf[60], cpubuf[61], cpubuf[62], cpubuf[63]);
#endif
	if((cpubuf[cpulen -2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	memcpy(cput_21_data, cpubuf, 0x44);
	tpYKTTxnPurchase.UserType = cput_21_data[0];
	/*******************住建部卡交易优惠类型出现FF的情况********************************** */
	if(tpYKTTxnPurchase.UserType == 0xFF)
	{
		tpYKTTxnPurchase.UserType = 0x00;
	}
	/************************************************** */
	for(i = 0; i < tpCard1912.mapnumber; i++)
	{
		if(cput_21_data[0] == tpCard1912.YKTPassengeMap_val[i].YKTpassenage)
		{
			cput_21_data[0] = tpCard1912.YKTPassengeMap_val[i].passenage;
			break;
		}
	}
	if(i >= tpCard1912.mapnumber)
		return CE_SETPARA;
	return 0;
}

/*
function:read file 18
parameter:
	rec_num: history 
*/
char CPUT_GetFiles18(char rec_num, unsigned char *out_buf)
{
int ret;
unsigned char buf[80], chCode;
unsigned char cpubuf[100], Le;
long 	i, j;
unsigned short cpulen;

	if(rec_num > 10)
		rec_num = 10;
	//
	memset(out_buf, 0x00, rec_num * XA_CPUT_18_LEN);
	//read history
	memcpy(buf,"\x00\xb2\x00\x00\x17", 5);
	buf[3] = (0x18 << 3) | 0x04;
	Le = XA_CPUT_18_LEN;
	
	for(i = 1; i < rec_num + 1; i++)
	{
		buf[2] = i;
#ifdef	DEBUG_2_PRINT
		PRINTK("read history data:");
		for(j = 0; j < 5; j++) PRINTK("%02x", buf[j]);
		PRINTK("\n");
#endif		
		ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
		if(ret != 0)
			break;
		//no record
#ifdef	DEBUG_PRINT
		PRINTK("read history data:");
		for(j = 0; j < cpulen; j++) PRINTK("%02x", cpubuf[j]);
		PRINTK("\n");
#endif		
		if((cpubuf[cpulen - 2] == 0x6a) && (cpubuf[cpulen - 1] == 0x83))
			break;
		//wrong length and set the correct length read again
		if((cpulen == 2) && (cpubuf[0] == 0x6c))
		{
			buf[4] = Le = cpubuf[1];
			ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
			if(ret != 0)
				break;
		}
		//expired length
		if(cpulen != Le + 2)
			break;
		memcpy(&out_buf[(i - 1) * XA_CPUT_18_LEN], cpubuf, XA_CPUT_18_LEN);
	}
	return (i - 1);
}
/*

*/
char CPUT_VerifyPIN(unsigned char p2, unsigned char *in_buf, unsigned char in_len, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40];
unsigned char cpubuf[100], Le;
unsigned short cpulen;

	memcpy(out_buf, "\xf1\x00", 2);
	memcpy(buf, "\x00\x20\x00\x00", 4);
	buf[3] = p2;
	buf[4] = in_len;
	memcpy(&buf[5], in_buf, in_len);	//\x03\x12\x34\x56
#ifdef	DEBUG_PRINT_ADD
	for(i = 0; i < 5 + in_len; i++)
		PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if((ret = mifpro_apdu(buf, 4 + in_len + 1, cpubuf, &cpulen)) != 0)
	{
#ifdef	DEBUG_PRINT_ADD
		PRINTK("verify pin return %d\n", ret);
#endif
		return CE_READ;
	}
	memcpy(out_buf, "\xf1\x01", 2);
#ifdef DEBUG_PRINT_ADD
	PRINTK("verify sz-tong pin :%02x status %02x %02x\n", cpulen, cpubuf[0], cpubuf[1]);
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
	
	return 0;
}
/*
function:read the file 05
parameter:
*/
char CPUT_GetFiles05(unsigned char *out_buf)
{
int ret, i, j;
unsigned char buf[40], chCode;
unsigned char cpubuf[100], Le;
unsigned short cpulen;
	
	//read file 05 - fixed file length record
	memcpy(out_buf, "\xf0\x22", 2);
	memcpy(buf, "\x00\xb0\x85\x00\x1e", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\xf0\x23", 2);
	if((cpubuf[cpulen -2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	memcpy(cput_05_data, cpubuf, cpulen - 2);
#ifdef DEBUG_PRINT
	PRINTK("file 0005:");
	for(i = 0; i < cpulen; i++)
		PRINTK("%02x", cput_05_data[i]);
	PRINTK("\n");
	PRINTK("ISSUE %02x%02x CITY %02x%02x BUSINESS %02x%02x RFU %02x DEPOSIT %02x sn %02x%02x%02x%02x %02x%02x%02x%02x TYPE %02x%02x DATE %02x%02x%02x%02x VER %02x%02x\n",
		cput_05_data[0], cput_05_data[1], cput_05_data[2], cput_05_data[3], cput_05_data[4], cput_05_data[5], cput_05_data[6], cput_05_data[7], cput_05_data[8],
		cput_05_data[9], cput_05_data[10], cput_05_data[11], cput_05_data[12], cput_05_data[13], cput_05_data[14], cput_05_data[15], cput_05_data[16], cput_05_data[17], 
		cput_05_data[18], cput_05_data[19], cput_05_data[20], cput_05_data[21], cput_05_data[28], cput_05_data[29]);
#endif
	return 0;
}

/*
function:read the file 19
parameter:
*/
char CPUT_GetFiles19(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40];
unsigned char cpubuf[80], Le;
unsigned short cpulen;
	
	//read balance
	memcpy(out_buf, "\xfa\x20", 2);
	memcpy(buf, "\x80\x5c\x00\x02\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
#ifdef DEBUG_PRINT_ADD
	PRINTK("read balance return len %d %02x %02x %02x %02x ", cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3]);
#endif
	memcpy(out_buf, "\xf0\x21", 2);
	if(cpulen != 6)
	{
		return CE_INVADLIDCARD;
	}
	ByteToLong(&tpCPU.balance, cpubuf);
#ifdef DEBUG_PRINT_ADD
	PRINTK("cpu balance:%d\n", tpCPU.balance);
#endif
	memcpy(cput_01_data, cpubuf, 4);

	//read file 19 - variable file length
	memcpy(out_buf, "\xf0\x22", 2);
	memcpy(buf, "\x00\xb2\x02\x00\x30", 5);
	buf[3] = (0x19 << 3) | 0x04;
	buf[4] = Le = XA_CPUT_19_LEN;
#ifdef DEBUG_PRINT_ADD
	PRINTK("read file 19:");
	for(i = 0; i < 5; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\xf0\x23", 2);
	if((cpulen == 2) && cpubuf[0] == 0x6c)
	{
		buf[4] = Le = cpubuf[1];
		ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
		if(ret != 0)
			return CE_READ;
	}
#ifdef DEBUG_PRINT_ADD
	PRINTK("return:len%d ", Le);
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf0\x24", 2);
	if(cpulen != Le + 2)
	{
		return CE_INVADLIDCARD;
	}
	memcpy(cpu_19_data, cpubuf, Le);
#ifdef DEBUG_PRINT_ADD
	PRINTK("file 19 consume flag %02x recordlen %02x lockedflag %02x\n", 
		cpu_19_data[0], cpu_19_data[1], cpu_19_data[2]);
	PRINTK(" en-time %02x-%02x-%02x %02x:%02x en-station:%02x%02x en-status %02x markamount %02x%02x pointer %02x\n", 
		cpu_19_data[3], cpu_19_data[4], cpu_19_data[5], cpu_19_data[6], cpu_19_data[7], cpu_19_data[8], cpu_19_data[9], cpu_19_data[10], cpu_19_data[11], cpu_19_data[12], cpu_19_data[13]);
	PRINTK(" ex-time %02x-%02x-%02x %02x:%02x ex-station:%02x%02x ex-status %02x tranamount %02x%02x pointer %02x \n",
		cpu_19_data[14], cpu_19_data[15], cpu_19_data[16], cpu_19_data[17], cpu_19_data[18], cpu_19_data[19], cpu_19_data[20], cpu_19_data[21], cpu_19_data[22], cpu_19_data[23], cpu_19_data[24]);
#endif
	//check the pointer valid
	//if((cpu_19_data[24] > 7) || (cpu_19_data[13] > 6))
	//entry pointer MUST be even number and exit pointer MUST be odd number
	//SO change the pointer 
	if(((cpu_19_data[13] % 2) != 0) || ((cpu_19_data[24] % 2) != 1))
	{
		cpu_19_data[13] = 0;
		cpu_19_data[24] = 1;
	}
	return 0;
}

/*
function:read the file 1A
parameter:
*/
char CPUT_GetFiles1A(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40], chCode;
unsigned char cpubuf[80], Le;
unsigned short cpulen;

	//read file 1A
	memcpy(out_buf, "\xf0\x22", 2);
	memcpy(buf,"\x00\xb2\x01\xd4\x1f", 5);
	buf[3] = (0x1A << 3) | 4;
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
#ifdef DEBUG_PRINT_ADD
	PRINTK("file 1A len %02x cardsn%02x%02x overdraft%02x%02x%02x txnamount %02x%02x%02x%02x txntype%02x samid%02x%02x%02x%02x%02x%02x txndate %02x%02x-%02x-%02x %02x%02x%02x aftbalance %02x%02x%02x%02x tac %02x%02x%02x%02x\n", 
		cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3], cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7], cpubuf[8], cpubuf[9], cpubuf[10], cpubuf[11], cpubuf[12], cpubuf[13], cpubuf[14], cpubuf[15], 
		cpubuf[16], cpubuf[17], cpubuf[18], cpubuf[19], cpubuf[20], cpubuf[21], cpubuf[22], cpubuf[23], cpubuf[24], cpubuf[25], cpubuf[26], cpubuf[27], cpubuf[28], cpubuf[29], cpubuf[30]);
#endif
	if((cpubuf[cpulen - 2] == 0x6A) && (cpubuf[cpulen - 1] == 0x83))
	{
		memset(cput_1A_data, 0x00, XA_CPUT_1A_LEN);
		return 0;
	}else if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	memcpy(cput_1A_data, cpubuf, 0x1F);
	return 0;
}

/*
function:
// 有效期类别
// 0: 对有效期不做检查；
// 1: 从第一次消费使用后（票卡DateOfCardFirstTransaction）的一段可配置时间内（票卡的Duration）有效；
      同时判断（票卡CSCEndDate，根据EOD的FixedEndDate获取）
// 2: 固定的起始时间A；POST当前的售票时间IssueDate的一段可配置时间内（EOD的Duration）有效；POST销售时写入票卡CSCStartDate和CSCEndDate
// 3: 以销售/充值后的一段时间有效；POST销售、充值(票卡LastAddingValueDate)后的一段可配置时间内（EOD的Duration）有效；仅限于车票有效期使用。
//	在POST销售，写入CSCStartDate和CSCEndDate,在POST充值时，更新CSCEndDate
// 4: 固定的起始时间B；POST当前的售票时间IssueDate到某一个可配置时间点结束（EOD的FixedEndDate）；
parameter:
	durationmode:the DuratinMode in the cpu card.
	>4:return 0,not expired date
return :
	11: not matching mode
	0: mode match ok
*/
char CPUT_ValidatePeriod(unsigned char durationmode, unsigned short shDays)
{
unsigned short	firstdate, durationdate, enddate;

	//current station is set to the DATE mode
	if(tpwaivermode.cur_sta_date)
		return 0;
	//other station is set to DATE mode or sensitive duaration and DATE mode in the ticket
	if((tpwaivermode.oth_sta_date || tpwaivermode.sen_sta_date))
		return 0;
	//
	enddate = datestr2days(&cput_15_data[24]);
	if(enddate < shDays)
		return CE_EXPIREDDATE;
	return 0;
}
/*
function:calculate the start date and end date for sale command
*/
char CPUT_CalPeriod(unsigned short shDays)
{
/*Date2_t	firstdate, durationdate, enddate;

	switch(tpTicketDef.DurationMode)
	{
	case 0:
		tpCPU.startdate = tpCPU.enddate = 0x00; 
		break;
	case 1:
	case 2:
	case 3:
		tpCPU.startdate = shDays;
		tpCPU.enddate = tpCPU.startdate + tpTicketDef.Duration;
		break;
	case 4:
		tpCPU.startdate = shDays;
		tpCPU.enddate = tpTicketDef.FixedEndDate;
		break;
	case 5://need to read file 14
	default:
		tpCPU.startdate = tpCPU.enddate = shDays;
		break;
	}
*/	return 0;
}
/*
function:
	check the card metro status 
parameter:
return :
	:
	0:entry ok
*/
char CPUT_TellEntry(unsigned char *expecting_status, unsigned char mode_check)
{
char chret;
unsigned char chInitStationFare, chInitSensitiveFare, chCode;
unsigned char eod_station[4], sta_close_entry[4], sen_close_entry[4];
unsigned long i, InitStationNum, InitSensitiveNum;
unsigned short shCalFare;
unsigned long lngsrcstation, lngdesstation, lngLosecond;
unsigned char exit_time[7];

	//
	chCode = 0;
	if((cpu_19_data[13] == 0x00) && (cpu_19_data[24] == 0x00))
	{
		cpu_19_data[24] = 0x01;
	}
//	initial		entry	exit	entry	exit	entry	exit	entry	exit
//p10x00		0x02	0x02	0x04	0x04	0x06	0x06	0x00	0x00
//p20x01		0x01	0x03	0x03	0x05	0x05	0x07	0x07	0x01
	if((cpu_19_data[24] > cpu_19_data[13]))
	{
		if((cpu_19_data[24] == 0x07) && (cpu_19_data[13] == 0x00))
		{
			chCode = 0;
			*expecting_status = 0x01;
		}
		else
		{
			chCode = CE_NO_ENTRY;
			*expecting_status = cpu_19_data[13] + 2;
			if(*expecting_status >= 8)
				*expecting_status = 0x00;
		}
	}else
	{
		chCode = 0;
		*expecting_status = cpu_19_data[24] + 2;
	}
	//if(chCode == 0x00)
	//{
	//	if(tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_exit)
	//	{
	//		chCode = CE_NO_ENTRY;
	//		*expecting_status = cpu_19_data[13];
	//	}
	//}
	//
	if(!mode_check)
		return chCode;
	
	if(chCode != 0x00)
	{
		//if current station is set to entry mode then the entry station/time is current station/time
		if(tpwaivermode.cur_sta_entry)
		{
			//entry time is set to the current time
			memcpy(&cpu_19_data[3], &tpCPU.time_bcd[1], 5);
			//entry station is set the current station
			memcpy(&cpu_19_data[8], tpCPU.curstation, 2);
			*expecting_status = cpu_19_data[24];
			chCode = 0;
		}else if(tpwaivermode.oth_sta_entry)
		//else if(tpwaivermode.oth_sta_entry || tpwaivermode.sen_sta_entry)
		{
			//entry time is set to the current time
			memcpy(&cpu_19_data[3], &tpCPU.time_bcd[1], 5);
			//select the most close entry station from station set
			chInitStationFare = 0xff;
			InitStationNum = 0;
			for(i = 0; i < tpStationWaiverMode.waivermode_len; i++)
			{
				if(tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 9] & 0x02)
				{
					lngsrcstation = (tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 4] << 24) + (tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 5] << 16)
									+ (tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 6] << 8) + tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 7];
					lngdesstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
					if(0 != (cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngdesstation, &shCalFare)))
						continue;
					InitStationNum += 1;
					if(shCalFare < chInitStationFare)
					{
						chInitStationFare = shCalFare;
						memcpy(sta_close_entry, &tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 4], 4);
					}
				}
			}
			//select the most close entry station from the sensitive period
			/*chInitSensitiveFare = 0xff;
			InitSensitiveNum = 0;
			for(i = 0; i < EodWaiverDateMasterConfig.StationModeInfo.StationModeInfo_len; i++)
			{
				ShortToByte(EodWaiverDateInfo[i].StationID, eod_station);
				if((EodWaiverDateInfo[i].ModeCode == SZ_WAIVER_ENTRY)
					&& (memcmp(tpCPU.curstation, eod_station, 2) != 0))
				{
					if(0 != (cal_station_fare(eod_station, tpCPU.curstation, &chCalFare)))
						continue;
					if(chCalFare < chInitSensitiveFare)
					{
						chInitSensitiveFare = chCalFare;
						memcpy(sen_close_entry, eod_station, 2);
					}
				}
			}*/
			if(InitStationNum == 0)
				return CE_NO_ENTRY;
			/*
			if(chInitSensitiveFare < chInitStationFare)
				memcpy(eod_station, sen_close_entry, 2);
			else*/
				memcpy(eod_station, sta_close_entry, 4);
			
			
			//entry station is set the current station
			memcpy(&cpu_19_data[8], &eod_station[2], 2);
			*expecting_status = cpu_19_data[24];
			chCode = 0;
		}
		//MUST check the exit station and time for current or other station ENTRY_MODE
		if (chCode == 0)
		{
			memset(exit_time, 0x00, 7);
			memcpy(&exit_time[1], &cpu_19_data[14], 5);
			lngLosecond = timestr2long(&exit_time[1]);
			if((cpu_19_data[19] == tpCPU.curstation[0]) && (cpu_19_data[20] == tpCPU.curstation[1]))
			{
				if(lngLosecond > tpCPU.lowsecond)
				{
					chCode = CE_CUR_EXIT;
				}else if ((tpCPU.lowsecond - lngLosecond) < 20 * 60)
					chCode = CE_CUR_EXIT;
			}
		}
	}
		
	return chCode;
}
/*
function select the file
*/
char CPUT_select_file(char *sfi, unsigned char *out_buf)
{
unsigned char buf[100], cpubuf[100];
unsigned short cpulen;
int ret, i;

	//select file
	memcpy(out_buf, "\xf0\x80", 2);
	memcpy(buf, "\x00\xa4\x00\x00\x02", 5);
	memcpy(&buf[5], sfi, 2);
	ret = mifpro_apdu(buf, 7, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\xf0\x81", 2);
	if((cpubuf[cpulen - 2] != 0x90)||(cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
#ifdef DEBUG_2_PRINT
	PRINTK("select file %02x%02x:", sfi[0], sfi[1]);
	for(i = 0;i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif

	return 0;
}
/*
function:external authorization
*/
char CPUT_externauth(char extern_auth_type, unsigned short shkey, unsigned char cardkey, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], factor[20], des[60], deslen, key[2];
unsigned char cpubuf[100], cpurandom[8];
unsigned short cpulen;

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
#ifdef DEBUG_PRINT
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
	//key index /key version / factor
	memcpy(out_buf, "\xf0\x12", 2);
	memcpy(factor, &cput_15_data[12], 8);
	//memcpy(key, "\x27\x02", 2);
	ShortToByte(shkey, key);
	if((ret = cpu_cal_dcmk(xa_tong_psam_index, key, factor, 8, 0, cpubuf, 8, des, &deslen)) != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("cpu cal dcmk return %d\n", ret);
#endif
		return ERR_SZ_PSAM;
	}
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	//external auth
	memset(buf, 0x00, 50);
	memcpy(buf, "\x00\x82\x00\x82\x08", 5);
	buf[3] = cardkey;
	memcpy(&buf[5], des, 8);
#ifdef DEBUG_PRINT
	PRINTK("external auth :");
	for(i = 0; i < 13; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf0\x13", 2);
	if(mifpro_apdu(buf, 5 + 8, cpubuf, &cpulen) != 0)
		return CE_READ;
#ifdef DEBUG_PRINT
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


char CPU_update_purchase(unsigned char SFI_index, unsigned char rec_index, unsigned char len, unsigned char *rec_buf, unsigned char cycleflag)
{
int ret, i;
unsigned char	buf[100], cpubuf[80];
unsigned char 	des[50], callen, cpurandom[8], factor[20];
unsigned short cpulen;

	//update record
	memset(buf, 0x00, 100);
	memcpy(buf, cpurandom, 8);
	memcpy(buf, "\x00\xdc", 2);
	buf[2] = rec_index;
	buf[3] = (SFI_index << 3) | 0x4;
	if(cycleflag)
	{
		buf[8] = 0;
		buf[10] = 0;
		buf[11] = (SFI_index << 3) | 0x03;
	}
	//buf[12] = len + 4;
	buf[4] = len;
	memcpy(&buf[5], rec_buf, len);
#ifdef DEBUG_PRINT
	PRINTK("capp update before:");
	for(i = 0; i < 15 ; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(buf, 5 + len , cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("capp update return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif	
	if((cpubuf[cpulen - 2] != 0x90)||(cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	return 0;
}

/************************************
suzhou tong entry
************************************/
char xa_tong_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode, chRejectCode, chFare, chEntryStatus;
unsigned char buf[50], factor[20], des[80], deslen;
unsigned char cpubuf[80], cpulen, Le, entry_time[4], last_timebcd[7];
unsigned char status, timecodeid, ch_rollback_temp;
unsigned long time1,time2;
unsigned short tempdate, shTicketType, cnt;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
long lngCardBalance, ret, i;
unsigned short shDays, datetypeid;
unsigned long lngMidnightSecond;
YKTTerminal_t	tpPurchase;

	tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t);
	*out_len = 4;
	memcpy(out_buf, "\x32\x01\x00\x00", 4);
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
		PRINTK("rollback %02x new phyical id", ch_sz_cput_rollback);
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cput_phyical_id[i]);
		PRINTK("  old phyical id ");
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cput_phyical_id_bak[i]);
		PRINTK("\n");
#endif
//	if((ch_sz_cput_rollback != 0) && (memcmp(ch_cput_phyical_id, ch_cput_phyical_id_bak, 8) == 0))
//	{
//#ifdef DEBUG_PRINT
//		PRINTK("need roll back %02x\n", ch_sz_cput_rollback);
//#endif
//		if(0 != (chCode = city_auth(out_buf)))
//			return chCode;
//		if((chRejectCode = CPUT_gettransprove(0x09, out_buf)) == 0)
//		{
//			blncputRollback = 1;
//			goto label_sz_city_rollback_1;
//		}else if(chRejectCode == CE_READ)
//			return CE_READ;
//	}
//	//clear the backup flag
	//YKT purchase transaction record
	tpYKTTxnPurchase.udSubtype = 0x0C;
	tpYKTTxnPurchase.udType = 0x21;
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_cput_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, ch_cput_psam_sn, 8);
	//tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_cput_phyical_id[4]);
	tpYKTTxnPurchase.CardCsn = XA_PAYTYPE_CARD;
	tpYKTTxnPurchase.Enlocation = 0;
	tpYKTTxnPurchase.Entime = 0;
	tpYKTTxnPurchase.OrigEntime = 0;
	tpYKTTxnPurchase.OrigEnlocation = 0;
	tpYKTTxnPurchase.mode = 0;
	//
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[7]);
	memcpy(tpYKTTxnPurchase.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnPurchase.TxnTime, &tpCPU.time_bcd[4], 3);
	
	get_degrade_mode(tpCPU.curstation);
	
	//select df01 and return file 15 information at the same time
	memcpy(out_buf, "\x71\x05", 2);
	//if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
	//	return chCode;
	//read file 15
	//memcpy(cput_15_data, out_buf, XA_CPUT_15_LEN);
#ifdef DEBUG_PRINT
	PRINTK("file 15 Issued:%02x%02x City:%02x%02x Bussiness:%02x%02x RFU%02x%02x Flag:%02x app ver:%02x \n", cput_15_data[0], cput_15_data[1], cput_15_data[2], cput_15_data[3],
			cput_15_data[4], cput_15_data[5], cput_15_data[6], cput_15_data[7], cput_15_data[8], cput_15_data[9]);
	PRINTK("CityUnion:%02x%02x sn:%02x%02x%02x%02x %02x%02x%02x%02x\n",
			 cput_15_data[10], cput_15_data[11], cput_15_data[12], cput_15_data[13], cput_15_data[14], cput_15_data[15], cput_15_data[16], cput_15_data[17], cput_15_data[18], cput_15_data[19]);
	PRINTK(" app startdate %02x%02x%02x%02x valid date %02x%02x%02x%02x M-type %02x S-type%02x\n", 
			cput_15_data[20], cput_15_data[21], cput_15_data[22], cput_15_data[23], cput_15_data[24], cput_15_data[25], cput_15_data[26], cput_15_data[27], cput_15_data[28], cput_15_data[29]);
#endif
	memcpy(tpYKTTxnPurchase.CityCode, &cput_15_data[2], 2);
	memcpy(tpYKTTxnPurchase.CardId, &cput_15_data[12], 8);
	xa_hex2bcd(&tpYKTTxnPurchase.CardId[4], &cput_15_data[16]);
	tpYKTTxnPurchase.CrdMKnd = cput_15_data[28];
	tpYKTTxnPurchase.CrdSKnd = cput_15_data[29];
	memcpy(ch_cput_logic_id, &cput_15_data[12], 8);
	tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount = 0;
	tpYKTTxnPurchase.CrdVerNo = cput_15_data[9];
label_sz_rollback_main:
	//card City
	if((0 != memcmp(&cput_15_data[2], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[2], "\x00\x00", 2)))
		return CE_CARDSTATUS;
	//card union
	if((0 != memcmp(&cput_15_data[10], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[10], "\x00\x00", 2)))
		return CE_CARDSTATUS;
	//card status
	if(0 == cput_15_data[8])
		return CE_CARDSTATUS;
	memcpy(out_buf, "\x71\x07", 2);
	tpYKTTxnPurchase.CrdModel = 0x01;		//CPU
	shTicketType = (cput_15_data[28] << 8) + cput_15_data[29];
	if((chCode = CPU_TellSysCard(shTicketType)) != 0)
	{
		return chCode;
	}
	if(0 != (chCode = get_purchase_ticket(cput_15_data[29], cput_15_data[28], &tpPurchase)))
		return chCode;
	//read file 21
	if(0 != (chCode = CPUT_GetFiles21(out_buf)))
		return chCode;
	memcpy(tpYKTTxnPurchase.Validday, &cput_15_data[24], 4);
	//verify pin and get balance
	if((chCode = CPUT_GetFiles19(out_buf)) != 0)
		return chCode;
	//balance < max remaining value
	tpYKTTxnPurchase.AftBalance = tpYKTTxnPurchase.BefBalance = tpCPU.balance;
	tpYKTTxnPurchase.lastlocation = 0x09000000 + (cpu_19_data[19] << 8) + cpu_19_data[20];
	tpYKTTxnPurchase.AftBalance = tpCPU.balance - tpCPU.tranamount;
	memcpy(out_buf, "\x71\x09", 2);
	//check the black list and lock
	if(CE_BLACKLIST == (chCode = check_YKT_Black_Lock("\x71\x00", "\x03\x01", &cput_15_data[12], 0, out_buf, out_len)))
	{
		return chCode;
	}else if(0 != chCode)
		return chCode;
	//verify date
	memcpy(out_buf, "\x71\x08", 2);
	if((memcmp(tpCPU.time_bcd, &cput_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_15_data[24], 4) > 0))
		return CE_EXPIREDDATE;
	switch(cput_21_data[0])
	{
 	case XA_PASSENGER_ELDER:	//old man
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
			return CE_EXPIREDDATE;
		memcpy(tpYKTTxnPurchase.Validday, &cput_21_data[60], 4);
		break; 
	case XA_PASSENGER_STUDENT:	//Student
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
			cput_21_data[0] = XA_PASSENGER_ADULT;
		memcpy(tpYKTTxnPurchase.Validday, &cput_21_data[60], 4);
		break;
	}

	//check the elder and peak
	if(0 != (chCode = check_peak_time(tpCPU.time_bcd, &timecodeid, &datetypeid)))
		return chCode;
	if((timecodeid == 3) && (cput_21_data[0] == XA_PASSENGER_ELDER))
	{
		return CE_OLD_PEAK;
	}
	memcpy(out_buf, "\x71\x0a", 2);
	if(tpCPU.balance < tpPurchase.minbalance)
	{
		return CE_ENOUGH_BALANCE;
	}
	//bonus and special city card such as lovely card, old man card.
	
	//metro status
	memset(last_timebcd, 0x00, 7);
	last_timebcd[0] = 0x20;
	memcpy(&last_timebcd[1], &cpu_19_data[3], 5);
	if(CPUT_TellEntry(&chEntryStatus, 0x00) == 0)
	{//in entry status
		get_degrade_sensitive_mode(NULL, last_timebcd);
		//sensitive MODE
		if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_exit))
		{
			chEntryStatus = cpu_19_data[13] = 0x02;
			cpu_19_data[24] = 0x01;
		}else
		{
			memcpy(out_buf, "\x71\x0b", 2);
			lngLosecond1 = timestr2long(&last_timebcd[1]);
			if(lngLosecond1 > tpCPU.lowsecond)
			{
				chCode = CE_FREE_UPDATE_ENTRY;
			}else if((tpCPU.lowsecond - lngLosecond1) < 20 * 60)
			{
				chCode = CE_FREE_UPDATE_ENTRY;
			}else
			{
				chCode = CE_FEE_UPDATE_ENTRY;
			}
			//
			if(ch_sz_cput_rollback == MCPU_ROLL_AFTER_ED)
			{
				if(0 != (city_auth(out_buf)))
					return CE_CONSUME_MOVED;
				if(0 == (chRejectCode = CPUT_gettransprove(0x09, out_buf)))
				{
					memcpy(tpYKTTxnPurchase.AFCHead_val.operatorid, tpXACPUProtect[tpXACPUProtectIndex].rec_buf, tpXACPUProtect[tpXACPUProtectIndex].rec_len);
					memcpy(tpCPU.sam_sn, tpXACPUProtect[tpXACPUProtectIndex].sam_sn, 4);
					memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
					goto label_sz_city_rollback_1;
				}else if(chRejectCode == CE_INVADLIDCARD)
					goto label_refuse_to_city_entry;
				else
					return CE_CONSUME_MOVED;
			}
			goto label_refuse_to_city_entry;
		}
	}
	//initial capp 
	if(0 != (chCode = city_auth(out_buf)))
		return chCode;
	memcpy(out_buf, "\x71\x10", 2);
	//city code
	memcpy(buf, &cput_15_data[2], 2);
	buf[2] = 0xFF;
	if(0 != (chCode = CPU_init_for_capp(1, tpCPU.tranamount, ch_cput_psam_id, ch_cput_logic_id, buf, 3, ch_cpu_mac_data)))
	{
		return chCode;
	}
	//update capp-file 19
	//time-bcd YYMMDDHHMM
	memcpy(&cpu_19_data[3], &tpCPU.time_bcd[1], 5);
	//line & station
	memcpy(&cpu_19_data[8], tpCPU.curstation, 2);
	//status:
	cpu_19_data[10] = 0x01;//SZ_METRO_ENTRY;
	//max price
	//cpu_19_data[11]
	//entry pointer1
	cpu_19_data[13] = chEntryStatus;
	memcpy(out_buf, "\x71\x11", 2);
	ret = CPU_update_capp(1, 0x19, cpu_19_data[0], XA_CPUT_19_LEN, cpu_19_data, 0);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif
	memcpy(out_buf, "\x71\x12", 2);
	ch_rollback_temp = SZ_CPU_CAPP_1;
	if(0 != CPU_debit_for_capppurchase(&ch_rollback_temp, NULL, out_buf))
	{
#ifdef DEBUG_PRINT
		PRINTK("debit failure %02x\n", ch_sz_cput_rollback);
#endif
		ch_sz_cput_rollback = MCPU_ROLL_AFTER_ED;
		xa_tong_rollback_write(tpYKTTxnPurchase.AFCHead_val.operatorid, sizeof(YKTTxnPurchase_t));
		return CE_CONSUME_MOVED;
	}
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif

label_sz_city_rollback_1:
	//clear rollback
	if(ch_sz_cput_rollback != 0)
	{
		tpXACPUProtect[tpXACPUProtectIndex].rollBack = 0;
		memset(tpXACPUProtect[tpXACPUProtectIndex].phyicalID, 0x00, 8);
	}
	//sn-2
	tpYKTTxnPurchase.CrdDebitCnt = ByteToShort(NULL, &capp_init[4]);
	//tacs-4
	memcpy(&tpYKTTxnPurchase.TAC, tpCPU.tac, 4);
	//sam sn
	tpYKTTxnPurchase.SamSeq = ByteToLong(NULL, tpCPU.sam_sn);

	//UDSN
	out_buf[0] = 1;
	//recycle	1	0x00:no，0x01:yes，0x02:废票回收
	out_buf[1] = 0x00;
	//black
	out_buf[2] = 0x00;
	//ticket family
	out_buf[3] = XA_CITY_FAMILY;
	//ticket type
	//memcpy(&out_buf[4], &shTicketType, 2);
	out_buf[4] = cput_15_data[29];
	out_buf[5] = cput_15_data[28];
	//logic card sn
	memcpy(&out_buf[6], &ch_cput_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpYKTTxnPurchase.AftBalance, 4);
	//lock flag
	out_buf[18] = 0;
	//rfu
	memset(&out_buf[19], 0x00, 14);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x01;
	//UD record length
	cnt = sizeof(YKTTxnPurchase_t);
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpYKTTxnPurchase.AFCHead_val.operatorid, cnt);
#ifdef	DEBUG_PRINT
	PRINTK("YKT PurchaseEntry:");
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", out_buf[39 + i]);
	PRINTK("\n");
#endif
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	if(cmd_buf[17] == 0x02)
	{
		ee_write_last_record(XA_CITY_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
		sem_post(&g_samreturn);
	}else 
	{
		(*out_len) += cnt;
		ee_write_last_record(XA_CITY_FAMILY, 0, &out_buf[33], cnt);
		reader_status = XA_RW_IDLE;
	}

	return CE_OK;
		
label_refuse_to_city_entry:
	if(ch_sz_cput_rollback != 0)
	{
		tpXACPUProtect[tpXACPUProtectIndex].rollBack = 0;
		memset(tpXACPUProtect[tpXACPUProtectIndex].phyicalID, 0x00, 8);
	}
	*out_len = 1;
	return chCode;
}
/************************************
suzhou tong exit
************************************/
char xa_tong_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode, chRejectCode, chExitStatus;
unsigned char buf[50], factor[20], des[80], deslen;
unsigned char cpubuf[80], cpulen, Le, ch_rollback_temp;
//unsigned char cnt;
unsigned char status, entry_station[2], entry_time[7];
unsigned long time1,time2, lngsrcstation;
unsigned short tempdate, shTicketType, shFare;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
long lngCardBalance, ret, i;
unsigned short shDays, cnt;
unsigned long lngMidnightSecond;

	tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t);
	*out_len = 4;
	memcpy(out_buf, "\x33\x01\x00\x00", 4);
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x id %02x%02x%02x%02x bak %02x%02x%02x%02x\n", ch_sz_cput_rollback, ch_cput_phyical_id[4], ch_cput_phyical_id[5], ch_cput_phyical_id[6], ch_cput_phyical_id[7]
					, ch_cput_phyical_id_bak[4], ch_cput_phyical_id_bak[5], ch_cput_phyical_id_bak[6], ch_cput_phyical_id_bak[7]);
#endif
//	if((ch_sz_cput_rollback != 0) && (memcmp(ch_cput_phyical_id, ch_cput_phyical_id_bak, 8) == 0))
//	{
//		if(0 != (chCode = city_auth(out_buf)))
//			return chCode;
//		if((chRejectCode = CPUT_gettransprove(0x09, out_buf)) == 0)
//		{
//			blncputRollback = 0xff;
//			goto label_sz_city_rollback_1;
//		}else if(chRejectCode == CE_READ)
//			return CE_READ;
//	}
	//

	tpYKTTxnPurchase.udSubtype = 0x0D;
	tpYKTTxnPurchase.udType = 0x21;
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_cput_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, ch_cput_psam_sn, 8);
	//tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_cput_phyical_id[4]);
	tpYKTTxnPurchase.CardCsn = XA_PAYTYPE_CARD;
	tpYKTTxnPurchase.OrigEntime = 0;
	tpYKTTxnPurchase.OrigEnlocation = 0;
	tpYKTTxnPurchase.mode = 0;
	//
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	tpCPU.lowsecond = lngMidnightSecond = timestr2long(&cmd_buf[7]);
	LongToByte(lngMidnightSecond, tpCPU.curtime);
	memcpy(tpYKTTxnPurchase.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnPurchase.TxnTime, &tpCPU.time_bcd[4], 3);

	get_degrade_mode(tpCPU.curstation);
	
	//select 3f01
	memcpy(out_buf, "\x72\x01", 2);
	//if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
	//	return chCode;
	//read file 15
	//memcpy(cput_15_data, out_buf, XA_CPUT_15_LEN);
	memcpy(ch_cput_logic_id, &cput_15_data[12], 8);
/*	memcpy(out_buf, "\x72\x05", 2);
	if((chCode = CPUT_GetFiles15(out_buf)) != 0)
		return chCode;
*/
	memcpy(tpYKTTxnPurchase.CityCode, &cput_15_data[2], 2);
	memcpy(tpYKTTxnPurchase.CardId, &cput_15_data[12], 8);
	xa_hex2bcd(&tpYKTTxnPurchase.CardId[4], &cput_15_data[16]);
	tpYKTTxnPurchase.CrdMKnd = cput_15_data[28];
	tpYKTTxnPurchase.CrdSKnd = cput_15_data[29];
	memcpy(ch_cput_logic_id, &cput_15_data[12], 8);
	tpYKTTxnPurchase.CrdVerNo = cput_15_data[9];

label_sz_rollback_main:
	//card City
	if((0 != memcmp(&cput_15_data[2], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[2], "\x00\x00", 2)))
		return CE_CARDSTATUS;
	//card union
	if((0 != memcmp(&cput_15_data[10], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[10], "\x00\x00", 2)))
		return CE_CARDSTATUS;
	//card status
	if(0 == cput_15_data[8])
		return CE_CARDSTATUS;
	//ticket definition
	memcpy(out_buf, "\x72\x07", 2);
	shTicketType = (cput_15_data[28] << 8) + cput_15_data[29];
	//XIAN CPU card
	//ticket definition
	tpYKTTxnPurchase.CrdModel = 0x01;		//CPU
	if((chCode = CPU_TellSysCard(shTicketType)) != 0)
	{
		return chCode;
	}
	//read file 21
	if(0 != (chCode = CPUT_GetFiles21(out_buf)))
		return chCode;
	
	//verify date
	memcpy(out_buf, "\x72\x08", 2);
	if((memcmp(tpCPU.time_bcd, &cput_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_15_data[24], 4) > 0))
		return CE_EXPIREDDATE;
	memcpy(tpYKTTxnPurchase.Validday, &cput_15_data[24], 4);
	switch(cput_21_data[0])
	{
 	case XA_PASSENGER_ELDER:	//old man
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
			return CE_EXPIREDDATE;
		memcpy(tpYKTTxnPurchase.Validday, &cput_21_data[60], 4);
		break; 
	case XA_PASSENGER_STUDENT:	//Student
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
			cput_21_data[0] = XA_PASSENGER_ADULT;
		memcpy(tpYKTTxnPurchase.Validday, &cput_21_data[60], 4);
		break;
	}
	//verify pin and get balance
	if((chCode = CPUT_GetFiles19(out_buf)) != 0)
		return chCode;
	tpYKTTxnPurchase.BefBalance = tpCPU.balance;
	//balance < max remaining value
	memcpy(out_buf, "\x72\x09", 2);
	//if(tpCPU.balance > tpTicketDef.MaxRemainingValue)
	{
		//return ERR_OVERMAX_AMOUNT;
	}
	memcpy(out_buf, "\x72\x0a", 2);
	//metro status
	if(0 != (chCode = CPUT_TellEntry(&chExitStatus, 0xff)))
	{
		if(chCode == CE_CUR_EXIT)
			goto label_refuse_to_city_exit;
		memcpy(out_buf, "\x72\x0a\x01\x00", 4);
		memset(entry_time, 0x00, 7);
		memcpy(&entry_time[1], &cpu_19_data[14], 5);
		lngLosecond1 = timestr2long(&entry_time[1]);
		if((cpu_19_data[19] == tpCPU.curstation[0]) && (cpu_19_data[20] == tpCPU.curstation[1]))
		{
			if(lngLosecond1 > tpCPU.lowsecond)
			{
				chCode = CE_CUR_EXIT;
			}else if((tpCPU.lowsecond - lngLosecond1) < 20 * 60)
			{
				chCode = CE_CUR_EXIT;
			}
		}
		//
		if(ch_sz_cput_rollback == MCPU_ROLL_AFTER_ED)
		{
			if(0 != (city_auth(out_buf)))
				return CE_CONSUME_MOVED;
			memcpy(capp_init, tpXACPUProtect[tpXACPUProtectIndex].capp_init, 19);
			if(0 == (chRejectCode = CPUT_gettransprove(0x09, out_buf)))
			{
				memcpy(tpYKTTxnPurchase.AFCHead_val.operatorid, tpXACPUProtect[tpXACPUProtectIndex].rec_buf, tpXACPUProtect[tpXACPUProtectIndex].rec_len);
				memcpy(tpCPU.sam_sn, tpXACPUProtect[tpXACPUProtectIndex].sam_sn, 4);
				tpCPU.balance = tpXACPUProtect[tpXACPUProtectIndex].balance;
				memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
				goto label_sz_city_rollback_1;
			}else if(chRejectCode == CE_INVADLIDCARD)
			{
				goto label_refuse_to_city_exit;
			}
			else
				return CE_CONSUME_MOVED;
		}
		goto label_refuse_to_city_exit;
	}
	//calculate the fare
	memcpy(out_buf, "\x72\x10", 2);
	lngsrcstation = 0x09000000 + (cpu_19_data[8] << 8) + cpu_19_data[9];
	tpYKTTxnPurchase.Enlocation = tpYKTTxnPurchase.lastlocation = lngsrcstation;
	if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, tpCmdInit.curstation, &shFare)))
		return chCode;
	//renew flag
	memcpy(out_buf, "\x72\x11", 2);
	//check
	memset(entry_time, 0x00, 7);
	memcpy(&entry_time[1], &cpu_19_data[3], 5);
	entry_time[0] = 0x20;
	if(tpTicketDef.IgnoreMaxJourneyTime == 0)
	{
		memcpy(out_buf, "\x72\x12", 2);
		if((chCode = cal_overtime(entry_time, tpCPU.time_bcd, shFare, 0)) != 0)
		{
			//
			if(chCode == CE_OVERTIME)
			{
				//if( (cput_21_data[0] != XA_PASSENGER_ELDER) && (cput_21_data[0] != XA_PASSENGER_DISABLED) )
				if(cput_21_data[0] != XA_PASSENGER_ELDER)
				{
					goto label_refuse_to_city_exit;
				}
					
			}else
				return chCode;
		}
	}
	//entry time ? current ttime
	if(tpTicketDef.ChargeFareOnCheckout == 0)
	{//using the entry time
		memcpy(out_buf, "\x72\x13", 2);
		if(0 != (chCode = cal_fare_value(entry_time, &tpTicketDef, shFare, cput_21_data[0], &tpSysPrice)))
			return chCode;
	}
	else
	{// using the current time
		memcpy(out_buf, "\x72\x14", 2);
		if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, cput_21_data[0], &tpSysPrice)))
			return chCode;
	}
	tpCPU.tranamount = tpSysPrice.price;
	//exit status
	cpu_19_data[21] = (cpu_19_data[21] & 0x80) | 0x02;
	if(tpwaivermode.cur_sta_fare)
	{
		if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngsrcstation, &shFare)))
			return chCode;
		if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, cput_21_data[0], &tpSysPrice)))
			return chCode;
		tpCPU.tranamount = tpSysPrice.price;
	}
	memcpy(out_buf, "\x72\x15", 2);
	//if current station is set to STATION FAILURE mode
	if(tpwaivermode.cur_sta_failure)
	{
		tpCPU.tranamount = 0;
		cpu_19_data[21] = (cpu_19_data[21] & 0x80) | 0x13;
	}
	//if OLDER card transaction amount is ZERO
	if((shTicketType == 0x0102) && (cput_21_data[0] == XA_PASSENGER_ELDER))
	{
		tpCPU.tranamount = 0;
	}
	if(tpCPU.balance < ((long)tpTicketDef.MinRemainingValue + tpCPU.tranamount))
	{//change to over fare
		if(((cpu_19_data[21] & 0x80) == 0) || (cpu_19_data[19] != tpCPU.curstation[0]) || (cpu_19_data[20] != tpCPU.curstation[1]))
		{
			chCode = CE_OVERRIDE;
			goto label_refuse_to_city_exit;
		}
		tpCPU.tranamount = 0; 
	}
	cpu_19_data[21] &= 0x7f;
	//debit for capp
	tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount;
	tpYKTTxnPurchase.Entime = timestr2long(&entry_time[1]) + TIME2000 - ZONE8;
	tpYKTTxnPurchase.AftBalance = tpCPU.balance - tpCPU.tranamount;

	memcpy(out_buf, "\x71\x20", 2);
	if(0 != (chCode = city_auth(out_buf)))
		return chCode;
	//city code
	memcpy(buf, &cput_15_data[2], 2);
	buf[2] = 0xFF;
	if(0 != (chCode = CPU_init_for_capp(1, tpCPU.tranamount, ch_cput_psam_id, ch_cput_logic_id, buf, 3, ch_cpu_mac_data)))
	{
		return chCode;
	}
	//update capp file 19
	//exit time- bcd YYMMDDHHMI
	memcpy(&cpu_19_data[14], &tpCPU.time_bcd[1], 5);
	//line station
	memcpy(&cpu_19_data[19],tpCPU.curstation, 2);
	//transaction amount
	ShortToByte((short)tpCPU.tranamount, &cpu_19_data[22]);
	//pointer 2
	cpu_19_data[24] = chExitStatus;
	memcpy(out_buf, "\x71\x21", 2);
	ret = CPU_update_capp(1, 0x19, cpu_19_data[0], XA_CPUT_19_LEN, cpu_19_data, 0);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	

	memcpy(out_buf, "\x71\x22", 2);
	ch_rollback_temp = SZ_CPU_CAPP_1;
	if(0 != CPU_debit_for_capppurchase(&ch_rollback_temp, NULL, out_buf))
	{
#ifdef DEBUG_PRINT
		PRINTK("debit failure %02x\n", ch_rollback_temp);
#endif
		ch_sz_cput_rollback = MCPU_ROLL_AFTER_ED;
		xa_tong_rollback_write(tpYKTTxnPurchase.AFCHead_val.operatorid, sizeof(YKTTxnPurchase_t));
		return CE_CONSUME_MOVED;
	}
#ifdef	DEBUG_ROLLBACK	
	if(tpYKTTxnPurchase.LocalTxnSeq == 1)
	{
		ch_sz_cput_rollback = MCPU_ROLL_AFTER_ED;
		xa_tong_rollback_write(tpYKTTxnPurchase.AFCHead_val.operatorid, sizeof(YKTTxnPurchase_t));
		return CE_CONSUME_MOVED;
	}
#endif

#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif
label_sz_city_rollback_1:

	if(ch_sz_cput_rollback != 0)
	{
		tpXACPUProtect[tpXACPUProtectIndex].rollBack = 0;
		memset(tpXACPUProtect[tpXACPUProtectIndex].phyicalID, 0x00, 8);
	}

	*out_len = 51;
	//card sn-2
	tpYKTTxnPurchase.CrdDebitCnt = ByteToShort(NULL, &capp_init[4]);
	//tac-4
	memcpy(&tpYKTTxnPurchase.TAC, tpCPU.tac, 4);
	//sam sn--4
	tpYKTTxnPurchase.SamSeq = ByteToLong(NULL, tpCPU.sam_sn);
		
	*out_len = 33;
	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_CITY_FAMILY;
	//ticket type
	//memcpy(&out_buf[4], &shTicketType, 2);
	out_buf[4] = cput_15_data[29];
	out_buf[5] = cput_15_data[28];
	//logic card sn
	memcpy(&out_buf[6], &ch_cput_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpYKTTxnPurchase.AftBalance, 4);
	//lock flag
	out_buf[18] = 0;
	//rfu
	memset(&out_buf[19], 0x00, 14);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x01;
	//UD record length
	cnt = sizeof(YKTTxnPurchase_t);
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpYKTTxnPurchase.AFCHead_val.operatorid, cnt);
#ifdef	DEBUG_PRINT
	PRINTK("YKT PurchaseExit:");
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", out_buf[39 + i]);
	PRINTK("\n");
#endif
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	if(cmd_buf[17] == 0x02)
	{
		ee_write_last_record(XA_CITY_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
		sem_post(&g_samreturn);
	}else 
	{
		(*out_len) += cnt;
		ee_write_last_record(XA_CITY_FAMILY, 0, &out_buf[33], cnt);
		reader_status = XA_RW_IDLE;
	}

	return CE_OK;

label_refuse_to_city_exit:
	//
	if(ch_sz_cput_rollback != 0)
	{
		tpXACPUProtect[tpXACPUProtectIndex].rollBack = 0;
		memset(tpXACPUProtect[tpXACPUProtectIndex].phyicalID, 0x00, 8);
	}
	*out_len = 4;
	return chCode;
	
}
/************************************
CPU update
************************************/
char xa_tong_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100], blnRec01, ch_rollback_temp;
unsigned char cpubuf[100], cpulen, Le, chEntryStatus;
unsigned char  last_timebcd[7];
unsigned long time2, lngsrcstation, lngLosecond1;
char chCode, chmonth, chRejectCode;
unsigned long lngTranTimes, lngTranAmount;
unsigned short cnt, shTicketType, shFare;
unsigned char blnOverfare, blnOvertime;

	tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t);
	*out_len = 4;
	memcpy(out_buf, "\x73\x01\x00\x00", 4);
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
		PRINTK("rollback %02x old phyical id", ch_sz_cput_rollback);
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cput_phyical_id[i]);
		PRINTK("  new phyical id ");
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cput_phyical_id_bak[i]);
#endif
	if((ch_sz_cput_rollback != 0) && (memcmp(ch_cput_phyical_id, ch_cput_phyical_id_bak, 8) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_cput_rollback);
#endif
		if((chRejectCode = CPUT_gettransprove(0x09, out_buf)) == 0)
		{
			goto label_sz_rollback_main;
		}else if(chRejectCode == CE_READ)
			return CE_READ;
	}
	memcpy(out_buf, "\x73\x02\x00\x00", 4);
	memcpy(tpCPU.time_bcd, &cmd_buf[10], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[11]);
	//record
	tpYKTTxnPurchase.udSubtype = 0x11;
	tpYKTTxnPurchase.udType = 0x21;
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[6], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_cput_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, ch_cput_psam_sn, 8);
	//调整为支付方式
	//tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_cput_phyical_id[4]);
	tpYKTTxnPurchase.CardCsn = (cmd_buf[20]);
	memcpy(tpYKTTxnPurchase.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnPurchase.TxnTime, &tpCPU.time_bcd[4], 3);
label_sz_rollback_main:
	memcpy(tpYKTTxnPurchase.CityCode, &cput_15_data[2], 2);
	memcpy(tpYKTTxnPurchase.CardId, &cput_15_data[12], 8);
	xa_hex2bcd(&tpYKTTxnPurchase.CardId[4], &cput_15_data[16]);
	tpYKTTxnPurchase.CrdMKnd = cput_15_data[28];
	tpYKTTxnPurchase.CrdSKnd = cput_15_data[29];
	memcpy(ch_cput_logic_id, &cput_15_data[12], 8);
	tpYKTTxnPurchase.CrdVerNo = cput_15_data[9];
	//read file 21
	if(0 != (chCode = CPUT_GetFiles21(out_buf)))
		return chCode;
	//verify pin and get balance
	if((chCode = CPUT_GetFiles19(out_buf)) != 0)
		return chCode;
	tpYKTTxnPurchase.AftBalance = tpYKTTxnPurchase.BefBalance = tpCPU.balance;
	//
	//ticket type
	memcpy(out_buf, "\x73\x08", 2);
	shTicketType = (cput_15_data[28] << 8) + cput_15_data[29];
	tpYKTTxnPurchase.CrdModel = 0x01;
	if((chRejectCode = CPU_TellSysCard(shTicketType)) != 0)
	{
		return chRejectCode;
	}
	//used
	memcpy(out_buf, "\x73\x09", 2);
	if(cput_15_data[8] == 0)
	{
		return CE_NONACTIVED;
	}
	//card City
	memcpy(out_buf, "\x73\x0A", 2);
	if((0 != memcmp(&cput_15_data[2], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[2], "\x00\x00", 2)))
	{
		return CE_CARDSTATUS;
	}
	//card union
	memcpy(out_buf, "\x73\x0B", 2);
	if((0 != memcmp(&cput_15_data[10], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[10], "\x00\x00", 2)))
	{
		return CE_CARDSTATUS;
	}
	//valid date
	memcpy(out_buf, "\x73\x0C", 2);
	if((memcmp(tpCPU.time_bcd, &cput_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_15_data[24], 4) > 0))
	{
		return CE_EXPIREDDATE;
	}
	memcpy(tpYKTTxnPurchase.Validday, &cput_15_data[24], 4);
	//check the year-date
	memcpy(out_buf, "\x73\x0D", 2);
	switch(cput_21_data[0])
	{
	case XA_PASSENGER_ELDER:	//old man
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
		{
			return CE_EXPIREDDATE;
		}
		memcpy(tpYKTTxnPurchase.Validday, &cput_21_data[60], 4);
		break;
	case XA_PASSENGER_STUDENT:	//Student
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
			cput_21_data[0] = XA_PASSENGER_ADULT;
		memcpy(tpYKTTxnPurchase.Validday, &cput_21_data[60], 4);
		break;
	}
	//
	memcpy(out_buf, "\x73\x10", 2);
	memset(last_timebcd, 0x00, 7);
	last_timebcd[0] = 0x20;
	memcpy(&last_timebcd[1], &cpu_19_data[3], 5);
	if(0 == (chRejectCode = CPUT_TellEntry(&chEntryStatus, 0)))
	{
		get_degrade_sensitive_mode(NULL, last_timebcd);
		if(tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_exit)
		{
			chRejectCode = CE_NO_ENTRY;
			chEntryStatus = cpu_19_data[13];
		}
	}
	
	if(cmd_buf[25] == 1)
	{//Fee area
		if(chRejectCode == 0)
		{//entry status
			tpYKTTxnPurchase.Entime = timestr2long(&last_timebcd[1]) + TIME2000 - ZONE8;
			//calculate the price
			lngsrcstation = 0x09000000 + (cpu_19_data[8] << 8) + cpu_19_data[9];
			tpYKTTxnPurchase.Enlocation = tpYKTTxnPurchase.lastlocation = lngsrcstation;
			//tpYKTTxnPurchase.OrigEnlocation = lngsrcstation;  //增加“延迟交易缺少进出站ID”的BUG修复
			if(0 != (chRejectCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, tpCmdInit.curstation, &shFare)))
			{
				return chRejectCode;
			}
			if(tpTicketDef.ChargeFareOnCheckout == 0)
			{
				if(0 != (chRejectCode = cal_fare_value(last_timebcd, &tpTicketDef, shFare, cput_21_data[0], &tpSysPrice)))
				{
					return chRejectCode;
				}
			}else
			{
				if(0 != (chRejectCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, cput_21_data[0], &tpSysPrice)))
				{
					return chRejectCode;
				}
			}
			if(((cpu_19_data[21] & 0x80) == 0) || (cpu_19_data[19] != tpCPU.curstation[0]) || (cpu_19_data[20] != tpCPU.curstation[1]))
			{
				if (tpCPU.balance < ((long)tpTicketDef.MinRemainingValue + tpSysPrice.price))
				{
					memcpy(&cpu_19_data[19], tpCPU.curstation, 2);
					cpu_19_data[21] |= 0x80;
				}
			}
			//check the overtime
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpCPU.time_bcd, 0, 0)))
				{
					if(cmd_buf[26] != 2)
						return CE_BADPARAM;
					memcpy(&cpu_19_data[3], &tpCPU.time_bcd[1], 5);
				}
				if( (cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
				{//deduct from card
					memcpy(&tpCPU.tranamount, &cmd_buf[21], 4);
					if(tpCPU.tranamount > tpCPU.balance)
						return CE_BADPARAM;
					tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount;
					tpYKTTxnPurchase.OrigAmt = 0;
				}else
				{//MONEY/ALIPay/WEPay/Union
					tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount = 0;
					memcpy(&tpYKTTxnPurchase.OrigAmt, &cmd_buf[21], 4);
				}
			}
		}else
		{
			if(cmd_buf[26] != 0x01)
				return CE_BADPARAM;
			memcpy(&cpu_19_data[3], &tpCPU.time_bcd[1], 5);
			//cmd_buf[27]---intel 
			cpu_19_data[8] = cmd_buf[28];
			cpu_19_data[9] = cmd_buf[27];
			lngsrcstation = 0x09000000 + (cpu_19_data[8] << 8) + cpu_19_data[9];
			tpYKTTxnPurchase.Enlocation = lngsrcstation;
			tpYKTTxnPurchase.lastlocation = 0;
			tpYKTTxnPurchase.Entime = 0;
			
			cpu_19_data[10] = 0x01;
			cpu_19_data[13] = chEntryStatus;
			tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount = 0;
		}
	}else
	{//NON-Fee area
		if(chRejectCode == 0)
		{//entry status
			if(cmd_buf[26] != 0x03)
				return CE_BADPARAM;
			cpu_19_data[21] = 0x02;
			cpu_19_data[24] = chEntryStatus;
			lngsrcstation = 0x09000000 + (cpu_19_data[8] << 8) + cpu_19_data[9];
			tpYKTTxnPurchase.OrigEnlocation = tpYKTTxnPurchase.Enlocation = tpYKTTxnPurchase.lastlocation = lngsrcstation;
			tpYKTTxnPurchase.OrigEntime = tpYKTTxnPurchase.Entime = timestr2long(&last_timebcd[1]) + TIME2000 - ZONE8;
			
			if( (cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
			{//FROM Card
				memcpy(&tpCPU.tranamount, &cmd_buf[21], 4);
				tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount;
			}else
			{//pay MONEY
				memcpy(&tpYKTTxnPurchase.OrigAmt, &cmd_buf[21], 4);
				tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount = 0;
			}
		}else
		{
			return CE_BADPARAM;
		}
	}
	tpYKTTxnPurchase.AftBalance = tpCPU.balance - tpCPU.tranamount;
	//initial capp 
	if(0 != (chCode = city_auth(out_buf)))
		return chCode;
	//
	memcpy(buf, &cput_15_data[2], 2);
	buf[2] = 0xFF;
	if(0 != (chCode = CPU_init_for_capp(1, tpCPU.tranamount, ch_cput_psam_id, ch_cput_logic_id, buf, 3, ch_cpu_mac_data)))
	{
		return chCode;
	}
	memcpy(out_buf, "\x4c\x17", 2);
	ret = CPU_update_capp(1, 0x19, cpu_19_data[0], XA_CPUT_19_LEN, cpu_19_data, 0);
	if(ret)
	{
		return CE_WRITE;
	}
	memcpy(out_buf, "\x4c\x18", 2);
	ch_rollback_temp = SZ_CPU_CAPP_1;
	if(CPU_debit_for_capppurchase(&ch_rollback_temp, sz_tong_ee_write, out_buf))
	{
		return CE_WRITE;
	}
	//card sn -2
	tpYKTTxnPurchase.CrdDebitCnt = ByteToShort(NULL, &capp_init[4]);
	//tac -4
	memcpy(&tpYKTTxnPurchase.TAC, tpCPU.tac, 4);
	//sam sn -4
	tpYKTTxnPurchase.SamSeq = ByteToLong(NULL, tpCPU.sam_sn);
	//UDSN
	out_buf[0] = 1;
	//recycle	1	0x00:no，0x01:yes，0x02:废票回收
	out_buf[1] = 0x00;
	//black
	out_buf[2] = 0x00;
	//ticket family
	out_buf[3] = XA_CITY_FAMILY;
	//ticket type
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cput_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpYKTTxnPurchase.AftBalance, 4);
	//lock flag
	out_buf[18] = 0;
	//rfu
	memset(&out_buf[19], 0x00, 14);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x01;
	//UD record length
	cnt = sizeof(YKTTxnPurchase_t);
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpYKTTxnPurchase.AFCHead_val.operatorid, cnt);
#ifdef	DEBUG_PRINT
	PRINTK("YKT PurchaseEntry:");
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", out_buf[39 + i]);
	PRINTK("\n");
#endif
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	(*out_len) += cnt;
	ee_write_last_record(XA_CITY_FAMILY, 0, &out_buf[33], cnt);
	reader_status = XA_RW_IDLE;

	return CE_OK;
	
}

/************************************
CPU inquire
************************************/
char xa_tong_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret;
unsigned char buf[100], factor[60], des[60], deslen;
unsigned char cpubuf[100], cpulen, Le, entry_time[4];
unsigned char cnt, last_timebcd[7];
unsigned long time2, i, lngsrcstation, lngLosecond1, lngenstation;
unsigned long lngOvertimePenalty, lngOverfarePenalty, lngPenalty;
unsigned char chCode, chRejectCode, chEntryStatus;
unsigned char chmonth, chEXRejectCode;
unsigned short shTicketType, shFare, datetypeid;
unsigned char blnOvertime, blnOverfare, timecodeid;
YKTTerminal_t	tpPurchase;
unsigned char 	YKTBlackCardId[8];
	
#ifdef DEBUG_PRINT_ADD
	PRINTK("\nYKT inquire command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("check %02x function %02x Fee %02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8]);
	PRINTK("time %02x%02x-%02x-%02x %02x:%02x:%02x antelena %02x flag %02x\n", 
		cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12], cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17]);
	
	PRINTK("station:%02x%02x%02x%02x history %02x\n", cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22]);
#endif
	*out_len = 4;
	memcpy(out_buf, "\x75\x01\x00\x00", 4);
	memcpy(tpCPU.time_bcd, &cmd_buf[9], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[10]);
	
	//check the deposit
	if(0 != (chCode = CPU_select_file("\x3F\x00", 2, out_buf, NULL)))
		return chCode;
	if(0 != (chCode = CPUT_GetFiles05(out_buf)))
		return chCode;
	if(0 != (chCode = CPU_select_file("\x3F\x01", 2, out_buf, NULL)))
		return chCode;
	//read file 1A
	if(0 != (chCode = CPUT_GetFiles1A(out_buf)))
		return chCode;
	//read file 21
	if(0 != (chCode = CPUT_GetFiles21(out_buf)))
		return chCode;
	//verify pin and get balance
	if((chCode = CPUT_GetFiles19(out_buf)) != 0)
		return chCode;

	get_degrade_mode(tpCPU.curstation);
	//*out_len = 214 + 22 + 1 + 9 * 23 + 31;
	*out_len = 214 + 22 + 1;
	chCode = CE_OK;
	//phiycial type
	out_buf[0] = XA_CITY_FAMILY;
	//length
	out_buf[1] = 214;
	//non-fee area
	out_buf[2] = 0;
	//feearea 
	out_buf[3] = 0;
	//amount for overfare-4
	memset(&out_buf[4], 0x00, 4);
	//amount for refund
	memset(&out_buf[8], 0x00, 4);
	//transaction for refund
	memset(&out_buf[12], 0x00, 4);
	//amount for RefundHandlingFee
	memset(&out_buf[16], 0x00, 4);
	//max balance
	out_buf[20] = 0;
	//antelena
	out_buf[21] = 0x01;
	//information
	//0015--30
	memcpy(&out_buf[22], cput_15_data, XA_CPUT_15_LEN);
	//0016--55
	memcpy(&out_buf[52], cput_16_data, 55);
	//balance--4
	memcpy(&out_buf[107], &tpCPU.balance, 4);
	//0019
	memcpy(&out_buf[111], &cpu_19_data[3], 22);
	//
	memset(last_timebcd, 0x00, 7);
	last_timebcd[0] = 0x20;
	memcpy(&last_timebcd[1], &cpu_19_data[3], 5);
	//
	memset(&out_buf[133], 0x00, 7);
	if((chEXRejectCode = CPUT_TellEntry(&chEntryStatus, 0)) == 0)
	{//entry status
		get_degrade_sensitive_mode(NULL, last_timebcd);
		if(tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_exit)
		{
			chEXRejectCode = CE_NO_ENTRY;
			out_buf[133] = 0;
		}else
			out_buf[133] = 1;
	}
	//001A record number
	out_buf[140] = 1;
	//001A---31
	memcpy(&out_buf[141], cput_1A_data, 31);
	//0021---64
	memcpy(&out_buf[172], cput_21_data, 64);
	//history
	out_buf[236] = 0x00;
	if(cmd_buf[22])
	{
		(*out_len) += 10 * 23;
		//history record number
		out_buf[236] = 0x0A;
		//history
		CPUT_GetFiles18(10, &out_buf[237]);
		//file 1A
		//memcpy(&out_buf[237 + 9 * XA_CPUT_18_LEN], cput_1A_data, 31);
	}
#ifdef	DEBUG_PRINT_ADD
	PRINTK("CPU tong inquire:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	//ticket type
	shTicketType = (cput_15_data[28] << 8) + cput_15_data[29];
	if((chRejectCode = CPU_TellSysCard(shTicketType)) != 0)
	{
		out_buf[2] = out_buf[3] = chRejectCode;
		return chCode;
	}
	if(0 != (chRejectCode = get_purchase_ticket(cput_15_data[29], cput_15_data[28], &tpPurchase)))
	{
		out_buf[2] = out_buf[3] = chRejectCode;
		return chCode;
	}
	//used
	if(cput_15_data[8] == 0)
	{
		out_buf[2] = out_buf[3] = CE_NONACTIVED;
		return chCode;
	}
	//
	if(0 != (chRejectCode = check_YKT_Black_Lock("\x71\x00", "\x03\x01", &cput_15_data[12], 0xFF, out_buf, out_len)))
	{
		out_buf[2] = out_buf[3] = chRejectCode;
#ifdef DEBUG_PRINT_ADD
		PRINTK("reject code black %02x\n", out_buf[2]);
#endif
		return chCode;
	}
	//card City
	if((0 != memcmp(&cput_15_data[2], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[2], "\x00\x00", 2)))
	{
		out_buf[2] = out_buf[3] = CE_CARDSTATUS;
		return chCode;
	}
	//card union
	if((0 != memcmp(&cput_15_data[10], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[10], "\x00\x00", 2)))
	{
		out_buf[2] = out_buf[3] = CE_CARDSTATUS;
		return chCode;
	}
	//valid date
	if((memcmp(tpCPU.time_bcd, &cput_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_15_data[24], 4) > 0))
	{
		out_buf[2] = out_buf[3] = CE_EXPIREDDATE;
		return chCode;
	}
	//check the year-date
	switch(cput_21_data[0])
	{
	case XA_PASSENGER_ELDER:	//old man
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
		{
			//for display the real expired date
			memcpy(&out_buf[46], &cput_21_data[60], 4);
			out_buf[2] = out_buf[3] = CE_EXPIREDDATE;
			return chCode;
		}
		break;
	case XA_PASSENGER_STUDENT:	//Student
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
			cput_21_data[0] = XA_PASSENGER_ADULT;
		break;
	}
	//reject the ELDER card on the peak time
	if(0 != (chRejectCode = check_peak_time(tpCPU.time_bcd, &timecodeid, &datetypeid)))
	{
		out_buf[2] = out_buf[3] = chRejectCode;
#ifdef DEBUG_PRINT_ADD
		PRINTK("reject code peak time %02x\n", out_buf[2]);
#endif
		return chCode;
	}
	if((timecodeid == 3) && (cput_21_data[0] == XA_PASSENGER_ELDER))
	{
		out_buf[2] = out_buf[3] = CE_OLD_PEAK;
#ifdef DEBUG_PRINT_ADD
		PRINTK("reject code peak %02x\n", out_buf[2]);
#endif
		return chCode;
	}
	//
	blnOverfare = blnOvertime = 0;
	if(cmd_buf[8] == 1)
	{//Fee area
		if(chEXRejectCode == 0)
		{//entry status
			//calculate the price
			lngsrcstation = 0x09000000 + (cpu_19_data[8] << 8) + cpu_19_data[9];
			if(0 != (chRejectCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, tpCmdInit.curstation, &shFare)))
			{
				out_buf[2] = chRejectCode;
				out_buf[3] = 0;
#ifdef DEBUG_PRINT_ADD
				PRINTK("reject code cal station fare %02x\n", out_buf[2]);
#endif
				return chCode;
			}
			if(tpTicketDef.ChargeFareOnCheckout == 0)
			{
				if(0 != (chRejectCode = cal_fare_value(last_timebcd, &tpTicketDef, shFare, cput_21_data[0], &tpSysPrice)))
				{
					out_buf[2] = chRejectCode;
					out_buf[3] =0;
#ifdef DEBUG_PRINT_ADD
					PRINTK("reject code cal entry-time value %02x\n", out_buf[2]);
#endif
					return chCode;
				}
			}else
			{
				if(0 != (chRejectCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, cput_21_data[0], &tpSysPrice)))
				{
					out_buf[2] = chRejectCode;
					out_buf[3] =0;
#ifdef DEBUG_PRINT_ADD
					PRINTK("reject code cal cur-time value %02x\n", out_buf[2]);
#endif
					return chCode;
				}
			}
			//check the overfare
			if(((cpu_19_data[21] & 0x80) == 0) || (cpu_19_data[19] != tpCPU.curstation[0]) || (cpu_19_data[20] != tpCPU.curstation[1]))
			{
				if (tpCPU.balance  < ((long)tpTicketDef.MinRemainingValue + tpSysPrice.price))
				{
					blnOverfare = 0xff;
					if(0 != (chRejectCode = cal_fare_value(tpCPU.time_bcd, &tpSJTTicketDef, shFare, 0x01, &tpSysPrice)))
					{
						out_buf[2] = chRejectCode;
						out_buf[3] =0;
#ifdef DEBUG_PRINT_ADD
						PRINTK("reject code cal SJT cur-time value %02x\n", out_buf[2]);
#endif
						return chCode;
					}
					out_buf[2] = CE_OVERRIDE;
					out_buf[3] = 0;
					lngOverfarePenalty = tpSysPrice.price;
					memcpy(&out_buf[4], &lngOverfarePenalty, 4);
				}
			}
			//check the overtime
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpCPU.time_bcd, 0, 0)))
				{
					blnOvertime = 0xff;
					out_buf[2] = chRejectCode;
					out_buf[3] = 0;
					//penalty
					lngOvertimePenalty = 0;
					//老人卡和爱心卡判断超时不收费
					if( (cput_21_data[0] != XA_PASSENGER_ELDER) && (cput_21_data[0] != XA_PASSENGER_DISABLED) )
					{
						if(tpStationPrice.SJTNum == 0)
						{
							lngOvertimePenalty = 500;
						}else
						{
							lngOvertimePenalty = tpStationPrice.SJTPrice[tpStationPrice.SJTNum - 1];
						}
					}
					memcpy(&out_buf[4], &lngOvertimePenalty, 4);
#ifdef DEBUG_PRINT
					PRINTK("reject code overtime %02x pennalty %08x \n", out_buf[2], lngOvertimePenalty);
#endif
				}
			}
			if(blnOverfare && blnOvertime)
			{
				lngPenalty = lngOverfarePenalty + lngOvertimePenalty;
				out_buf[2] = CE_OVERFARETIME;
				out_buf[3] = 0;
				memcpy(&out_buf[4], &lngPenalty ,4);
			}
		}else
		{
			out_buf[2] = chEXRejectCode;
			out_buf[3] = 0;
		}
	}else 
	{//NO-Fee-area
		if(chEXRejectCode == 0)
		{//entry status
			out_buf[2] = 0;
			memset(last_timebcd, 0x00, 7);
			last_timebcd[0] = 0x20;
			memcpy(&last_timebcd[1], &cpu_19_data[3], 5);
			lngLosecond1 = timestr2long(&last_timebcd[1]);
			//
			if((lngLosecond1 > tpCPU.lowsecond) && (memcmp(&tpCPU.curstation[0], &cpu_19_data[8], 2) == 0))
			{
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
			}else if(((tpCPU.lowsecond - lngLosecond1) < 20 * 60) && (memcmp(&tpCPU.curstation[0], &cpu_19_data[8], 2) == 0))
			{
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
			}else 
			{
				out_buf[3] = CE_FEE_UPDATE_ENTRY;
				if(cmd_buf[17] == 1)
				{//according to the exit station calculate the price
					memcpy(&lngsrcstation, &cmd_buf[18], 4);
					lngenstation = 0x09000000 + (cpu_19_data[8] << 8) + (cpu_19_data[9]);
					if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngenstation, &shFare)))
						return chCode;
					//using the current time
					if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, cput_21_data[0], &tpSysPrice)))
						return chCode;
					//
					memcpy(&out_buf[4], &tpSysPrice.price, 4);
				}else
				{//max price
					if(tpStationPrice.SJTNum == 0)
						lngPenalty = 500;
					else
						lngPenalty = tpStationPrice.SJTPrice[tpStationPrice.SJTNum - 1];
					memcpy(&out_buf[4], &lngPenalty, 4);
				}
			}
		}else
		{
			out_buf[2] = out_buf[3] = 0;
		}
	}
	return chCode;
}



/************************************
CPU add value 
************************************/
char xa_tong_add(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[800], key[8], mac2[4], pin[8];
unsigned char cpubuf[100], Le;
unsigned char ee_buf[20];
unsigned char chCode, chRejectCode;
unsigned long lngLoadvalue;
unsigned short shTicketType, cnt, shcnt, cpulen;
char temp;//20230710
//unsigned char out_bufbak[500];

YKTLoad_t	tpLoad;

	*out_len = 4;
	memcpy(out_buf, "\x38\x00\x00\x00", 4);
	//check whether rollback the last transation or not
	if(0 == ee_read(EE_CITY_BACKUP, 9, ee_buf))
	{
		if((ee_buf[0] !=0) && (memcmp(ch_cput_phyical_id, &ee_buf[1], 8) == 0))
			sz_tong_ee_read();
	}
	//10.0.9.11
	//22000
	if((ch_sz_cput_rollback != 0) && (memcmp(ch_cput_phyical_id, ch_cput_phyical_id_bak, 8) == 0))
	{
#ifdef DEBUG_PRINT_ADD
		PRINTK("need roll back %02x\n", ch_sz_cput_rollback);
#endif
		if(ch_sz_cput_rollback == SZ_CPU_LOAD_1)
		{
			if((chRejectCode = CPUT_gettransprove(0x02, out_buf)) == 0)
			{
				goto label_sz_rollback_1;//maybe special return code for the application can rollback the add value
			}else if(chRejectCode == CE_READ)
				return CE_ADD_MOVED;
		}
	}
	ch_sz_cput_rollback = 0;
	ee_write(EE_CITY_BACKUP, 1, &ch_sz_cput_rollback);
	//
	memcpy(out_buf, "\x38\x01\x00\x00", 4);
	memcpy(tpCPU.time_bcd, &cmd_buf[10], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[11]);
	if(memcmp(tpCPU.time_bcd, tpauthLogin.LimiTime, 7) > 0)
		return CE_NOAUTH;
	//check the deposit
	memcpy(out_buf, "\x38\x02\x00\x00", 4);
	if(0 != (chCode = CPU_select_file("\x3F\x00", 2, out_buf, NULL)))
		return chCode;
	memcpy(out_buf, "\x38\x03\x00\x00", 4);
	if(0 != (chCode = CPUT_GetFiles05(out_buf)))
		return chCode;
	memcpy(out_buf, "\x38\x04\x00\x00", 4);
	if(0 != (chCode = CPU_select_file("\x3F\x01", 2, out_buf, NULL)))
		return chCode;
	//YKT ADD
	tpYKTTxnLoad.udSubtype = 0x0e;
	tpYKTTxnLoad.udType = 0x08;
	memcpy(&tpYKTTxnLoad.LocalTxnSeq, &cmd_buf[6], 4);
	memcpy(tpYKTTxnLoad.SamId, ch_cput_isam_sn, 8);
	memcpy(tpYKTTxnLoad.PosId, ch_cput_isam_id, 6);
	tpYKTTxnLoad.SamSeq = 0;
	tpYKTTxnLoad.Deposit = (long)bcd2bin(cput_05_data[7]) * 100;
	//must be set to zero for the add value correctly.
	//card City
	memcpy(out_buf, "\x38\x10\x00\x00", 4);
	if((0 != memcmp(&cput_15_data[2], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[2], "\x00\x00", 2)))
		return CE_CARDSTATUS;
	//card union
	memcpy(out_buf, "\x38\x11\x00\x00", 4);
	if((0 != memcmp(&cput_15_data[10], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[10], "\x00\x00", 2)))
		return CE_CARDSTATUS;
	//card status
	memcpy(out_buf, "\x38\x12\x00\x00", 4);
	if(0 == cput_15_data[8])
		return CE_CARDSTATUS;
	//
	memcpy(tpYKTTxnLoad.CityCode, &cput_15_data[2], 2);
	memcpy(tpYKTTxnLoad.CardId, &cput_15_data[12], 8);
	xa_hex2bcd(&tpYKTTxnLoad.CardId[4], &cput_15_data[16]);
	//tpYKTTxnLoad.CardCsn = ByteToLong(NULL, &ch_cput_phyical_id[4]);
	tpYKTTxnLoad.CardCsn = (cmd_buf[17]);
	tpYKTTxnLoad.CardModel = 1;
	ch_sz_cput_rollback = 0;
	
	memcpy(out_buf, "\x38\x15", 2);
	shTicketType = (cput_15_data[28] << 8) + cput_15_data[29];
	if((chCode = CPU_TellSysCard(shTicketType)) != 0)
		return chCode;
	memcpy(out_buf, "\x38\x16\x00\x00", 4);
	if((chCode = get_load_ticket(cput_15_data[29], cput_15_data[28], &tpLoad)) != 0)
		return chCode;
	//black list card
	if(0 != (chCode = check_YKT_Black_Lock("\x71\x00", "\x03\x01", &cput_15_data[12], 0xFF, out_buf, out_len)))
		return chCode;
	memcpy(out_buf, "\x38\x17\x00\x00", 4);
	if(tpauthLogin.LimiAmt < tpLoad.perloadvalue)
		return CE_NOAUTH;
	//transaction
	memcpy(&tpCPU.tranamount, &cmd_buf[20], 4);
	//transaction amount must be times for basic amount
	memcpy(out_buf, "\x38\x18", 2);
	if(0 != (tpCPU.tranamount % tpLoad.perloadvalue))
		return CE_BADPARAM;
	memcpy(out_buf, "\x38\x19", 2);
	if(tpCPU.tranamount > tpLoad.maxloadvalue)
		return CE_BADPARAM;
	memcpy(out_buf, "\x38\x20", 2);
	if(tpLoad.enableload == 0)
		return CE_BADPARAM;
	tpYKTTxnLoad.CrdMKnd = cput_15_data[28];
	tpYKTTxnLoad.CrdSKnd = cput_15_data[29];
	tpYKTTxnLoad.TransType = 0x02;
	memcpy(tpYKTTxnLoad.SaleDate, &cput_15_data[20], 4);
	tpYKTTxnLoad.SaleMode = 1;
	memcpy(tpYKTTxnLoad.CardValDate, &cput_15_data[24], 4);
	memcpy(tpYKTTxnLoad.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnLoad.TxnTime, &tpCPU.time_bcd[4], 3);
	tpYKTTxnLoad.CrdVerNo = cput_15_data[9];
	memcpy(tpYKTTxnLoad.BatchNo, tpauthLogin.BatchNo, 3);
	memset(tpYKTTxnLoad.KeySeq, 0x00, 9);
	//20131015
	memset(tpYKTTxnLoad.AuthSeq, 0x00, 4);
	//verify date
	memcpy(out_buf, "\x38\x21", 2);
	//if((memcmp(tpCPU.time_bcd, &cput_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_15_data[24], 4) > 0))
	//	return CE_EXPIREDDATE;
	memcpy(out_buf, "\x38\x22\x00\x00", 4);
	if(0 != (chCode = CPUT_GetFiles1A(out_buf)))
		return chCode;
	if(cput_1A_data[9] == 0x02)
		tpYKTTxnLoad.Lasttype = 0x080e;
	else 
		tpYKTTxnLoad.Lasttype = cput_1A_data[9];
	
	memcpy(tpYKTTxnLoad.LastPosid, &cput_1A_data[10], 6);
	tpYKTTxnLoad.LastTxtAmt = ByteToLong(NULL, &cput_1A_data[5]);
	tpYKTTxnLoad.LastCrdDebitCnt = ByteToShort(NULL, &cput_1A_data[0]);
	memcpy(tpYKTTxnLoad.LastTxtTime, &cput_1A_data[16], 7);
	//20130620--return the file 1A real last
	tpYKTTxnLoad.LastBefBalance = ByteToLong(NULL, &cput_1A_data[23]);	// - ByteToLong(NULL, &cput_1A_data[5]);
	//20130927
	//tpYKTTxnLoad.LastTAC = ByteToLong(NULL, &cput_1A_data[27]);
	memcpy(tpYKTTxnLoad.LastTAC, &cput_1A_data[27], 4);
	memcpy(out_buf, "\x38\x23\x00\x00", 4);
	if(0 != (chCode = CPU_init_for_credit(tpCPU.tranamount, ch_cput_isam_id, out_buf)))
		return chCode;
#ifdef DEBUG_PRINT_ADD
	PRINTK("balance:%02x%02x%02x%02x ", capp_init[0], capp_init[1], capp_init[2], capp_init[3]);
	PRINTK("inline sn:%02x%02x ", capp_init[4], capp_init[5]);
	PRINTK("key version %02x algorithm %02x \n", capp_init[6], capp_init[7]);
	PRINTK("random %02x%02x%02x%02x mac1 %02x%02x%02x%02x\n", capp_init[8], capp_init[9], capp_init[10], capp_init[11], capp_init[12], capp_init[13], capp_init[14], capp_init[15]);
#endif
	ByteToLong(&tpCPU.balance, capp_init);
	tpYKTTxnLoad.BefBalance = tpCPU.balance;
	tpYKTTxnLoad.TxnAmt = tpCPU.tranamount;
	tpYKTTxnLoad.AftBalance = tpCPU.balance + tpCPU.tranamount;
	memcpy(out_buf, "\x38\x24", 2);
	if(tpYKTTxnLoad.AftBalance > tpLoad.maxbalance)
		return CE_BADPARAM;
	//inline-sn, overdraft, tranamount, trantype, isam, date/time, balance, tac
	ByteToShort(&shcnt, &capp_init[4]);
	tpYKTTxnLoad.CrdDebitCnt = shcnt;
	shcnt++;
	//memcpy(&cput_1A_data[0], &capp_init[4], 2);
	ShortToByte(shcnt, &cput_1A_data[0]);
	memset(&cput_1A_data[2], 0x00, 3);
	LongToByte(tpCPU.tranamount, &cput_1A_data[5]);
	cput_1A_data[9] = 0x02;
	memcpy(&cput_1A_data[10], ch_cput_isam_id, 6);
	memcpy(&cput_1A_data[16], tpCPU.time_bcd, 7);
	LongToByte((tpCPU.tranamount + tpCPU.balance), &cput_1A_data[23]);
	//back up the phyical id
	memcpy(ch_cput_phyical_id_bak, ch_cput_phyical_id, 8);
	//check the MAC1 and calculate the MAC2
	memcpy(out_buf, "\x38\x25\x00\x00", 4);
	if(0 != xa_load_mac2(mac2, out_buf))
		return CE_MACERR; 
	//credit for load
	memcpy(out_buf, "\x38\x26", 2);
	memcpy(buf, "\x80\x52\x00\x00\x0b", 5);
	memcpy(&buf[5], tpCPU.time_bcd, 7);
	memcpy(&buf[12], mac2, 4);
#ifdef DEBUG_PRINT_ADD
	PRINTK("credit mac2:");
	for(i = 0; i < 16; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x38\x27\x00\x00", 4);
	ret = mifpro_apdu(buf, 16, cpubuf, &cpulen);
	if(ret != 0)
	{
		ch_sz_cput_rollback = SZ_CPU_LOAD_1;
		sz_tong_ee_write(0);
		return CE_ADD_MOVED;
	}
#ifdef DEBUG_PRINT_ADD
	PRINTK("credit return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x38\x28", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))
	{
		memcpy(&out_buf[2], cpubuf, cpulen);
		*out_len = 4 + cpulen;
		return CE_ADD_MAC2;
	}
	memcpy(tpCPU.tac, cpubuf, 4);
#ifdef DEBUG_PRINT_ADD
	/***************add read balance again*******************
	printf("read balance again:\n");
	chCode = CPUT_GetFiles19(out_bufbak);
	printf("CPUT_GetFiles19:%02x\n",chCode);
	if(memcmp(&tpYKTTxnLoad.BefBalance,&tpCPU.balance,4))
	{
		printf("Balance different\n");
	}
	else
	{
		printf("Balance different\n");
	}*****************************************************/	
#endif
	
label_sz_rollback_1:
	//清除备份数据区
	if(ch_sz_cput_rollback != 0)
	{
		ch_sz_cput_rollback = 0;
		ee_write(EE_CITY_BACKUP, 1, &ch_sz_cput_rollback);
#ifdef DEBUG_PRINT_ADD
	printf("EE_CITY_BACKUP:\n");
#endif
	}
	memcpy(&cput_1A_data[27], tpCPU.tac, 4);
	//20130927
	//memcpy(&tpYKTTxnLoad.TAC, &tpCPU.tac, 4);
	memcpy(&tpYKTTxnLoad.TAC[0], &tpCPU.tac[0], 4);
	//tpYKTTxnLoad.TAC[1] = tpCPU.tac[2];
	//tpYKTTxnLoad.TAC[2] = tpCPU.tac[1];
	//tpYKTTxnLoad.TAC[3] = tpCPU.tac[0];

	tpauthLogin.LimiAmt -= tpCPU.tranamount;
	
	memcpy(tpYKTTxnLoad.CardValDate, &cput_15_data[24], 4);
	if(tpLoad.extentiondays != 0)
	{
#ifdef DEBUG_PRINT_ADD
		printf("tpLoad.extentiondays != 0\n");
#endif
		days2datestr((datestr2days(tpCPU.time_bcd) - DAY2000 + tpLoad.extentiondays), &cput_15_data[24]);
		//because add susseccfully so CAN'T check update 15 result
		if(0 == xa_update_city_15(cput_15_data, out_buf))
		{
#ifdef DEBUG_PRINT_ADD
			printf("xa_update_city_15 == 0\n");
#endif
			memcpy(tpYKTTxnLoad.CardValDate, &cput_15_data[24], 4);
		}		
	}
	//
	ch_sz_cput_rollback = SZ_CPU_LOAD_2;
	
	memcpy(out_buf, "\x38\x28", 2);
	calPin(&cput_15_data[12], pin);
	//if(0 != (CPUT_VerifyPIN(1, pin, 6, out_buf)))
	 temp = CPUT_VerifyPIN(1, pin, 6, out_buf);
   if(temp != 0)
	{
#ifdef DEBUG_PRINT_ADD
		printf("CPUT_VerifyPIN: %02x\n",temp);
#endif
		goto label_sz_rollback_main;
	}
		
	memset(buf, 0x00, 80);
	memcpy(&buf[8], "\x00\xE2\x00\x00", 4);
	buf[11] = (0x1A << 3);
	buf[12] = XA_CPUT_1A_LEN;
	memcpy(&buf[13], cput_1A_data, XA_CPUT_1A_LEN);
	
	memcpy(out_buf, "\x38\x29", 2);
#ifdef DEBUG_PRINT_ADD
	PRINTK("append record 1A:");
	for(i = 8; i < 5 + XA_CPUT_1A_LEN + 8; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(&buf[8], 5 + XA_CPUT_1A_LEN, cpubuf, &cpulen);
	if(ret != 0)
	{
		goto label_sz_rollback_main;
	}
#ifdef DEBUG_PRINT_ADD
	PRINTK("file 1A:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x38\x30", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))	
	{
		goto label_sz_rollback_main;
	}
label_sz_rollback_2:
label_sz_rollback_main:
	
	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_CITY_FAMILY;
	//ticket type-2
	memcpy(&out_buf[4], &cput_15_data[28], 2);
	//logic id-4
	//out_buf[6]
	//BefBalance-4
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//memcpy(&out_buf[10], &tpYKTTxnLoad.BefBalance, 4);//20230711
	//balance-4
	memcpy(&out_buf[14], &tpYKTTxnLoad.AftBalance, 4);
	//lock status -1
	out_buf[18] = 0x00;
	//purse type-1
	out_buf[19] = 1;
	//validity date-4
	memcpy(&out_buf[20], tpYKTTxnLoad.CardValDate, 4);
	//rfu-9
	memset(&out_buf[24], 0x00, 9);
	
	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x01;
	//UD record length
	cnt = sizeof(YKTTxnLoad_t);
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpYKTTxnLoad.AFCHead_val.operatorid, cnt);
#ifdef	DEBUG_PRINT_ADD
	PRINTK("YKT Add:");
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", out_buf[39 + i]);
	PRINTK("\n");
#endif

	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//
	//ee_write_last_record(XA_CITY_FAMILY, 0, &out_buf[33], cnt);
	reader_status = XA_RW_IDLE;
	(*out_len) += cnt;
	return CE_OK;
}

/************************************
CPU issue
************************************/
char xa_CPU_sale(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret;
unsigned char buf[80], factor[20], des[60], deslen;
unsigned char cpubuf[100], cpulen, Le;
unsigned char i, cpurandom[8];
unsigned char chCode, chRejectCode;
long lngSysBalance;
unsigned short cnt, shTicketType;
YKTLoad_t	tpLoad;

#ifdef DEBUG_PRINT
	PRINTK("\nsale command is %02x%02x and length is %02x%02x:\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("SN %02x%02x%02x%02x time %02x%02x-%02x-%02x %02x:%02x:%02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12], cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16]);
	PRINTK("chip type %02x ticket %02x type %02x%02x subtype %02x%02x saletype %02x passenger %02x ", cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22], cmd_buf[23], cmd_buf[24]);
	PRINTK("amount %02x%02x%02x%02x\n", cmd_buf[25], cmd_buf[26], cmd_buf[27], cmd_buf[28]);
	PRINTK("start %02x%02x%02x%02x end %02x%02x%02x%02x \n", cmd_buf[29], cmd_buf[30], cmd_buf[31], cmd_buf[32], cmd_buf[33], cmd_buf[34], cmd_buf[35], cmd_buf[36]);
	PRINTK("times:%02x%02x validduration %02x%02x%02x%02x\n", cmd_buf[37], cmd_buf[38], cmd_buf[39], cmd_buf[40], cmd_buf[41], cmd_buf[42]);
#endif
	*out_len = 4;
	//get the time infor
	memcpy(tpCPU.time_bcd, &cmd_buf[10], 7);
	//check the deposit
	if(0 != (chCode = CPU_select_file("\x3F\x00", 2, out_buf, NULL)))
		return chCode;
	if(0 != (chCode = CPUT_GetFiles05(out_buf)))
		return chCode;
	if(0 != (chCode = CPU_select_file("\x3F\x01", 2, out_buf, NULL)))
		return chCode;
	
	//YKT sale
	tpYKTTxnLoad.udSubtype = 0x0f;
	tpYKTTxnLoad.udType = 0x08;
	memcpy(&tpYKTTxnLoad.LocalTxnSeq, &cmd_buf[6], 4);
	memcpy(tpYKTTxnLoad.PosId, ch_cput_isam_id, 6);
	memcpy(tpYKTTxnLoad.SamId, ch_cput_isam_sn, 8);
	//tpYKTTxnLoad.CardCsn = ByteToLong(NULL, &ch_cput_phyical_id[4]);
	tpYKTTxnLoad.CardCsn = toMoto(cmd_buf[43]);
	memcpy(tpYKTTxnLoad.SaleDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnLoad.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnLoad.TxnTime, &tpCPU.time_bcd[4], 3);
	tpYKTTxnLoad.Deposit = (long)bcd2bin(cput_05_data[7]) * 100;
	
	memcpy(out_buf, "\x40\x05\x00\x00", 4);
	//card City
	if((0 != memcmp(&cput_15_data[2], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[2], "\x00\x00", 2)))
		return CE_CARDSTATUS;
	//card union
	if((0 != memcmp(&cput_15_data[10], "\x71\x00", 2)) && (0 != memcmp(&cput_15_data[10], "\x00\x00", 2)))
		return CE_CARDSTATUS;
	memcpy(out_buf, "\x40\x06", 2);
	//card status
	if(cput_15_data[8] != 0)
	{
		return ERR_TICKET_STATUS;
	}
	//test mode
	//transaction amount
	tpCPU.tranamount = 0;
	//select file
#ifdef DEBUG_PRINT
	PRINTK("file 15 Issued:%02x%02x City:%02x%02x Bussiness:%02x%02x RFU%02x%02x Flag:%02x app ver:%02x \n", cput_15_data[0], cput_15_data[1], cput_15_data[2], cput_15_data[3],
			cput_15_data[4], cput_15_data[5], cput_15_data[6], cput_15_data[7], cput_15_data[8], cput_15_data[9]);
	PRINTK("CityUnion:%02x%02x sn:%02x%02x%02x%02x %02x%02x%02x%02x\n",
			 cput_15_data[10], cput_15_data[11], cput_15_data[12], cput_15_data[13], cput_15_data[14], cput_15_data[15], cput_15_data[16], cput_15_data[17], cput_15_data[18], cput_15_data[19]);
	PRINTK(" app startdate %02x%02x%02x%02x valid date %02x%02x%02x%02x M-type %02x S-type%02x\n", 
			cput_15_data[20], cput_15_data[21], cput_15_data[22], cput_15_data[23], cput_15_data[24], cput_15_data[25], cput_15_data[26], cput_15_data[27], cput_15_data[28], cput_15_data[29]);
#endif
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, CE_OK, 0xF0, out_buf, 2);
#endif
	memcpy(tpYKTTxnLoad.CityCode, &cput_15_data[2], 2);
	memcpy(tpYKTTxnLoad.CardId, &cput_15_data[12], 8);
	xa_hex2bcd(&tpYKTTxnLoad.CardId[4], &cput_15_data[16]);
	tpYKTTxnLoad.CardModel = 1;
	tpYKTTxnLoad.CrdMKnd = cput_15_data[28]; 
	tpYKTTxnLoad.CrdSKnd = cput_15_data[29];
	tpYKTTxnLoad.TransType = 0x80;
	tpYKTTxnLoad.SaleMode = 1;
	//ticket definition
	memcpy(out_buf, "\x40\x07", 2);
	shTicketType = (cput_15_data[28] << 8) + cput_15_data[29];
	if((chCode = CPU_TellSysCard(shTicketType)) != 0)
	{
		return chCode;
	}
	if((chCode = get_load_ticket(cput_15_data[29], cput_15_data[28], &tpLoad)) != 0)
		return chCode;
	//	
	if(tpLoad.enabledsale == 0)
		return CE_FORBID_TICKET;
	tpYKTTxnLoad.CrdVerNo = cput_15_data[9];
	memcpy(tpYKTTxnLoad.BatchNo, tpauthLogin.BatchNo, 3);
	memset(tpYKTTxnLoad.KeySeq, 0x00, 9);
	//20131015
	memset(tpYKTTxnLoad.AuthSeq, 0x00, 4);
	//read blance and file 19
	memcpy(out_buf, "\x32\x0a", 2);
	if((chCode = CPUT_GetFiles19(out_buf)) != 0)
	{
		return chCode;
	}
	//balance may be zero
	tpYKTTxnLoad.BefBalance = tpCPU.balance;
	tpYKTTxnLoad.TxnAmt = tpCPU.balance;
	tpYKTTxnLoad.AftBalance = tpCPU.balance;
	tpYKTTxnLoad.CrdDebitCnt = 0;
	//udpate 15 file using line protected mac write-get random
	cput_15_data[8] = 0x01;
	memcpy(&cput_15_data[20], tpCPU.time_bcd, 4);
	if(tpLoad.extentiondays != 0)
	{
		days2datestr((datestr2days(tpCPU.time_bcd) - DAY2000 + tpLoad.extentiondays), &cput_15_data[24]);
	}
	memcpy(tpYKTTxnLoad.CardValDate, &cput_15_data[24], 4);
	if((chCode = xa_update_city_15(cput_15_data, out_buf)) != 0)
		return chCode;
	
	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_CITY_FAMILY;
	//ticket type-2
	out_buf[4] = cput_15_data[28];
	out_buf[5] = cput_15_data[29];
	//logic id -4
	//BefBalance-4
	LongToByte(tpCPU.balance, &out_buf[10]);
	//balance-4
	LongToByte(tpCPU.balance, &out_buf[14]);
	//lock status -1
	out_buf[18] = 0x00;
	//rfu-14
	memset(&out_buf[19], 0x00, 14);
	
	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x01;
	//UD record length
	cnt = sizeof(YKTTxnLoad_t);
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpYKTTxnLoad.AFCHead_val.operatorid, cnt);
#ifdef	DEBUG_PRINT
	PRINTK("YKT Sale:");
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", out_buf[39 + i]);
	PRINTK("\n");
#endif
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	ee_write_last_record(XA_CITY_FAMILY, 0, &out_buf[33], cnt);
	reader_status = XA_RW_IDLE;
	(*out_len) += cnt;
	return CE_OK;
}

char xa_update_city_15(unsigned char *file_buf, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], cpurandom[8], key[2];
unsigned char cpubuf[100], Le;
unsigned char chCode, chRejectCode;
unsigned char factor[20], des[80], deslen, time_bcd[7];
unsigned short cpulen;

	//udpate 15 file
	memcpy(buf, "\x00\x84\x00\x00\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		PRINTK("get cpu card random failure\n");
		return CE_READERROR;
	}
#ifdef DEBUG_PRINT
	PRINTK("get random :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x13", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READERROR;
	}
	memset(cpurandom, 0x00, 8);
	memcpy(cpurandom, cpubuf, 4);
	memcpy(out_buf, "\x44\x14", 2);
	memset(buf, 0x00, 80);
	memcpy(&buf[8], "\x04\xd6\x95\x00", 4);
	buf[12] = XA_CPUT_15_LEN + 4;
	memcpy(&buf[13], file_buf, XA_CPUT_15_LEN);
	memcpy(buf, cpurandom, 8);
	buf[5 + 8 + XA_CPUT_15_LEN] = 0x80;	
	//cal mac
	memcpy(key, "\x45\x01", 2);
	memset(factor, 0x00, 16);
	memcpy(factor, &cput_15_data[12], 8);
	memcpy(&factor[8], ch_cput_ats, 8);

	if((ret = cpu_cal_protect_mac(xa_tong_isam_index, &factor[8], 8, "\x25\x02", buf, 5 + 8 + XA_CPUT_15_LEN + 5, des)) != 0)
	{
		PRINTK("line mac return %d\n", ret);
		return  CE_NOMETROSAM;
	}
	memcpy(&buf[5 + 8 + XA_CPUT_15_LEN], des, 4);
	
#ifdef DEBUG_PRINT
	PRINTK("credit:");
	for(i = 8; i < 5 + XA_CPUT_15_LEN + 4 + 8; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x15", 2);
	ret = mifpro_apdu(&buf[8], 5 + XA_CPUT_15_LEN + 4, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITEERROR;
	}
#ifdef DEBUG_PRINT
	PRINTK("file 15:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x16", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))	
	{
		return CE_INVADLIDCARD;
	}
	return 0;
}

int get_purchase_ticket(unsigned char subtype, unsigned char maintype, YKTTerminal_t *td)
{
unsigned long	i;
	
	if(tpTerminal1919.offset[0].section_rec == 0)
		return CE_EOD_FILE;
	//set the truct to zero
	memset(td, 0x00, sizeof(YKTTerminal_t));
	
	for(i = 0; i < tpTerminal1919.offset[0].section_rec; i++)
	{
		if((subtype == tpTerminal1919.YKTTerminal_val[i].subtype) &&
			(maintype == tpTerminal1919.YKTTerminal_val[i].maintype))
		{
			memcpy(td, &tpTerminal1919.YKTTerminal_val[i].phyical, sizeof(YKTTerminal_t));
			return 0;
		}
	}
	return CE_EOD_FILE;
}

int get_load_ticket(unsigned char subtype, unsigned char maintype, YKTLoad_t *td)
{
unsigned long	i;
	
	if(tpLoad1914.offset[0].section_rec == 0)
		return CE_EOD_FILE;
	//set the truct to zero
	memset(td, 0x00, sizeof(YKTLoad_t));
	
	for(i = 0; i < tpLoad1914.offset[0].section_rec; i++)
	{
		if((subtype == tpLoad1914.YKTLoad_val[i].subtype) &&
			(maintype == tpLoad1914.YKTLoad_val[i].maintype))
		{
			memcpy(td, &tpLoad1914.YKTLoad_val[i].phyical, sizeof(YKTLoad_t));
			return 0;
		}
	}
	return CE_EOD_FILE;
}

/*
parameter:
	1 card SN(8 bytes)
	2 PIN(8 bytes)
*/
void calPin(char *pszCardsn, char *pin)
{
char szMAK[17] = {0};
char szCardNo[17] = {0};
int a, b;
long i;

	memcpy(szMAK, "4163958262438749", 16);
	sprintf(szCardNo, "%02X%02X%02X%02X%02X%02X%02X%02X", 
		pszCardsn[0], pszCardsn[1], pszCardsn[2], pszCardsn[3], pszCardsn[4], pszCardsn[5], pszCardsn[6], pszCardsn[7]);

	for (i = 0; i < 16; i++)
	{
		a = szCardNo[i] - '0';
		b = szMAK[i] - '0';
		if((i % 2) == 0)
			pin[i / 2] = ((a + b) % 10) << 4;
		else
			pin[i / 2] += ((a + b) % 10);
	}
	
	return ;
}

char xa_load_mac2(unsigned char *mac2, unsigned char *out_buf)
{
unsigned char cpubuf[100], sambuf[100], samlen;
long i;

	//initial mac1
	memset(cpubuf, 0x00, 100);
	memcpy(cpubuf, "\x80\x1A\x2B\x01\x08", 5);
	memcpy(&cpubuf[5], &cput_15_data[12], 8);
#ifdef DEBUG_PRINT
	for(i = 0; i < 13; i++)
		PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if(0 != sam_apdu(xa_tong_isam_index, cpubuf, 13, sambuf, &samlen, 0, 2))
	{
#ifdef	DEBUG_PRINT
		PRINTK("init ret len %02x %02x%02x\n", samlen, sambuf[0], sambuf[1]);
#endif
		return CE_NOPARAM;
	}
	//calculate mac1 and check
	memcpy(cpubuf, "\x80\xFA\x01\x00\x18", 5);
	//initial load random
	memcpy(&cpubuf[5], &capp_init[8], 4);
	//inline SN
	memcpy(&cpubuf[9], &capp_init[4], 2);
	//balance
	memcpy(&cpubuf[11], &capp_init[0], 4);
	//transaction amount
	LongToByte(tpCPU.tranamount, &cpubuf[15]);
	//transaction type
	cpubuf[19] = 0x02;
	//deviceid
	memcpy(&cpubuf[20], ch_cput_isam_id, 6);
	//0x800000
	memcpy(&cpubuf[26], "\x80\x00\x00", 3);
#ifdef DEBUG_PRINT
	for(i = 0; i < 29; i++)
		PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if(0 != sam_apdu(xa_tong_isam_index, cpubuf, 29, sambuf, &samlen, 0, 2))
	{
#ifdef	DEBUG_PRINT
		PRINTK("mac ret len %02x %02x%02x\n", samlen, sambuf[0], sambuf[1]);
#endif
		return CE_NOPARAM;
	}
#ifdef	DEBUG_PRINT
	PRINTK("mac ret len %02x %02x%02x\n", samlen, sambuf[0], sambuf[1]);
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(cpubuf, "\x00\xc0\x00\x00", 4);
		cpubuf[4] = sambuf[1];
		if(sam_apdu(xa_tong_isam_index, cpubuf, 5, sambuf, &samlen, 0, 0) != 0)
		{
			return -6;
		}
	}
#ifdef	DEBUG_PRINT
	PRINTK("reget len %02x:", samlen);
	for(i = 0; i < samlen; i++)
		PRINTK(" %02x", sambuf[i]);
	PRINTK("\n");
#endif
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
	{
		return -10;
	}
	//error
	if(memcmp(&capp_init[12], sambuf, 4) != 0)
	;//	return CE_MACERR;
	
	//initial mac2
	memset(cpubuf, 0x00, 100);
	memcpy(cpubuf, "\x80\x1A\x2B\x01\x08", 5);
	memcpy(&cpubuf[5], &cput_15_data[12], 8);
#ifdef DEBUG_PRINT
	for(i = 0; i < 13; i++)
		PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if(0 != sam_apdu(xa_tong_isam_index, cpubuf, 13, sambuf, &samlen, 0, 2))
	{
#ifdef	DEBUG_PRINT
		PRINTK("init ret len %02x %02x%02x\n", samlen, sambuf[0], sambuf[1]);
#endif
		return CE_NOPARAM;
	}
	//calculate mac2
	memcpy(cpubuf, "\x80\xFA\x01\x00\x18", 5);
	//initial load random
	memcpy(&cpubuf[5], &capp_init[8], 4);
	//inline SN
	memcpy(&cpubuf[9], &capp_init[4], 2);
	//transaction amount
	LongToByte(tpCPU.tranamount, &cpubuf[11]);
	//transaction type
	cpubuf[15] = 0x02;
	//deviceid
	memcpy(&cpubuf[16], ch_cput_isam_id, 6);
	//date time
	memcpy(&cpubuf[22], tpCPU.time_bcd, 7);
#ifdef DEBUG_PRINT
	for(i = 0; i < 29; i++)
		PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if(0 != sam_apdu(xa_tong_isam_index, cpubuf, 29, sambuf, &samlen, 0, 2))
	{
#ifdef	DEBUG_PRINT
		PRINTK("mac ret len %02x %02x%02x\n", samlen, sambuf[0], sambuf[1]);
#endif
		return CE_NOPARAM;
	}
#ifdef	DEBUG_PRINT
	PRINTK("mac ret len %02x %02x%02x\n", samlen, sambuf[0], sambuf[1]);
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(cpubuf, "\x00\xc0\x00\x00", 4);
		cpubuf[4] = sambuf[1];
		if(sam_apdu(xa_tong_isam_index, cpubuf, 5, sambuf, &samlen, 0, 0) != 0)
		{
			return -6;
		}
	}
#ifdef	DEBUG_PRINT
	PRINTK("reget len %02x:", samlen);
	for(i = 0; i < samlen; i++)
		PRINTK(" %02x ", sambuf[i]);
	PRINTK("\n");
#endif
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
	{
		return -10;
	}
	//
	memcpy(mac2, sambuf, 4);
	return 0;
}

char xa_auth_login_init(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char buf[100], sambuf[100], sambytes;
unsigned char chCode;
long 	i;	
	
#ifdef	DEBUG_PRINT
	PRINTK("login-init command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("time %02x%02x-%02x-%02x %02x:%02x:%02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	PRINTK("SN:%02x%02x\n", cmd_buf[13], cmd_buf[14]);
#endif
	*out_len = 2;
	//auth log-in or log-out
	tpauthLogin.head.startFlag = 0xF0;
	tpauthLogin.head.length = sizeof(tpauthLogin);
	tpauthLogin.head.protocalFlag = 0x01;
	tpauthLogin.head.protocalVersion = 0x01;
	tpauthLogin.head.formatVersion = 0x01;
	tpauthLogin.head.transfertype = 0x00;
	tpauthLogin.head.rfu1 = 0xff;
	tpauthLogin.head.command = 0x03;
	memcpy(tpauthLogin.head.acc, "\x5F\x00\x00\x01\x98\x04", 6);
	tpauthLogin.head.rfu2 = 0x00;
	tpauthLogin.head.packagetotal = 0x01;
	tpauthLogin.head.packagesn = 0x01;
	tpauthLogin.head.rfu3 = 0x00;
	tpauthLogin.head.crc = 0x00;
	tpauthLogin.head.rfu4 = 0xFF;
	memcpy(&tpauthLogout.head.startFlag, &tpauthLogin.head.startFlag, sizeof(struct auth_head));
	//set the 
	memset(tpauthLogin.MsgCode, 0x00, (sizeof(struct auth_in) - sizeof(struct auth_head)));
	memcpy(tpauthLogin.MsgCode, "\x05\x08", 2);
	memcpy(tpauthLogin.Unitid, "\x71\x00\x03\x01", 4);
	tpauthLogin.TxnMode = 0x00;
	memcpy(tpauthLogin.IsamId, ch_cput_isam_sn, 8);
	memcpy(tpauthLogin.PosId, ch_cput_isam_id, 6);
	memcpy(&tpauthLogin.Termid[2], tpCmdInit.deviceID, 4);
	memcpy(&tpauthLogin.Operid[5], tpCmdInit.operationid, 3);
	//
	memcpy(tpauthLogin.head.curtime, &cmd_buf[6], 7);
	memcpy(&tpauthLogin.head.sn, &cmd_buf[13], 2);
	//
	memcpy(buf, "\x00\x84\x00\x00\x08", 5);
	if(0 != sam_apdu(xa_tong_isam_index, buf, 5, sambuf, &sambytes, 0, 0))
		return CE_NOMETROSAM;
	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
		return CE_NOMETROSAM;
	//
	memcpy(tpauthLogin.Random, sambuf, 8);
	//
	tpauthLogin.endFlag = 0xFF;
	*out_len = sizeof(struct auth_in);
	memcpy(out_buf, &tpauthLogin.head.startFlag, sizeof(struct auth_in));
#ifdef DEBUG_PRINT
	PRINTK("login init: %04x\n", *out_len);
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	return CE_OK;
}

char xa_auth_login(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char buf[100], sambuf[100], sambytes;
unsigned char chCode;
long 	i;	
short cnt;
FILE	*fl;

#ifdef	DEBUG_PRINT
	PRINTK("longin command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	memcpy(&cnt, &cmd_buf[1], 2);
	PRINTK("login: %04x\n", cnt);
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", cmd_buf[i + 6]);
	PRINTK("\n");
#endif
	*out_len = 4;
	//set
	memcpy(&tpauthLogin.head.startFlag, &cmd_buf[6], sizeof(struct auth_in));
	//check package valid
	memcpy(out_buf, "\x54\x02\x00\x01", 4);
	if(tpauthLogin.head.startFlag != 0xF0)
		return CE_BADPARAM;
	memcpy(out_buf, "\x54\x02\x00\x02", 4);
	if(memcmp(tpauthLogin.IsamId, ch_cput_isam_sn, 8) != 0)
		return CE_BADPARAM;
	memcpy(out_buf, "\x54\x02\x00\x03", 4);
	if(tpauthLogin.ResponseCode != 0)
		return CE_BADPARAM;
	//extern authorization
	memcpy(buf, "\x00\x82\x00\x01\x08", 5);
	memcpy(&buf[5], tpauthLogin.DivElement, 8);
	if(0 != sam_apdu(xa_tong_isam_index, buf, 13, sambuf, &sambytes, 0, 0))
		return CE_NOMETROSAM;
	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
		return CE_NOMETROSAM;
#ifdef DEBUG_PRINT
	PRINTK("auth amount %d auth time %02x%02x-%02x-%02x %02x:%02x:%02x\n", 
			tpauthLogin.LimiAmt, tpauthLogin.LimiTime[0], tpauthLogin.LimiTime[1], tpauthLogin.LimiTime[2], tpauthLogin.LimiTime[3], tpauthLogin.LimiTime[4], tpauthLogin.LimiTime[5], tpauthLogin.LimiTime[6]);
#endif	
	//write auth info to file
	fl = fopen("./para/auth", "w+");
		fwrite(&tpauthLogin.head.startFlag, 1, sizeof(struct auth_in), fl);
	fclose(fl);
	//settle date
	memcpy(tpauthLogout.SettDate, tpauthLogin.SettDate, 4);
	memcpy(tpauthLogout.BatchNo, tpauthLogin.BatchNo, 3);
	
	*out_len = 11;
	//auth amount
	memcpy(&out_buf[0], &tpauthLogin.LimiAmt, 4);
	//auth time
	memcpy(&out_buf[4], tpauthLogin.LimiTime, 7);
	return CE_OK;
}

char xa_auth_logout_init(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char buf[100], sambuf[100], sambytes;
unsigned char chCode;
long 	i;	
	
#ifdef	DEBUG_PRINT
	PRINTK("logout-init command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("time %02x%02x-%02x-%02x %02x:%02x:%02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	PRINTK("SN:%02x%02x\n", cmd_buf[13], cmd_buf[14]);
	PRINTK("TotalNumber %02x%02x%02x%02x TotalAmount %02x%02x%02x%0x TotalDeposit %02x%02x%02x%02x\n", 
			cmd_buf[15], cmd_buf[16], cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22], cmd_buf[23], cmd_buf[24], cmd_buf[25], cmd_buf[26]);
#endif
	//auth log-in or log-out
	tpauthLogout.head.startFlag = 0xF0;
	tpauthLogout.head.length = sizeof(tpauthLogout);
	tpauthLogout.head.protocalFlag = 0x01;
	tpauthLogout.head.protocalVersion = 0x01;
	tpauthLogout.head.formatVersion = 0x01;
	tpauthLogout.head.transfertype = 0x00;
	tpauthLogout.head.rfu1 = 0xff;
	tpauthLogout.head.command = 0x03;
	memcpy(tpauthLogout.head.acc, "\x5F\x00\x00\x01\x98\x04", 6);
	tpauthLogout.head.rfu2 = 0x00;
	tpauthLogout.head.packagetotal = 0x01;
	tpauthLogout.head.packagesn = 0x01;
	tpauthLogout.head.rfu3 = 0x00;
	tpauthLogout.head.crc = 0x00;
	tpauthLogout.head.rfu4 = 0xFF;
	//set zero
	memcpy(tpauthLogout.MsgCode, "\x06\x08", 2);
	memcpy(tpauthLogout.Unitid, "\x71\x00\x03\x01", 4);
	tpauthLogout.TxnMode = 0x00;
	memcpy(tpauthLogout.IsamId, ch_cput_isam_sn, 8);
	memcpy(tpauthLogout.PosId, ch_cput_isam_id, 6);
	memcpy(&tpauthLogout.Termid[2], tpCmdInit.deviceID, 4);
	memcpy(&tpauthLogout.Operid[5], tpCmdInit.operationid, 3);
	//
	memcpy(tpauthLogout.head.curtime, &cmd_buf[6], 7);
	memcpy(&tpauthLogout.head.sn, &cmd_buf[13], 2);
	//memcpy(tpauthLogout.SettDate, tpauthLogin.SettDate, 4);
	//memcpy(tpauthLogout.BatchNo, tpauthLogin.BatchNo, 3);
	//statistic
	memcpy(&tpauthLogout.TotalSvNum, &cmd_buf[15], 4);
	memcpy(&tpauthLogout.TotalSvAmt, &cmd_buf[19], 4);
	memcpy(&tpauthLogout.TotalSaleDep, &cmd_buf[23], 4);
	memset(tpauthLogout.Reserved, 0xFF, 10);
	//
	tpauthLogout.endFlag = 0xFF;
	*out_len = sizeof(struct auth_out);
	memcpy(out_buf, &tpauthLogout.head.startFlag, sizeof(struct auth_out));
#ifdef DEBUG_PRINT
	PRINTK("logout init: %04x\n", *out_len);
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	return CE_OK;
}

char xa_auth_logout(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char buf[100], sambuf[100], sambytes;
unsigned char chCode;
long 	i;	
short cnt;
FILE	*fl;

#ifdef	DEBUG_PRINT
	PRINTK("longout command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	memcpy(&cnt, &cmd_buf[1], 2);
	PRINTK("logout: %04x\n", cnt);
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", cmd_buf[i + 6]);
	PRINTK("\n");
#endif
	//set 
	memcpy(&tpauthLogout.head.startFlag, &cmd_buf[6], sizeof(struct auth_out));
	//check 
	if(tpauthLogout.head.startFlag != 0xF0)
		return CE_BADPARAM;
	//delete the auth file
	remove("./para/auth");
	
	tpauthLogin.LimiAmt = 0;
	memset(tpauthLogin.LimiTime, 0x00, 7);
	
	*out_len = 0;
	return CE_OK;
}

void xa_hex2bcd(unsigned char *bcd, unsigned char *hex)
{
unsigned long lnghex;
unsigned char buf[100];

	lnghex = ByteToLong(NULL, hex);
	memset(buf, 0x00, 100);
	sprintf(buf, "%08d", lnghex);
	//
	bcd[0] = ((buf[0] - '0') << 4) + (buf[1] - '0');
	bcd[1] = ((buf[2] - '0') << 4) + (buf[3] - '0');
	bcd[2] = ((buf[4] - '0') << 4) + (buf[5] - '0');
	bcd[3] = ((buf[6] - '0') << 4) + (buf[7] - '0');
	
	return ;
}

char check_YKT_Black_Lock(unsigned char *city, unsigned char *business, unsigned char *SN, unsigned char blnBlock, unsigned char *out_buf, unsigned short *out_len)
{
unsigned long lngBlacklist;
long lngCSCLowIndex, lngCSCHighIndex, lngCSCMidIndex;
unsigned char bytBlackCSCId[12], bytParaCSCId[12], blnBlacklist, des[8];
int ret, i;
unsigned char buf[100], cpubuf[100], cpurandom[8], factor[16];
unsigned short	cnt, cpulen;

    blnBlacklist = 0xff;
    memcpy(&bytBlackCSCId[0], city, 2);
    memcpy(&bytBlackCSCId[2], business, 2);
    memcpy(&bytBlackCSCId[4], SN, 4);
    xa_hex2bcd(&bytBlackCSCId[8], &SN[4]);
#ifdef	DEBUG_PRINT
	PRINTK("%02x%02x %02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
		bytBlackCSCId[0], bytBlackCSCId[1], bytBlackCSCId[2], bytBlackCSCId[3], bytBlackCSCId[4], bytBlackCSCId[5],
		bytBlackCSCId[6], bytBlackCSCId[7], bytBlackCSCId[8], bytBlackCSCId[9], bytBlackCSCId[10], bytBlackCSCId[11]);
#endif
    //
    if(tpBlacklist1901.offset == NULL)
    	return 0;
    //use the binary method to find the BLACK No
    lngCSCLowIndex = 0;
    lngCSCHighIndex = tpBlacklist1901.offset[0].section_rec;
    if(tpBlacklist1901.offset[0].section_rec == 0)
    {
    	return 0;
    }
    //do
    while(lngCSCLowIndex <= lngCSCHighIndex)
    {
	    lngCSCMidIndex = (lngCSCLowIndex + lngCSCHighIndex) / 2;
        memcpy(&bytParaCSCId[0], tpBlacklist1901.YKTBlack_val[lngCSCMidIndex].CityCode, 2);
        memcpy(&bytParaCSCId[2], tpBlacklist1901.YKTBlack_val[lngCSCMidIndex].Business, 2);
        memcpy(&bytParaCSCId[4], tpBlacklist1901.YKTBlack_val[lngCSCMidIndex].cardid, 8);
        ret = memcmp(bytParaCSCId, bytBlackCSCId, 12);
        if(ret > 0)
        //要检索的卡号在前半部分
            lngCSCHighIndex = lngCSCMidIndex - 1;
        else if(ret < 0)
        //要检索的卡号在后半部分
            lngCSCLowIndex = lngCSCMidIndex + 1;
        else if(ret == 0)
        {
        //
            blnBlacklist = 0;
            break;
        }
        lngCSCMidIndex = (lngCSCLowIndex + lngCSCHighIndex) / 2;
    }
    if(blnBlacklist)
    	return 0;
    if(blnBlock)
    	return CE_BLACKLIST;
    //lock City CPU Card
    memset(buf, 0x00, 100);
    memcpy(buf, "\x00\x84\x00\x00\x04", 5);
    ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
    if(ret != 0)
    {
    	return CE_READ;
    }
    if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
    	return CE_READ;
    //
    memset(cpurandom, 0x00, 8);
    memcpy(cpurandom, cpubuf, 4);
    //
    memset(factor, 0x00, 16);
    memcpy(factor, ch_cput_ats, 8);
    memcpy(&factor[8], &cput_15_data[2], 2);
    factor[10] = 0xFF;
    //
    memset(buf, 0x00, 100);
    memcpy(buf, cpurandom, 8);
    //application lock 
    memcpy(&buf[8], "\x84\x1e\x00\x00\x04", 5);
    //application unlock
    //memcpy(&buf[8], "\x84\x18\x00\x00\x04", 5);
    buf[5 + 8] = 0x80;
    ret = cpu_cal_protect_mac(xa_tong_psam_index, factor, 16, "\x45\x01", buf, 5 + 8 + 3, des);
    if(ret)
    {
    	return CE_NOMETROSAM;
    }
    memcpy(&buf[5 + 8], des, 4);
#ifdef DEBUG_PRINT
	PRINTK("applock:");
	for(i = 8; i < 5 + 4 + 8; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	ret = mifpro_apdu(&buf[8], 5 + 4, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITEERROR;
	}
#ifdef DEBUG_PRINT
	PRINTK("return:");
	PRINTK("%02x%02x \n", cpubuf[0], cpubuf[1]);
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))	
	{
		return CE_WRITEERROR;
	}
	//generate the Record
	tpYKTTxnPurchase.udSubtype = 0x16;
	tpYKTTxnPurchase.udType = 0x21;
	tpYKTTxnPurchase.CrdDebitCnt = 0;
	tpYKTTxnPurchase.TAC = 0;
	tpYKTTxnPurchase.SamSeq = 0;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x01;
	//UD record length
	cnt = sizeof(YKTTxnPurchase_t);
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpYKTTxnPurchase.AFCHead_val.operatorid, cnt);
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	ee_write_last_record(XA_CITY_FAMILY, 1, &out_buf[33], cnt);
	reader_status = XA_RW_RECORD;
	sem_post(&g_samreturn);
	
   return 1;

}