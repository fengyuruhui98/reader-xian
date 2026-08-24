#include "xa_transport_operation.h"

#include "linux2440lib.h"
#include "xa_error_code.h"
#include "xdr_file_manage.h"
#include "bin_file_manage.h"
#include "xa_sam.h"
#include "serial.h"
#include "eeprom.h"
#include "hh_cpu_operation.h"
#include "xa_operation.h"
#include "time_tools.h"
#include "xa_tong_operation.h"

unsigned char bln_transport_xian;			//

unsigned char transport_15_data[30];		//公共应用信息文件
unsigned char transport_16_data[55];		//持卡人基本信息文件
unsigned char transport_17_data[60];		//管理信息文件
unsigned char transport_1A_data[128];		//公共交通过程信息变长记录文件
unsigned char transport_1E_data[48];		//公共交通过程信息循环记录文件

extern unsigned char cpu_19_data[64];
extern unsigned char cput_05_data[30];			//public application file


char Transport_GetFiles15(unsigned char *out_buf)
{
int ret;
unsigned char buf[40];
unsigned char cpubuf[300], Le;
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
	memcpy(transport_15_data, cpubuf, SZ_CPUT_15_LEN);
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	return 0;
}

char Transport_GetFiles16(unsigned char *out_buf)
{
unsigned char buf[100], cpubuf[300];
unsigned short cpulen;
int ret;

	memcpy(out_buf, "\x4b\x0a", 2);
	memcpy(buf, "\x00\xb0\x87\x00", 4);
	buf[4] = SZ_TRANSPORT_16_LEN;
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		memcpy(&out_buf[2], cpubuf, 2);
		return CE_READ;
	}
	memcpy(out_buf, "\x4b\x0b", 2);
#ifdef DEBUG_PRINT
	PRINTK("file 16 len %02x persontype:%02x name %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x idtype:%02x persionid:%02x%02x%02x%02x%02x%02x\n", 
		cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3], cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7], cpubuf[8], cpubuf[9], cpubuf[10],
		cpubuf[21], cpubuf[22], cpubuf[23], cpubuf[24], cpubuf[25], cpubuf[26], cpubuf[27]);
#endif
	if( (cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00) )
		return CE_INVADLIDCARD;
	memcpy(transport_16_data, cpubuf, SZ_TRANSPORT_16_LEN);
}

char Transport_GetFiles17(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[80];
unsigned char cpubuf[300], Le;
unsigned short cpulen;

	//read file 17 
	memcpy(out_buf, "\x4b\xa3", 2);
	memcpy(buf,"\x00\xb0\x97\x00\x0b", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_INVADLIDCARD;
	}
	if((cpulen == 2) && (cpubuf[0] == 0x6c))
	{
		buf[4] = Le = cpubuf[1];
		ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
		if(ret != 0)
			return CE_INVADLIDCARD;
	}
	if( (cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00) )
	{
#ifdef	DEBUG_PRINT
		PRINTK("file 17:%02x %02x\n", cpubuf[0], cpubuf[1]);
#endif
		return CE_INVADLIDCARD;
	}
	memcpy(transport_17_data, cpubuf, cpulen - 2);
#ifdef DEBUG_PRINT
	for(i = 0; i < cpulen; i++)
		PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
	PRINTK("file 17:len%02x code:%02x%02x%02x%02x province:%02x%02x city:%02x%02x union:%02x%02x type:%02x\n", 
		cpulen, transport_17_data[0], transport_17_data[1], transport_17_data[2], transport_17_data[3], transport_17_data[4], transport_17_data[5], transport_17_data[6], transport_17_data[7], transport_17_data[8], transport_17_data[9], transport_17_data[10]);
#endif

	//
	return 0;
}


char Transport_GetFiles19(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40];
unsigned char cpubuf[300], Le;
unsigned short cpulen;
	
	//read file 19 - variable file length
	memcpy(out_buf, "\xf0\x22", 2);
	memcpy(buf, "\x00\xb2\x02\x00\x30", 5);
	buf[3] = (0x19 << 3) | 0x04;
	buf[4] = Le = XA_TRANSPROT_19_LEN;
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
#ifdef DEBUG_PRINT
	PRINTK("read file 19:len%d ", Le);
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf0\x24", 2);
	if(cpulen != Le + 2)
	{
		return CE_INVADLIDCARD;
	}
	memcpy(cpu_19_data, cpubuf, XA_TRANSPROT_19_LEN);
#ifdef DEBUG_PRINT
	for(i = 0; i < cpulen; i++)
		PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
	PRINTK("file 19 consume flag %02x recordlen %02x lockedflag %02x recordver %02x\n", 
		cpu_19_data[0], cpu_19_data[1], cpu_19_data[2], cpu_19_data[3]);
	PRINTK("file 19:entry station %02x%02x device %02x%02x entry time: %02x%02x%02x%02x entry mode:%02x\n", 
		cpu_19_data[4], cpu_19_data[5], cpu_19_data[6], cpu_19_data[7], cpu_19_data[8], cpu_19_data[9], cpu_19_data[10], cpu_19_data[11], cpu_19_data[12]);
	PRINTK(" used no.%02x%02x status %02x trantimes:%02x%02x%02x%02x rejectcode %02x reject time %02x%02x%02x%02x\n",
		cpu_19_data[13], cpu_19_data[14], cpu_19_data[15], cpu_19_data[16], cpu_19_data[17], cpu_19_data[18], cpu_19_data[19], cpu_19_data[20], cpu_19_data[21], cpu_19_data[22], cpu_19_data[23], cpu_19_data[24]);
	PRINTK(" renewflag %02x renewstation %02x%02x exitstation %02x%02x exit device %02x%02x exit time %02x%02x%02x%02x firsttrandate%02x%02x times %02x\n",
		cpu_19_data[25], cpu_19_data[26], cpu_19_data[27], cpu_19_data[28], cpu_19_data[29], cpu_19_data[30], cpu_19_data[31], cpu_19_data[32], cpu_19_data[33], cpu_19_data[34], cpu_19_data[35], cpu_19_data[36], cpu_19_data[37], cpu_19_data[38]);
#endif

	//read balance
	memcpy(out_buf, "\xfa\x20", 2);
	memcpy(buf, "\x80\x5c\x00\x02\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("read balance return len %d %02x %02x %02x %02x \n", cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3]);
#endif
	memcpy(out_buf, "\xf0\x21", 2);
	if(cpulen != 6)
	{
		return CE_INVADLIDCARD;
	}
	ByteToLong(&tpCPU.balance, cpubuf);
#ifdef DEBUG_PRINT
	PRINTK("cpu balance:%d\n", tpCPU.balance);
#endif

	return 0;
}

char Transport_GetFiles1A(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40];
unsigned char cpubuf[300], Le;
unsigned short cpulen;	

	//read file 1a - variable file length
	memcpy(out_buf, "\xf0\x22", 2);
	memcpy(buf, "\x00\xb2\x01\x00\x30", 5);
	buf[3] = (0x1a << 3) | 0x04;
	buf[4] = Le = TRANSPORT_1A_LEN;
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
#ifdef DEBUG_PRINT
	PRINTK("read file 1a:len%d ", Le);
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf0\x24", 2);
	if(cpulen != Le + 2)
	{
		return CE_INVADLIDCARD;
	}
	memcpy(transport_1A_data, cpubuf, TRANSPORT_1A_LEN);
#ifdef DEBUG_PRINT
	for(i = 0; i < cpulen; i++)
		PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
	PRINTK("file 1a consume flag %02x%02x recordlen %02x %02x UNFlag %02x lockedflag %02x\n", 
		transport_1A_data[0], transport_1A_data[1], transport_1A_data[2], transport_1A_data[3], transport_1A_data[4], transport_1A_data[5]);
	PRINTK(" SN %02x%02x%02x%02x %02x%02x%02x%02x status %02x en-city %02x%02x ex-city %02x%02x en-organition %02x%02x%02x%02x %02x%02x%02x%02x ex-organition %02x%02x%02x%02x %02x%02x%02x%02x\n", 
		transport_1A_data[6], transport_1A_data[7], transport_1A_data[8], transport_1A_data[9], transport_1A_data[10], transport_1A_data[11], transport_1A_data[12], transport_1A_data[13], 
		transport_1A_data[14], transport_1A_data[15], transport_1A_data[16], transport_1A_data[17], transport_1A_data[18], transport_1A_data[19], transport_1A_data[20], transport_1A_data[21], transport_1A_data[22], 
		transport_1A_data[23], transport_1A_data[24], transport_1A_data[25], transport_1A_data[26], transport_1A_data[27], transport_1A_data[28], transport_1A_data[29], transport_1A_data[30], transport_1A_data[31], transport_1A_data[32], transport_1A_data[33], transport_1A_data[34]);
	PRINTK(" en-station %02x%02x%02x%02x %02x%02x%02x%02x ex-station %02x%02x%02x%02x %02x%02x%02x%02x en-device %02x%02x%02x%02x %02x%02x%02x%02x ex-device %02x%02x%02x%02x %02x%02x%02x%02x \n",
		transport_1A_data[35], transport_1A_data[36], transport_1A_data[37], transport_1A_data[38], transport_1A_data[39], transport_1A_data[40], transport_1A_data[41], transport_1A_data[42], transport_1A_data[43],
		transport_1A_data[44], transport_1A_data[45], transport_1A_data[46], transport_1A_data[47], transport_1A_data[48], transport_1A_data[49], transport_1A_data[50], transport_1A_data[51], transport_1A_data[52],
		transport_1A_data[53], transport_1A_data[54], transport_1A_data[55], transport_1A_data[56], transport_1A_data[57], transport_1A_data[58], transport_1A_data[59], transport_1A_data[60], transport_1A_data[61], 
		transport_1A_data[62], transport_1A_data[63], transport_1A_data[64], transport_1A_data[65], transport_1A_data[66]);
	PRINTK(" en-time %02x%02x-%02x-%02x %02x:%02x:%02x ex-time %02x%02x-%02x-%02x %02x:%02x:%02x maxConsume %02x%02x%02x%02x en-line %02x ex-line %02x \n",
		transport_1A_data[67], transport_1A_data[68], transport_1A_data[69], transport_1A_data[70], transport_1A_data[71], transport_1A_data[72] ,transport_1A_data[73],
		transport_1A_data[74], transport_1A_data[75], transport_1A_data[76], transport_1A_data[77], transport_1A_data[78], transport_1A_data[79], transport_1A_data[80],
		transport_1A_data[81], transport_1A_data[82], transport_1A_data[83], transport_1A_data[84], transport_1A_data[85], transport_1A_data[86]);
	PRINTK("  en-tranamount %02x%02x%02x%02x en-balance %02x%02x%02x%02x ex-tranamount %02x%02x%02x%02x \n",
		transport_1A_data[87], transport_1A_data[88], transport_1A_data[89], transport_1A_data[90], transport_1A_data[91], transport_1A_data[92], transport_1A_data[93], transport_1A_data[94], transport_1A_data[95], transport_1A_data[96], transport_1A_data[97], transport_1A_data[98]);
	PRINTK("  reject %02x renewflag %02x reject time %02x%02x%02x%02x\n", transport_1A_data[122], transport_1A_data[123], transport_1A_data[124], transport_1A_data[125], transport_1A_data[126], transport_1A_data[127]);
#endif

	//read balance---P1 00表示含透支金额  修改为03可以读取不含透支金额
	memcpy(out_buf, "\xfa\x20", 2);
	memcpy(buf, "\x80\x5c\x03\x02\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("read balance return len %d %02x %02x %02x %02x \n", cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3]);
#endif
	memcpy(out_buf, "\xf0\x21", 2);
	if(cpulen != 6)
	{
		return CE_INVADLIDCARD;
	}
	ByteToLong(&tpCPU.balance, cpubuf);
#ifdef DEBUG_PRINT
	PRINTK("cpu balance:%d\n", tpCPU.balance);
#endif
	
	return 0;
}

void xa_Transport_rollback_write(unsigned char *rec1, unsigned short rec1_len, unsigned char *rec2, unsigned short rec2_len)
{
unsigned short addr;
unsigned char i, j;
unsigned long protectSecond;

	//find the first NO rollBack
	for(i = 0; i < 10; i++)
	{
		if(memcmp(tpTransportProtect[i].logicID, ch_transport_logic_id, 10) == 0)
			break;
	}
	if(i >= 10)
	{
		for(i = 0; i < 10; i++)
		{
			if(tpTransportProtect[i].rollBack == 0)
				break;
		}
	}
	//if list is full then rewrite the FIRST record
	if(i >= 10)
	{
		i = 0;
		protectSecond = tpTransportProtect[i].usecond;
		for(j = 0; j < 10; j++)
		{
			if(protectSecond < tpTransportProtect[j].usecond)
			{
				i = j;
				protectSecond = tpTransportProtect[j].usecond;
			}
		}
	}
	//
	tpTransportProtect[i].rollBack = ch_sz_transport_rollback;
	//tpTransportProtect[i].Code = ch_transport_code_bak;
	memcpy(tpTransportProtect[i].phyicalID, ch_transport_phyical_id, 8);
	memcpy(tpTransportProtect[i].logicID, ch_transport_logic_id, 10);
	tpTransportProtect[i].usecond = ~tpCPU.lowsecond;
	tpTransportProtect[i].tranAmount = tpCPU.tranamount;
	tpTransportProtect[i].balance = tpCPU.balance;
	//
	if(rec1 != NULL)
		memcpy(tpTransportProtect[i].rec_buf, rec1, rec1_len);
	tpTransportProtect[i].rec_len = rec1_len;
	memcpy(&tpTransportProtect[i].rec_buf[rec1_len], &rec2_len, 2);
	if(rec2 != NULL)
	{
		memcpy(&tpTransportProtect[i].rec_buf[rec1_len + 2], rec2, rec2_len);
	}
	//memcpy(tpTransportProtect[i].cpu_15_data, transport_15_data, 50);
	//memcpy(tpTransportProtect[i].cpu_17_data, transport_17_data, 60);
	//memcpy(tpTransportProtect[i].cpu_19_data, cpu_19_data, 64);
	memcpy(tpTransportProtect[i].capp_init, capp_init, 19);
	//memcpy(&tpTransportProtect[i].tpCPU, &tpCPU, sizeof(struct cpu));
	//memcpy(&tpTransportProtect[i].tpTicketDef, &tpTicketDef, sizeof(Ticket_t));
	
	return ;
}

/*
*/
void xa_Transport_rollback_read(unsigned char *rec1, unsigned char *rec2)
{
unsigned short addr;
unsigned char i;

	//
	memcpy(rec1, tpTransportProtect[tpTransportProtectIndex].rec_buf, tpTransportProtect[tpTransportProtectIndex].rec_len);
	memcpy(&addr, &tpTransportProtect[tpTransportProtectIndex].rec_buf[tpTransportProtect[tpTransportProtectIndex].rec_len], 2);
	memcpy(rec2, &tpTransportProtect[tpTransportProtectIndex].rec_buf[tpTransportProtect[tpTransportProtectIndex].rec_len + 2], addr);
	
	memcpy(capp_init, tpTransportProtect[tpTransportProtectIndex].capp_init, 19);
	memcpy(ch_transport_logic_id, tpTransportProtect[tpTransportProtectIndex].logicID, 10);

	return ;
}

/*
function:external authorization
*/
char Transport_gettransprove(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], factor[20], des[60], deslen;
unsigned char cpubuf[300], cpurandom[8], device_id[6];
char chCode, purchase_init_bak[19];
unsigned short purchase_sn, purchase_sn_bak, cpulen;

	//get transaction prove
	memcpy(out_buf, "\xf0\xa0", 2);
	memset(buf, 0x00, 40);
	memcpy(buf, "\x80\x5a\x00\x00\x02", 5);
	buf[3] = 0x09;
	memcpy(purchase_init_bak, &tpTransportProtect[tpTransportProtectIndex].capp_init[0], 19);
	ByteToShort(&purchase_sn, &purchase_init_bak[4]);
	purchase_sn += 1;
	ShortToByte(purchase_sn, &purchase_init_bak[4]);
	memcpy(&buf[5], &purchase_init_bak[4], 2);
	ret = mifpro_apdu(buf, 7, cpubuf, &cpulen);
#ifdef DEBUG_PRINT
	PRINTK("get trans prove data:");
	for(i = 0; i < 7; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	if(ret != 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("get transaction prove failure\n");
#endif
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
		memcpy(capp_init, tpTransportProtect[tpTransportProtectIndex].capp_init, 19);
		memcpy(tpCPU.tac, &cpubuf[4], 4);
		return 0;
	}

	return CE_INVADLIDCARD;
}

/*
function:
	1. check the entry block MAC
	2. whether check the mode or not. if FALSE then return the entry mac result.
return:
	0: check MAC ok and return correctly MAC
	18: entry MAC wrong .
*/
char Transport_TellEntry(char entryMAC, unsigned char mode_check)
{
char mac[4], chret;
unsigned long i, InitStationNum, InitSensitiveNum;
unsigned char chInitStationFare, chInitSensitiveFare, chCode;
unsigned char eod_station[4], sta_close_entry[4], sen_close_entry[4];
unsigned short shCalFare;
unsigned long lngsrcstation, lngdesstation;

	chret = CE_NO_ENTRY;
	
	//0x01 for last is entry status and 0x04 for last is update
	if((entryMAC == 0x01) || (entryMAC == 0x04) )
		chret = 0;

	if(!mode_check)
		return chret;
	if(chret == CE_NO_ENTRY)
	{
	//if current station is set to entry mode then the entry station/time is current station/time
		if(tpwaivermode.cur_sta_entry)
		{
			//entry time is set to the current time
			memcpy(&cpu_19_data[11], &tpCPU.time_bcd[1], 5);
			//entry station is set the current station
			cpu_19_data[9] = tpCPU.curstation[0];
			cpu_19_data[10] = tpCPU.curstation[1];
			//
			memcpy(&transport_1A_data[15], XA_CODE_CITY, 2);
			memcpy(&transport_1A_data[19], XA_CODE_ORGANIZATION, 8);
			memcpy(&transport_1A_data[35], "\x44\x12\x79\x10\x00\x00", 6);
			memcpy(&transport_1A_data[41], tpCPU.curstation, 2);
			memcpy(&transport_1A_data[51], "\x79\x10\x00\x00", 4);
			memcpy(&transport_1A_data[55], &tpCPU.curstation[0], 4);
			memcpy(&transport_1A_data[67], tpCPU.time_bcd, 7);
			chret = 0;
		}else if(tpwaivermode.oth_sta_entry || tpwaivermode.sen_sta_entry)
		{
			//entry time is set to the current time
			memcpy(&cpu_19_data[11], &tpCPU.time_bcd[1], 5);
			memcpy(&transport_1A_data[67], tpCPU.time_bcd, 7);
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
			//if mode list waiver_entry station is zero
			if((InitStationNum == 0))
				return chret;
			memcpy(eod_station, sta_close_entry, 4);
			//entry station is set the most close station
			lngsrcstation = (eod_station[0] << 24) + (eod_station[1] << 16) + (eod_station[2] << 8) + eod_station[3];
			cpu_19_data[9] = eod_station[2];
			cpu_19_data[10] = eod_station[3];

			memcpy(&transport_1A_data[15], XA_CODE_CITY, 2);
			memcpy(&transport_1A_data[19], XA_CODE_ORGANIZATION, 8);
			memcpy(&transport_1A_data[35], "\x44\x12\x79\x10\x00\x00", 6);
			memcpy(&transport_1A_data[41], &eod_station[2], 2);
			memcpy(&transport_1A_data[51], "\x79\x10\x00\x00\x00\x00\x00\x00", 8);
			memcpy(&transport_1A_data[55], &eod_station[2], 2);
			chret = 0;
		}
	}
	
	return chret;
}


char Transport_Map(unsigned char *organitionCode, unsigned char transport_subType, unsigned char transport_mainType, unsigned short *metro_type)
{
long i;
unsigned char pboc_type[2];
unsigned char Organition[11];

	//
	if(tpProperty1933.offset == NULL)
		return CE_SETPARA;
	//
	for( i = 0; i < 4; i++)
		sprintf(&Organition[i * 2], "%02X", organitionCode[i]);
	for(i = 0; i < tpProperty1933.offset[0].section_rec; i++)
	{
		//PRINTK("issuer %s\n", tpProperty1933.JTBProperty_val[i].cardIssuer);
		if((transport_subType == tpProperty1933.JTBProperty_val[i].subtype) && 
			(transport_mainType == tpProperty1933.JTBProperty_val[i].maintype) && 
			(memcmp(Organition, tpProperty1933.JTBProperty_val[i].cardIssuer, 8) == 0))
		{
			*metro_type = tpProperty1933.JTBProperty_val[i].ProductType;
			return 0;
		}
	}
//	//
//	memset(Organition, 'F', 11);
//	for(i = 0; i < tpProperty1933.offset[0].section_rec; i++)
//	{
//		PRINTK("issuer %s\n", tpProperty1933.JTBProperty_val[i].cardIssuer);
//		if((transport_subType == tpProperty1933.JTBProperty_val[i].subtype) && 
//			(transport_mainType == tpProperty1933.JTBProperty_val[i].maintype) && 
//			(memcmp(Organition, tpProperty1933.JTBProperty_val[i].cardIssuer, 11) == 0))
//		{
//			*metro_type = tpProperty1933.JTBProperty_val[i].ProductType;
//			return 0;
//		}
//	}
	return CE_SETPARA;
}


unsigned char Transport_Whitelist(unsigned char *organitionCode)
{
long i;
char IIN[12], WhiteIIN[12];

	if(tpWhite1932.offset == NULL)
		return CE_WHITELIST;
		
	memset(IIN, ' ', 12);
	sprintf(IIN, "%02x%02x%02x%02x", organitionCode[0], organitionCode[1], organitionCode[2], organitionCode[3]);
	for(i = 0; i < tpWhite1932.offset[0].section_rec; i++)
	{
		if(memcmp(IIN, tpWhite1932.JTBWhite_val[i].cardIssuer, 8) == 0)
			return 0;
	}

	return CE_WHITELIST;
}

char get_transport_purchase_ticket(unsigned short type, JTBTerminal_t *td)
{
unsigned long	i;
	
	if(tpTerminal1934.offset[0].section_rec == 0)
		return CE_EOD_FILE;
	//set the truct to zero
	memset(td, 0x00, sizeof(JTBTerminal_t));
	
	for(i = 0; i < tpTerminal1934.offset[0].section_rec; i++)
	{
		if((type == tpTerminal1934.JTBTerminal_val[i].type))
		{
			memcpy(td, &tpTerminal1934.JTBTerminal_val[i].phyical, sizeof(YKTTerminal_t));
			return 0;
		}
	}
	return CE_EOD_FILE;
}

char get_transport_bonus(unsigned char *issuerCode, unsigned short mainType, long *transAmount, long *bonus)
{
int i;
unsigned char IIN[11];
long price;
float fPrice;

	*bonus = 0;
	
	if(tpPreferential1935.offset == NULL)
		return 0;
	if(tpPreferential1935.JTBPreferential_val == NULL)
		return 0;
		
	memset(IIN, ' ', 11);
	sprintf(IIN, "%02X%02X%02X%02X", issuerCode[0], issuerCode[1], issuerCode[2], issuerCode[3]);
	for(i = 0; i < tpPreferential1935.offset[0].section_rec; i++)
	{
		if( (memcmp(IIN, tpPreferential1935.JTBPreferential_val[i].cardIssuer, 8) == 0)
			&& (mainType == tpPreferential1935.JTBPreferential_val[i].type) )
		{
			fPrice = ((*transAmount) * tpPreferential1935.JTBPreferential_val[i].bonusPercent) / 100.00;
			fPrice -= tpPreferential1935.JTBPreferential_val[i].bonusValue;
			if(fPrice < 0)
			{
				*bonus = *transAmount;
				*transAmount = 0;
				return 0;
			}
			price = fPrice;
			//不需要分四舍五入？
			PRINTK("percent fprice %f price %d\n", fPrice, price);
//			if((price % 10) > 4)
//				price = ((price / 10) + 1 ) * 10;
//			else
//				price = (price / 10) * 10;
			PRINTK("after percent fprice %d\n", price);
			*bonus = (*transAmount) - price;
			*transAmount = price;
			return 0;
		}
	}
	return CE_EOD_FILE;
}

//blnBlock:
char check_JTB_Black_Lock(unsigned char *issuerCode, unsigned char *SN, unsigned char blnBlock, unsigned char *out_buf, unsigned short *out_len)
{
unsigned long lngBlacklist;
long lngCSCLowIndex, lngCSCHighIndex, lngCSCMidIndex;
unsigned char bytBlackCSCId[31], bytCSCId[30], blnBlacklist, des[8];
int i, ret;
unsigned short cnt, transLen, cpulen;
unsigned char cpubuf[300], buf[300], factor[16], cpurandom[8];

    blnBlacklist = 0xff;
    memset(bytBlackCSCId, ' ', 40);
    bytBlackCSCId[30] = 0x00;
    sprintf(bytCSCId, "%02X%02X%02X%02X", issuerCode[0], issuerCode[1], issuerCode[2], issuerCode[3]);
    memcpy(bytBlackCSCId, bytCSCId, 8);
    for(i = 0; i < 10; i++)
    	sprintf(&bytCSCId[i * 2], "%02X", SN[i]);

    memcpy(&bytBlackCSCId[11], &bytCSCId[1], 19);
#ifdef	DEBUG_PRINT
	PRINTK("check black: %s\n", bytBlackCSCId);
#endif
    //
    if(tpBlacklist1931.offset == NULL)
    	return 0;
    //use the binary method to find the BLACK No
    lngCSCLowIndex = 0;
    lngCSCHighIndex = tpBlacklist1931.offset[0].section_rec;
    if(tpBlacklist1931.offset[0].section_rec == 0)
    {
    	return 0;
    }
    //do
    while(lngCSCLowIndex <= lngCSCHighIndex)
    {
	    lngCSCMidIndex = (lngCSCLowIndex + lngCSCHighIndex) / 2;
        ret = memcmp(tpBlacklist1931.JTBBlack_val[lngCSCMidIndex].cardIssuer, bytBlackCSCId, 30);
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
    memcpy(factor, &transport_15_data[12], 8);
    memcpy(&factor[8], transport_15_data, 8);
     //
    memset(buf, 0x00, 100);
    memcpy(buf, cpurandom, 8);
    //application lock 
    memcpy(&buf[8], "\x84\x1e\x00\x00\x04", 5);
    //application unlock
    //memcpy(&buf[8], "\x84\x18\x00\x00\x04", 5);
    buf[5 + 8] = 0x80;
    ret = cpu_cal_protect_mac(xa_transport_psam_index, factor, 16, "\x45\x02", buf, 5 + 8 + 3, des);
    if(ret)
    {
    	return CE_SAMERR;
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
	tpYKTTxnPurchase.CrdDebitCnt = 0;
	tpYKTTxnPurchase.TAC = 0;
	tpYKTTxnPurchase.SamSeq = 0;
	
	memset(tpJTBTxnPurchaseEx.cardIssuer, ' ', 11);
	sprintf(buf, "%02X%02X%02X%02x", transport_15_data[0], transport_15_data[1], transport_15_data[2], transport_15_data[3]);
	memcpy(tpJTBTxnPurchaseEx.cardIssuer, buf, 8);
	memcpy(tpJTBTxnPurchaseExII.cardIssuer, tpJTBTxnPurchaseEx.cardIssuer, 11);
	//memcpy(tpJTBTxnPurchaseEx.cardIssuer, &transport_15_data[0], 8);
	memcpy(tpJTBTxnPurchaseEx.businessCode, "\x71\x00\x03\x01", 4);
	memcpy(tpJTBTxnPurchaseExII.businessCode, tpJTBTxnPurchaseEx.businessCode, 4);
	memcpy(tpJTBTxnPurchaseEx.businessType, "\x44\x12", 2);
	memcpy(tpJTBTxnPurchaseExII.businessType, tpJTBTxnPurchaseEx.businessType, 2);
	//
	if( memcmp(&transport_15_data[0], XA_CODE_ORGANIZATION, 4) == 0 )
		memcpy(tpJTBTxnPurchaseExII.unionCity, "\x71\x00", 2);
	else
		memcpy(tpJTBTxnPurchaseEx.unionCity, &transport_17_data[6], 2);
	tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = 0xff;
	tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x00;
	
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x03;
	//UD record length
	transLen = cnt = sizeof(YKTTxnPurchase_t);
	//UD
	memcpy(&out_buf[39], tpYKTTxnPurchase.AFCHead_val.operatorid, cnt);
	if( memcmp(transport_15_data, XA_CODE_ORGANIZATION, 4) == 0)
	{
		transLen += sizeof(JTBTxnPurchaseExII_t);
		memcpy(&out_buf[39 + cnt], tpJTBTxnPurchaseExII.cardIssuer, sizeof(JTBTxnPurchaseExII_t));
		cnt += sizeof(JTBTxnPurchaseExII_t);
	}else
	{
		transLen += sizeof(JTBTxnPurchaseEx_t);
		memcpy(&out_buf[39 + cnt], tpJTBTxnPurchaseEx.cardIssuer, sizeof(JTBTxnPurchaseEx_t));
		cnt += sizeof(JTBTxnPurchaseEx_t);
	}
	memcpy(&out_buf[37], &transLen, 2);
	cnt += 4;

	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	ee_write_last_record(XA_TRANSPORT_FAMILY, 1, &out_buf[33], cnt);
	reader_status = XA_RW_RECORD;
	sem_post(&g_samreturn);
	
   return 1;

}
/************************************
entry
************************************/
char xa_transport_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode, chRejectCode, chFare;
unsigned char buf[50], factor[20], des[80], deslen;
unsigned char cpubuf[300], cpulen, Le, entry_time[4], entry_bcd[7];
unsigned char status, chTicketType, last_timebcd[7];
unsigned long time1,time2, transport_index;
unsigned short tempdate;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
long lngCardBalance, ret, i;
unsigned short shDays, shTicketType, cnt, transLen;
unsigned long lngMidnightSecond;
JTBTerminal_t	tpPurchase;

	*out_len = 4;
	memcpy(out_buf, "\x81\x01\x00\x00", 4);
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
		PRINTK("rollback %02x old phyical id", ch_sz_transport_rollback);
		for(i = 0; i < 10; i++) PRINTK("%02x", ch_transport_logic_id[i]);
		PRINTK("  new phyical id ");
		for(i = 0; i < 10; i++) PRINTK("%02x", tpTransportProtect[tpTransportProtectIndex].logicID[i]);
		PRINTK("\n");
#endif
	if((ch_sz_transport_rollback != 0) && (memcmp(ch_transport_logic_id, tpTransportProtect[tpTransportProtectIndex].logicID, 10) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_transport_rollback);
#endif
		if((chRejectCode = Transport_gettransprove(out_buf)) == 0)
		{
			bln_transport_xian = 0x00;
			if( memcmp(&transport_15_data[0], XA_CODE_ORGANIZATION, 4) == 0 )
			{
				bln_transport_xian = 0xff;
				xa_Transport_rollback_read(tpYKTTxnPurchase.AFCHead_val.operatorid, tpJTBTxnPurchaseExII.cardIssuer);
			}else
				xa_Transport_rollback_read(tpYKTTxnPurchase.AFCHead_val.operatorid, tpJTBTxnPurchaseEx.cardIssuer);
			goto label_sz_city_rollback_1;
		}else if(chRejectCode == CE_READ)
			return CE_READ;
		else
			tpTransportProtect[tpTransportProtectIndex].rollBack = 0;
	}
	tpYKTTxnPurchase.udSubtype = 0x0C;
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_transport_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, XA_CODE_CITY, 2);
	memcpy(&tpYKTTxnPurchase.SamId[2], ch_transport_psam_id, 6);
	tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_transport_phyical_id[4]);
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
	
#ifdef DEBUG_PRINT
	PRINTK("file 15 logicid: %02x%02x%02x%02x %02x%02x%02x%02x appindex:%02x app ver:%02x \n", transport_15_data[0], transport_15_data[1], transport_15_data[2], transport_15_data[3],
			transport_15_data[4], transport_15_data[5], transport_15_data[6], transport_15_data[7], transport_15_data[8], transport_15_data[9]);
	PRINTK("app sn: %02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
			 transport_15_data[10], transport_15_data[11], transport_15_data[12], transport_15_data[13], transport_15_data[14], transport_15_data[15], transport_15_data[16], transport_15_data[17], transport_15_data[18], transport_15_data[19]);
	PRINTK(" app startdate %02x%02x%02x%02x valid date %02x%02x%02x%02x fci %02x%02x\n", 
			transport_15_data[20], transport_15_data[21], transport_15_data[22], transport_15_data[23], transport_15_data[24], transport_15_data[25], transport_15_data[26], transport_15_data[27], transport_15_data[28], transport_15_data[29]);
#endif
	memset(tpJTBTxnPurchaseEx.cardIssuer, 0x20, 11);
	sprintf(buf, "%02X%02X%02X%02x", transport_15_data[0], transport_15_data[1], transport_15_data[2], transport_15_data[3]);
	memcpy(tpJTBTxnPurchaseEx.cardIssuer, buf, 8);
	memcpy(tpJTBTxnPurchaseExII.cardIssuer, tpJTBTxnPurchaseEx.cardIssuer, 11);
	//memcpy(tpJTBTxnPurchaseEx.cardIssuer, &transport_15_data[0], 8);
	memcpy(tpJTBTxnPurchaseEx.businessCode, "\x71\x00\x03\x01", 4);
	memcpy(tpJTBTxnPurchaseExII.businessCode, tpJTBTxnPurchaseEx.businessCode, 4);
	memcpy(tpJTBTxnPurchaseEx.businessType, "\x44\x12", 2);
	memcpy(tpJTBTxnPurchaseExII.businessType, tpJTBTxnPurchaseEx.businessType, 2);
	
	memcpy(tpYKTTxnPurchase.CityCode, &transport_15_data[10], 2);
	memcpy(tpYKTTxnPurchase.CardId, &transport_15_data[12], 8);

	memcpy(ch_transport_logic_id, &transport_15_data[10], 10);
	tpYKTTxnPurchase.CrdVerNo = transport_15_data[9];
	//card satus
	if(transport_15_data[9] == SZ_TRANSPORT_ENABLE)
		return CE_CARDSTATUS;
label_sz_rollback_main:
	
	memcpy(out_buf, "\x81\x07", 2);
	//verify date
	memcpy(out_buf, "\x81\x08", 2);
	if((memcmp(tpCPU.time_bcd, &transport_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &transport_15_data[24], 4) > 0))
		return CE_EXPIREDDATE;
	memcpy(tpYKTTxnPurchase.Validday, &transport_15_data[24], 4);
	
	//
	if( (chCode = Transport_Whitelist(&transport_15_data[0])) != 0 )
	{
		return chCode;
	}
	//verify pin and get balance
	bln_transport_xian = 0xff;
	if( memcmp(&transport_15_data[0], XA_CODE_ORGANIZATION, 4) == 0 )
	{//西安本地交通部卡
		tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t) + sizeof(JTBTxnPurchaseExII_t);
		tpYKTTxnPurchase.udType = 0x22;
		if( 0 != (chCode = CPUT_GetFiles05(out_buf)) )
			return chCode;
		if((memcmp(tpCPU.time_bcd, &cput_05_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_05_data[24], 4) > 0))
			return CE_EXPIREDDATE;
		//
		if(0 != (chCode = Transport_Map(&transport_15_data[0], cput_05_data[29], cput_05_data[28], &shTicketType)))
			return chCode;
		if((chCode = CPU_TellSysCard(shTicketType)) != 0)
		{
			return chCode;
		}
		if((chCode = Transport_GetFiles19(out_buf)) != 0)
			return chCode;
		//锁卡标志
		if( cpu_19_data[2] != 0)
			return CE_LOCKED_TICKET;
		//
		memcpy(tpJTBTxnPurchaseExII.unionCity, "\x71\x00", 2);
	}else
	{
		bln_transport_xian = 0x00;
		if( (chCode = Transport_GetFiles17(out_buf)) != 0)
			return chCode;
		//
		memcpy(tpJTBTxnPurchaseEx.unionCity, &transport_17_data[6], 2);
		if(0 != (chCode = Transport_Map("\xFF\xFF\xFF\xFF", 0, transport_17_data[10], &shTicketType)))
			return chCode;
		if((chCode = CPU_TellSysCard(shTicketType)) != 0)
		{
			return chCode;
		}
		tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t) + sizeof(JTBTxnPurchaseEx_t);
		tpYKTTxnPurchase.udType = 0x23;
		if((chCode = Transport_GetFiles1A(out_buf)) != 0)
			return chCode;
		if( ((transport_1A_data[14] == 0x01) || (transport_1A_data[14] == 0x03)) && (memcmp(&transport_1A_data[19], XA_CODE_ORGANIZATION, 4) != 0) )
			return CE_FINISHED;
		//
		if(memcmp(&transport_17_data[8], "\x00\x01", 2) != 0)
			return CE_WHITELIST;
		//
		cpu_19_data[3] = transport_1A_data[14];
		//进站信息
		memcpy(&cpu_19_data[4], &transport_1A_data[68], 5);
		memcpy(&cpu_19_data[9], &transport_1A_data[41], 2);
		cpu_19_data[9] = bcd2bin(cpu_19_data[9]);
		cpu_19_data[10] = bcd2bin(cpu_19_data[10]);
		//出站信息
		memcpy(&cpu_19_data[11], &transport_1A_data[75], 5);
		memcpy(&cpu_19_data[16], &transport_1A_data[49], 2);
		cpu_19_data[16] = bcd2bin(cpu_19_data[16]);
		cpu_19_data[17] = bcd2bin(cpu_19_data[17]);
	}
	//交通部卡定义为Intel序的两字节
	tpYKTTxnPurchase.CrdSKnd = shTicketType >> 8;
	tpYKTTxnPurchase.CrdMKnd  = (unsigned char) shTicketType;
	//balance < max remaining value
	memcpy(out_buf, "\x81\x09", 2);
	//bonus and special city card such as lovely card, old man card.
	//balance < max remaining value
	if( 0 != (chCode = get_transport_purchase_ticket(shTicketType, &tpPurchase)) )
		return chCode;
	if(tpCPU.balance < tpPurchase.minbalance)
	{
		return CE_ENOUGH_BALANCE;
	}
	tpYKTTxnPurchase.AftBalance = tpYKTTxnPurchase.BefBalance = tpCPU.balance;
	tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount = 0;
	tpYKTTxnPurchase.UserType = 0x00;
	tpYKTTxnPurchase.lastlocation = 0x09000000 + (cpu_19_data[16] << 8) + cpu_19_data[17];
	
	//check the black list and lock
	if(CE_BLACKLIST == (chCode = check_JTB_Black_Lock(&transport_15_data[0], &transport_15_data[10], 0x00,  out_buf, out_len)))
	{
		return chCode;
	}else if(0 != chCode)
		return chCode;
	//metro status
	memset(last_timebcd, 0x00, 7);
	last_timebcd[0] = 0x20;
	memcpy(&last_timebcd[1], &cpu_19_data[3], 5);
	if(cpu_19_data[3] == XA_TRANSPORT_ENTRY)
	{//in entry status
		memcpy(out_buf, "\x81\x0b", 2);
		get_degrade_sensitive_mode(NULL, last_timebcd);
		//sensitive MODE
		if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_exit))
		{
			cpu_19_data[3] = XA_TRANSPORT_EXIT;
		}else
		{
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
			goto label_refuse_to_city_entry;
		}
	}
	//进行实际消费	
	memcpy(out_buf, "\x81\x10", 2);
	if(0 != (chCode = CPU_init_for_capp(1, tpCPU.tranamount, ch_transport_psam_id, &transport_15_data[12], transport_15_data, 8, ch_cpu_mac_data)))
	{
		return chCode;
	}
	//更改本地消费记录文件
	cpu_19_data[3] = XA_TRANSPORT_ENTRY;
	memcpy(&cpu_19_data[4], &tpCPU.time_bcd[1], 5);
	memcpy(&cpu_19_data[9], tpCPU.curstation, 2);
	//更改异地消费记录文件
	transport_1A_data[14] = XA_TRANSPORT_ENTRY;
	//city code
	memcpy(&transport_1A_data[15], XA_CODE_CITY, 2);
	//oragination code
	memcpy(&transport_1A_data[19], XA_CODE_ORGANIZATION, 8);
	//entry station code
	memcpy(&transport_1A_data[35], "\x44\x12\x79\x10\x00\x00", 6);
	memcpy(&transport_1A_data[41], tpCPU.curstation, 2);
	transport_1A_data[41] = bin2bcd(transport_1A_data[41]);
	transport_1A_data[42] = bin2bcd(transport_1A_data[42]);
	//entry station device 
	memcpy(&transport_1A_data[51], "\x79\x10\x00\x00", 4);
	memcpy(&transport_1A_data[55], &tpCPU.curstation[0], 4);
	transport_1A_data[55] = bin2bcd(transport_1A_data[55]);
	transport_1A_data[56] = bin2bcd(transport_1A_data[56]);
	//
	memcpy(&transport_1A_data[67], tpCPU.time_bcd, 7);
	//max consume transaction amount
	//LongToByte(tpTicketDef.MinEntryAmount, &transport_1A_data[81]);
	//entry line code
	transport_1A_data[85] = tpCPU.curstation[0];
	memset(&transport_1A_data[87], 0x00, 4);
	LongToByte(tpCPU.balance, &transport_1A_data[91]);
	//
	memset(&transport_1A_data[122], 0x00, 6);
	memcpy(out_buf, "\x81\x11", 2);
	if(bln_transport_xian)
	{
		ret = CPU_update_capp(1, 0x19, cpu_19_data[0], XA_TRANSPROT_19_LEN, cpu_19_data, 0);
	}else
	{
		ret = CPU_update_capp(1, 0x1a, transport_1A_data[1], TRANSPORT_1A_LEN, transport_1A_data, 0);
	}
	if(ret != 0)
	{
		return CE_WRITE;
	}
	//更改公共交通过程信息循环记录文件
	memset(transport_1E_data, 0x00, 48);
	//transaction type
	transport_1E_data[0] = 0x03;
	//entry station device
	memcpy(&transport_1E_data[1], "\x79\x10\x00\x00", 4);
	memcpy(&transport_1E_data[5], tpCPU.curstation, 4);
	//business
	transport_1E_data[9] = 0x01;
	//line
	transport_1E_data[11] = tpCPU.curstation[0];
	//station
	transport_1E_data[13] = tpCPU.curstation[1];
	//operation code
	//transport_1E_data[14]
	//balance
	LongToByte(tpCPU.balance, &transport_1E_data[21]);
	//
	memcpy(&transport_1E_data[25], tpCPU.time_bcd, 7);
	//
	memcpy(&transport_1E_data[32], XA_CODE_CITY, 2);
	memcpy(&transport_1E_data[34], XA_CODE_ORGANIZATION, 8);
	transport_1E_data[34] |= 0x10;
	ret = CPU_update_capp(0, 0x1e, 0x00, TRANSPORT_1E_LEN, transport_1E_data, 0);
	if(ret != 0)
	{
		return CE_WRITE;
	}

#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	memcpy(out_buf, "\x81\x12", 2);
	ch_sz_transport_rollback = SZ_CPU_CAPP_1;
	if(0 != CPU_debit_for_capppurchase(&ch_sz_transport_rollback, NULL, out_buf))
	{
#ifdef DEBUG_PRINT
		PRINTK("debit failure %02x\n", ch_sz_transport_rollback);
#endif
		if(ch_sz_transport_rollback != 0)
		{
			if( bln_transport_xian )
				xa_Transport_rollback_write(tpYKTTxnPurchase.AFCHead_val.operatorid, sizeof(YKTTxnPurchase_t), tpJTBTxnPurchaseExII.cardIssuer, sizeof(JTBTxnPurchaseExII_t));
			else
				xa_Transport_rollback_write(tpYKTTxnPurchase.AFCHead_val.operatorid, sizeof(YKTTxnPurchase_t), tpJTBTxnPurchaseEx.cardIssuer, sizeof(JTBTxnPurchaseEx_t));
		}
		return CE_WRITE;
	}

#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif

label_sz_city_rollback_1:
	*out_len = 37 + 37;
	if(ch_sz_transport_rollback != 0)
		tpTransportProtect[tpTransportProtectIndex].rollBack = 0;
	tpJTBTxnPurchaseEx.version = 0x01;
	tpJTBTxnPurchaseEx.index = capp_init[9];
	//01-3des 02-SM2 04-SM4
	tpJTBTxnPurchaseEx.algorithm = 0x01;
	tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x00;
	//tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = 0x02;
	tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = XA_PAYTYPE_CARD;
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
	out_buf[3] = XA_TRANSPORT_FAMILY;
	//ticket type
	//main type
	out_buf[4] = shTicketType >> 8;
	//sub type
	out_buf[5] = (unsigned char)shTicketType;
	//logic card sn
	memcpy(&out_buf[6], &ch_transport_phyical_id[4], 4);
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
	out_buf[36] = 0x03;
	//UD record length
	transLen = cnt = sizeof(YKTTxnPurchase_t);
	//UD
	memcpy(&out_buf[39], tpYKTTxnPurchase.AFCHead_val.operatorid, cnt);
	if(bln_transport_xian)
	{
		memcpy(&out_buf[39 + cnt], tpJTBTxnPurchaseExII.cardIssuer, sizeof(JTBTxnPurchaseExII_t));
		transLen += sizeof(JTBTxnPurchaseExII_t);
		cnt += sizeof(JTBTxnPurchaseExII_t);
	}else
	{
		memcpy(&out_buf[39 + cnt], tpJTBTxnPurchaseEx.cardIssuer, sizeof(JTBTxnPurchaseEx_t));
		transLen += sizeof(JTBTxnPurchaseEx_t);
		cnt += sizeof(JTBTxnPurchaseEx_t);
	}
	memcpy(&out_buf[37], &transLen, 2);
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
		ee_write_last_record(XA_TRANSPORT_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
		sem_post(&g_samreturn);
	}else 
	{
		(*out_len) += cnt;
		ee_write_last_record(XA_TRANSPORT_FAMILY, 0, &out_buf[33], cnt);
		reader_status = XA_RW_IDLE;
	}

	return CE_OK;
	
label_refuse_to_city_entry:
	if(ch_sz_transport_rollback != 0)
		tpTransportProtect[tpTransportProtectIndex].rollBack = 0;

	return chCode;
}
/************************************
exit
************************************/
char xa_transport_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode, chRejectCode;
unsigned char buf[50], factor[20], des[100], deslen;
unsigned char cpubuf[300], cpulen, Le, chTicketType;
unsigned char status, entry_station[2], entry_time[7];
unsigned long time1,time2, transport_index, lngsrcstation;
unsigned short tempdate, shTempbonus, shFare;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
long lngCardBalance, ret, i;
unsigned short shDays, shTicketType, cnt, transLen;
unsigned long lngMidnightSecond;

	*out_len = 4;
	memcpy(out_buf, "\x72\x01\x00\x00", 4);
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
		PRINTK("rollback %02x old phyical id", ch_sz_transport_rollback);
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_transport_logic_id[i]);
		PRINTK("  new phyical id ");
		for(i = 0; i < 8; i++) PRINTK("%02x", tpTransportProtect[tpTransportProtectIndex].logicID[i]);
		PRINTK("\n");
#endif
	if((ch_sz_transport_rollback != 0) && (memcmp(ch_transport_logic_id, tpTransportProtect[tpTransportProtectIndex].logicID, 10) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_transport_rollback);
#endif
		if((chRejectCode = Transport_gettransprove(out_buf)) == 0)
		{
			bln_transport_xian = 0x00;
			if( memcmp(&transport_15_data[0], XA_CODE_ORGANIZATION, 4) == 0 )
			{
				bln_transport_xian = 0xff;
				xa_Transport_rollback_read(tpYKTTxnPurchase.AFCHead_val.operatorid, tpJTBTxnPurchaseExII.cardIssuer);
			}else
				xa_Transport_rollback_read(tpYKTTxnPurchase.AFCHead_val.operatorid, tpJTBTxnPurchaseEx.cardIssuer);
			goto label_sz_city_rollback_1;
		}else if(chRejectCode == CE_READ)
			return CE_READ;
		else
			tpTransportProtect[tpTransportProtectIndex].rollBack = 0;
	}
	tpYKTTxnPurchase.udSubtype = 0x0D;
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_transport_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, XA_CODE_CITY, 2);
	memcpy(&tpYKTTxnPurchase.SamId[2], ch_transport_psam_id, 6);
	tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_transport_phyical_id[4]);
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
	
label_sz_rollback_main:
	memset(tpJTBTxnPurchaseEx.cardIssuer, ' ', 11);
	sprintf(buf, "%02X%02X%02X%02x", transport_15_data[0], transport_15_data[1], transport_15_data[2], transport_15_data[3]);
	memcpy(tpJTBTxnPurchaseEx.cardIssuer, buf, 8);
	memcpy(tpJTBTxnPurchaseExII.cardIssuer, tpJTBTxnPurchaseEx.cardIssuer, 11);
	//memcpy(tpJTBTxnPurchaseEx.cardIssuer, &transport_15_data[0], 8);
	memcpy(tpJTBTxnPurchaseEx.businessCode, "\x71\x00\x03\x01", 4);
	memcpy(tpJTBTxnPurchaseExII.businessCode, tpJTBTxnPurchaseEx.businessCode, 4);
	memcpy(tpJTBTxnPurchaseEx.businessType, "\x44\x12", 2);
	memcpy(tpJTBTxnPurchaseExII.businessType, tpJTBTxnPurchaseEx.businessType, 2);
	
	memcpy(tpYKTTxnPurchase.CityCode, &transport_15_data[10], 2);
	memcpy(tpYKTTxnPurchase.CardId, &transport_15_data[12], 8);

	memcpy(ch_transport_logic_id, &transport_15_data[10], 10);
	tpYKTTxnPurchase.CrdVerNo = transport_15_data[9];
	//card satus
	if(transport_15_data[9] == SZ_TRANSPORT_ENABLE)
		return CE_CARDSTATUS;
	
	//verify date
	memcpy(out_buf, "\x72\x08", 2);
	if((memcmp(tpCPU.time_bcd, &transport_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &transport_15_data[24], 4) > 0))
		return CE_EXPIREDDATE;
	memcpy(tpYKTTxnPurchase.Validday, &transport_15_data[24], 4);

	//verify local & remote card
	if((chCode = Transport_Whitelist(&transport_15_data[0])) != 0)
	{
		return chCode;
	}
	bln_transport_xian = 0xff;
	if( memcmp(&transport_15_data[0], XA_CODE_ORGANIZATION, 4) == 0 )
	{//西安本地交通部卡
		tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t) + sizeof(JTBTxnPurchaseExII_t);
		tpYKTTxnPurchase.udType = 0x22;
		if( 0 != (chCode = CPUT_GetFiles05(out_buf)) )
			return chCode;
		if((memcmp(tpCPU.time_bcd, &cput_05_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_05_data[24], 4) > 0))
			return CE_EXPIREDDATE;
		//
		if(0 != (chCode = Transport_Map(&transport_15_data[0], cput_05_data[29], cput_05_data[28], &shTicketType)))
			return chCode;
		if((chCode = CPU_TellSysCard(shTicketType)) != 0)
		{
			return chCode;
		}
		if((chCode = Transport_GetFiles19(out_buf)) != 0)
			return chCode;
		//锁卡标志
		if( cpu_19_data[2] != 0)
			return CE_LOCKED_TICKET;
		//
		memcpy(tpJTBTxnPurchaseExII.unionCity, "\x71\x00", 2);
	}else
	{
		bln_transport_xian = 0x00;
		tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t) + sizeof(JTBTxnPurchaseEx_t);
		tpYKTTxnPurchase.udType = 0x23;
		//ticket definition
		memcpy(out_buf, "\x72\x07", 2);
		if( (chCode = Transport_GetFiles17(out_buf)) != 0)
			return chCode;
		memcpy(tpJTBTxnPurchaseEx.unionCity, &transport_17_data[6], 2);
		//ticket definition
		if(0 != (chCode = Transport_Map("\xFF\xFF\xFF\xFF", 0, transport_17_data[10], &shTicketType)))
			return chCode;
		if((chCode = CPU_TellSysCard(shTicketType)) != 0)
		{
			return chCode;
		}
		if( (chCode = Transport_GetFiles1A(out_buf)) != 0)
			return chCode;
		if( ((transport_1A_data[14] == 0x01) || (transport_1A_data[14] == 0x03)) && (memcmp(&transport_1A_data[19], XA_CODE_ORGANIZATION, 4) != 0) )
			return CE_FINISHED;
		//
		if(memcmp(&transport_17_data[8], "\x00\x01", 2) != 0)
			return CE_WHITELIST;
		//change to the file 19
		//
		memset(&cpu_19_data[4], 0x00, 46);
		//entry station
		memcpy(&cpu_19_data[9], &transport_1A_data[41], 2);
		cpu_19_data[9] = bcd2bin(cpu_19_data[9]);
		cpu_19_data[10] = bcd2bin(cpu_19_data[10]);
		//entry time
		memcpy( &cpu_19_data[4], &transport_1A_data[68], 5);
		//status
		cpu_19_data[3] = transport_1A_data[14];
		if( (transport_1A_data[14] == 0x02) || (transport_1A_data[14] == 0x04) )
			cpu_19_data[3] = 0x00;
		if( transport_1A_data[122] != 0)
			cpu_19_data[3] = transport_1A_data[122];
		//exitDevicecode
		memcpy(&cpu_19_data[16], &transport_1A_data[49], 2);
		cpu_19_data[16] = bcd2bin(cpu_19_data[16]);
		cpu_19_data[17] = bcd2bin(cpu_19_data[17]);
		//exit time
		memcpy(&cpu_19_data[11], &transport_1A_data[75], 5);
	}
	//交通部定义
	tpYKTTxnPurchase.CrdSKnd = shTicketType >> 8;
	tpYKTTxnPurchase.CrdMKnd = (unsigned char) shTicketType;
	//balance < max remaining value
	tpYKTTxnPurchase.BefBalance = tpCPU.balance;
	memcpy(out_buf, "\x72\x09", 2);
	//check the last station and time(1min)
	//check the black list and lock
	if(CE_BLACKLIST == (chCode = check_JTB_Black_Lock(&transport_15_data[0], &transport_15_data[10], 0x00,  out_buf, out_len)))
	{
		return chCode;
	}else if(0 != chCode)
		return chCode;
	memcpy(out_buf, "\x72\x0a", 2);
	//metro status
	if(0 != (chCode = Transport_TellEntry(cpu_19_data[3], 0xff)))
	{
		goto label_refuse_to_city_exit;
	}
	//calculate the fare
	lngsrcstation = 0x09000000 + (cpu_19_data[9] << 8) + cpu_19_data[10];
	tpYKTTxnPurchase.Enlocation = tpYKTTxnPurchase.lastlocation = lngsrcstation;
	tpCPU.tranamount = 0;
	memcpy(out_buf, "\x72\x10", 2);
	if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, tpCmdInit.curstation, &shFare)))
		return chCode;
	memcpy(out_buf, "\x72\x11", 2);
	entry_time[0] = 0x20;
	memcpy(&entry_time[1], &cpu_19_data[4], 5);
	if( tpTicketDef.IgnoreMaxJourneyTime == 0)
	{
		memcpy(out_buf, "\x72\x12", 2);
		if((chCode = cal_overtime(entry_time, tpCPU.time_bcd, shFare, 0)) != 0)
		{
			if(chCode == CE_OVERTIME)
				goto label_refuse_to_city_exit;
			else
				return chCode;
		}
	}
		//entry time ? current ttime
	memcpy(out_buf, "\x72\x13", 2);
	//other card using the current time
	if(tpTicketDef.ChargeFareOnCheckout == 0)
	{//using the entry time
		if(0 != (chCode = cal_fare_value(entry_time, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
			return chCode;
	}else
	{
		if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
			return chCode;
	}
	//if(0 != (chCode = cal_bonus(tpCPU.TicketType, cpu_19_data[38], &tpCPU.tranamount, &tpCPU.lngBonus, chFare, &cmd_buf[27])))
	//	return chCode;
	tpCPU.tranamount = tpSysPrice.price;
	memcpy(out_buf, "\x72\x14", 2);
	//if current station is set to fare mode
	if(tpwaivermode.cur_sta_fare)
	{
		if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngsrcstation, &shFare)))
			return chCode;
		if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
			return chCode;
		tpCPU.tranamount = tpSysPrice.price;
	}
	if(tpwaivermode.cur_sta_failure)
	{
		tpCPU.tranamount = 0;
		cpu_19_data[3] = 2;
	}
	get_transport_bonus(&transport_15_data[0], shTicketType, &tpCPU.tranamount, &tpCPU.lngBonus);
	if(tpCPU.balance < (tpTicketDef.MinRemainingValue + tpCPU.tranamount) )
	{
		if( (cpu_19_data[3] != 4) || (cpu_19_data[16] != tpCPU.curstation[0]) || (cpu_19_data[17] != tpCPU.curstation[1]) )
		{
			chCode = CE_OVERRIDE;
			goto label_refuse_to_city_exit;
		}
		tpCPU.tranamount = 0; 
	}
	tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount;
	tpYKTTxnPurchase.Entime = timestr2long(&entry_time[1]) + TIME2000 - ZONE8;
	tpYKTTxnPurchase.AftBalance = tpCPU.balance - tpCPU.tranamount;
	//进行实际消费	
	memcpy(out_buf, "\x71\x20", 2);
	if(0 != (chCode = CPU_init_for_capp(1, tpCPU.tranamount, ch_transport_psam_id, &transport_15_data[12], transport_15_data, 8, ch_cpu_mac_data)))
	{
		return chCode;
	}
	//更改本地消费记录文
	//出口线路码
	memcpy(&cpu_19_data[16], tpCPU.curstation, 2);
	//出口时间-YYMMDDHHMI
	memcpy(&cpu_19_data[11], &tpCPU.time_bcd[1], 5);
	//交易状态
	cpu_19_data[3] = XA_TRANSPORT_EXIT;
	//
	ShortToByte(tpCPU.tranamount, &cpu_19_data[18]);

	//更改异地消费记录文件
	transport_1A_data[14] = XA_TRANSPORT_EXIT;
	memcpy(&transport_1A_data[17], XA_CODE_CITY, 2);
	memcpy(&transport_1A_data[27], XA_CODE_ORGANIZATION, 8);
	//exit station code
	memcpy(&transport_1A_data[43], "\x44\x12\x79\x10\x00\x00", 6);
	memcpy(&transport_1A_data[49], tpCPU.curstation, 2);
	transport_1A_data[49] = bin2bcd(transport_1A_data[49]);
	transport_1A_data[50] = bin2bcd(transport_1A_data[50]);
	//exit station device code
	memcpy(&transport_1A_data[59], "\x79\x10\x00\x00", 4);
	memcpy(&transport_1A_data[63], tpCPU.curstation, 4);
	transport_1A_data[63] = bin2bcd(transport_1A_data[63]);
	transport_1A_data[64] = bin2bcd(transport_1A_data[64]);
	memcpy(&transport_1A_data[74], tpCPU.time_bcd, 7);
	transport_1A_data[86] = tpCPU.curstation[0];
	LongToByte(tpCPU.tranamount, &transport_1A_data[95]);
	
	memset(&transport_1A_data[122], 0x00, 6);

	memcpy(out_buf, "\x71\x21", 2);
	if(bln_transport_xian)
	{
		ret = CPU_update_capp(1, 0x19, cpu_19_data[0], XA_TRANSPROT_19_LEN, cpu_19_data, 0);
	}else
	{
		ret = CPU_update_capp(1, 0x1a, transport_1A_data[1], TRANSPORT_1A_LEN, transport_1A_data, 0);
	}
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif	
	//更改公共交通过程信息循环记录文件
	memset(transport_1E_data, 0x00, 48);
	transport_1E_data[0] = 0x04;
	memcpy(&transport_1E_data[1], "\x79\x10\x00\x00", 4);
	memcpy(&transport_1E_data[5], tpCPU.curstation, 4);
	transport_1E_data[9] = 0x01;
	//line
	transport_1E_data[11] = tpCPU.curstation[0];
	//station
	transport_1E_data[13] = tpCPU.curstation[1];
	//operation code
	//transport_1E_data[14]
	//transaction amount
	LongToByte(tpCPU.tranamount, &transport_1E_data[17]);
	//
	LongToByte(tpCPU.balance, &transport_1E_data[21]);
	memcpy(&transport_1E_data[25], tpCPU.time_bcd, 7);
	
	memcpy(&transport_1E_data[32], XA_CODE_CITY, 2);
	memcpy(&transport_1E_data[34], XA_CODE_ORGANIZATION, 8);
	transport_1E_data[34] |= 0x10;
	ret = CPU_update_capp(0, 0x1e, 0x00, TRANSPORT_1E_LEN, transport_1E_data, 0);
	if(ret != 0)
	{
		return CE_WRITE;
	}

	memcpy(out_buf, "\x71\x22", 2);
	ch_sz_transport_rollback = SZ_CPU_CAPP_1;
	if(0 != CPU_debit_for_capppurchase(&ch_sz_transport_rollback, NULL, out_buf))
	{
#ifdef DEBUG_PRINT
		PRINTK("debit failure %02x\n", ch_sz_transport_rollback);
#endif	
		if(ch_sz_transport_rollback != 0)
		{
			if( bln_transport_xian )
				xa_Transport_rollback_write(tpYKTTxnPurchase.AFCHead_val.operatorid, sizeof(YKTTxnPurchase_t), tpJTBTxnPurchaseExII.cardIssuer, sizeof(JTBTxnPurchaseExII_t));
			else
				xa_Transport_rollback_write(tpYKTTxnPurchase.AFCHead_val.operatorid, sizeof(YKTTxnPurchase_t), tpJTBTxnPurchaseEx.cardIssuer, sizeof(JTBTxnPurchaseEx_t));
		}
		return CE_WRITE;
	}

#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif
label_sz_city_rollback_1:
	tpJTBTxnPurchaseEx.version = 0x01;
	tpJTBTxnPurchaseEx.index = capp_init[9];
	//01-3des 02-SM2 04-SM4
	tpJTBTxnPurchaseEx.algorithm = 0x01;
	tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x00;
	//tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = 0x02;
	tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = XA_PAYTYPE_CARD;
	//card sn-2
	tpYKTTxnPurchase.CrdDebitCnt = ByteToShort(NULL, &capp_init[4]);
	//tac-4
	memcpy(&tpYKTTxnPurchase.TAC, tpCPU.tac, 4);
	//sam sn--4
	tpYKTTxnPurchase.SamSeq = ByteToLong(NULL, tpCPU.sam_sn);

	*out_len = 51 + 37;
	if(ch_sz_transport_rollback != 0)
		tpTransportProtect[tpTransportProtectIndex].rollBack = 0;

	*out_len = 33;
	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_TRANSPORT_FAMILY;
	//ticket type
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_transport_phyical_id[4], 4);
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
	out_buf[36] = 0x03;
	//UD record length
	transLen = cnt = sizeof(YKTTxnPurchase_t);
	//UD
	memcpy(&out_buf[39], tpYKTTxnPurchase.AFCHead_val.operatorid, cnt);
	if( bln_transport_xian)
	{
		transLen += sizeof(JTBTxnPurchaseExII_t);
		memcpy(&out_buf[39 + cnt], tpJTBTxnPurchaseExII.cardIssuer, sizeof(JTBTxnPurchaseExII_t));
		cnt += sizeof(JTBTxnPurchaseExII_t);
	}else
	{
		transLen += sizeof(JTBTxnPurchaseEx_t);
		memcpy(&out_buf[39 +cnt], tpJTBTxnPurchaseEx.cardIssuer, sizeof(JTBTxnPurchaseEx_t));
		cnt += sizeof(JTBTxnPurchaseEx_t);
	}
	memcpy(&out_buf[37], &transLen, 2);
#ifdef	DEBUG_PRINT
	PRINTK("YKT PurchaseExit:");
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", out_buf[39 + i]);
	PRINTK("\n");
#endif

	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	if(cmd_buf[17] == 0x02)
	{
		ee_write_last_record(XA_TRANSPORT_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
		sem_post(&g_samreturn);
	}else 
	{
		(*out_len) += cnt;
		ee_write_last_record(XA_TRANSPORT_FAMILY, 0, &out_buf[33], cnt);
		reader_status = XA_RW_IDLE;
	}

	return CE_OK;
label_refuse_to_city_exit:

	if(ch_sz_transport_rollback != 0)
		tpTransportProtect[tpTransportProtectIndex].rollBack = 0;

	*out_len = 4;

	return chCode;
}
/************************************
CPU update
************************************/
char xa_transport_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100], blnRec01, entry_bcd[7], last_timebcd[7], entry_time[7];
unsigned char cpubuf[300], cpulen, Le;
unsigned char chTicketType, chEntryStatus;
unsigned long time2, transport_index, lngsrcstation;
char chCode, chmonth, chRejectCode;
long lngTranTimes, lngTranAmount;
unsigned short	shTicketType, cnt, shFare, transLen;
unsigned char blnFinished, blnOvertime, blnOverfare;

	*out_len = 4;
	memcpy(out_buf, "\x73\x01\x00\x00", 4);
//	/*
//	/check whether rollback the last transation or not
//#ifdef DEBUG_PRINT
//		PRINTK("rollback %02x old phyical id", ch_sz_cpu_rollback);
//		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id[i]);
//		PRINTK("  new phyical id ");
//		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id_bak[i]);
//#endif
//	if((ch_sz_cpu_rollback != 0) && (memcmp(ch_cpu20_phyical_id, ch_cpu20_phyical_id_bak, 8) == 0))
//	{
//#ifdef DEBUG_PRINT
//		PRINTK("need roll back %02x\n", ch_sz_cpu_rollback);
//#endif
//		if((chRejectCode = CPU_gettransprove(0x09, sfi_bak, out_buf)) == 0)
//		{
//			goto label_sz_rollback_main;
//		}else if(chRejectCode == ERR_READ)
//			return ERR_READ;
//	}*/
	memcpy(tpCPU.time_bcd, &cmd_buf[10], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[11]);
	//record
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[6], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_transport_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, XA_CODE_CITY, 2);
	memcpy(&tpYKTTxnPurchase.SamId[2], ch_transport_psam_id, 6);
	tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_transport_phyical_id[4]);

	memcpy(tpYKTTxnPurchase.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnPurchase.TxnTime, &tpCPU.time_bcd[4], 3);

	memset(tpJTBTxnPurchaseEx.cardIssuer, ' ', 11);
	sprintf(buf, "%02X%02X%02X%02x", transport_15_data[0], transport_15_data[1], transport_15_data[2], transport_15_data[3]);
	memcpy(tpJTBTxnPurchaseEx.cardIssuer, buf, 8);
	memcpy(tpJTBTxnPurchaseExII.cardIssuer, tpJTBTxnPurchaseEx.cardIssuer, 11);
	//memcpy(tpJTBTxnPurchaseEx.cardIssuer, &transport_15_data[0], 8);
	memcpy(tpJTBTxnPurchaseEx.businessCode, "\x71\x00\x03\x01", 4);
	memcpy(tpJTBTxnPurchaseExII.businessCode, tpJTBTxnPurchaseEx.businessCode, 4);
	memcpy(tpJTBTxnPurchaseEx.businessType, "\x44\x12", 2);
	memcpy(tpJTBTxnPurchaseExII.businessType, tpJTBTxnPurchaseEx.businessType, 2);
	memcpy(tpYKTTxnPurchase.CityCode, &transport_15_data[10], 2);
	memcpy(tpYKTTxnPurchase.CardId, &transport_15_data[12], 8);

	memcpy(ch_transport_logic_id, &transport_15_data[10], 10);
	tpYKTTxnPurchase.CrdVerNo = transport_15_data[9];
	//	memcpy(out_buf, "\x73\x08", 2);
	//
	if( transport_15_data[9] == SZ_TRANSPORT_ENABLE)
		return CE_CARDSTATUS;

	//verify date
	memcpy(out_buf, "\x73\x78", 2);
	if((memcmp(tpCPU.time_bcd, &transport_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &transport_15_data[24], 4) > 0))
		return CE_EXPIREDDATE;
	memcpy(tpYKTTxnPurchase.Validday, &transport_15_data[24], 4);
	//verify local & remote card
	if((chCode = Transport_Whitelist(&transport_15_data[0])) != 0)
	{
		return chCode;
	}
	bln_transport_xian = 0xff;
	if( memcmp(&transport_15_data[0], XA_CODE_ORGANIZATION, 4) == 0 )
	{
		blnFinished = 0x00;
		tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t) + sizeof(JTBTxnPurchaseExII_t);
		tpYKTTxnPurchase.udType = 0x22;
		if( 0 != (chCode = CPUT_GetFiles05(out_buf)) )
			return chCode;
		if((memcmp(tpCPU.time_bcd, &cput_05_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_05_data[24], 4) > 0))
			return CE_EXPIREDDATE;
		//
		if(0 != (chCode = Transport_Map(&transport_15_data[0], cput_05_data[29], cput_05_data[28], &shTicketType)))
			return chCode;
		if((chCode = CPU_TellSysCard(shTicketType)) != 0)
		{
			return chCode;
		}
		if((chCode = Transport_GetFiles19(out_buf)) != 0)
			return chCode;
		//锁卡标志
		if( cpu_19_data[2] != 0)
			return CE_LOCKED_TICKET;
		//
		memcpy(tpJTBTxnPurchaseExII.unionCity, "\x71\x00", 2);
	}else
	{
		bln_transport_xian = 0x00;
		tpYKTTxnPurchase.AFCHead_val.length = sizeof(YKTTxnPurchase_t) + sizeof(JTBTxnPurchaseEx_t);
		tpYKTTxnPurchase.udType = 0x23;
		if( (chCode = Transport_GetFiles17(out_buf)) != 0)
			return chCode;
		memcpy(tpJTBTxnPurchaseEx.unionCity, &transport_17_data[6], 2);
		//ticket definition
		if(0 != (chCode = Transport_Map("\xFF\xFF\xFF\xFF", 0, transport_17_data[10], &shTicketType)))
			return chCode;
		if((chCode = CPU_TellSysCard(shTicketType)) != 0)
		{
			return chCode;
		}
		if((chCode = Transport_GetFiles1A(out_buf)) != 0)
			return chCode;
		//
		if(memcmp(&transport_17_data[8], "\x00\x01", 2) != 0)
			return CE_WHITELIST;
		//
		memset(&cpu_19_data[4], 0x00, 46);
		if( ((transport_1A_data[14] == 0x01) || (transport_1A_data[14] == 0x03)) && (memcmp(&transport_1A_data[19], XA_CODE_ORGANIZATION, 4) != 0) )
		{
			blnFinished = 0xff;
			cpu_19_data[3] = 0x01;
		}else
		{
			blnFinished = 0x00;
			//status
			cpu_19_data[3] = transport_1A_data[14];
			if( (transport_1A_data[14] == 0x02) || (transport_1A_data[14] == 0x04) )
				cpu_19_data[3] = 0x02;
			if( transport_1A_data[122] != 0)
				cpu_19_data[3] = transport_1A_data[122];
			//entry time
			memcpy(&cpu_19_data[4], &transport_1A_data[68], 5);
			//entry station
			memcpy(&cpu_19_data[9], &transport_1A_data[41], 2);
			cpu_19_data[9] = bcd2bin(cpu_19_data[9]);
			cpu_19_data[10] = bcd2bin(cpu_19_data[10]);
			//exit time
			memcpy(&cpu_19_data[11], &transport_1A_data[75], 5);
			//exit station
			memcpy(&cpu_19_data[16], &transport_1A_data[49], 2);
			cpu_19_data[16] = bcd2bin(cpu_19_data[16]);
			cpu_19_data[17] = bcd2bin(cpu_19_data[17]);
			//exit transaction amount
			memcpy(&cpu_19_data[18], &transport_1A_data[97], 2);
		}
	}
	//适应文档高低字节序调整
	tpYKTTxnPurchase.CrdSKnd = shTicketType >> 8;
	tpYKTTxnPurchase.CrdMKnd = (unsigned char)shTicketType;
	tpYKTTxnPurchase.AftBalance = tpYKTTxnPurchase.BefBalance = tpCPU.balance;
	//transaction amount initial value is zero
	tpCPU.tranamount = 0;
	memset(transport_1E_data, 0x00, 48);
	memset(last_timebcd, 0x00, 7);
	last_timebcd[0] = 0x20;
	memcpy(&last_timebcd[1], &cpu_19_data[4], 5);
	if(0 == (chRejectCode = Transport_TellEntry(cpu_19_data[3], 0)))
	{
		get_degrade_sensitive_mode(NULL, last_timebcd);
		if(tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_exit)
		{
			chRejectCode = CE_NO_ENTRY;
		}
	}
	blnOvertime = blnOverfare = 0x00;
	tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = (cmd_buf[20]);
	if(cmd_buf[25] == 1)
	{//Fee area
		if(chRejectCode == 0)
		{//entry status
			tpYKTTxnPurchase.Entime = timestr2long(&last_timebcd[1]) + TIME2000 - ZONE8;
			//calculate the price
			lngsrcstation = 0x09000000 + (cpu_19_data[9] << 8) + cpu_19_data[10];
			tpYKTTxnPurchase.Enlocation = tpYKTTxnPurchase.lastlocation = lngsrcstation;
			if(0 != (chRejectCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, tpCmdInit.curstation, &shFare)))
			{
				return chRejectCode;
			}
			if(tpTicketDef.ChargeFareOnCheckout == 0)
			{
				if(0 != (chRejectCode = cal_fare_value(last_timebcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
				{
					return chRejectCode;
				}
			}else
			{
				if(0 != (chRejectCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
				{
					return chRejectCode;
				}
			}
			if(((cpu_19_data[3] ) != 4) || (cpu_19_data[16] != tpCPU.curstation[0]) || (cpu_19_data[17] != tpCPU.curstation[1]))
			{
				if (tpCPU.balance < ((long)tpTicketDef.MinRemainingValue + tpSysPrice.price))
				{
					//
					memcpy(&cpu_19_data[16], tpCPU.curstation, 2);
					//
					cpu_19_data[3] = 4;
					//
					transport_1A_data[122] = 4;
					transport_1A_data[49] = bin2bcd(tpCPU.curstation[0]);
					transport_1A_data[50] = bin2bcd(tpCPU.curstation[1]);
					tpYKTTxnPurchase.udSubtype = 0x11;
					tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x01;
					blnOverfare = 0xff;
					if( (cmd_buf[20] == 0x01) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
						return CE_BADPARAM;
				}
			}
			//check the overtime
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpCPU.time_bcd, 0, 0)))
				{
					if(cmd_buf[26] != 2)
						return CE_BADPARAM;
					tpYKTTxnPurchase.udSubtype = 0x11;
					tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x03;
					blnOvertime = 0xff;
					//
					memcpy(&cpu_19_data[4], &tpCPU.time_bcd[1], 5);
					//
					memcpy(&transport_1A_data[67], tpCPU.time_bcd, 7);
				}
				if((cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD))
				{//deduct from card
					memcpy(&tpCPU.tranamount, &cmd_buf[21], 4);
					if(tpCPU.tranamount > tpCPU.balance)
						return CE_BADPARAM;
					tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount;
					tpYKTTxnPurchase.OrigAmt = 0;
					//tpYKTTxnPurchase.OrigAmt = tpCPU.tranamount; //20230411 应扣金额=200.实扣金额=200
					//tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = 0x02;
				}else
				{//MONEY
					tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount = 0;
					memcpy(&tpYKTTxnPurchase.OrigAmt, &cmd_buf[21], 4);
					//tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = 0x01;
				}
			}
			if( blnOvertime && blnOverfare )
			{
				tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x03;
			}
		}else
		{
			if(cmd_buf[26] != 0x01)
				return CE_BADPARAM;
			//补进站
			tpYKTTxnPurchase.udSubtype = 0x0E;
			tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x10;
			memcpy(&cpu_19_data[4], &tpCPU.time_bcd[1], 5);
			//cmd_buf[27]---intel 
			cpu_19_data[3] = XA_TRANSPORT_ENTRY;
			cpu_19_data[9] = cmd_buf[28];
			cpu_19_data[10] = cmd_buf[27];
			//
			lngsrcstation = 0x09000000 + (cpu_19_data[9] << 8) + cpu_19_data[10];
			tpYKTTxnPurchase.Enlocation = lngsrcstation;
			tpYKTTxnPurchase.lastlocation = 0;
			tpYKTTxnPurchase.Entime = 0;
			//
			transport_1A_data[14] = XA_TRANSPORT_ENTRY;
			//city code
			memcpy(&transport_1A_data[15], XA_CODE_CITY, 2);
			//oragination code
			memcpy(&transport_1A_data[19], XA_CODE_ORGANIZATION, 8);
			//entry station code
			memcpy(&transport_1A_data[35], "\x44\x12\x79\x10\x00\x00", 6);
			transport_1A_data[41] = bin2bcd(cmd_buf[28]);
			transport_1A_data[42] = bin2bcd(cmd_buf[27]);
			//entry station device 
			memcpy(&transport_1A_data[51], "\x79\x10\x00\x00\x00\x00\x00\x00", 8);
			transport_1A_data[55] = bin2bcd(cmd_buf[28]);
			transport_1A_data[56] = bin2bcd(cmd_buf[27]);
			//
			memcpy(&transport_1A_data[67], tpCPU.time_bcd, 7);
			//entry line code
			transport_1A_data[85] = cmd_buf[28];
			memset(&transport_1A_data[87], 0x00, 4);
			LongToByte(tpCPU.balance, &transport_1A_data[91]);
			//
			memset(&transport_1A_data[122], 0x00, 6);
			
			tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount = 0;
		}
	}else
	{//NON-Fee area
		if(chRejectCode == 0)
		{//entry status
			if(cmd_buf[26] != 0x03)
				return CE_BADPARAM;
			tpYKTTxnPurchase.udSubtype = 0x0F;
			tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x11;
			//
			lngsrcstation = 0x09000000 + (cpu_19_data[9] << 8) + cpu_19_data[10];
			tpYKTTxnPurchase.OrigEnlocation = tpYKTTxnPurchase.Enlocation = tpYKTTxnPurchase.lastlocation = lngsrcstation;
			tpYKTTxnPurchase.OrigEntime = tpYKTTxnPurchase.Entime = timestr2long(&last_timebcd[1]) + TIME2000 - ZONE8;
			//
			cpu_19_data[3] = XA_TRANSPORT_EXIT;
			transport_1A_data[14] = XA_TRANSPORT_EXIT;
			if((cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
			{//FROM Card
				memcpy(&tpCPU.tranamount, &cmd_buf[21], 4);
				tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount;
				//tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = 0x02;
			}else
			{//pay MONEY
				memcpy(&tpYKTTxnPurchase.OrigAmt, &cmd_buf[21], 4);
				tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount = 0;
				//tpJTBTxnPurchaseEx.payType = tpJTBTxnPurchaseExII.payType = 0x01;
			}
		}else
		{
			return CE_BADPARAM;
		}
	}
	tpYKTTxnPurchase.AftBalance = tpCPU.balance - tpCPU.tranamount;
	//
	if(CPU_init_for_capp(1, tpCPU.tranamount, ch_transport_psam_id, &transport_15_data[12], transport_15_data, 8, ch_cpu_mac_data))
	{
		return CE_INVADLIDCARD;
	}
	memcpy(out_buf, "\x4c\x17", 2);
	if(bln_transport_xian)
		ret = CPU_update_capp(1, 0x19, cpu_19_data[0], XA_TRANSPROT_19_LEN, cpu_19_data, 0);
	else
		ret = CPU_update_capp(1, 0x1A, transport_1A_data[1], TRANSPORT_1A_LEN, transport_1A_data, 0);
	
	if(ret)
	{
		return CE_WRITE;
	}
	//更改公共交通过程信息循环记录文件
	transport_1E_data[0] = 0x08;
	memcpy(&transport_1E_data[1], "\x79\x10\x00\x00", 4);
	memcpy(&transport_1E_data[5], tpCPU.curstation, 4);
	transport_1E_data[9] = 0x01;
	//line
	transport_1E_data[11] = tpCPU.curstation[0];
	//station
	transport_1E_data[13] = tpCPU.curstation[1];
	//operation code
	//transport_1E_data[14]
	//transaction amount
	LongToByte(tpCPU.tranamount, &transport_1E_data[17]);
	//
	LongToByte(tpCPU.balance, &transport_1E_data[21]);
	memcpy(&transport_1E_data[25], tpCPU.time_bcd, 7);
	
	memcpy(&transport_1E_data[32], XA_CODE_CITY, 2);
	memcpy(&transport_1E_data[34], XA_CODE_ORGANIZATION, 8);
	transport_1E_data[34] |= 0x10;
	
	ret = CPU_update_capp(0, 0x1e, 0x00, TRANSPORT_1E_LEN, transport_1E_data, 0);
	if(ret != 0)
	{
		return CE_WRITE;
	}

	memcpy(out_buf, "\x4c\x18", 2);
	ch_sz_transport_rollback = SZ_CPU_CAPP_1;
	if(CPU_debit_for_capppurchase(&ch_sz_transport_rollback, NULL, out_buf))
	{
		return CE_WRITE;
	}

	//
	tpJTBTxnPurchaseEx.version = 0x01;
	tpJTBTxnPurchaseEx.index = capp_init[9];
	//01-3des 02-SM2 04-SM4
	tpJTBTxnPurchaseEx.algorithm = 0x01;
	tpJTBTxnPurchaseEx.updateType = tpJTBTxnPurchaseExII.updateType = 0x00;
	//card sn -2
	tpYKTTxnPurchase.CrdDebitCnt = ByteToShort(NULL, &capp_init[4]);
	//tac -4
	memcpy(&tpYKTTxnPurchase.TAC, tpCPU.tac, 4);
	//sam sn -4
	tpYKTTxnPurchase.SamSeq = ByteToLong(NULL, tpCPU.sam_sn);

	//
	//UDSN
	out_buf[0] = 1;
	//recycle	1	0x00:no，0x01:yes，0x02:废票回收
	out_buf[1] = 0x00;
	//black
	out_buf[2] = 0x00;
	//ticket family
	out_buf[3] = XA_TRANSPORT_FAMILY;
	//ticket type
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_transport_phyical_id[4], 4);
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
	out_buf[36] = 0x03;
	//UD record length
	transLen = cnt = sizeof(YKTTxnPurchase_t);
	//UD
	memcpy(&out_buf[39], tpYKTTxnPurchase.AFCHead_val.operatorid, cnt);
	if( bln_transport_xian )
	{
		transLen += sizeof(JTBTxnPurchaseExII_t);
		memcpy(&out_buf[39 + cnt], tpJTBTxnPurchaseExII.cardIssuer, sizeof(JTBTxnPurchaseExII_t));
		cnt += sizeof(JTBTxnPurchaseExII_t);
	}else
	{
		transLen += sizeof(JTBTxnPurchaseEx_t);
		memcpy(&out_buf[39 + cnt], tpJTBTxnPurchaseEx.cardIssuer, sizeof(JTBTxnPurchaseEx_t));
		cnt += sizeof(JTBTxnPurchaseEx_t);
	}
	memcpy(&out_buf[37], &transLen, 2);
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
	ee_write_last_record(XA_TRANSPORT_FAMILY, 0, &out_buf[33], cnt);
	reader_status = XA_RW_IDLE;

	return CE_OK;
}


/************************************
CPU inquire
************************************/
char xa_transport_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100], factor[60], des[60], deslen;
unsigned char cpubuf[300], cpulen, Le, entry_time[4], entry_bcd[7], last_timebcd[7];
unsigned char cnt, chEXRejectCode, chEntryStatus;
unsigned long time2, transport_index, lngsrcstation, lngLosecond1;
unsigned long lngOverfarePenalty, lngOvertimePenalty, lngPenalty, lngenstation;
unsigned char chTicketType, chCode, chRejectCode;
unsigned char chmonth, blnOverfare, blnOvertime, blnFinished;
unsigned short	shFare, shTicketType;
JTBTerminal_t	tpPurchase;

	*out_len = 4;
	memcpy(out_buf, "\x85\x01\x00\x00", 4);
	memcpy(tpCPU.time_bcd, &cmd_buf[9], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[10]);

	blnFinished = 0x00;

	//verify local & remote card
	memcpy(out_buf, "\x85\x08", 2);
	bln_transport_xian = 0xff;
	if( memcmp(&transport_15_data[0], XA_CODE_ORGANIZATION, 4) == 0)
	{
		blnFinished = 0x00;
		if( 0 != (chCode = CPUT_GetFiles05(out_buf)) )
			return chCode;
		if((chCode = Transport_GetFiles19(out_buf)) != 0)
			return chCode;
		memcpy(tpJTBTxnPurchaseExII.unionCity, "\x71\x00", 2);
	}else
	{
		bln_transport_xian = 0x00;
		if((chCode = Transport_GetFiles1A(out_buf)) != 0)
			return chCode;
		//read file 17
		memcpy(out_buf, "\x85\x07", 2);
		if((chCode = Transport_GetFiles17(out_buf)) != 0)
			return chCode;
		memcpy(tpJTBTxnPurchaseEx.unionCity, &transport_17_data[6], 2);
		//调整到下面，作为补票建议代码
		//if(memcmp(&transport_17_data[8], "\x00\x01", 2) != 0)
		//	return CE_WHITELIST;
		//
		memset(&cpu_19_data[4], 0x00, 46);
		if( ((transport_1A_data[14] == 0x01) || (transport_1A_data[14] == 0x03)) && (memcmp(&transport_1A_data[19], XA_CODE_ORGANIZATION, 4) != 0) )
		{
			blnFinished = 0xff;
		}else
		{
			blnFinished = 0x00;
		}
			//status
			cpu_19_data[3] = transport_1A_data[14];
			if( (transport_1A_data[14] == 0x02) || (transport_1A_data[14] == 0x04) )
				cpu_19_data[3] = 0x02;
			if( transport_1A_data[122] != 0)
				cpu_19_data[3] = transport_1A_data[122];
			//entry time
			memcpy(&cpu_19_data[4], &transport_1A_data[68], 5);
			//entry station
			memcpy(&cpu_19_data[9], &transport_1A_data[41], 2);
			cpu_19_data[9] = bcd2bin(cpu_19_data[9]);
			cpu_19_data[10] = bcd2bin(cpu_19_data[10]);
			//exit time
			memcpy(&cpu_19_data[11], &transport_1A_data[75], 5);
			//exit station
			memcpy(&cpu_19_data[16], &transport_1A_data[49], 2);
			cpu_19_data[16] = bcd2bin(cpu_19_data[16]);
			cpu_19_data[17] = bcd2bin(cpu_19_data[17]);
			//exit transaction amount
			memcpy(&cpu_19_data[18], &transport_1A_data[97], 2);
		
	}
	chRejectCode = 0;
	
	*out_len = 214 + 22 + 1;
	chCode = CE_OK;
	//phiycial type
	out_buf[0] = XA_TRANSPORT_FAMILY;
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
	memcpy(&out_buf[22], transport_15_data, 28);
	//0016--55
	memcpy(&out_buf[52], transport_16_data, 55);
	//balance--4
	memcpy(&out_buf[107], &tpCPU.balance, 4);
	//0019
	//入口时间	5	YYMMDDHHMI
	//入口线路码	1
	//入口站码	1
	memcpy(&out_buf[111], &cpu_19_data[4], 7);
	//入口状态码	1
	//标注金额	2
	//指针1	1
	memcpy(&out_buf[118], "\x00\x00\x00\x00", 4);
	//出口时间	5	YYMMDDHHMI
	//出口线路码	1	
	//出口站码	1	
	memcpy(&out_buf[122], &cpu_19_data[11], 7);
	//出口状态码	1
	out_buf[129] = 0x00;
	//本次交易金额（分）	2
	memcpy(&out_buf[130], &cpu_19_data[18], 2);
	//指针2	1
	out_buf[132] = 0x00;
	//
	memset(last_timebcd, 0x00, 7);
	last_timebcd[0] = 0x20;
	memcpy(&last_timebcd[1], &cpu_19_data[4], 5);
	//进/出站状态	1
	memset(&out_buf[133], 0x00, 7);
	if((chEXRejectCode = Transport_TellEntry(cpu_19_data[3], 0)) == 0)
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
	memset(&out_buf[141], 0x00, 31);
	//0021---64
	memset(&out_buf[172], 0x00, 64);
	memcpy(&out_buf[172], transport_16_data, 55);
	//history
	out_buf[236] = 0x00;
	if(cmd_buf[22])
	{
		(*out_len) += 10 * 23;
		//history record number
		out_buf[236] = 0x0A;
		//history
		CPUT_GetFiles18(10, &out_buf[237]);
	}
#ifdef	DEBUG_PRINT
	PRINTK("CPU:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	chCode = CE_OK;
	//white list
	//if( (!bln_transport_xian) && ((chRejectCode = Transport_Whitelist(&transport_15_data[0])) != 0) )
	if( (chRejectCode = Transport_Whitelist(&transport_15_data[0])) != 0 )
	{
		out_buf[2] = out_buf[3] = chRejectCode;
		return chCode;
	}
	if( (!bln_transport_xian) && ( memcmp(&transport_17_data[8], "\x00\x01", 2) != 0) )
	{
		out_buf[2] = out_buf[3] = CE_WHITELIST;
		return chCode;
	}
	
	//ticket type
	if( bln_transport_xian)
	{
		if(0 != (chRejectCode = Transport_Map(&transport_15_data[0], cput_05_data[29], cput_05_data[28], &shTicketType)))
		{
			out_buf[2] = out_buf[3] = chRejectCode;
			return chCode;
		}
	}else
	{
		if(0 != (chRejectCode = Transport_Map("\xFF\xFF\xFF\xFF", 0, transport_17_data[10], &shTicketType)))
		{
			out_buf[2] = out_buf[3] = chRejectCode;
			return chCode;
		}
	}
	//车票类型
	out_buf[50] = (unsigned char)(shTicketType >> 8);
	out_buf[51] = (unsigned char)shTicketType;
	if((chRejectCode = CPU_TellSysCard(shTicketType)) != 0)
	{
		out_buf[2] = out_buf[3] = chRejectCode;
		return chCode;
	}
	//
	if( transport_15_data[9] == SZ_TRANSPORT_ENABLE)
	{
		out_buf[2] = out_buf[3] = CE_CARDSTATUS;
		return chCode;
	}
	if(0 != (chRejectCode = get_transport_purchase_ticket(shTicketType, &tpPurchase)))
	{
		out_buf[2] = out_buf[3] = chRejectCode;
		return chCode;
	}

	//black list 
	//check the black list and lock
	if(0 != (check_JTB_Black_Lock(&transport_15_data[0], &transport_15_data[10], 0xff,  out_buf, out_len)))
	{
		out_buf[2] = out_buf[3] = CE_BLACKLIST;
#ifdef DEBUG_PRINT
		PRINTK("reject code black %02x\n", out_buf[2]);
#endif
		return chCode;
	}
	//check the date every year
	if((memcmp(tpCPU.time_bcd, &transport_15_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &transport_15_data[24], 4) > 0))
	{
		out_buf[2] = out_buf[3] = CE_EXPIREDDATE;
		return chCode;
	}
	if(bln_transport_xian)
	{
		if((memcmp(tpCPU.time_bcd, &cput_05_data[20], 4) < 0) || (memcmp(tpCPU.time_bcd, &cput_05_data[24], 4) > 0))
		{
			out_buf[2] = out_buf[3] = CE_EXPIREDDATE;
			return chCode;
		}
	}
	//异地交易位完成
	if(blnFinished)
	{
		out_buf[2] = out_buf[3] = CE_FINISHED;
		return chCode;
	}
	//reject code check
	blnOverfare = blnOvertime = 0;
	if(cmd_buf[8] == 1)
	{//Fee area
		if(chEXRejectCode == 0)
		{//entry status
			//calculate the price
			lngsrcstation = 0x09000000 + (cpu_19_data[9] << 8) + cpu_19_data[10];
			if(0 != (chRejectCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, tpCmdInit.curstation, &shFare)))
			{
				out_buf[2] = chRejectCode;
				out_buf[3] = 0;
#ifdef DEBUG_PRINT
				PRINTK("reject code cal station fare %02x\n", out_buf[2]);
#endif
				return chCode;
			}
			if(tpTicketDef.ChargeFareOnCheckout == 0)
			{
				if(0 != (chRejectCode = cal_fare_value(last_timebcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
				{
					out_buf[2] = chRejectCode;
					out_buf[3] =0;
#ifdef DEBUG_PRINT
					PRINTK("reject code cal entry-time value %02x\n", out_buf[2]);
#endif
					return chCode;
				}
			}else
			{
				if(0 != (chRejectCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
				{
					out_buf[2] = chRejectCode;
					out_buf[3] =0;
#ifdef DEBUG_PRINT
					PRINTK("reject code cal cur-time value %02x\n", out_buf[2]);
#endif
					return chCode;
				}
			}
			//check the overfare
			tpCPU.tranamount = tpSysPrice.price;
			get_transport_bonus(&transport_15_data[0], transport_17_data[10], &tpCPU.tranamount, &tpCPU.lngBonus);
			tpSysPrice.price = tpCPU.tranamount;
			if( ((cpu_19_data[3] ) != 4) || ( (cpu_19_data[16] != tpCPU.curstation[0]) || (cpu_19_data[17] != tpCPU.curstation[1]) ) )
			{
				if (tpCPU.balance  < ((long)tpTicketDef.MinRemainingValue + tpSysPrice.price))
				{
					blnOverfare = 0xff;
					if(0 != (chRejectCode = cal_fare_value(tpCPU.time_bcd, &tpSJTTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
					{
						out_buf[2] = chRejectCode;
						out_buf[3] =0;
#ifdef DEBUG_PRINT
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
					//if( (cput_21_data[0] != XA_PASSENGER_ELDER) && (cput_21_data[0] != XA_PASSENGER_DISABLED) )
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
			memcpy(&last_timebcd[1], &cpu_19_data[4], 5);
			lngLosecond1 = timestr2long(&last_timebcd[1]);
			//
			if((lngLosecond1 > tpCPU.lowsecond) && (memcmp(&tpCPU.curstation[0], &cpu_19_data[9], 2) == 0))
			{
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
			}else if(((tpCPU.lowsecond - lngLosecond1) < 20 * 60) && (memcmp(&tpCPU.curstation[0], &cpu_19_data[9], 2) == 0))
			{
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
			}else 
			{
				out_buf[3] = CE_FEE_UPDATE_ENTRY;
				if(cmd_buf[17] == 1)
				{//according to the exit station calculate the price
					memcpy(&lngsrcstation, &cmd_buf[18], 4);
					lngenstation = 0x09000000 + (cpu_19_data[9] << 8) + (cpu_19_data[10]);
					if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngenstation, &shFare)))
						return chCode;
					//using the current time
					if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
						return chCode;
					tpCPU.tranamount = tpSysPrice.price;
					get_transport_bonus(&transport_15_data[0], transport_17_data[10], &tpCPU.tranamount, &tpCPU.lngBonus);
					tpSysPrice.price = tpCPU.tranamount;
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

	return CE_OK;
}
