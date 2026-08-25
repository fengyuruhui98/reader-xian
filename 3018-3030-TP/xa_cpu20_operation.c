#include "xa_cpu20_operation.h"
//需去掉lib定义
#include "linux2440lib.h"
#include "xa_error_code.h"
#include "xdr_file_manage.h"
#include "bin_file_manage.h"
#include "xa_sam.h"
#include "serial.h"
#include "hh_cpu_operation.h"
#include "xa_operation.h"
#include "time_tools.h"
#include "eeprom.h"
//#ifndef TESTLOG2022
//#define TESTLOG2022  1
//#endif
//#define	DEBUG_ROLLBACK	1
//#define DEBUG_PRINT_EM 1
//#define DEBUG_PRINT   1
extern unsigned char cput_15_data[30];

unsigned char cpu_02_data[4];			//电子钱包文件
unsigned char cpu_05_data[32];			//发行信息文件
unsigned char cpu_17_data[16];			//
unsigned char cpu_15_data[16];			//公共信息文件
unsigned char cpu_1a_data[16];
unsigned char cpu_1b_data[16];			//
unsigned char cpu_1c_data[16];			//
unsigned char cpu_1d_data[16];			//
unsigned char key[2];					//key head
unsigned char str2023[100]; //testlog fprintf string 20230714
unsigned char buf2023[50];//tsetlog fprintf string 20230714

void TestLog(unsigned char *buf,unsigned char *string,unsigned char len)
{
	int size, i = 0;
    struct stat statbuf;
	FILE *fp;
	
	fp = fopen("LOGS.log", "a+");
	stat(fp, &statbuf);
	size = statbuf.st_size;
	if (size >= 1048576*20) //1M= 1048576字节
	{
		remove("LOGS.log");
	}
	fprintf(fp, "%s ", string);
	if(len != 1)
	{
		for(i=0;i<len;i++)
		{
			fprintf(fp, "%02x ", buf[i]);
		}
	}

	fprintf(fp, "\n");
   // fprintf(fp, "[%x]\r\n", str);
	fclose(fp);
}
void sz_timetype2bcd(unsigned char *timetype, unsigned char *timebcd,unsigned char flag)
{
	unsigned char i;
	if(flag)
	{
		timebcd[0] = timetype[0]/2;
		timebcd[1] = timetype[0]&0x01;
		timebcd[1]*= 2;
		timebcd[1]+= timetype[1]/32;
		timebcd[2] = timetype[1]&0x1f;
		for(i = 0; i < 3; i++)
		{
			timebcd[i] = bin2bcd(timebcd[i]);
		}
	}
	else
	{
		timebcd[0] = timetype[0]/4;
		timebcd[1] = timetype[0]&0x03;
		timebcd[1]*= 4;
		timebcd[1]+= timetype[1]/64;
		timebcd[2] = (timetype[1]/2)&0x1f;
		timebcd[3] = timetype[1]&0x01;
		timebcd[3]*= 16;
		timebcd[3]+= timetype[2]/16;
		timebcd[4] = timetype[2]&0x0f;
		timebcd[4]*= 4;
		timebcd[4]+= timetype[3]/64;
		timebcd[5] = timetype[3]&0x3f;
		for(i=0;i<6;i++)
		{
			timebcd[i] = bin2bcd(timebcd[i]);
		}
	}
	return ;
}
/*
*/
void sz_CPU20_ee_write(unsigned char sn_bak)
{
unsigned short addr;
	
	if(sn_bak)
		memcpy(ch_cpu20_phyical_id_bak, ch_cpu20_phyical_id, 8);
	addr = EE_MCPU_BACKUP;
	ee_write(addr, 1, &ch_sz_cpu_rollback);
	addr += 1;
	ee_write(addr, 8, ch_cpu20_phyical_id);
	addr += 8;
	ee_write(addr, 1, &chCode_bak);
	addr += 1;
	ee_write(addr, 2, sfi_bak);
	addr += 2;
	ee_write(addr, XA_CPU_05_LEN, cpu_05_data);
	addr += XA_CPU_05_LEN;
	ee_write(addr, XA_CPU_15_LEN, cpu_15_data);
	addr += XA_CPU_15_LEN;
	ee_write(addr, XA_CPU_17_LEN, cpu_17_data);
	addr += XA_CPU_17_LEN;
	ee_write(addr, XA_CPU_1A_LEN, cpu_1a_data);
	addr += XA_CPU_1A_LEN;
	ee_write(addr, XA_CPU_1B_LEN, cpu_1b_data);
	addr += XA_CPU_1B_LEN;
	ee_write(addr, XA_CPU_1C_LEN, cpu_1c_data);
	addr += XA_CPU_1C_LEN;
	ee_write(addr, XA_CPU_1D_LEN, cpu_1d_data);
	addr += XA_CPU_1D_LEN;

	ee_write(addr, sizeof(tpCPU), tpCPU.curtime);
	addr += sizeof(tpCPU);
	ee_write(addr, 19, capp_init);

}

/*
*/
void xa_MCPU_rollback_write(unsigned char *record, unsigned short len)
{
unsigned short addr;
unsigned char i, j;
unsigned long protectSecond;

	//find the first NO rollBack
	for(i = 0; i < 10; i++)
	{
		if(memcmp(tpMCPUProtect[i].phyicalID, ch_cpu20_phyical_id, 8) == 0)
			break;
	}
	if(i >= 10)
	{
		for(i = 0; i < 10; i++)
		{
			if(tpMCPUProtect[i].rollBack == 0)
				break;
		}
	}
	//if list is full then rewrite the FIRST record
	if(i >= 10)
	{
		i = 0;
		protectSecond = tpMCPUProtect[i].usecond;
		for(j = 0; j < 10; j++)
		{
			if(protectSecond < tpMCPUProtect[j].usecond)
			{
				i = j;
				protectSecond = tpMCPUProtect[j].usecond;
			}
		}
	}
	//
	tpMCPUProtect[i].rollBack = ch_sz_cpu_rollback;
	memcpy(tpMCPUProtect[i].phyicalID, ch_cpu20_phyical_id, 8);
	tpMCPUProtect[i].usecond = ~tpCPU.lowsecond;
	tpMCPUProtect[i].tranAmount = tpCPU.tranamount;
	tpMCPUProtect[i].balance = tpCPU.balance;
	//
	if(record != NULL)
		memcpy(tpMCPUProtect[i].rec_buf, record, len);
	tpMCPUProtect[i].rec_len = len;
	
	return ;
}

/*
*/
void xa_CPU_rollback_read(unsigned char *record, unsigned short len)
{
unsigned short addr;
unsigned char i;

	//find the first NO rollBack
	for(i = 0; i < 10; i++)
	{
		if(tpMCPUProtect[i].rollBack == 0)
			break;
	}
	//if list is full then rewrite the FIRST record
	if(i >= 10)
		i = 0;
	//
	tpMCPUProtect[i].rollBack = ch_sz_cpu_rollback;
	memcpy(tpMCPUProtect[i].phyicalID, ch_cpu20_phyical_id, 8);
	tpMCPUProtect[i].tranAmount = tpCPU.tranamount;
	//
	memcpy(tpMCPUProtect[i].rec_buf, record, len);
	tpMCPUProtect[i].rec_len = len;
	
	return ;
}


/*
function:
	1. if TestingFlag in the issue file is 1 and device mode is normal return 11
	2. if TestingFlag in the issue file is 0 and device mode is testing mode return 11
parameter:
	0:working mode
	1:test mode
return :
	11: not matching mode
	0: mode match ok
*/
char CPU_TellTesting(unsigned char chTestMode)
{
	if(chTestMode)//test mode
	{
		if(tpfile05.testCard == 0)
		{
			return CE_TESTING_STATUS;
		}
	}
	else
	{
		if(tpfile05.testCard != 0)
		{
			return CE_TESTING_STATUS;
		}
	}
	return 0;
}

/*
function:
	1. check the entry block MAC
	2. whether check the mode or not. if FALSE then return the entry mac result.
return:
	0: check MAC ok and return correctly MAC
	18: entry MAC wrong .
*/
char CPU_TellEntry(char entryMAC, unsigned char mode_check)
{
char mac[4], chret;
unsigned long i, InitStationNum, InitSensitiveNum;
unsigned char chInitStationFare, chInitSensitiveFare, chCode;
unsigned char eod_station[4], sta_close_entry[4], sen_close_entry[4];
unsigned short shCalFare;
unsigned long lngsrcstation, lngdesstation;

	chret = CE_NO_ENTRY;
	
	//0x01 for last is entry status and 0x04 for last is update
	if((entryMAC == 0x01) || (entryMAC == 0x04))
		chret = 0;

	if(!mode_check)
		return chret;
	if(chret == CE_NO_ENTRY)
	{
	//if current station is set to entry mode then the entry station/time is current station/time
		if(tpwaivermode.cur_sta_entry)
		{
			//entry time is set to the current time
			xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
			//entry station is set the current station
			lngsrcstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
			if(0 != (chCode = location_to_card(lngsrcstation, &tpfile15.lastLocation)))
				return chCode;
			chret = 0;
		}else if(tpwaivermode.oth_sta_entry || tpwaivermode.sen_sta_entry)
		{
			//entry time is set to the current time
			xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
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
			//if set broadcast entry number is zero then
			//select the most close entry station from the sensitive period
			/*chInitSensitiveFare = 0xff;
			InitSensitiveNum = 0;
			for(i = 0; i < EodWaiverDateMasterConfig.StationModeInfo.StationModeInfo_len; i++)
			{
				ShortToByte(EodWaiverDateInfo[i].StationID, eod_station);
				if(EodWaiverDateInfo[i].ModeCode == SZ_WAIVER_ENTRY)
					//&& (memcmp(tpSJT.curstation, eod_station, 2) != 0))
				{
					if(0 != (cal_station_fare(eod_station, tpSJT.curstation, &chCalFare)))
						continue;
					InitSensitiveNum += 1;
					if(chCalFare < chInitSensitiveFare)
					{
						chInitSensitiveFare = chCalFare;
						memcpy(sen_close_entry, eod_station, 2);
					}
				}
			}*/
			//if mode list waiver_entry station is zero
			if((InitStationNum == 0))// && (InitSensitiveNum == 0))
				return chret;
			/*else if(InitStationNum == 0)
				memcpy(eod_station, sen_close_entry, 2);
			else if(InitSensitiveNum == 0)
				memcpy(eod_station, sta_close_entry, 2);
			else if(chInitSensitiveFare < chInitStationFare)
				memcpy(eod_station, sen_close_entry, 2);
			else*/
				memcpy(eod_station, sta_close_entry, 4);
			//entry station is set the most close station
			lngsrcstation = (eod_station[0] << 24) + (eod_station[1] << 16) + (eod_station[2] << 8) + eod_station[3];
			if(0 != (chCode = location_to_card(lngsrcstation, &tpfile15.lastLocation)))
				return chCode;
			chret = 0;
		}
	}
	
	return chret;
}

/*
function:external authorization
*/
char CPU_gettransprove(unsigned char transtypeid, unsigned char *sfi, char sam_index, unsigned char *cpu_factor, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], factor[20], des[60], deslen;
unsigned char cpubuf[100], cpurandom[8];
char chCode, capp_init_bak[19];
unsigned short capp_sn, capp_sn_bak, cpulen;

	//select file
	memcpy(out_buf, "\xf0\x90", 2);
	if(0 != (chCode = CPU_select_file(sfi, 2, out_buf, NULL)))
		return chCode;

	//extern auth
	memcpy(out_buf, "\xf0\x91", 2);
	if((chCode = CPU_externauth(0, sam_index, cpu_factor, out_buf)) != 0)
		return chCode;
	
	if(transtypeid == 0x09)
	{
		//initilize for capp according to the sn to check whether transaction finished or not
		memcpy(capp_init_bak, capp_init, 19);
		memcpy(out_buf, "\xf0\x92", 2);
		if(0 != CPU_init_for_capp(1, tpCPU.tranamount, ch_cpu20_psam_id, ch_cpu20_logic_id, "\x21\x50\x80", 3, ch_cpu_mac_data))
		{
			memcpy(capp_init, capp_init_bak, 19);
			memcpy(out_buf, "\xf0\x93", 2);
			if(0 != CPU_select_file("\x3f\x00", 2, out_buf, NULL))
				return CE_READ;
		}
	}else
	{
		//initilize for credit according to the sn to check whether transaction finished or not
		memcpy(capp_init_bak, capp_init, 19);
		memcpy(out_buf, "\xf0\x92", 2);
		if(0 != CPU_init_for_credit(tpCPU.tranamount, ch_cpu20_psam_id, out_buf))
		{
			return CE_READ;
		}
	}
	
	ByteToShort(&capp_sn_bak, &capp_init_bak[4]);
	ByteToShort(&capp_sn, &capp_init[4]);
	memcpy(out_buf, "\xf0\x94", 2);
	if(capp_sn == (capp_sn_bak + 1))
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
		if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		{
			//select file
			if(0 != CPU_select_file("\x3f\x00", 2, out_buf, NULL))
				return CE_READ;
			return CE_INVADLIDCARD;
		}
#ifdef DEBUG_TIME
		ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif
		memcpy(capp_init, capp_init_bak, 19);
		memcpy(tpCPU.tac, &cpubuf[4], 4);
		return 0;
	}
	//not need rollback then clean the flag in ee
	ch_sz_cpu_rollback = 0;
//	{
//#ifdef DEBUG_PRINT
//		PRINTK("get transprove success but update the ee failure\n");
//#endif
//	}
//	ee_read(1, 1, buf);
	memcpy(out_buf, "\xf0\x95", 2);
	if(0 != CPU_select_file("\x3f\x00", 2, out_buf, NULL))
		return CE_READ;
	return CE_INVADLIDCARD;
}



/*
function:read the file 05
parameter:
*/
char CPU_GetFiles05(unsigned char *out_buf)
{
int ret;
unsigned char buf[40], chCode;
unsigned char cpubuf[300], Le, sellen;
unsigned short out_len, cpulen;
	
	//pboc card PPSE: 2PAY.SYS.DDF01
	memcpy(buf, "\x32\x50\x41\x59\x2e\x53\x59\x53\x2e\x44\x44\x46\x30\x31", 14);
	chCode = CPU_select_file(buf, 14, &cpubuf[0], &sellen);
	if(chCode == 0)
	{
		//select JTB AID:\xA0\x00\x00\x06\x32\x01\x01\x05
		xa_ticket_family = XA_TRANSPORT_FAMILY;
		PPSE_len = sellen;
		memcpy(PPSE, cpubuf, PPSE_len);

		tpCPU.EDorEP = CPU_ED;
		tpCPU.thread_mac1 = 6;
		tpCPU.thread_mac2 = 7;
		tpCPU.capp_type = 9;
		tpCPU.capp_len = 0x24;
		tpCPU.sz_psam_index = xa_transport_psam_index;

		chCode = xa_pboc_select(PPSE, PPSE_len, &xa_ticket_family);
		if(chCode == 0)
			return 0;
		if(CE_INVADLIDCARD != chCode) 
			return chCode;
	}else if (chCode == 0xFF)
		return CE_READ;
	//select 3f01---a00000000386980701
	memcpy(out_buf, "\xf0\x17", 2);
	memcpy(buf, "\xa0\x00\x00\x00\x03\x86\x98\x07\x01", 9);
	chCode = CPU_select_file(buf, 9, &cpubuf[0], &sellen);
	if((chCode != 0) && (cpubuf[0] == 0x6A) && (cpubuf[1] == 0x81))
	{
		//check_YKT_Black_Lock("\x71\x00", "\x03\x01", "\x00\x00\x00\x00\x00\x00\x00\x00", out_buf, &out_len);
		xa_ticket_family = XA_CITY_FAMILY;
		return CE_READ;
	}
	if(chCode == 0)
	{
		//read file 15
		if(sellen > XA_CPUT_15_LEN)
			memcpy(cput_15_data, &cpubuf[sellen - XA_CPUT_15_LEN], XA_CPUT_15_LEN);
		else
			memset(cput_15_data, 0x00, XA_CPUT_15_LEN);
		tpCPU.EDorEP = CPU_ED;
		tpCPU.thread_mac1 = 6;
		tpCPU.thread_mac2 = 7;
		tpCPU.capp_type = 9;
		tpCPU.capp_len = 0x24;
		tpCPU.sz_psam_index = xa_tong_psam_index;
		xa_ticket_family = XA_CITY_FAMILY;
#ifdef DEBUG_PRINT
		PRINTK("file 15 Issued:%02x%02x City:%02x%02x Bussiness:%02x%02x RFU%02x%02x Flag:%02x app ver:%02x \n", cput_15_data[0], cput_15_data[1], cput_15_data[2], cput_15_data[3],
			cput_15_data[4], cput_15_data[5], cput_15_data[6], cput_15_data[7], cput_15_data[8], cput_15_data[9]);
		PRINTK("CityUnion:%02x%02x sn:%02x%02x%02x%02x %02x%02x%02x%02x\n",
			 cput_15_data[10], cput_15_data[11], cput_15_data[12], cput_15_data[13], cput_15_data[14], cput_15_data[15], cput_15_data[16], cput_15_data[17], cput_15_data[18], cput_15_data[19]);
		PRINTK(" app startdate %02x%02x%02x%02x valid date %02x%02x%02x%02x M-type %02x S-type%02x\n", 
			cput_15_data[20], cput_15_data[21], cput_15_data[22], cput_15_data[23], cput_15_data[24], cput_15_data[25], cput_15_data[26], cput_15_data[27], cput_15_data[28], cput_15_data[29]);
#endif
		return 0;
	}

	//xian metro card
	tpCPU.EDorEP = CPU_ED;
	tpCPU.thread_mac1 = 1;
	tpCPU.thread_mac2 = 2;
	tpCPU.capp_type = 6;
	tpCPU.capp_len = 0x24;
	tpCPU.sz_psam_index = xa_metro_psam_index;

	//read file 05
	memcpy(out_buf, "\xf0\x05", 2);
	memcpy(buf, "\x00\xb0\x85\x00\x20", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
#ifdef DEBUG_2_PRINT
		PRINTK("read file 05 failure return %d\n", ret);
#endif
		return CE_READ;
	}
	memcpy(out_buf, "\xf0\x06", 2);
#ifdef DEBUG_PRINT
	PRINTK("file 05 len :%02x,%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
			cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3],	cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7], cpubuf[8], cpubuf[9], cpubuf[10], cpubuf[11], cpubuf[12], cpubuf[13], cpubuf[14], cpubuf[15]);
	PRINTK("   %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
			cpubuf[16], cpubuf[17], cpubuf[18], cpubuf[19],	cpubuf[20], cpubuf[21], cpubuf[22], cpubuf[23], cpubuf[24], cpubuf[25], cpubuf[26], cpubuf[27], cpubuf[28], cpubuf[29], cpubuf[30], cpubuf[31]);
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	memcpy(cpu_05_data, cpubuf, 32);
	//
	memcpy(xaFile05.buff, &cpu_05_data[12], 20);
	tpfile05.cardissuerId = xaFile05._File05.cardissuerId;
	tpfile05.testCard = xaFile05._File05.testCard;
	tpfile05.cardBatchNumber = (xaFile05._File05.cardBatchNumber_1 << 7) + xaFile05._File05.cardBatchNumber;
	tpfile05.cardBaseDateTime = (xaFile05._File05.cardBaseDateTime_1 << 8) + xaFile05._File05.cardBaseDateTime;
	tpfile05.checksum = xaFile05._File05.checksum;
	tpfile05.keySetNumber = xaFile05._File05.keySetNumber;
	tpfile05.version = xaFile05._File05.version;
	tpfile05.language = xaFile05._File05.language;
	tpfile05.passengerType = xaFile05._File05.passengerType;
	tpfile05.cardDepositValue = xaFile05._File05.cardDepositValue;
	tpfile05.lifecycleCount = (xaFile05._File05.lifecycleCount_1 << 3) + xaFile05._File05.lifecycleCount;
	tpfile05.productIssuerId = xaFile05._File05.productIssuerId;
	tpfile05.productCategory = xaFile05._File05.productCategory;
	tpfile05.purseId = xaFile05._File05.purseId;
	tpfile05.productId = (xaFile05._File05.productId_1 << 4) + xaFile05._File05.productId;
	tpfile05.productSerialNumber = (xaFile05._File05.productSerialNumber_1 << 8) + xaFile05._File05.productSerialNumber;
	tpfile05.purseIssuerId = xaFile05._File05.purseIssuerId;
	tpfile05.productPurchaseValue = (xaFile05._File05.productPurchaseValue_1 << 16) + (xaFile05._File05.productPurchaseValue_2 << 8) + xaFile05._File05.productPurchaseValue;
	tpfile05.cardManufactureID = xaFile05._File05.cardManufactureID;
	tpfile05.MaxRideOneDay = (xaFile05._File05.MaxRideOneDay_1 << 1) + xaFile05._File05.MaxRideOneDay;
#ifdef DEBUG_PRINT
	PRINTK("cardissuerID %02x test:%02x batch:%04x date:%04x keyset:%02x ver:%02x lang %02x passenger %02x depsoit:%02x life %04x product %02x\n", 
			tpfile05.cardissuerId, tpfile05.testCard, tpfile05.cardBatchNumber, tpfile05.cardBaseDateTime, tpfile05.keySetNumber, tpfile05.version, tpfile05.language, tpfile05.passengerType, tpfile05.cardDepositValue, tpfile05.lifecycleCount, tpfile05.productCategory);
	PRINTK(" productisser %02x purseid:%02x productid:%02x productsn %04x purseissuer %02x productvalue %06x manufacture:%02x maxride %02x\n", 
			tpfile05.productIssuerId, tpfile05.purseId, tpfile05.productId, tpfile05.productSerialNumber, tpfile05.purseIssuerId, tpfile05.productPurchaseValue, tpfile05.cardManufactureID, tpfile05.MaxRideOneDay);
#endif
#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif
/*	memcpy(buf, "\x3f\x01\x00\x00\x03\x86\x98\x07\x01", 2);
	chCode = CPU_select_file(buf, 2, out_buf, NULL);
	CPU_GetFiles15(out_buf);
	CPU_GetFiles1b(out_buf);
	CPU_GetFiles1c(out_buf);
	CPU_GetFiles1d(out_buf);*/
	xa_ticket_family = XA_MCPU_FAMILY;
	return 0;
}

/*
function:read file 17
parameter:
	*entrytime:
	*curtime:
	*mileclass:
*/
char CPU_GetFiles17(char rec_num, unsigned char *out_buf)
{
int ret, i, j;
unsigned char buf[80];
unsigned char cpubuf[100], Le;
unsigned short	cpulen;
	
	if(rec_num > 9)
		rec_num = 9;
	//
	memset(out_buf, 0x00, rec_num * XA_CPU_17_LEN);
	//read history
	memcpy(buf,"\x00\xb2\x00\x00\x17", 5);
	buf[3] = (0x17 << 3) | 0x04;
	Le = XA_CPU_17_LEN;
	
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
		PRINTK("read 17 data:");
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
#ifdef	DEBUG_PRINT
		memcpy(xaFile17.buff, cpubuf, 16);
		tpfile17.dateTime = (xaFile17._File17.dateTime_1 << 24) + (xaFile17._File17.dateTime_2 << 16) + (xaFile17._File17.dateTime_3 << 8) + xaFile17._File17.dateTime;
		tpfile17.serviceProviderId = xaFile17._File17.serviceProviderId;
		tpfile17.category = xaFile17._File17.category;
		tpfile17.productIssuerId = xaFile17._File17.productIssuerId;
		tpfile17.transactionType = xaFile17._File17.transactionType;
		tpfile17.paymentMethod = xaFile17._File17.paymentMethod;
		tpfile17.productTypeId = xaFile17._File17.productTypeId;
		tpfile17.location = (xaFile17._File17.location_1 << 10) + (xaFile17._File17.location_2 << 2) + xaFile17._File17.location;
		tpfile17.value = (xaFile17._File17.value_1 << 11) + (xaFile17._File17.value_2 << 3) + xaFile17._File17.value;
		tpfile17.remainingValue = (xaFile17._File17.remainingValue_1 << 12) + (xaFile17._File17.remainingValue_2 << 4) + xaFile17._File17.remainingValue;
		PRINTK("datetime %08x sp %02x category %02x pII %02x trantype %02x pay %02x producttype %02x location %04x value %06x remaing %06x\n",
			tpfile17.dateTime, tpfile17.serviceProviderId, tpfile17.category, tpfile17.productIssuerId, tpfile17.transactionType,
			tpfile17.paymentMethod, tpfile17.productTypeId, tpfile17.location, tpfile17.value, tpfile17.remainingValue);
#endif
		memcpy(&out_buf[(i - 1) * XA_CPU_17_LEN], cpubuf, XA_CPU_17_LEN);
	}
	return 0;
}

/*
function:read file 18
parameter:
	rec_num: history 
*/
char CPU_GetFiles18(char rec_num, unsigned char *out_buf)
{
int ret;
unsigned char buf[80], chCode;
unsigned char cpubuf[100], Le;
long 	i, j;
unsigned short cpulen;

	if(rec_num > 10)
		rec_num = 10;
	//
	memset(out_buf, 0x00, rec_num * XA_CPU_18_LEN);
	//read history
	memcpy(buf,"\x00\xb2\x00\x00\x17", 5);
	buf[3] = (0x18 << 3) | 0x04;
	Le = XA_CPU_18_LEN;
	
	for(i = 1; i < rec_num + 1; i++)
	{
		buf[2] = i;
#ifdef	DEBUG_PRINT
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
		memcpy(&out_buf[(i - 1) * XA_CPU_18_LEN], cpubuf, XA_CPU_18_LEN);
	}
	return (i - 1);
}

/*
function:read the file 15
parameter:
*/
char CPU_GetFiles15(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40];
unsigned char cpubuf[80], Le;
unsigned short cpulen;

	//read balance
	memcpy(out_buf, "\xf5\x20", 2);
	memcpy(buf, "\x80\x5c\x00\x02\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("read balance return len %d %02x %02x %02x %02x ", cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3]);
#endif
	memcpy(out_buf, "\xf5\x21", 2);
	if(cpulen != 6)
	{
		return CE_INVADLIDCARD;
	}
	ByteToLong(&tpCPU.balance, cpubuf);
#ifdef DEBUG_PRINT
	PRINTK("cpu balance:%d\n", tpCPU.balance);
#endif
	memcpy(cpu_02_data, cpubuf, 4);

	//read file 15 - binary file
	memcpy(out_buf, "\xf5\x22", 2);
	memcpy(buf, "\x00\xb0\x95\x00\x10", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\xf5\x23", 2);
#ifdef DEBUG_PRINT
	PRINTK("read file 15:len%d ", cpulen);
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf5\x24", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memcpy(cpu_15_data, cpubuf, 16);
#ifdef DEBUG_PRINT
	PRINTK("file 15 len:%02x,%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
			cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3],	cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7], cpubuf[8], cpubuf[9], cpubuf[10], cpubuf[11], cpubuf[12], cpubuf[13], cpubuf[14], cpubuf[15]);
#endif
	memcpy(xaFile15.buff, cpu_15_data, 13);
	tpfile15.cardStatus = xaFile15._File15.cardStatus;
	tpfile15.productId = xaFile15._File15.productId;
	tpfile15.startDateTime = (xaFile15._File15.startDateTime_1 << 16) + (xaFile15._File15.startDateTime_2 << 8) + xaFile15._File15.startDateTime;
	tpfile15.origin = (xaFile15._File15.origin_1 << 4) + xaFile15._File15.origin;
	tpfile15.destination = (xaFile15._File15.destination_1 << 8) + xaFile15._File15.destination;
	tpfile15.lastDateTime = xaFile15._File15.lastDateTime;
	tpfile15.totalPurchaseValue = (xaFile15._File15.totalPurchaseValue_1 << 8) + xaFile15._File15.totalPurchaseValue;
	tpfile15.lastLocation = (xaFile15._File15.lastLocation_1 << 4) + xaFile15._File15.lastLocation;
	tpfile15.transfersTaken = xaFile15._File15.transfersTaken;
	tpfile15.journeyStatus = xaFile15._File15.journeyStatus;
#ifdef DEBUG_PRINT
	PRINTK("status %02x productid %02x starttime %06x orgin %03x des %03x lasttime %02x totalvalue %04x lastlocation %03x transfer %02x journey %02x\n",
		tpfile15.cardStatus, tpfile15.productId, tpfile15.startDateTime, tpfile15.origin, tpfile15.destination, tpfile15.lastDateTime, tpfile15.totalPurchaseValue, tpfile15.lastLocation, tpfile15.transfersTaken, tpfile15.journeyStatus);
#endif	
	return 0;
}

/*
function:read the file 1b
parameter:
*/
char CPU_GetFiles1b(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40];
unsigned char cpubuf[80], Le;
unsigned short cpulen;
	
	//read file 1b - binary file
	memcpy(out_buf, "\xfb\x22", 2);
	memcpy(buf, "\x00\xb0\x9b\x00\x10", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\xfb\x23", 2);
#ifdef DEBUG_2_PRINT
	PRINTK("read file 1b:len%d ", Le);
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xfb\x24", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memcpy(cpu_1b_data, cpubuf, 16);
#ifdef DEBUG_PRINT
	PRINTK("file 1b len:%02x,%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
			cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3],	cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7], cpubuf[8], cpubuf[9], cpubuf[10], cpubuf[11], cpubuf[12], cpubuf[13], cpubuf[14], cpubuf[15]);
#endif
	memcpy(xaFile1B.buff, cpu_1b_data, 11);
	tpfile1b.actionSequenceNumber = xaFile1B._File1B.actionSequenceNumber;
	tpfile1b.productStatus = xaFile1B._File1B.productStatus;
	tpfile1b.transactionSequenceNumber = (xaFile1B._File1B.transactionSequenceNumber_1 << 8) + xaFile1B._File1B.transactionSequenceNumber;
	tpfile1b.lavSamId = (xaFile1B._File1B.lavSamId_1 << 8) + xaFile1B._File1B.lavSamId;
	tpfile1b.validityStartDate = (xaFile1B._File1B.validityStartDate_1 << 4) + xaFile1B._File1B.validityStartDate;
	tpfile1b.lavPaymentMethod = xaFile1B._File1B.lavPaymentMethod;
	tpfile1b.activated = xaFile1B._File1B.activated;
	tpfile1b.lavValue = (xaFile1B._File1B.lavValue_1 << 9) + (xaFile1B._File1B.lavValue_2 << 1) + xaFile1B._File1B.lavValue;
	tpfile1b.validityDuration = xaFile1B._File1B.validityDuration;
	tpfile1b.validityDurationType = xaFile1B._File1B.validityDurationType;
	tpfile1b.invoicePrinted = xaFile1B._File1B.invoicePrinted;
#ifdef DEBUG_PRINT
	PRINTK("status %02x actionSn %02x transSN %04x samid %04x validityStartdate %03x payment %02x activate %02x value %06x validduration %02x durationtype %02x printed %02x\n",
		tpfile1b.productStatus, tpfile1b.actionSequenceNumber, tpfile1b.transactionSequenceNumber, tpfile1b.lavSamId, tpfile1b.validityStartDate, tpfile1b.lavPaymentMethod, tpfile1b.activated, tpfile1b.lavValue,
		tpfile1b.validityDuration, tpfile1b.validityDurationType, tpfile1b.invoicePrinted);
#endif	
	return 0;
}

/*
function:read the file 1c
parameter:
*/
char CPU_GetFiles1c(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40];
unsigned char cpubuf[80], Le;
unsigned short cpulen;
	
	//read file 1b - binary file
	memcpy(out_buf, "\xfc\x22", 2);
	memcpy(buf, "\x00\xb0\x9c\x00\x10", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\xfc\x23", 2);
#ifdef DEBUG_2_PRINT
	PRINTK("read file 1c:len%d ", Le);
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xfc\x24", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memcpy(cpu_1c_data, cpubuf, 16);
#ifdef DEBUG_PRINT
	PRINTK("file 1c len:%02x,%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
			cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3],	cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7], cpubuf[8], cpubuf[9], cpubuf[10], cpubuf[11], cpubuf[12], cpubuf[13], cpubuf[14], cpubuf[15]);
#endif
	memcpy(xaFile1C.buff, cpu_1c_data, 16);
	tpfile1c.productStatus = xaFile1C._File1C.productStatus;
	tpfile1c.actionSequenceNumber = xaFile1C._File1C.actionSequenceNumber;
	tpfile1c.activated = xaFile1C._File1C.activated;
	tpfile1c.invoicePrinted = xaFile1C._File1C.invoicePrinted;
	tpfile1c.validityStartDateTime = (xaFile1C._File1C.validityStartDateTime_1 << 16) + (xaFile1C._File1C.validityStartDateTime_2 << 8) + xaFile1C._File1C.validityStartDateTime;
	tpfile1c.validityOrigin = (xaFile1C._File1C.validityOrigin_1 << 4) + xaFile1C._File1C.validityOrigin;
	tpfile1c.validityDestination = (xaFile1C._File1C.validityDestination_1 << 8) + xaFile1C._File1C.validityDestination;
	tpfile1c.transactionSequenceNumber = (xaFile1C._File1C.transactionSequenceNumber_1 << 8) + xaFile1C._File1C.transactionSequenceNumber;
	tpfile1c.lavSamId = (xaFile1C._File1C.lavSamId_1 << 8) + xaFile1C._File1C.lavSamId;
	tpfile1c.lavValue = (xaFile1C._File1C.lavValue_1 << 9) + (xaFile1C._File1C.lavValue_2 << 1) + xaFile1C._File1C.lavValue;
	tpfile1c.lavPaymentMethod = xaFile1C._File1C.lavPaymentMethod;
	tpfile1c.validityDurationType = xaFile1C._File1C.validityDurationType;
	tpfile1c.validityDuration = (xaFile1C._File1C.validityDuration_1 << 5) + xaFile1C._File1C.validityDuration;
	tpfile1c.checksum = xaFile1C._File1C.checksum;
#ifdef DEBUG_PRINT
	PRINTK("status %02x actionSn %02x activated %02x printed %02x validityStartdate %06x origin %03x des %03x transSN %04x samid %04x value %05x payment %02x durstiontyep %02x validduration %02x durationtype %02x\n",
		tpfile1c.productStatus, tpfile1c.actionSequenceNumber, tpfile1c.activated, tpfile1c.invoicePrinted, tpfile1c.validityStartDateTime, tpfile1c.validityOrigin, tpfile1c.validityDestination, tpfile1c.transactionSequenceNumber,
		tpfile1c.lavSamId, tpfile1c.lavValue, tpfile1c.lavPaymentMethod, tpfile1c.validityDurationType, tpfile1c.validityDuration);
#endif	
	return 0;
}

/*
function:read the file 1d
parameter:
*/

//20260826: employee-ticket productId whitelist for 1D fallback repair
//TODO: fill from issue data, e.g. {0x1234, 0x5678, 0}; {0} means repair disabled (safe default)
static const unsigned short emp_product_ids[] = {0x0000};

static char is_employee_ticket(unsigned short pid)
{
	int i;
	for(i = 0; emp_product_ids[i] != 0; i++)
	{
		if(emp_product_ids[i] == pid)
			return 1;
	}
	return 0;
}
char CPU_GetFiles1d(unsigned char *out_buf)
{
int ret, i;
unsigned char buf[40];
unsigned char cpubuf[80], Le ,full_duration,chCode;
unsigned short cpulen;
unsigned char startbcdtime[7], start_timebcd[7];
unsigned long lnsecondtime;
	
	//read file 1b - binary file
	memcpy(out_buf, "\xfd\x22", 2);
	memcpy(buf, "\x00\xb0\x9d\x00\x10", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\xfd\x23", 2);
#ifdef DEBUG_PRINT_EM
	PRINTK("read file 1d:len%d ", Le);
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xfd\x24", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memcpy(cpu_1d_data, cpubuf, 16);
#ifdef DEBUG_PRINT_EM
	PRINTK("file 1d len:%02x,%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
			cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3],	cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7], cpubuf[8], cpubuf[9], cpubuf[10], cpubuf[11], cpubuf[12], cpubuf[13], cpubuf[14], cpubuf[15]);
#endif
	memcpy(xaFile1C.buff, cpu_1d_data, 16);
	tpfile1d.productStatus = xaFile1C._File1C.productStatus;
	tpfile1d.actionSequenceNumber = xaFile1C._File1C.actionSequenceNumber;
	tpfile1d.activated = xaFile1C._File1C.activated;
	tpfile1d.invoicePrinted = xaFile1C._File1C.invoicePrinted;
	tpfile1d.validityStartDateTime = (xaFile1C._File1C.validityStartDateTime_1 << 16) + (xaFile1C._File1C.validityStartDateTime_2 << 8) + xaFile1C._File1C.validityStartDateTime;
	tpfile1d.validityOrigin = (xaFile1C._File1C.validityOrigin_1 << 4) + xaFile1C._File1C.validityOrigin;
	tpfile1d.validityDestination = (xaFile1C._File1C.validityDestination_1 << 8) + xaFile1C._File1C.validityDestination;
	tpfile1d.transactionSequenceNumber = (xaFile1C._File1C.transactionSequenceNumber_1 << 8) + xaFile1C._File1C.transactionSequenceNumber;
	tpfile1d.lavSamId = (xaFile1C._File1C.lavSamId_1 << 8) + xaFile1C._File1C.lavSamId;
	tpfile1d.lavValue = (xaFile1C._File1C.lavValue_1 << 9) + (xaFile1C._File1C.lavValue_2 << 1) + xaFile1C._File1C.lavValue;
	tpfile1d.lavPaymentMethod = xaFile1C._File1C.lavPaymentMethod;
	tpfile1d.validityDurationType = xaFile1C._File1C.validityDurationType;
	tpfile1d.validityDuration = (xaFile1C._File1C.validityDuration_1 << 5) + xaFile1C._File1C.validityDuration;
	tpfile1d.checksum = xaFile1C._File1C.checksum;

	/********************20260715 check employee-card validity-start field (20260826 keep fallback, hardened) ****************** */
	xa_MinuteTolocaltime(&startbcdtime[0], tpfile05.cardBaseDateTime, tpfile1d.validityStartDateTime, &lnsecondtime);
	if(tpfile1d.activated == 1 && (memcmp(tpCPU.time_bcd, startbcdtime, 4) < 0))
	{
		//20260826 keep-fallback hardening:
		//1) ticket gate: repair only employee period tickets (whitelist above), others log only
		if((tpfile05.productCategory != XA_FEETYPE_PERIOD) || (!is_employee_ticket(tpfile05.productId)))
		{
			PRINTK("1D-SUSPECT-NOT-EMPLOYEE logicID %08lx productId %04x category %02x\n",
				(unsigned long)ByteToLong(NULL, &cpu_05_data[4]),
				(unsigned int)tpfile05.productId, (unsigned int)tpfile05.productCategory);
		}
		else
		{
			//2) repair values: keep original hardcoded 0x03(month)/0x18(24 months);
			//   exact per-card values must come from issue records (see docs 05 section 3)
			tpfile1d.validityDurationType = 0x03;  //type: month
			tpfile1d.validityDuration = 0x18;		//24 months

			xaFile1C._File1C.validityDurationType = tpfile1d.validityDurationType ;

			full_duration = tpfile1d.validityDuration; 
			xaFile1C._File1C.validityDuration_1 = (full_duration >> 5) & 0x03; 
			xaFile1C._File1C.validityDuration = full_duration & 0x1F;          

			memset(start_timebcd, 0x00, 7);
			memcpy(start_timebcd, tpCPU.time_bcd, 7);
			xa_localtimeToMinute(start_timebcd, tpfile05.cardBaseDateTime, &tpfile1d.validityStartDateTime);
			xaFile1C._File1C.validityStartDateTime_1 = (tpfile1d.validityStartDateTime >> 16) & 0x3F;
			xaFile1C._File1C.validityStartDateTime_2 = (tpfile1d.validityStartDateTime >> 8) & 0xFF;
			xaFile1C._File1C.validityStartDateTime = tpfile1d.validityStartDateTime & 0xFF;

			memcpy(cpu_1d_data, xaFile1C.buff, 16);

			//3) write failure must NOT reject the transaction (old code returned chCode);
			//   log only and continue
			if(0 != (chCode = xa_update_file_1d(cpu_1d_data, out_buf)))
			{
				PRINTK("1D-FALLBACK-REPAIR-WRITE-FAIL logicID %08lx chCode %02x\n",
					(unsigned long)ByteToLong(NULL, &cpu_05_data[4]), (unsigned int)chCode);
			}
			else
			{
				//4) repair success log: card id / old-new start date / duration, for backend reconcile
				PRINTK("1D-FALLBACK-REPAIRED logicID %08lx oldStart %02x%02x%02x%02x newStart %02x%02x%02x%02x durType %02x dur %02x\n",
					(unsigned long)ByteToLong(NULL, &cpu_05_data[4]),
					startbcdtime[0], startbcdtime[1], startbcdtime[2], startbcdtime[3],
					tpCPU.time_bcd[0], tpCPU.time_bcd[1], tpCPU.time_bcd[2], tpCPU.time_bcd[3],
					tpfile1d.validityDurationType, tpfile1d.validityDuration);
			}
		}
	}
	/******************************************************* */
#ifdef DEBUG_PRINT_EM
	PRINTK("status %02x actionSn %02x activated %02x printed %02x validityStartdate %06x origin %03x des %03x transSN %04x samid %04x value %05x payment %02x durstiontype %02x validduration %02x\n",
		tpfile1d.productStatus, tpfile1d.actionSequenceNumber, tpfile1d.activated, tpfile1d.invoicePrinted, tpfile1d.validityStartDateTime, tpfile1d.validityOrigin, tpfile1d.validityDestination, tpfile1d.transactionSequenceNumber,
		tpfile1d.lavSamId, tpfile1d.lavValue, tpfile1d.lavPaymentMethod, tpfile1d.validityDurationType, tpfile1d.validityDuration);
#endif	
	return 0;
}

/*
function:read the file 1A
parameter:
*/
char CPU_GetFiles1A(char rec_num, unsigned char *out_buf)
{
int ret, i, j;
unsigned char buf[40], time_bcd[7];
unsigned char cpubuf[80], Le;
unsigned short adddate, cpulen;
unsigned long lngHisecond1;

	if(rec_num > 3)
		rec_num = 3;
	//read file 19 - variable file length
	memcpy(out_buf, "\xf0\x22", 2);
	//
	memset(out_buf, 0x00, rec_num * XA_CPU_1A_LEN);
	//read history
	memcpy(buf,"\x00\xb2\x00\x00\x17", 5);
	buf[3] = (0x1A << 3) | 0x04;
	buf[4] = Le = XA_CPU_1A_LEN;
	
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
		PRINTK("read 1A data:");
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
		adddate = ((cpubuf[6] << 8) + (cpubuf[7] & 0xF0)) >> 4;
		xa_daytodate(tpfile05.cardBaseDateTime, adddate, &lngHisecond1, &time_bcd[0]);
		cpubuf[6] = time_bcd[1];
		cpubuf[7] = time_bcd[2];
		cpubuf[11] = time_bcd[3];
		memcpy(&out_buf[(i - 1) * XA_CPU_1A_LEN], cpubuf, XA_CPU_1A_LEN);
		
	}
	
	return 0;
}

char CPU_TellSysCard(unsigned short tickettype)
{
int ret;

	
	if((ret = get_ticket_para(tickettype, &tpTicketDef)) != 0)
		return ret;
	
	return 0;
}
///*
//function:
//// 有效期类�????
//// 0: 对有效期不做检查；
//// 1: 从第一次消费使用后（票卡DateOfCardFirstTransaction）的一段可配置时间内（票卡的Duration）有效；
//      同时判断（票卡CSCEndDate，根据EOD的FixedEndDate获取�????
//// 2: 固定的起始时间A；POST当前的售票时间IssueDate的一段可配置时间内（EOD的Duration）有效；POST销售时写入票卡CSCStartDate和CSCEndDate
//// 3: 以销�????/充值后的一段时间有效；POST销售、充�????(票卡LastAddingValueDate)后的一段可配置时间内（EOD的Duration）有效；仅限于车票有效期使用�????
////	在POST销售，写入CSCStartDate和CSCEndDate,在POST充值时，更新CSCEndDate
//// 4: 固定的起始时间B；POST当前的售票时间IssueDate到某一个可配置时间点结束（EOD的FixedEndDate）；
//parameter:
//	durationmode:the DuratinMode in the cpu card.
//	>4:return 0,not expired date
//return :
//	11: not matching mode
//	0: mode match ok
//*/
//char CPU_ValidatePeriod(unsigned char durationmode, Date2_t shDays)
//{
//Date2_t	firstdate, durationdate, enddate;
//char chRejectCode;
//
//	chRejectCode = 0;
//	//
///*	switch(durationmode)
//	{
//	case 0:
//		tpCPU.startdate = tpCPU.enddate = 0x00; 
//		return 0;
//	case 1:
//		ByteToShort((short *)&firstdate, &cpu_19_data[36]);
//		enddate = tpTicketDef.FixedEndDate;
//		ByteToShort(&tpCPU.startdate, &cpu_06_data[7]);
//		if(firstdate != 0)
//		{
//			tpCPU.startdate = firstdate;
//			ByteToShort((short *)&durationdate, &cpu_06_data[11]);
//			firstdate += durationdate;
//			if(firstdate < enddate)
//				enddate = firstdate;
//		}
//		tpCPU.enddate = enddate;
//		//check with the current day
//		if(enddate < shDays)
//			chRejectCode = CE_EXPIREDDATE;
//		break;
//	case 2:
//	case 3:
//		ByteToShort((short *)&firstdate, &cpu_06_data[7]);
//		ByteToShort((short *)&enddate, &cpu_06_data[9]);
//		tpCPU.startdate = firstdate;
//		tpCPU.enddate = enddate;
//		if(tpTicketDef.FixedEndDate == 0)
//		{
//			if(enddate < shDays)
//				chRejectCode = CE_EXPIREDDATE;
//		}else
//		{
//			tpCPU.enddate = tpTicketDef.FixedEndDate;
//			if(tpTicketDef.FixedEndDate < shDays)
//				chRejectCode = CE_EXPIREDDATE;
//		}
//		break;
//	case 4:
//		ByteToShort((short *)&enddate, &cpu_06_data[17]);
//		ByteToShort((short *)&enddate, &cpu_06_data[9]);
//		tpCPU.startdate = firstdate;
//		tpCPU.enddate = enddate;
//		if(enddate < shDays)
//			chRejectCode = CE_EXPIREDDATE;
//		break;
//	case 5://need to read file 14
//		break;
//	default:
//		break;
//	}
//
//	//current station is set to the DATE mode
//	if(tpwaivermode.cur_sta_date)
//		return 0;
//	//other station is set to DATE mode or sensitive duaration and DATE mode in the ticket
//	if((tpwaivermode.oth_sta_date || tpwaivermode.sen_sta_date) && (cpu_19_data[12] == SZ_WAIVER_DATE))
//		return 0;
//*/
//	return chRejectCode;
//}
/*
function:calculate the start date and end date for sale command
*/
char CPU_CalPeriod(unsigned short shDays)
{
unsigned short	firstdate, durationdate, enddate;

/*	switch(tpTicketDef.DurationMode)
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
	
parameter:
	1�????
	2�????
	3�????
return :
%// 车票使用范围限制标志
%// 0: 无使用范围限�????
%// 1: 在限定的区域使用
%// 2: 在限定的区段使用
*/
char CPU_TravelsRights(char metrostatus, unsigned char *out_buf)
{
char chCode;
unsigned int	i, j;
unsigned short	curstation, secstation1, secstation2;

/*	//
	if(Eod01 == NULL)
		return CE_EOD_FILE;
	//
	ByteToShort((short *)&curstation, tpCPU.curstation);
	switch(cpu_08_data[9])
	{
	case 0://forbidden
		return ERR_USED_FORBIDED_ZONE;
	case 1://only allowed station and FREE entry/exit
		if(memcmp(tpCPU.curstation, &cpu_08_data[12], 2) == 0)
			cpu_19_data[15] = metrostatus;
		else
			return ERR_USED_FORBIDED_ZONE;
		break;
	case 2://only section station and MUST entry/exit
		if((memcmp(tpCPU.curstation, &cpu_08_data[15], 2) != 0) && (memcmp(tpCPU.curstation, &cpu_08_data[17], 2) != 0))
			return ERR_USED_FORBIDED_ZONE;
		break;
	case 3://only zone station and must entry/exit
		for(i = 0; i < Eod01->Zone.Zone_len; i++)
		{//first check the zone id
			if(Eod01->Zone.Zone_val[i].ZoneID == cpu_08_data[14])
			{
				for(j = 0; j < Eod01->Zone.Zone_val[i].StationID.StationID_len; j++)
				{//then check whether the current station is in the zone station or not
					if(curstation == Eod01->Zone.Zone_val[i].StationID.StationID_val[j])
						break;
				}
				//the current station not in the zone station
				if(j >= Eod01->Zone.Zone_val[i].StationID.StationID_len)
					return ERR_USED_FORBIDED_ZONE;
				//the zone id disable
				if(Eod01->Zone.Zone_val[i].EnableFlag == 0)
					return ERR_USED_FORBIDED_ZONE;
				break;
			}
			
		}
		//don't exist the zone id in the card
		if(i >= Eod01->Zone.Zone_len)
			return CE_EOD_FILE;
		//
#ifdef	DEBUG_PRINT
		PRINTK("index is %02x zone len is %d\n", i , Eod01->Zone.Zone_len);
#endif
		break;
	case 5://free entry/exit
		cpu_19_data[15] = metrostatus;
		break;
	default://not defined 
		break;
	}
*/	return 0;
}

/************************************
CPU卡进�????
************************************/
char xa_CPU_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100];
unsigned char cpubuf[100], cpulen, Le;
unsigned char cnt;
unsigned char chCode, chRejectCode;
unsigned long lngLogicID;

	*out_len = 4;
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
		PRINTK("rollback %02x old phyical id", ch_sz_cpu_rollback);
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id[i]);
		PRINTK("  new phyical id ");
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id_bak[i]);
		PRINTK("\n");
#endif
	if((ch_sz_cpu_rollback != 0) && (memcmp(ch_cpu20_phyical_id, ch_cpu20_phyical_id_bak, 8) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_cpu_rollback);
#endif
//		if((chRejectCode = CPU_gettransprove(0x09, sfi_bak, xa_metro_psam_index, cpu_05_data, out_buf)) == 0)
//		{
//			blncpuRollback = 1;
//			goto label_sz_rollback_main;
//		}else if(chRejectCode == CE_READ)
//			return CE_READ;
	}
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[7]);
	//
	get_degrade_mode(tpCPU.curstation);
	//test mode
	memcpy(out_buf, "\x32\x05", 2);
	if((chCode = CPU_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//card status
	memcpy(out_buf, "\x32\x06", 2);
	//select file
	if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
		return chCode;
	memcpy(sfi_bak, "\x3f\x01", 2);
	//extern auth
	memcpy(out_buf, "\x32\x0e", 2);
	if((chCode = CPU_externauth(0, xa_metro_psam_index, cpu_05_data, out_buf)) != 0)
		return chCode;
	//read blance and file 15
	memcpy(out_buf, "\x32\x10", 2);
	if((chCode = CPU_GetFiles15(out_buf)) != 0)
	{
		return chCode;
	}
	//not locked status
	if(tpfile15.cardStatus != 1)
		return CE_CARDSTATUS;
	//card logic black list
	lngLogicID = ByteToLong(NULL, &cpu_05_data[4]);
	//20260826 P0-4: blnBlock 0 -> 0xff. A blacklist hit still returns CE_BLACKLIST(0x01)
	//(gate behavior unchanged), but the TP no longer writes file15/17 on the card and no
	//longer triggers the in-card Applet-linked lock - one false hit can no longer destroy a
	//card (same root as the 0x03/0x08 gates). If business confirms real blacklist cards must
	//be locked on card at entry, restore the write path only with an explicit authorization
	//bit agreed with the backend (see docs 04 section 4.4).
	if(0 != (chCode = check_metro_Black_Lock(lngLogicID, 0xff, cmd_buf, out_buf, out_len)))
		return chCode;

	tpCPU.tranamount = 0;
label_sz_rollback_main:
	memcpy(out_buf, "\x32\x11", 2);
	switch(tpfile05.productCategory)
	{
	case XA_FEETYPE_VALUE:			//value
	case 0:
		//ticket definition
		memcpy(out_buf, "\x32\x07", 2);
		if((chCode = CPU_TellSysCard(tpfile05.purseId)) != 0)
		{
			return chCode;
		}
		//
		tpTxnProductPurseEntry.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
		tpTxnProductPurseEntry.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
		tpTxnProductPurseEntry.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]);
		tpTxnProductPurseEntry.SysComHdr_val.udType = toMoto(3);
		tpTxnProductPurseEntry.SysComHdr_val.udSubtype = toMoto(88);
		//
		tpTxnProductPurseEntry.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductPurseEntry.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
		tpTxnProductPurseEntry.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
		tpTxnProductPurseEntry.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);
		//
		tpTxnProductPurseEntry.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
		tpTxnProductPurseEntry.SysProductCom_val.productIssuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductPurseEntry.SysProductCom_val.productSerialNumber = toMoto(tpfile05.productSerialNumber);
		tpTxnProductPurseEntry.SysProductCom_val.productType = toMoto(tpfile05.purseId);
		//
		tpTxnProductPurseEntry.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
		tpTxnProductPurseEntry.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideEntry.SysComHdr_val.deviceLocation;
		tpTxnProductPurseEntry.DevUdJourneyHdr_val.tripOriginLocation = 0;
		tpTxnProductPurseEntry.DevUdJourneyHdr_val.tripPreviousLocation = 0;

		memset(&tpTxnProductPurseEntry.DevUdPurseLavHdr_val.lavSamId, 0x00, sizeof(DevUdPurseLavHdr_t));
		return xa_CPU_entry_dis(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_TIMES:			//rides
		//ticket definition
		memcpy(out_buf, "\x32\x07", 2);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		//
		tpTxnProductMultirideEntry.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
		tpTxnProductMultirideEntry.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
		tpTxnProductMultirideEntry.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]);//toMoto(*((long *)(&cmd_buf[13])));
		tpTxnProductMultirideEntry.SysComHdr_val.udType = toMoto(3);
		tpTxnProductMultirideEntry.SysComHdr_val.udSubtype = toMoto(90);
		//
		tpTxnProductMultirideEntry.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductMultirideEntry.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
		tpTxnProductMultirideEntry.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
		tpTxnProductMultirideEntry.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);
		//
		tpTxnProductMultirideEntry.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
		tpTxnProductMultirideEntry.SysProductCom_val.productIssuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductMultirideEntry.SysProductCom_val.productSerialNumber = toMoto(tpfile05.productSerialNumber);
		tpTxnProductMultirideEntry.SysProductCom_val.productType = toMoto(tpfile05.productId);
		//
		tpTxnProductMultirideEntry.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
		tpTxnProductMultirideEntry.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideEntry.SysComHdr_val.deviceLocation;
		tpTxnProductMultirideEntry.DevUdJourneyHdr_val.tripOriginLocation = 0;
		tpTxnProductMultirideEntry.DevUdJourneyHdr_val.tripPreviousLocation = 0;
		//
		tpTxnProductMultirideEntry.DevUdMultirideCommonHdr_val.numRides = 0;
		tpTxnProductMultirideEntry.DevUdMultirideCommonHdr_val.remainingRides = toMoto(tpCPU.balance);
		
		memset(&tpTxnProductMultirideEntry.DevUdMultirideLavHdr_val.lavSamId, 0x00, sizeof(DevUdMultirideLavHdr_t));
		return xa_CPU_entry_cnt(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_PERIOD:		//period
		//ticket definition
		memcpy(out_buf, "\x32\x07", 2);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		//
		tpTxnProductPassEntry.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
		tpTxnProductPassEntry.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
		tpTxnProductPassEntry.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]);
		tpTxnProductPassEntry.SysComHdr_val.udType = toMoto(3);
		tpTxnProductPassEntry.SysComHdr_val.udSubtype = toMoto(89);
		//
		tpTxnProductPassEntry.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductPassEntry.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
		tpTxnProductPassEntry.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
		tpTxnProductPassEntry.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);
		//
		tpTxnProductPassEntry.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
		tpTxnProductPassEntry.SysProductCom_val.productIssuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductPassEntry.SysProductCom_val.productSerialNumber = toMoto(tpfile05.productSerialNumber);
		tpTxnProductPassEntry.SysProductCom_val.productType = toMoto(tpfile05.productId);
		//
		tpTxnProductPassEntry.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
		tpTxnProductPassEntry.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideEntry.SysComHdr_val.deviceLocation;
		tpTxnProductPassEntry.DevUdJourneyHdr_val.tripOriginLocation = 0;
		tpTxnProductPassEntry.DevUdJourneyHdr_val.tripPreviousLocation = 0;
		
		memset(&tpTxnProductPassEntry.DevUdPassLavHdr_val.lavSamId, 0x00, sizeof(DevUdPassLavHdr_t));
		return xa_CPU_entry_em(cmd_buf, out_buf, out_len);
	default:
		return CE_NON_FEETYPE;
	}

}

/************************************
CPU计程票进�????
************************************/
char xa_CPU_entry_dis(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i ;
unsigned char buf[50], start_timebcd[7], end_timebcd[7], entry_timebcd[7];
unsigned char cpubuf[80], cpulen, last_timebcd[7];
unsigned char status;
unsigned long time1, time2, lngstation, lngtxnDatetime;
unsigned short tempdate, shTicketType, cnt, cnt2;
char chCode;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;

	//read the value
	memcpy(out_buf, "\x32\x20", 2);
	if((chCode = CPU_GetFiles1b(out_buf)) != 0)
		return chCode;
	//actived
	xa_daytodate(tpfile05.cardBaseDateTime, tpfile1b.validityStartDate, &lngHisecond1, &start_timebcd[0]);
	if(tpfile1b.activated == 0)
	{
		if(0 != (chCode = xa_CPU_active_dis(start_timebcd, out_buf, out_len)))
			return chCode;
	}
	xa_DurationTolocaltime(lngHisecond1, tpfile1b.validityDurationType, tpfile1b.validityDuration, &end_timebcd[0]);
	lngLosecond2 = timestr2long(&end_timebcd[1]) - 1;
	long2timestr(lngLosecond2, &end_timebcd[0]);
	// validate
	memcpy(out_buf, "\x32\x23", 2);
	xa_MinuteTolocaltime(&last_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
	get_degrade_sensitive_mode(NULL, last_timebcd);
	if((chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, last_timebcd, tpfile1b.validityDurationType)) != 0)
	{
		return chCode;
	}
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit) 
		&& (memcmp(tpCPU.time_bcd, last_timebcd, 4) != 0))
	{
		tpfile15.journeyStatus = 0;
	}
	//balance < max remaining value
	if(tpCPU.balance <= 0)
	{
		return CE_ENOUGH_BALANCE;
	}
	tpTxnProductPurseEntry.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpCPU.balance);
	//metro status
	memcpy(out_buf, "\x32\x25", 2);
	if((tpfile15.journeyStatus == 0x01) || (tpfile15.journeyStatus == 0x04))
	{
		xa_MinuteTolocaltime(&entry_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
		if(lngHisecond1 > tpCPU.lowsecond)
			chCode = CE_FREE_UPDATE_ENTRY;
		else if((tpCPU.lowsecond - lngHisecond1) <= 20 * 60)
			chCode = CE_FREE_UPDATE_ENTRY;
		else
			chCode = CE_FEE_UPDATE_ENTRY;
		//
		if(ch_sz_cpu_rollback == MCPU_ROLL_CHECK_15)
		{
			goto label_dis_update_17;
		}
		goto label_refuse_to_entry;
	}
/*	
	//
	memcpy(out_buf, "\x46\x10", 2);
	if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6 , out_buf)) != 0)
		return chCode;

	//
	memcpy(out_buf, "\x46\x11", 2);
	memset(buf, 0x00, 8);
	memcpy(buf, &cpu_05_data[4], 4);
	if(0 != (chCode = CPU_init_for_purchase(1, tpCPU.tranamount, ch_cpu20_psam_id, buf, NULL, ch_cpu_mac_data)))
	{
		return chCode;
	}
	//更改消费记录文件
	memcpy(out_buf, "\x46\xb2", 2);
	ch_sz_cpu_rollback = SZ_CPU_CAPP_1;
	if(0 != (chCode = CPU_debit_for_purchase(&ch_sz_cpu_rollback, sz_CPU20_ee_write, out_buf)))
	{
#ifdef DEBUG_PRINT
		PRINTK("debit failure %02x\n", ch_sz_cpu_rollback);
#endif		
		return CE_WRITE;
	}

#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif
*/
	tpfile15.journeyStatus = 1;
	xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
	lngstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
		return chCode;
	memcpy(out_buf, "\x32\x2B", 2);
	xaFile15._File15.origin_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.origin = tpfile15.lastLocation & 0xF;
	xaFile15._File15.destination_1 = 0xF;
	xaFile15._File15.destination = 0xFF;
	xaFile15._File15.transfersTaken = 0;
	xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xF;
	xaFile15._File15.lastDateTime = 0;
	xaFile15._File15.totalPurchaseValue_1 = 0;
	xaFile15._File15.totalPurchaseValue = 0;
	xaFile15._File15.journeyStatus = tpfile15.journeyStatus;
	xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
	xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
	xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
	memcpy(cpu_15_data, xaFile15.buff, 13);
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
	{
		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write(tpTxnProductPurseEntry.AFCHead_val.operatorid, sizeof(TxnProductPurseUseOnEntry_t));
		return CE_CONSUME_MOVED;
	}

label_dis_update_17:
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 14;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = (tpCPU.balance >> 12) & 0x1F;
	xaFile17._File17.remainingValue_2 = (tpCPU.balance >> 4) & 0xFF;
	xaFile17._File17.remainingValue = tpCPU.balance & 0xF;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	//
	tpTxnProductPurseEntry.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductPurseUseOnEntry_t);
	sh_mac_len = cnt - 22;
	memcpy(ch_mac_data, &tpTxnProductPurseEntry.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPurseEntry.SysComHdr_val.reservedField, sh_mac_len - 40 - 48);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[sh_mac_len - 48], &tpTxnProductPurseEntry.DevUdPurseLavHdr_val.lavSamId, 48);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPurseEntry.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPurseEntry.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;

label_sz_rollback_1:
	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.purseId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_VALUE;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductPurseEntry.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	//calculate the TAC
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);

	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}
	return CE_OK;
label_refuse_to_entry:
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	*out_len = 2;
	return chCode;
}
/********************************************
CPU计次票进�????
********************************************/
char xa_CPU_entry_cnt(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret;
char chCode;
unsigned char buf[50], start_timebcd[7], end_timebcd[7], entry_timebcd[7];
unsigned char cpubuf[80], cpulen, last_timebcd[7];
unsigned char status;
unsigned long time1,time2, lngstation;
unsigned short tempdate, shTicketType, cnt, cnt2;
long lngHisecond1, lngLosecond1, lngHisecond2, lngtxnDatetime;	

	memset(cpu_1c_data,0x00,16);//20230714
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the multiride
	memcpy(out_buf, "\x32\x20", 2);
	if((chCode = CPU_GetFiles1c(out_buf)) != 0)
		return chCode;
	#ifdef TESTLOG2022
		TestLog(tpCPU.time_bcd,"CPU_entry_cnt_GetFiles1c:",7);	
		TestLog(cpu_1c_data,"FF:",16);
		TestLog(tpCPU.time_bcd,"CPU_entry_cnt_GetFiles15:",7);	
		TestLog(cpu_15_data,"FE:",16);
    #endif
	tpTxnProductMultirideEntry.SysProductCom_val.Ptsn = toMoto(tpfile1c.transactionSequenceNumber);
	//actived
	xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1c.validityStartDateTime, &lngHisecond1);
	tpTxnProductMultirideEntry.DevUdProductValidity_val.vStartDateTime = toMoto(lngHisecond1 + TIME2000);
	if(tpfile1c.activated == 0)
	{
		if (0 != (chCode = xa_CPU_active_cnt(start_timebcd, out_buf, out_len))) 
		{
        	#ifdef TESTLOG2022
			TestLog(start_timebcd,"CPU_entry_active_cnt fail",7);
			TestLog(start_timebcd,"start_timebcd:",7);
   			#endif
			return chCode;
		}
		
		tpTxnProductMultirideEntry.DevUdProductValidity_val.vStartDateTime = toMoto(tpCPU.lowsecond + TIME2000);
	}
	xa_DurationTolocaltime(lngHisecond1, tpfile1c.validityDurationType, tpfile1c.validityDuration, &end_timebcd[0]);
	lngHisecond2 = timestr2long(&end_timebcd[1]) - 1;
	long2timestr(lngHisecond2, &end_timebcd[0]);
	
	xa_MinuteTolocaltime(&last_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
	tpTxnProductMultirideEntry.DevUdProductValidity_val.vEndDateTime = toMoto(lngHisecond1 + TIME2000);
	tpTxnProductMultirideEntry.DevUdProductValidity_val.vDuration = toMoto(tpfile1c.validityDurationType);
	//check the date
	memcpy(out_buf, "\x32\x23", 2);
	get_degrade_sensitive_mode(NULL, last_timebcd);

	if(0 != (chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, last_timebcd, tpfile1c.validityDurationType)))
	{
#ifdef TESTLOG2022
		TestLog(tpCPU.time_bcd,"CPU_entry_cnt_TellDate fail",7);
		TestLog(tpCPU.time_bcd,"tpCPU.time_bcd:",7);
		TestLog(start_timebcd,"start_timebcd:",7);
		TestLog(end_timebcd,"end_timebcd:",7);
		TestLog(last_timebcd,"last_timebcd:",7);
#endif
		return chCode;
	}
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit) 
		&& (memcmp(tpCPU.time_bcd, last_timebcd, 4) != 0))
	{
		tpfile15.journeyStatus = 0;
	}
	//
	tpTxnProductMultirideEntry.SysCardCom_val.cardActionSequenceNumber = toMoto(tpfile1c.actionSequenceNumber);
	tpTxnProductMultirideEntry.SysProductCom_val.productActionSequenceNumber = toMoto(tpfile1c.actionSequenceNumber);
	tpTxnProductMultirideEntry.SysProductCom_val.invoicePrinted = toMoto(tpfile1c.invoicePrinted);
	//balance > 0
	memcpy(out_buf, "\x32\x24", 2);
	if(tpCPU.balance <= 0)
	{
		return CE_ENOUGH_BALANCE;
	}
	//metro status
	memcpy(out_buf, "\x32\x25", 2);
	if(tpTicketDef.IgnoreEntryExitSequence == 0)
	{
		if((tpfile15.journeyStatus == 0x01) || (tpfile15.journeyStatus == 0x04))
		{
			xa_MinuteTolocaltime(&entry_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
			if(lngHisecond1 > tpCPU.lowsecond)
				chCode = CE_FREE_UPDATE_ENTRY;
			else if((tpCPU.lowsecond - lngHisecond1) <= 20 * 60)
				chCode = CE_FREE_UPDATE_ENTRY;
			else
				chCode = CE_FEE_UPDATE_ENTRY;
			//
			if(ch_sz_cpu_rollback == MCPU_ROLL_CHECK_15)
			{
				goto label_cnt_update_17;
			}
			goto label_refuse_to_entry;
		}
	}
	//check the validity orgin
	memcpy(out_buf, "\x32\x26", 2);
	if(0 != (chCode = card_to_location(tpfile1c.validityOrigin, &lngstation)))
	{

		return chCode;
	}
		
	tpTxnProductMultirideEntry.DevUdProductValidity_val.vOrigin = toMoto(lngstation);
	if(0 != (chCode = xa_ValidateArea(tpCmdInit.curstation, lngstation)))
	{

		return chCode;
	}
	card_to_location(tpfile1c.validityDestination, &lngstation);
	tpTxnProductMultirideEntry.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	/*
	//
	memcpy(out_buf, "\x32\x27", 2);
	if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6 , out_buf)) != 0)
		return chCode;
	//purchase
	memcpy(out_buf, "\x32\x28", 2);
	memset(buf, 0x00, 8);
	memcpy(buf, &cpu_05_data[4], 4);
	if(0 != (chCode = CPU_init_for_purchase(1, tpCPU.tranamount, ch_cpu20_psam_id, buf, NULL, ch_cpu_mac_data)))
	{
		return chCode;
	}
	memcpy(out_buf, "\x32\x29", 2);
	ch_sz_cpu_rollback = SZ_CPU_CAPP_1;
	if(0 != (chCode = CPU_debit_for_purchase(&ch_sz_cpu_rollback, sz_CPU20_ee_write, out_buf)))
	{
		return chCode;
	}
	*/
	memcpy(out_buf, "\x32\x2A", 2);
	tpfile15.journeyStatus = 1;
	xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
	lngstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
	{
			#ifdef TESTLOG2022

   			#endif
		return chCode;
	}
		
	memcpy(out_buf, "\x32\x2B", 2);
	xaFile15._File15.origin_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.origin = tpfile15.lastLocation & 0xF;
	xaFile15._File15.destination_1 = 0xF;
	xaFile15._File15.destination = 0xFF;
	xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xF;
	xaFile15._File15.lastDateTime = 0;
	xaFile15._File15.totalPurchaseValue_1 = 0;
	xaFile15._File15.totalPurchaseValue = 0;
	xaFile15._File15.journeyStatus = tpfile15.journeyStatus;
	xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
	xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
	xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
	memcpy(cpu_15_data, xaFile15.buff, 13);
#ifdef TESTLOG2022
		TestLog(cpu_15_data,"CPU_entry_cnt writeback 15:",16);

#endif
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
	{

		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write(tpTxnProductMultirideEntry.AFCHead_val.operatorid, sizeof(TxnProductMultirideUseOnEntry_t));
		return CE_CONSUME_MOVED;
	}

label_cnt_update_17:
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 14;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = (tpCPU.balance >> 12) & 0x1F;
	xaFile17._File17.remainingValue_2 = (tpCPU.balance >> 4) & 0xFF;
	xaFile17._File17.remainingValue = tpCPU.balance & 0xF;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	//
	tpTxnProductMultirideEntry.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductMultirideUseOnEntry_t);
	sh_mac_len = cnt - 22;
	memcpy(ch_mac_data, &tpTxnProductMultirideEntry.SysComHdr_val.formatVersion, 40);
	memcpy(&ch_mac_data[40], &tpTxnProductMultirideEntry.SysComHdr_val.reservedField, sh_mac_len - 40 - 4);
	sh_mac_len -= 4;
	//bakup the TXN
	g_sha1txnsn = tpTxnProductMultirideEntry.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductMultirideEntry.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;

label_sz_rollback_1:
	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.productId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_TIMES;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductMultirideEntry.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	//calculate the TAC
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);

	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}

	return CE_OK;
label_refuse_to_entry:
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	*out_len = 2;

	return chCode;
}

/******************************************************
员工票进�????
******************************************************/
char xa_CPU_entry_em(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
char chCode, endChCode;
unsigned char buf[50], factor[20], des[80], deslen, last_timebcd[7];
unsigned char cpubuf[80], cpulen, start_timebcd[7], end_timebcd[7];
unsigned short cnt, cnt2, allowedusedtimes, hadusedtimes, shTicketType;
unsigned long lngstation, lngtxnDatetime, lngHisecond1;
long 		  beforeStartTransTime;
unsigned char out_buffEnd[1024], cpu_1d_data_end[16];

	memset(cpu_1d_data,0x00,16);//20230714 
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the priod
	memcpy(out_buf, "\x32\x40", 2);
	if((chCode = CPU_GetFiles1d(out_buf)) != 0)
		return chCode;
#ifdef TESTLOG2022
	TestLog(tpCPU.time_bcd,"CPU_entry_em_GetFiles1d:",7);	
	TestLog(cpu_1d_data,"FF",16);
	TestLog(tpCPU.time_bcd,"CPU_entry_em_GetFiles15:",7);	
	TestLog(cpu_15_data,"FE:",16);
#endif

	//20260826 P1-2: baseline taken after CPU_GetFiles1d read (0715 fallback may have repaired first); recovery rolls back to this value
	beforeStartTransTime = tpfile1d.validityStartDateTime;

	tpTxnProductPassEntry.SysProductCom_val.Ptsn = toMoto(tpfile1d.transactionSequenceNumber);
	//actived
	xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1d.validityStartDateTime, &lngHisecond1);
	if(tpfile1d.activated == 0)
	{
		if(0 != (chCode = xa_CPU_active_em(start_timebcd, out_buf, out_len)))
		{
			#ifdef TESTLOG2022
			TestLog(start_timebcd,"CPU_entry_active_em fail",7);
			TestLog(start_timebcd,"start_timebcd:",7);
			#endif
			return chCode;
		}	
	}
	xa_DurationTolocaltime(lngHisecond1, tpfile1d.validityDurationType, tpfile1d.validityDuration, &end_timebcd[0]);
	lngtxnDatetime = timestr2long(&end_timebcd[1]) - 1;
	long2timestr(lngtxnDatetime, &end_timebcd[0]);
	//check the date
	xa_MinuteTolocaltime(&last_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
	long2timestr(lngHisecond1, last_timebcd);
	get_degrade_sensitive_mode(NULL, last_timebcd);
	if(0 != (chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, last_timebcd, tpfile1d.validityDurationType)))
	{
#ifdef TESTLOG2022
		TestLog(tpCPU.time_bcd,"CPU_entry_em_TellDate fail",7);
		TestLog(tpCPU.time_bcd,"tpCPU.time_bcd:",7);
		TestLog(start_timebcd,"start_timebcd:",7);
		TestLog(end_timebcd,"end_timebcd:",7);
		TestLog(last_timebcd,"last_timebcd:",7);
#endif
		return chCode;
	}
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit) 
		&& (memcmp(tpCPU.time_bcd, last_timebcd, 4) != 0))
	{
		tpfile15.journeyStatus = 0;
	}
	tpTxnProductPassEntry.SysProductCom_val.productActionSequenceNumber = toMoto(tpfile1d.actionSequenceNumber);
	tpTxnProductPassEntry.SysProductCom_val.invoicePrinted = toMoto(tpfile1d.invoicePrinted);
	
	tpTxnProductPassEntry.passEndDateTime = toMoto(timestr2long(&end_timebcd[1]) + TIME2000);
	//
	tpTxnProductPassEntry.DevUdProductValidity_val.vStartDateTime = toMoto(timestr2long(&start_timebcd[1]) + TIME2000);
	tpTxnProductPassEntry.DevUdProductValidity_val.vEndDateTime = tpTxnProductPassEntry.passEndDateTime;
	tpTxnProductPassEntry.DevUdProductValidity_val.vDuration = toMoto(((tpfile1d.validityDurationType & 0xf) << 12) + tpfile1d.validityDuration);
	card_to_location(tpfile1d.validityOrigin, &lngstation);
	tpTxnProductPassEntry.DevUdProductValidity_val.vOrigin = toMoto(lngstation);
	card_to_location(tpfile1d.validityDestination, &lngstation);
	tpTxnProductPassEntry.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	//
	tpTxnProductPassEntry.DevUdPassLavHdr_val.lavSamId = toMoto(tpfile1d.lavSamId);
	tpTxnProductPassEntry.DevUdPassLavHdr_val.lavPariticipantId = 0;
	tpTxnProductPassEntry.DevUdPassLavHdr_val.lavDate = 0;
	tpTxnProductPassEntry.DevUdPassLavHdr_val.lavTxnValue = toMoto(tpfile1d.lavValue);
	tpTxnProductPassEntry.DevUdPassLavHdr_val.lavPassExpiryDateTime = 0;
	tpTxnProductPassEntry.DevUdPassLavHdr_val.lavPtsn = 0;
	tpTxnProductPassEntry.DevUdPassLavHdr_val.lavMethodOfPayment = toMoto(tpfile1d.lavPaymentMethod);
	tpTxnProductPassEntry.DevUdPassLavHdr_val.dataIsValid = 0;
	tpTxnProductPassEntry.DevUdPassLavHdr_val.invoicePrinted = toMoto(tpfile1d.invoicePrinted);
	
	tpTxnProductPassEntry.startOfJourney = 0;
	tpTxnProductPassEntry.firstUseActivation = toMoto(tpfile1d.activated);
	//check the metro status
	memcpy(out_buf, "\x32\x4A", 2);
	if(tpTicketDef.IgnoreEntryExitSequence == 0)
	{//MUST check the entry/exit status
		if((tpfile15.journeyStatus == 0x01) || (tpfile15.journeyStatus == 0x04))
		{//in entry status
			chCode = CE_FREE_UPDATE_ENTRY;
			if(ch_sz_cpu_rollback == MCPU_ROLL_CHECK_15)
			{
				goto label_em_update_17;
			}
			goto label_refuse_to_entry;
		}
	}
	//check the validity origin
	if(0 != (chCode = card_to_location(tpfile1d.validityOrigin, &lngstation)))
	{
		return chCode;
	}
		
	if(0 != (chCode = xa_ValidateArea(tpCmdInit.curstation, lngstation)))
	{

		return chCode;
	}
		
	//according to the founder period card not purse only update the file 15
	memcpy(out_buf, "\x32\x4B", 2);
	lngstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
	{

		return chCode;
	}
		
	//
	memcpy(out_buf, "\x32\x4C", 2);
	tpfile15.journeyStatus = 1;
	xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
	xaFile15._File15.origin_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.origin = tpfile15.lastLocation & 0xF;
	xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xF;
	xaFile15._File15.journeyStatus = tpfile15.journeyStatus;
	xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
	xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
	xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
	xaFile15._File15.lastDateTime = 0;
	xaFile15._File15.totalPurchaseValue_1 = 0;
	xaFile15._File15.totalPurchaseValue = 0;
	memcpy(cpu_15_data, xaFile15.buff, 13);
#ifdef TESTLOG2022
		TestLog(cpu_15_data,"CPU_entry_em writeback 15:",16);

#endif	
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
	{

		
		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write(tpTxnProductPassEntry.AFCHead_val.operatorid, sizeof(TxnProductPassUseOnEntry_t));
		return CE_CONSUME_MOVED;
	}

label_em_update_17:
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = 3;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 14;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = 0;
	xaFile17._File17.remainingValue_2 = 0;
	xaFile17._File17.remainingValue = 0;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	//mac
	tpTxnProductPassEntry.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductPassUseOnEntry_t);
	sh_mac_len = cnt - 22;
	memcpy(ch_mac_data, &tpTxnProductPassEntry.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPassEntry.SysComHdr_val.reservedField, sh_mac_len - 40);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPassEntry.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPassEntry.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;

label_sz_rollback_1:
	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.productId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_PERIOD;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductPassEntry.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	//calculate the TAC
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);

	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}

	memset(cpu_1d_data,0x00,16);//20230714 
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the priod
	memcpy(out_buffEnd, "\x32\x4E", 2);
	if((endChCode = CPU_GetFiles1d(out_buffEnd)) == 0){
		if(beforeStartTransTime != tpfile1d.validityStartDateTime){
			xaFile1C._File1C.validityStartDateTime_1 = (beforeStartTransTime >> 16) & 0x3F;
			xaFile1C._File1C.validityStartDateTime_2 = (beforeStartTransTime >> 8) & 0xFF;
			xaFile1C._File1C.validityStartDateTime = beforeStartTransTime & 0xFF;
			memset(cpu_1d_data_end, 0x00, 16);
			memcpy(cpu_1d_data_end, xaFile1C.buff, 16);
			endChCode = xa_update_file_1d(cpu_1d_data_end, out_buffEnd);
		}
	}

	return CE_OK;
label_refuse_to_entry:
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	*out_len = 2;

	memset(cpu_1d_data,0x00,16);//20230714 
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the priod
	memcpy(out_buffEnd, "\x32\x4E", 2);
	if((endChCode = CPU_GetFiles1d(out_buffEnd)) == 0){
		if(beforeStartTransTime != tpfile1d.validityStartDateTime){
			xaFile1C._File1C.validityStartDateTime_1 = (beforeStartTransTime >> 16) & 0x3F;
			xaFile1C._File1C.validityStartDateTime_2 = (beforeStartTransTime >> 8) & 0xFF;
			xaFile1C._File1C.validityStartDateTime = beforeStartTransTime & 0xFF;
			memset(cpu_1d_data_end, 0x00, 16);
			memcpy(cpu_1d_data_end, xaFile1C.buff, 16);
			endChCode = xa_update_file_1d(cpu_1d_data_end, out_buffEnd);
		}
	}

	return chCode;
}

/************************************
function:CPU exit
************************************/
char xa_CPU_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100];
unsigned char cpubuf[100], cpulen;
unsigned char cnt, chCode, chRejectCode;

	*out_len = 4;
#ifdef DEBUG_PRINT
		PRINTK("rollback %02x old phyical id ", ch_sz_cpu_rollback);
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id[i]);
		PRINTK("  new phyical id ");
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id_bak[i]);
		PRINTK("\n");
#endif
/*
	if((ch_sz_cpu_rollback == MCPU_ROLL_AFTER_ED) && (memcmp(ch_cpu20_phyical_id, ch_cpu20_phyical_id_bak, 8) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_cpu_rollback);
#endif
		if((chRejectCode = CPU_gettransprove(0x09, sfi_bak, xa_metro_psam_index, cpu_05_data, out_buf)) == 0)
		{
			blncpuRollback = 1;
			goto label_sz_rollback_main;
		}else if(chRejectCode == CE_READ)
			return CE_READ;
	}
*/
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[7]);
	//
	get_degrade_mode(tpCPU.curstation);
	//read file 05 and 06--had read in the polling card function
	memcpy(out_buf, "\x53\x33\x00\x00", 4);
	//if((chCode = CPU_GetFiles05(out_buf)) != 0)
	//	return chCode;
	//test mode
	memcpy(out_buf, "\x53\x33\x00\x01", 4);
	if((chCode = CPU_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//card status
	memcpy(out_buf, "\x53\x33\x47\x09", 4);
	//select file
	if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
		return chCode;
	memcpy(sfi_bak, "\x3f\x01", 2);
	//extern auth
	memcpy(out_buf, "\x53\x33\x48\x0e", 4);
	if((chCode = CPU_externauth(0, xa_metro_psam_index, cpu_05_data, out_buf)) != 0)
		return chCode;
	//read blance and file 15
	if((chCode = CPU_GetFiles15(out_buf)) != 0)
	{
		return chCode;
	}
	//not locked status
	if(tpfile15.cardStatus != 1)
		return CE_CARDSTATUS;
	
label_sz_rollback_main:
	memcpy(out_buf, "\x53\x33\x01\x07", 4);
	switch(tpfile05.productCategory)
	{
	case XA_FEETYPE_VALUE:
	case 0:
		//ticket definition
		memcpy(out_buf, "\x53\x33\x47\x0a", 4);
		if((chCode = CPU_TellSysCard(tpfile05.purseId)) != 0)
		{
			return chCode;
		}
		//
		tpTxnProductPurseExit.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
		tpTxnProductPurseExit.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
		tpTxnProductPurseExit.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]); //toMoto(*(long *)&cmd_buf[13]);
		tpTxnProductPurseExit.SysComHdr_val.udType = toMoto(3);
		tpTxnProductPurseExit.SysComHdr_val.udSubtype = toMoto(91);
		//
		tpTxnProductPurseExit.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductPurseExit.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
		tpTxnProductPurseExit.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
		tpTxnProductPurseExit.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);
		
		tpTxnProductPurseExit.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
		
		tpTxnProductPurseExit.SysProductCom_val.productIssuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductPurseExit.SysProductCom_val.productType = toMoto(tpfile05.purseId);
		tpTxnProductPurseExit.SysProductCom_val.productSerialNumber = toMoto(tpfile05.productIssuerId);

		memset(&tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavSamId, 0x00, sizeof(DevUdPurseLavHdr_t));
		return xa_CPU_exit_dis(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_TIMES:	
		//ticket definition
		memcpy(out_buf, "\x53\x33\x02\x0a", 4);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		//
		tpTxnProductMultirideExit.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
		tpTxnProductMultirideExit.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
		tpTxnProductMultirideExit.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]); //toMoto(*(long *)&cmd_buf[13]);
		tpTxnProductMultirideExit.SysComHdr_val.udType = toMoto(3);
		tpTxnProductMultirideExit.SysComHdr_val.udSubtype = toMoto(93);
		//
		tpTxnProductMultirideExit.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductMultirideExit.SysCardCom_val.cardType = toMoto(tpfile05.productId);
		tpTxnProductMultirideExit.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
		tpTxnProductMultirideExit.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);
		
		tpTxnProductMultirideExit.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
		
		tpTxnProductMultirideExit.SysProductCom_val.productIssuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductMultirideExit.SysProductCom_val.productType = toMoto(tpfile05.productId);
		tpTxnProductMultirideExit.SysProductCom_val.productSerialNumber = toMoto(tpfile05.productIssuerId);

		memset(&tpTxnProductMultirideExit.DevUdMultirideLavHdr_val.lavSamId, 0x00, sizeof(DevUdMultirideLavHdr_t));
		return xa_CPU_exit_cnt(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_PERIOD:
		//ticket definition
		memcpy(out_buf, "\x53\x33\x03\x0a", 4);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		//
		tpTxnProductPassExit.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
		tpTxnProductPassExit.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
		tpTxnProductPassExit.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]);
		tpTxnProductPassExit.SysComHdr_val.udType = toMoto(3);
		tpTxnProductPassExit.SysComHdr_val.udSubtype = toMoto(92);
		
		tpTxnProductPassExit.SysProductCom_val.productIssuerId = tpTxnProductPassExit.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
		tpTxnProductPassExit.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
		tpTxnProductPassExit.SysProductCom_val.productSerialNumber = tpTxnProductPassExit.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
		tpTxnProductPassExit.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);
		
		tpTxnProductPassExit.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
		
		tpTxnProductPassExit.SysProductCom_val.productType = toMoto(tpfile05.productId);
		
		memset(&tpTxnProductPassExit.DevUdPassLavHdr_val.lavSamId, 0x00, sizeof(DevUdPassLavHdr_t));
		tpCPU.tranamount = 0;
		return xa_CPU_exit_em(cmd_buf, out_buf, out_len);
	default:
		return CE_NON_FEETYPE;
	}
}
/************************************
function:CPU exit for value card
************************************/
char xa_CPU_exit_dis(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret;
char chCode;
unsigned char buf[50], start_timebcd[7], end_timebcd[7], entry_timebcd[7];
unsigned char cpubuf[80], cpulen;
unsigned char status, ch_roll_temp;
unsigned short shDays;
unsigned long lngMidnightSecond, lngTranTimes, lngsrcstation, lngdesstation;
long lngHisecond1, lngLosecond1, lngHisecond2, lngtxnDatetime;
long 	lngCardBalance;
unsigned short shFare, shTicketType, cnt, cnt2;
unsigned short shLaststation;
	
	//check whether goto the rollback place or not
	//read the value
	if((chCode = CPU_GetFiles1b(out_buf)) != 0)
		return chCode;

	//check the valid date and actived flag
	xa_daytodate(tpfile05.cardBaseDateTime, tpfile1b.validityStartDate, &lngHisecond1, &start_timebcd[0]);
	xa_DurationTolocaltime(lngHisecond1, tpfile1b.validityDurationType, tpfile1b.validityDuration, &end_timebcd[0]);
	lngHisecond1 = timestr2long(&end_timebcd[1]) - 1;
	long2timestr(lngHisecond1, &end_timebcd[0]);
	//entry time
	xa_MinuteTolocaltime(&entry_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
	lngHisecond1 += (tpfile15.lastDateTime * 60);
	tpTxnProductPurseExit.entryTime = toMoto(lngHisecond1 + TIME2000 - ZONE8);
	long2timestr(lngHisecond1, &entry_timebcd[0]);
	if((chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, entry_timebcd, tpfile1b.validityDurationType)) != 0)
		return chCode;

	tpTxnProductPurseExit.endOfJourney= toMoto(1);
	tpTxnProductPurseExit.totalJourneyAmount = 0;
	tpTxnProductPurseExit.firstUseActivation = toMoto(tpfile1b.activated);
	tpTxnProductPurseExit.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);

	//origin station
	if(0 != (chCode = card_to_location(tpfile15.origin, &lngsrcstation)))
		return chCode;
	tpTxnProductPurseExit.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngsrcstation);
	tpTxnProductPurseExit.DevUdJourneyHdr_val.currentLocation = tpTxnProductPurseExit.SysComHdr_val.deviceLocation;
	tpTxnProductPurseExit.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductPurseExit.SysComHdr_val.deviceLocation;
	//metro entry/exit status
	memcpy(out_buf, "\x53\x33\x01\x12", 4);
	if(0 != (chCode = CPU_TellEntry(tpfile15.journeyStatus, 0xff)))
	{//exit status
		if(ch_sz_cpu_rollback == MCPU_ROLL_CHECK_15)
		{
			tpCPU.tranamount = tpMCPUProtect[tpMCPUProtectIndex].tranAmount;
			goto label_dis_debit;
		}else if(ch_sz_cpu_rollback == MCPU_ROLL_AFTER_ED)
		{
			memcpy(&tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue, tpMCPUProtect[tpMCPUProtectIndex].rec_buf, sizeof(DevUdPurseCommonHdr_t) + sizeof(SysFinDetails_t));
			tpCPU.tranamount = tpMCPUProtect[tpMCPUProtectIndex].tranAmount;
			if(tpCPU.balance == toMoto(tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue))
			{
				tpCPU.balance = tpMCPUProtect[tpMCPUProtectIndex].balance;
				goto label_dis_update_17;
			}
			else if(tpCPU.balance == toMoto(tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue) + toMoto(tpTxnProductPurseExit.SysFinDetails_val.transactionValue))
				goto label_dis_debit;
			else 
				goto label_refuse_to_exit;
		}else
		{
			goto label_refuse_to_exit;
		}
	}

	//calculate the fare
	memcpy(out_buf, "\x53\x33\x01\x14", 4);
	lngdesstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
	if(0 != (chCode = location_to_card(lngdesstation, &shLaststation)))
		return chCode;
	if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngdesstation, &shFare)))
		return chCode;
	if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
		return chCode;
	tpCPU.tranamount = tpSysPrice.price;

	//renew flag
	memcpy(out_buf, "\x53\x33\x01\x15", 4);
	if(tpfile15.journeyStatus != 4)
	{
		memcpy(out_buf, "\x53\x33\x01\x16", 4);
		if((chCode = cal_overtime(entry_timebcd, tpCPU.time_bcd, 0, 0)) != 0)
		{
			if(chCode == CE_OVERTIME)
				goto label_refuse_to_exit;
			else
				return chCode;
		}
	}
	memcpy(out_buf, "\x53\x33\x01\x18", 4);
	//if current station is set to fare mode
	if(tpwaivermode.cur_sta_fare)
	{//on FARE mode the price is the same station price
		if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngdesstation, lngdesstation, &shFare)))
			return chCode;
		if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
			return chCode;
		tpCPU.tranamount = tpSysPrice.price;
	}
	//check the transaction
	if((tpfile15.journeyStatus == 4) && (tpfile15.lastLocation == shLaststation) && (tpCPU.tranamount > tpCPU.balance))
	{
		tpCPU.tranamount = tpCPU.balance;	
	}
	//
	tpfile15.journeyStatus = 2;
	if(tpwaivermode.cur_sta_failure)
	{
		tpCPU.tranamount = 0;
		tpfile15.journeyStatus = 3;
	}
	if((tpCPU.balance - tpCPU.tranamount) < 0)
	{
		if(tpTicketDef.IgnoreInsufficientFunds == 0)
		{
			chCode = CE_ENOUGH_BALANCE;
			goto label_refuse_to_exit;
		}else
		{
			tpCPU.tranamount = tpCPU.balance;
		}
	}	
label_sz_rollback_1:
	//更改交易明细记录文件
	//update file 15
	tpfile15.lastLocation = shLaststation;
	xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
	xaFile15._File15.destination_1 = (tpfile15.lastLocation >> 8) & 0xF;
	xaFile15._File15.destination = tpfile15.lastLocation & 0xFF;
	xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xF;
	xaFile15._File15.lastDateTime = 0;
	xaFile15._File15.totalPurchaseValue_1 = tpCPU.tranamount >> 8;
	xaFile15._File15.totalPurchaseValue = tpCPU.tranamount & 0xFF;
	xaFile15._File15.journeyStatus = tpfile15.journeyStatus;
	xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
	xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
	xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
	memcpy(cpu_15_data, xaFile15.buff, 13);
	//verify pin
	//if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6, out_buf)) != 0)
	//	return chCode;
	//
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
	{
		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write((unsigned char *)&tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue, sizeof(DevUdPurseCommonHdr_t) + sizeof(SysFinDetails_t));
		return CE_CONSUME_MOVED;
	}

label_dis_debit:
	//进行实际消费
	memcpy(out_buf, "\x53\x33\x01\x1b", 4);
	tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpCPU.balance - tpCPU.tranamount);
	tpTxnProductPurseExit.SysFinDetails_val.transactionValue = toMoto(tpCPU.tranamount);
	tpTxnProductPurseExit.SysFinDetails_val.paymentMethod = toMoto(1);
	tpTxnProductPurseExit.SysFinDetails_val.partialTransactionValue = 0;
	memset(buf, 0x00, 8);
	memcpy(buf, &cpu_05_data[4], 4);
	if(0 != (chCode = CPU_init_for_purchase(1, tpCPU.tranamount, ch_cpu20_psam_id, buf, NULL, ch_cpu_mac_data)))
	{
		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write((unsigned char *)&tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue, sizeof(DevUdPurseCommonHdr_t) + sizeof(SysFinDetails_t));
		return CE_CONSUME_MOVED;
	}
	ch_sz_cpu_rollback = SZ_CPU_CAPP_1;
	if(CPU_debit_for_purchase(&ch_roll_temp, NULL, out_buf))
	{
		ch_sz_cpu_rollback = MCPU_ROLL_AFTER_ED;
		xa_MCPU_rollback_write((unsigned char *)&tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue, sizeof(DevUdPurseCommonHdr_t) + sizeof(SysFinDetails_t));
		return CE_CONSUME_MOVED;
	}

label_dis_update_17:
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 15;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = (tpCPU.tranamount >> 11) & 0x3F;
	xaFile17._File17.value_2 = (tpCPU.tranamount >> 3) & 0xFF;
	xaFile17._File17.value = tpCPU.tranamount & 0x7;
	xaFile17._File17.remainingValue_1 = ((tpCPU.balance - tpCPU.tranamount) >> 12) & 0x1F;
	xaFile17._File17.remainingValue_2 = ((tpCPU.balance - tpCPU.tranamount) >> 4) & 0xFF;
	xaFile17._File17.remainingValue = (tpCPU.balance - tpCPU.tranamount) & 0xF;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	//record
	tpTxnProductPurseExit.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductPurseUseOnExit_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPurseExit.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPurseExit.SysComHdr_val.reservedField, sh_mac_len - 36 - 20 - 4);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[sh_mac_len - 36 - 20], &tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavSamId, 36 + 20);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPurseExit.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPurseExit.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;

	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.productId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	lngCardBalance = tpCPU.balance - tpCPU.tranamount;
	memcpy(&out_buf[14], &lngCardBalance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_VALUE;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductPurseExit.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	//calculate the TAC
	memcpy(tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);

	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}
	return CE_OK;
label_refuse_to_exit:
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}

	memcpy(out_buf, "\x53\x33\x01\x23", 4);
label_sz_rollback_2:	
	*out_len = 4;
	return chCode;
}
/************************************
CPU计次票出�????
************************************/
char xa_CPU_exit_cnt(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret;
unsigned char buf[80], entry_timebcd[7], start_timebcd[7], end_timebcd[7];
unsigned char cpubuf[80], cpulen, ch_roll_temp;
char chCode;
unsigned long lngMidnightSecond, lngTranTimes, lngstation, lngLocation;
long lngHisecond1, lngLosecond1, lngHisecond2, lngtxnDatetime;
long 	lngCardBalance;
unsigned short shFare, shTicketType, cnt, cnt2;
unsigned short i;

	memset(cpu_1c_data,0x00,16);//20230714
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the multiride
	if((chCode = CPU_GetFiles1c(out_buf)) != 0)
		return chCode;
	#ifdef TESTLOG2022
		TestLog(tpCPU.time_bcd,"CPU_exit_cnt_GetFiles1c:",7);	
		TestLog(cpu_1c_data,"FF:",16);
		TestLog(tpCPU.time_bcd,"CPU_exit_cnt_GetFiles15:",7);	
		TestLog(cpu_15_data,"FE:",16);
    #endif
	//
	tpTxnProductMultirideExit.SysProductCom_val.productActionSequenceNumber = toMoto(tpfile1c.actionSequenceNumber);
	tpTxnProductMultirideExit.SysProductCom_val.Ptsn = toMoto(tpfile1c.transactionSequenceNumber);
	tpTxnProductMultirideExit.SysProductCom_val.invoicePrinted = toMoto(tpfile1c.invoicePrinted);
	
	tpTxnProductMultirideExit.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
	tpTxnProductMultirideExit.DevUdJourneyHdr_val.currentLocation = tpTxnProductPurseEntry.SysComHdr_val.deviceLocation;
	tpTxnProductMultirideExit.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductPurseEntry.SysComHdr_val.deviceLocation;
	//check the valid date and actived flag
	xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1c.validityStartDateTime, &lngHisecond1);
	tpTxnProductMultirideExit.DevUdProductValidity_val.vStartDateTime = toMoto(lngHisecond1 + TIME2000);
	xa_DurationTolocaltime(lngHisecond1, tpfile1c.validityDurationType, tpfile1c.validityDuration, &end_timebcd[0]);
	lngHisecond1 = timestr2long(&end_timebcd[1]) - 1;
	long2timestr(lngHisecond1, &end_timebcd[0]);
	tpTxnProductMultirideExit.DevUdProductValidity_val.vEndDateTime = toMoto(lngHisecond1 + TIME2000);
	tpTxnProductMultirideExit.DevUdProductValidity_val.vDuration = toMoto(tpfile1c.validityDurationType);
	//entry time
	xa_MinuteTolocaltime(&entry_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngLosecond1);
	lngLosecond1 += (tpfile15.lastDateTime * 60);
	tpTxnProductMultirideExit.entryTime = toMoto(lngLosecond1 + TIME2000 - ZONE8);
	long2timestr(lngLosecond1, &entry_timebcd[0]);
	if((chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, entry_timebcd, tpfile1c.validityDurationType)) != 0)
	{
#ifdef TESTLOG2022
		TestLog(tpCPU.time_bcd,"CPU_exit_cnt_TellDate fail",7);
		TestLog(tpCPU.time_bcd,"tpCPU.time_bcd:",7);
		TestLog(start_timebcd,"start_timebcd:",7);
		TestLog(end_timebcd,"end_timebcd:",7);
		TestLog(entry_timebcd,"entry_timebcd:",7);
#endif	
		return chCode;
	}

	//
	memcpy(out_buf, "\x47\x19", 2);
	if(0 != (chCode = card_to_location(tpfile1c.validityOrigin, &lngLocation)))
	{
	    return chCode;
	}
		
	if(0 != (chCode = card_to_location(tpfile1c.validityDestination, &lngstation)))
	{	
		return chCode;
	}
		
	tpTxnProductMultirideExit.DevUdProductValidity_val.vOrigin = toMoto(lngLocation);
	tpTxnProductMultirideExit.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	if(0 != (chCode = xa_ValidateArea(tpCmdInit.curstation, lngstation)))
	{
		
		return chCode;
	}
	//calculate the fare
	memcpy(out_buf, "\x47\x1a", 2);
	if(0 != (chCode = card_to_location(tpfile15.origin, &lngstation)))
	{
	    return chCode;
	}
		
	tpTxnProductMultirideExit.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngstation);
	tpTxnProductMultirideExit.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductMultirideExit.SysComHdr_val.deviceLocation;
	//if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, cpu_19_data[4], cmd_buf[7], &shFare)))
	//	return chCode;
	//calculate the times deducted
	//
	tpTxnProductMultirideExit.cardCaptured = 0;
	tpTxnProductMultirideExit.endOfJourney = toMoto(1);
	tpTxnProductMultirideExit.valuePerRide = 0;
	tpTxnProductMultirideExit.firstUseActivation = toMoto(1);

	//metro entry/exit status
	memcpy(out_buf, "\x47\x18", 2);
	if(tpTicketDef.IgnoreEntryExitSequence == 0)
	{
		if(0 != (chCode = CPU_TellEntry(tpfile15.journeyStatus, 0xff)))
		{//check the roll back
			if(ch_sz_cpu_rollback == MCPU_ROLL_CHECK_15)
			{
				tpCPU.tranamount = tpMCPUProtect[tpMCPUProtectIndex].tranAmount;
				goto label_cnt_debit;
			}	
			else if(ch_sz_cpu_rollback == MCPU_ROLL_AFTER_ED)
			{
				memcpy(&tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides, tpMCPUProtect[tpMCPUProtectIndex].rec_buf, tpMCPUProtect[tpMCPUProtectIndex].rec_len);
				tpCPU.tranamount = tpMCPUProtect[tpMCPUProtectIndex].tranAmount;
#ifdef	DEBUG_ROLLBACK				
				PRINTK("back up balance %08x numrides %08x\n", 
					toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.remainingRides),
					toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides));
#endif
				if(tpCPU.balance == toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.remainingRides))
				{
					tpCPU.balance = tpMCPUProtect[tpMCPUProtectIndex].balance;
					goto label_cnt_update_17;
				}else if(tpCPU.balance == toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.remainingRides) + toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides))
				{
					tpCPU.tranamount = tpMCPUProtect[tpMCPUProtectIndex].tranAmount;
					goto label_cnt_debit;
				}else
				{
					goto label_refuse_to_exit;
				}
			}
			else
			{
				goto label_refuse_to_exit;
			}
		}
	}else
	{
		memcpy(entry_timebcd, tpCPU.time_bcd, 7);
		if(ch_sz_cpu_rollback == MCPU_ROLL_CHECK_15)
		{
			tpCPU.tranamount = tpMCPUProtect[tpMCPUProtectIndex].tranAmount;
			goto label_cnt_debit;
		}	
		else if(ch_sz_cpu_rollback == MCPU_ROLL_AFTER_ED)
		{
			memcpy(&tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides, tpMCPUProtect[tpMCPUProtectIndex].rec_buf, tpMCPUProtect[tpMCPUProtectIndex].rec_len);
			tpCPU.tranamount = tpMCPUProtect[tpMCPUProtectIndex].tranAmount;
#ifdef	DEBUG_ROLLBACK				
			PRINTK("back up balance %08x numrides %08x\n", 
				toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.remainingRides),
				toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides));
#endif
			if(tpCPU.balance == toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.remainingRides))
			{
				tpCPU.balance = tpMCPUProtect[tpMCPUProtectIndex].balance;
				goto label_cnt_update_17;
			}else if(tpCPU.balance == toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.remainingRides) + toMoto(tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides))
			{
				tpCPU.tranamount = tpMCPUProtect[tpMCPUProtectIndex].tranAmount;
				goto label_cnt_debit;
			}
		}
	}
	//update flag
	memcpy(out_buf, "\x47\x1b", 2);
	if((tpfile15.journeyStatus != 4) && (tpTicketDef.IgnoreMaxJourneyTime == 0))
	{
		if((chCode = cal_overtime(entry_timebcd, tpCPU.time_bcd, 0, 0)) != 0)
		{
			if(chCode == CE_OVERTIME)
				goto label_refuse_to_exit;
			else
			{

				return chCode;			
			}
				
		}
	}
	
	//if the current station is set to the failure mode then 0 times deducted
	tpCPU.tranamount = 1;
	tpfile15.journeyStatus = 2;
	if(tpwaivermode.cur_sta_failure)
	{
		tpCPU.tranamount = 0;
		tpfile15.journeyStatus = 3;
	}
	//check balance
	if(tpCPU.balance <= 0)
	{
		tpCPU.tranamount = 0;
		chCode = CE_ENOUGH_BALANCE;
		goto label_refuse_to_exit;
	}	
	//update file 15
	xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
	//last station
	lngstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
	{

		return chCode;
	}


	//update file 15
	xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xF;
	xaFile15._File15.destination_1 = (tpfile15.lastLocation >> 8) & 0xF;
	xaFile15._File15.destination = tpfile15.lastLocation & 0xFF;
	xaFile15._File15.lastDateTime = 0;
	xaFile15._File15.totalPurchaseValue_1 = tpCPU.tranamount >> 8;
	xaFile15._File15.totalPurchaseValue = tpCPU.tranamount & 0xFF;
	xaFile15._File15.journeyStatus = tpfile15.journeyStatus;
	xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
	xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
	xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
	memcpy(cpu_15_data, xaFile15.buff, 13);
	//verify pin
	//if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6, out_buf)) != 0)
	//	return chCode;
	//
#ifdef TESTLOG2022
		TestLog(cpu_15_data,"CPU_exit_cnt_writeback 15:",16);

#endif	
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
	{

		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write(tpTxnProductMultirideExit.AFCHead_val.operatorid, sizeof(TxnProductMultirideUseOnExit_t));
		return CE_CONSUME_MOVED;
	}
#ifdef	DEBUG_ROLLBACK	
	if(toMoto(tpTxnProductMultirideExit.SysComHdr_val.udsn) == 1)
	{
		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write(tpTxnProductMultirideExit.AFCHead_val.operatorid, sizeof(TxnProductMultirideUseOnExit_t));
		return CE_CONSUME_MOVED;
	}
#endif
label_cnt_debit:
	tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides = toMoto(tpCPU.tranamount);
	tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.remainingRides = toMoto(tpCPU.balance - tpCPU.tranamount);
	//purchase
	memcpy(out_buf, "\x46\x1b", 2);
	memset(buf, 0x00, 8);
	memcpy(buf, &cpu_05_data[4], 4);
	if(0 != (chCode = CPU_init_for_purchase(1, tpCPU.tranamount, ch_cpu20_psam_id, buf, NULL, ch_cpu_mac_data)))
	{
		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write(tpTxnProductMultirideExit.AFCHead_val.operatorid, sizeof(TxnProductMultirideUseOnExit_t));
		return CE_CONSUME_MOVED;
	}
	memcpy(out_buf, "\x46\x1d", 2);

#ifdef	DEBUG_ROLLBACK	
	if(toMoto(tpTxnProductMultirideExit.SysComHdr_val.udsn) == 3)
	{
		ch_sz_cpu_rollback = MCPU_ROLL_AFTER_ED;
		xa_MCPU_rollback_write((unsigned char *)&tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides, sizeof(DevUdMultirideCommonHdr_t));
		return CE_CONSUME_MOVED;
	}
#endif
	if(CPU_debit_for_purchase(&ch_roll_temp, NULL, out_buf))
	{
		ch_sz_cpu_rollback = MCPU_ROLL_AFTER_ED;
		xa_MCPU_rollback_write((unsigned char *)&tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides, sizeof(DevUdMultirideCommonHdr_t));
		return CE_CONSUME_MOVED;
	}
#ifdef	DEBUG_ROLLBACK	
	if(toMoto(tpTxnProductMultirideExit.SysComHdr_val.udsn) == 2)
	{
		ch_sz_cpu_rollback = MCPU_ROLL_AFTER_ED;
		xa_MCPU_rollback_write((unsigned char *)&tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides, sizeof(DevUdMultirideCommonHdr_t));
		return CE_CONSUME_MOVED;
	}
#endif

label_cnt_update_17:
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 15;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = (tpCPU.tranamount >> 11) & 0x3F;
	xaFile17._File17.value_2 = (tpCPU.tranamount >> 3) & 0xFF;
	xaFile17._File17.value = tpCPU.tranamount & 0x7;
	xaFile17._File17.remainingValue_1 = ((tpCPU.balance - tpCPU.tranamount) >> 12) & 0x1F;
	xaFile17._File17.remainingValue_2 = ((tpCPU.balance - tpCPU.tranamount) >> 4) & 0xFF;
	xaFile17._File17.remainingValue = (tpCPU.balance - tpCPU.tranamount) & 0xF;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	//record
	tpTxnProductMultirideExit.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductMultirideUseOnExit_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductMultirideExit.SysComHdr_val.formatVersion, 40);
	memcpy(&ch_mac_data[40], &tpTxnProductMultirideExit.SysComHdr_val.reservedField, sh_mac_len - 40 - 4);
	sh_mac_len -= 4;
	//bakup the TXN
	g_sha1txnsn = tpTxnProductMultirideExit.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductMultirideExit.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;


label_sz_rollback_1:
	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.productId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	lngCardBalance = tpCPU.balance - tpCPU.tranamount;
	memcpy(&out_buf[14], &lngCardBalance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_TIMES;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductMultirideExit.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
#ifdef	DEBUG_PRINT
	for(i = 0; i < tpYPT_txn_val.YPT_txnlen; i++)
		PRINTK("%02x", out_buf[33 + i]);
	PRINTK("\n");
#endif
	//calculate TAC
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);
	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}

	return CE_OK;
label_refuse_to_exit:
	memcpy(out_buf, "\x47\x23", 2);
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}

label_sz_rollback_2:	
	*out_len = 4;
	return chCode;
}

/************************************
employee exit function
************************************/
char xa_CPU_exit_em(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
char chCode, endChCode;
unsigned char buf[50], factor[20], des[80], deslen;
unsigned char cpubuf[80], cpulen, entry_timebcd[7], start_timebcd[7], end_timebcd[7];
unsigned short allowedusedtimes, hadusedtimes, shTicketType, cnt, cnt2;
unsigned long lngstation, lngtxnDatetime, lngHisecond1, lngLocation, lngLosecond1;
long		  beforeStartTransTime;
unsigned char out_buffEnd[1024], cpu_1d_data_end[16];

    memset(cpu_1d_data,0x00,16);//20230714
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the priod
	if((chCode = CPU_GetFiles1d(out_buf)) != 0)
		return chCode;
	#ifdef TESTLOG2022
		TestLog(tpCPU.time_bcd,"CPU_exit_em_GetFiles1d:",7);	
		TestLog(cpu_1d_data,"FF:",16);
		TestLog(tpCPU.time_bcd,"CPU_exit_em_GetFiles15:",7);	
		TestLog(cpu_15_data,"FE:",16);
    #endif

	//20260826 P1-2: baseline taken after CPU_GetFiles1d read (0715 fallback may have repaired first); recovery rolls back to this value
	beforeStartTransTime = tpfile1d.validityStartDateTime;
	//	
	tpTxnProductPassExit.SysProductCom_val.productActionSequenceNumber = toMoto(tpfile1d.actionSequenceNumber);
	tpTxnProductPassExit.SysProductCom_val.Ptsn = toMoto(tpfile1d.transactionSequenceNumber);
	tpTxnProductPassExit.SysProductCom_val.invoicePrinted = toMoto(tpfile1d.invoicePrinted);

	tpTxnProductPassExit.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
	tpTxnProductPassExit.DevUdJourneyHdr_val.currentLocation = tpTxnProductPurseEntry.SysComHdr_val.deviceLocation;
	tpTxnProductPassExit.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductPurseEntry.SysComHdr_val.deviceLocation;

	//check the valid date and actived flag
	xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1d.validityStartDateTime, &lngHisecond1);
	tpTxnProductPassExit.DevUdProductValidity_val.vStartDateTime = toMoto(lngHisecond1 + TIME2000);
	xa_DurationTolocaltime(lngHisecond1, tpfile1d.validityDurationType, tpfile1d.validityDuration, &end_timebcd[0]);
	lngHisecond1 = timestr2long(&end_timebcd[1]) - 1;
	long2timestr(lngHisecond1, &end_timebcd[0]);
	
	tpTxnProductPassExit.passEndDateTime = tpTxnProductPassExit.DevUdProductValidity_val.vEndDateTime = toMoto(lngHisecond1 + TIME2000);
	tpTxnProductPassExit.DevUdProductValidity_val.vDuration = toMoto(((tpfile1d.validityDurationType & 0xF) << 12) + tpfile1d.validityDuration);
	//entry time
	xa_MinuteTolocaltime(&entry_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngLosecond1);
	lngLosecond1 += (tpfile15.lastDateTime * 60);
	long2timestr(lngLosecond1, &entry_timebcd[0]);
	if((chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, entry_timebcd, tpfile1d.validityDurationType)) != 0)
	{
#ifdef TESTLOG2022
		TestLog(tpCPU.time_bcd,"CPU_exit_em_TellDate fail",7);
		TestLog(tpCPU.time_bcd,"tpCPU.time_bcd:",7);
		TestLog(start_timebcd,"start_timebcd:",7);
		TestLog(end_timebcd,"end_timebcd:",7);
		TestLog(entry_timebcd,"end_timebcd:",7);
#endif	
		return chCode;
	}	
	//check the metro status
	if(tpTicketDef.IgnoreEntryExitSequence == 0)
	{
		//if((tpfile15.journeyStatus != 0x01) && (tpfile15.journeyStatus != 0x04))
		if(0 != (chCode = CPU_TellEntry(tpfile15.journeyStatus, 0xff)))
		{//in exit status
			if(ch_sz_cpu_rollback == MCPU_ROLL_CHECK_15)
			{
				goto label_em_update_17;
			}
			//chCode = CE_NO_ENTRY;
			goto label_refuse_to_exit;
		}
	}
	//
	if(0 != (chCode = card_to_location(tpfile1d.validityOrigin, &lngLocation)))
	{
		return chCode;
	}
		
	if(0 != (chCode = card_to_location(tpfile1d.validityDestination, &lngstation)))
	{

		return chCode;
	}
		
	tpTxnProductPassExit.DevUdProductValidity_val.vOrigin = toMoto(lngLocation);
	tpTxnProductPassExit.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	//check the validity destination
	if(0 != (chCode = xa_ValidateArea(tpCmdInit.curstation, lngstation)))
	{

		return chCode;
	}
	//
	if(0 != (chCode = card_to_location(tpfile15.origin, &lngstation)))
	{

		return chCode;
	}
		
	tpTxnProductPassExit.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngstation);

	lngstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
		return chCode;
	tpTxnProductPassExit.entryTime = toMoto(lngLosecond1 + TIME2000 - ZONE8);
	tpTxnProductPassExit.endOfJourney = toMoto(1);
	tpTxnProductPassExit.firstUseActivation = toMoto(tpfile1d.activated);

	//update file 15
	tpfile15.journeyStatus = 2;
	xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
	
	xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xF;
	xaFile15._File15.lastDateTime = 0;
	xaFile15._File15.destination_1 = (tpfile15.lastLocation >> 8) & 0xF;
	xaFile15._File15.destination = tpfile15.lastLocation & 0xFF;
	xaFile15._File15.journeyStatus = tpfile15.journeyStatus;
	xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
	xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
	xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
	memcpy(cpu_15_data, xaFile15.buff, 13);
#ifdef TESTLOG2022
		TestLog(cpu_15_data,"CPU_exit_em_writeback 15:",16);

#endif
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
	{	
		ch_sz_cpu_rollback = MCPU_ROLL_CHECK_15;
		xa_MCPU_rollback_write(tpTxnProductPassExit.AFCHead_val.operatorid, sizeof(TxnProductPassUseOnExit_t));
		return CE_CONSUME_MOVED;
	}

label_em_update_17:
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 15;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = 0;
	xaFile17._File17.remainingValue_2 = 0;
	xaFile17._File17.remainingValue = 0;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);

	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}

	//record
	tpTxnProductPassExit.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductPassUseOnExit_t);
	sh_mac_len = cnt - 22;
	memcpy(ch_mac_data, &tpTxnProductPassExit.SysComHdr_val.formatVersion, 40);
	memcpy(&ch_mac_data[40], &tpTxnProductPassExit.SysComHdr_val.reservedField, sh_mac_len - 40 - 4);
	sh_mac_len -= 4;
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPassExit.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPassExit.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;

label_sz_rollback_1:
	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.productId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_PERIOD;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductPassExit.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	//calculate the TAC
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);

	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}

	memset(cpu_1d_data,0x00,16);//20230714 
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the priod
	memcpy(out_buffEnd, "\x32\x4E", 2);
	if((endChCode = CPU_GetFiles1d(out_buffEnd)) == 0){
		if(beforeStartTransTime != tpfile1d.validityStartDateTime){
			xaFile1C._File1C.validityStartDateTime_1 = (beforeStartTransTime >> 16) & 0x3F;
			xaFile1C._File1C.validityStartDateTime_2 = (beforeStartTransTime >> 8) & 0xFF;
			xaFile1C._File1C.validityStartDateTime = beforeStartTransTime & 0xFF;
			memset(cpu_1d_data_end, 0x00, 16);
			memcpy(cpu_1d_data_end, xaFile1C.buff, 16);
			endChCode = xa_update_file_1d(cpu_1d_data_end, out_buffEnd);
		}
	}

	return CE_OK;
label_refuse_to_exit:
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}

	//xa_update_file_15(NULL, out_buf);
label_sz_rollback_2:
	*out_len = 47;

	memset(cpu_1d_data,0x00,16);//20230714 
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the priod
	memcpy(out_buffEnd, "\x32\x4E", 2);
	if((endChCode = CPU_GetFiles1d(out_buffEnd)) == 0){
		if(beforeStartTransTime != tpfile1d.validityStartDateTime){
			xaFile1C._File1C.validityStartDateTime_1 = (beforeStartTransTime >> 16) & 0x3F;
			xaFile1C._File1C.validityStartDateTime_2 = (beforeStartTransTime >> 8) & 0xFF;
			xaFile1C._File1C.validityStartDateTime = beforeStartTransTime & 0xFF;
			memset(cpu_1d_data_end, 0x00, 16);
			memcpy(cpu_1d_data_end, xaFile1C.buff, 16);
			endChCode = xa_update_file_1d(cpu_1d_data_end, out_buffEnd);
		}
	}

	return chCode;
}
/************************************
CPU update
************************************/
char xa_CPU_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100];
unsigned char cpubuf[100], cpulen, Le;
unsigned char cnt;
unsigned long time2;
char chCode, chmonth, chRejectCode;
long lngTranTimes;

#ifdef DEBUG_PRINT
unsigned char localtime[7];
	PRINTK("\ncpu update command is %02x and length is %02x:\n", cmd_buf[6], cmd_buf[5]);
	PRINTK("current station is %02x%02x device id is %02x%02x%02x%02x\n", cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	PRINTK("phycial ID %02x%02x%02x%02x %02x%02x%02x%02x SN:%02x%02x%02x%02x\n", 
		cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22], cmd_buf[23], cmd_buf[24]);
	  
	PRINTK("update amount:%02x%02x%02x%02x chip type %02x ticket type %02x language %02x device test mode %02x\n", 
		cmd_buf[25], cmd_buf[26], cmd_buf[27], cmd_buf[28], cmd_buf[29], cmd_buf[30], cmd_buf[31], cmd_buf[32]);
	LocalDateTime2BCD(&cmd_buf[33], localtime);
	PRINTK("local date time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", localtime[0], localtime[1], localtime[2], localtime[3], localtime[4], localtime[5], localtime[6]);
	PRINTK("entry station:%02x%02x\n", cmd_buf[37], cmd_buf[38]);
#endif
	*out_len = 4;
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
		PRINTK("rollback %02x old phyical id", ch_sz_cpu_rollback);
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id[i]);
		PRINTK("  new phyical id ");
		for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id_bak[i]);
#endif
	if((ch_sz_cpu_rollback != 0) && (memcmp(ch_cpu20_phyical_id, ch_cpu20_phyical_id_bak, 8) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_cpu_rollback);
#endif
		if((chRejectCode = CPU_gettransprove(0x09, sfi_bak, xa_metro_psam_index, cpu_05_data, out_buf)) == 0)
		{
			goto label_sz_rollback_main;
		}else if(chRejectCode == CE_READ)
			return CE_READ;
	}
	//
	memcpy(tpCPU.time_bcd, &cmd_buf[10], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[11]);
	//test mode
	memcpy(out_buf, "\x4c\x07", 2);
	if((chCode = CPU_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//select file
	if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
		return chCode;

	//read blance and file 15
	if((chCode = CPU_GetFiles15(out_buf)) != 0)
	{
		return chCode;
	}
	//not locked status
	if(tpfile15.cardStatus != 1)
		return CE_CARDSTATUS;
label_sz_rollback_main:
	//fee type
	switch(tpfile05.productCategory)
	{
	case XA_FEETYPE_VALUE:
	case 0:
		//ticket definition
		memcpy(out_buf, "\x4c\x08", 2);
		if((chCode = CPU_TellSysCard(tpfile05.purseId)) != 0)
		{
			return chCode;
		}
		return xa_CPU_update_dis(cmd_buf, out_buf, out_len);			
	case XA_FEETYPE_TIMES:
		//ticket definition
		memcpy(out_buf, "\x4c\x08", 2);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		return xa_CPU_update_cnt(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_PERIOD:
		//ticket definition
		memcpy(out_buf, "\x4c\x08", 2);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		return xa_CPU_update_em(cmd_buf, out_buf, out_len);
	default:
		return CE_NON_FEETYPE;
	}
}

char xa_CPU_update_dis(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100], blnPurchase;
unsigned char cpubuf[100], cpulen, Le;
unsigned char start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned long time2, lngHisecond1, lngLosecond, lngTravelsecond, lngLastsecond, lngLocation, lngstation;
char chCode, chmonth, chRejectCode;
unsigned long lngTranTimes, lngTranamount,lngtxnDatetime;
unsigned short cnt, cnt2, shTicketType;

	//record
	tpTxnProductPurseCompensate.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
	tpTxnProductPurseCompensate.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
	tpTxnProductPurseCompensate.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[6]);
	tpTxnProductPurseCompensate.SysComHdr_val.udType = toMoto(3);
	tpTxnProductPurseCompensate.SysComHdr_val.udSubtype = toMoto(118);
	
	tpTxnProductPurseCompensate.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
	tpTxnProductPurseCompensate.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);

	tpTxnProductPurseCompensate.SysProductCom_val.productIssuerId = tpTxnProductPurseCompensate.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
	tpTxnProductPurseCompensate.SysProductCom_val.productSerialNumber = tpTxnProductPurseCompensate.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
	tpTxnProductPurseCompensate.SysProductCom_val.productType = toMoto(tpfile05.purseId);
	
	tpTxnProductPurseCompensate.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
	//read multiride file
	if(0 != (chCode = CPU_GetFiles1b(out_buf)))
		return chCode;
	tpTxnProductPurseCompensate.SysProductCom_val.productActionSequenceNumber = toMoto(tpfile1b.actionSequenceNumber);
	tpTxnProductPurseCompensate.SysProductCom_val.Ptsn = toMoto(tpfile1b.transactionSequenceNumber);
	tpTxnProductPurseCompensate.SysProductCom_val.invoicePrinted = toMoto(tpfile1b.invoicePrinted);
	//check the valid date and actived flag
	xa_daytodate(tpfile05.cardBaseDateTime, tpfile1b.validityStartDate, &lngHisecond1, &start_timebcd[0]);
	if(tpfile1b.activated == 0)
	{
		if(memcmp(tpCPU.time_bcd, start_timebcd, 4) > 0)
			return CE_EXPIREDDATE;
		else
		{
			lngHisecond1 = tpCPU.lowsecond;
			//memcpy(start_timebcd, tpCPU.time_bcd, 7);
			//2018/8/21 13:02:13
			if(0 != (chCode = xa_CPU_active_dis(start_timebcd, out_buf, out_len)))
				return chCode;
		}
	}
	//validate 
	memcpy(out_buf, "\x4c\x14", 2);
	xa_DurationTolocaltime(lngHisecond1, tpfile1b.validityDurationType, tpfile1b.validityDuration, &end_timebcd[0]);
	//travel start date-time
	memcpy(out_buf, "\x25\x0a", 2);
	xa_MinuteTolocaltime(&last_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngTravelsecond);
	//last used time
	lngLastsecond = lngTravelsecond + (tpfile15.lastDateTime * 60);
	long2timestr(lngLastsecond, &last_timebcd[0]);
	get_degrade_sensitive_mode(NULL, last_timebcd);
	if((chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, last_timebcd, tpfile1b.validityDurationType)) != 0)
		return chCode;
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpCPU.time_bcd, last_timebcd, 4) != 0))
	{
		tpfile15.journeyStatus = 0;
	}
	//lock flag
	if(tpfile1b.productStatus != 1)
		return CE_LOCKED_TICKET;
	tpTxnProductPurseCompensate.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
	tpTxnProductPurseCompensate.DevUdJourneyHdr_val.currentLocation = tpTxnProductPurseCompensate.SysComHdr_val.deviceLocation;

	//metro entry,exit status check
	blnPurchase = 0;
	chRejectCode = CPU_TellEntry(tpfile15.journeyStatus, 0);
	memcpy(&lngTranamount, &cmd_buf[21], 4);
	//
	tpTxnProductPurseCompensate.SysFinDetails_val.paymentMethod = toMoto(cmd_buf[20]);
	if(cmd_buf[25] == 1)
	{//FEE area
		tpTxnProductPurseCompensate.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpCPU.balance);
		tpTxnProductPurseCompensate.SysFinDetails_val.transactionValue = toMoto(lngTranamount);
		tpTxnProductPurseCompensate.SysFinDetails_val.partialTransactionValue = 0;
		if(chRejectCode == 0)
		{//entry status
			//travel start station
			if(0 != (chCode = card_to_location(tpfile15.origin, &lngLocation)))
				return chCode;
			tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			if(0 != (chCode = card_to_location(tpfile15.lastLocation, &lngLocation)))
				return chCode;
			tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripPreviousLocation = toMoto(lngLocation);
			//overfare
			if(0 != (chRejectCode = UL_TellOverRide(tpCPU.time_bcd, tpfile15.origin, &tpCPU.curstation[0], tpfile15.lastLocation, tpfile15.journeyStatus, tpCPU.balance)))
			{
				if(chRejectCode == CE_OVERRIDE)
				{
					if((cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
						return CE_BADPARAM;
					xaFile15._File15.journeyStatus = 4;
					lngstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
					if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
						return chCode;
					xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
					xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xF;
				}else
					return chRejectCode;
			}
			//overtime
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpCPU.time_bcd, 0, 0)))
				{
					//according to the overtime/overfare
					if(cmd_buf[26] != 0x02)
						return CE_BADPARAM;
					//according to the MONEY
					if((cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
						return CE_BADPARAM;
					if(lngTravelsecond == lngLastsecond)
					{//travel start time is the ENTRY time
						if((tpCPU.lowsecond - lngLastsecond) > 255 * 60)
						{//need change the travel start time
							cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
							xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
							xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
							xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
							xaFile15._File15.lastDateTime = 0;
						}else 
						{//only change the last date/time
							xaFile15._File15.lastDateTime = (tpCPU.lowsecond - lngLastsecond) / 60;
						}
					}else 
					{
						cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
						xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
						xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
						xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
						xaFile15._File15.lastDateTime = 0;
					}
				}
			}
		}else
		{
			//according to the NO-ENTRY
			if(cmd_buf[26] != 0x01)
				return CE_BADPARAM;
			//entry datetime
			cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
			xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
			xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
			xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
			xaFile15._File15.lastDateTime = 0;
			//entry station
			memcpy(&lngstation, &cmd_buf[27], 4);
			tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngstation);
			tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripPreviousLocation = toMoto(lngstation);
			if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
				return chCode;
			xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
			xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xf;
			xaFile15._File15.origin_1 = (tpfile15.lastLocation >> 4) & 0xff;
			xaFile15._File15.origin = tpfile15.lastLocation & 0xf;
			
			xaFile15._File15.journeyStatus = 1;
			xaFile15._File15.totalPurchaseValue_1 = 0;
			xaFile15._File15.totalPurchaseValue = 0;
		}
	}else
	{//NON-Fee area
		if(chRejectCode == 0)
		{
			//travel start station
			if(0 != (chCode = card_to_location(tpfile15.origin, &lngLocation)))
				return chCode;
			tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			if(0 != (chCode = card_to_location(tpfile15.lastLocation, &lngLocation)))
				return chCode;
			tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripPreviousLocation = toMoto(lngLocation);
			//according to the NOEXIT
			if(cmd_buf[26] != 0x03)
				return CE_BADPARAM;
			xaFile15._File15.journeyStatus = 2;
			if(lngTranamount != 0)
			{
				if((cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
				{//FROM Card
					blnPurchase = 0xFF;
					if(lngTranamount > tpCPU.balance)
						return CE_BADPARAM;
					tpCPU.tranamount = lngTranamount;
					tpTxnProductPurseCompensate.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpCPU.balance - lngTranamount);
					tpTxnProductPurseCompensate.SysFinDetails_val.transactionValue = toMoto(lngTranamount);
					//tpTxnProductPurseCompensate.SysFinDetails_val.paymentMethod = toMoto(2);
				}else
				//if(cmd_buf[20] == 0x01)
				{//pay MONEY
					tpTxnProductPurseCompensate.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpCPU.balance);
					tpTxnProductPurseCompensate.SysFinDetails_val.transactionValue = toMoto(lngTranamount);
					//tpTxnProductPurseCompensate.SysFinDetails_val.paymentMethod = toMoto(1);
				}
			}else
			{
				tpTxnProductPurseCompensate.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpCPU.balance);
				tpTxnProductPurseCompensate.SysFinDetails_val.transactionValue = 0;
				tpTxnProductPurseCompensate.SysFinDetails_val.paymentMethod = toMoto(1);
			}
			tpTxnProductPurseCompensate.SysFinDetails_val.partialTransactionValue = 0;
		}else
		{
			return CE_BADPARAM;
		}
	}
	memcpy(cpu_15_data, xaFile15.buff, 13);
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
		return chCode;
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 5;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = (tpCPU.balance >> 12) & 0x1F;
	xaFile17._File17.remainingValue_2 = (tpCPU.balance >> 4) & 0xFF;
	xaFile17._File17.remainingValue = tpCPU.balance & 0xF;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	
	if(blnPurchase)
	{
		memset(buf, 0x00, 8);
		memcpy(buf, &cpu_05_data[4], 4);
		if(0 != (chCode = CPU_init_for_purchase(1, tpCPU.tranamount, ch_cpu20_psam_id, buf, NULL, ch_cpu_mac_data)))
		{
			return chCode;
		}
		memcpy(out_buf, "\x46\x1d", 2);
		ch_sz_cpu_rollback = SZ_CPU_CAPP_1;
		if(CPU_debit_for_purchase(&ch_sz_cpu_rollback, sz_CPU20_ee_write, out_buf))
		{
			return CE_WRITE;
		}
		
	}
label_sz_rollback_main:
	//mac
	tpTxnProductPurseCompensate.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductPurseCompensationFare_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPurseCompensate.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPurseCompensate.SysComHdr_val.reservedField, sh_mac_len - 36 - 4);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[sh_mac_len - 36], &tpTxnProductPurseCompensate.DevUdPurseLavHdr_val.lavSamId, 36);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPurseCompensate.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPurseCompensate.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 0;

	ch_mac_sel = 4;
	sem_init(&g_samreturn, 0, 0);
	sem_post(&g_samcalwait);

	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.purseId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_VALUE;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	sem_wait(&g_samreturn);
	memcpy(&out_buf[39], tpTxnProductPurseCompensate.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;

	(*out_len) += cnt;
	reader_status = XA_RW_IDLE;

#ifdef	DEBUG_PRINT
	PRINTK("CPU update:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	return CE_OK;
}

char xa_CPU_update_em(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100];
unsigned char cpubuf[100], cpulen, Le;
unsigned char start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned long time2, lngHisecond1, lngLosecond, lngTravelsecond, lngLastsecond, lngLocation, lngstation;
char chCode, chmonth, chRejectCode, endChCode;
unsigned long lngTranTimes;
unsigned short cnt, cnt2, shTicketType;
unsigned long 	lngtxnDatetime;
long            beforeStartTransTime;
unsigned char   out_buffEnd[1024], cpu_1d_data_end[16];

	//record
	tpTxnProductPassCompensate.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
	tpTxnProductPassCompensate.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
	tpTxnProductPassCompensate.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[6]);
	tpTxnProductPassCompensate.SysComHdr_val.udType = toMoto(3);
	tpTxnProductPassCompensate.SysComHdr_val.udSubtype = toMoto(119);
	//
	tpTxnProductPassCompensate.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
	tpTxnProductPassCompensate.SysProductCom_val.productSerialNumber = tpTxnProductPassCompensate.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
	tpTxnProductPassCompensate.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
	tpTxnProductPassCompensate.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);
	//
	tpTxnProductPassCompensate.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
	tpTxnProductPassCompensate.SysProductCom_val.productIssuerId = toMoto(tpfile05.productIssuerId);
	tpTxnProductPassCompensate.SysProductCom_val.productSerialNumber = toMoto(tpfile05.productSerialNumber);
	tpTxnProductPassCompensate.SysProductCom_val.productType = toMoto(tpfile05.productId);
	//
	tpTxnProductPassCompensate.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
	tpTxnProductPassCompensate.DevUdJourneyHdr_val.currentLocation = tpTxnProductPassCompensate.SysComHdr_val.deviceLocation;
	tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripOriginLocation = 0;
	tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripPreviousLocation = 0;
	memset(&tpTxnProductPassCompensate.DevUdPassLavHdr_val.lavSamId, 0x00, sizeof(DevUdPassLavHdr_t));
	//read period file
	if(0 != (chCode = CPU_GetFiles1d(out_buf)))
		return chCode;


	//20260826 P1-2: baseline taken after CPU_GetFiles1d read (0715 fallback may have repaired first); recovery rolls back to this value
	beforeStartTransTime = tpfile1d.validityStartDateTime;

	tpTxnProductPassCompensate.SysProductCom_val.invoicePrinted = toMoto(tpfile1d.invoicePrinted);
	tpTxnProductPassCompensate.SysProductCom_val.productActionSequenceNumber = toMoto(tpfile1d.actionSequenceNumber);
	tpTxnProductPassCompensate.SysProductCom_val.Ptsn = toMoto(tpfile1d.transactionSequenceNumber);
	//check the valid date and actived flag
	xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1d.validityStartDateTime, &lngHisecond1);
	if(tpfile1d.activated == 0)
	{
		if(memcmp(tpCPU.time_bcd, start_timebcd, 4) > 0)
			return CE_EXPIREDDATE;
		else
		{
			lngHisecond1 = tpCPU.lowsecond;
			//memcpy(start_timebcd, tpCPU.time_bcd, 7);
			//2018/8/21 13:01:42
			if(0 != (chCode = xa_CPU_active_em(start_timebcd, out_buf, out_len)))
				return chCode;
		}
	}
	tpTxnProductPassCompensate.DevUdProductValidity_val.vStartDateTime = toMoto(lngHisecond1 + TIME2000);
	xa_DurationTolocaltime(lngHisecond1, tpfile1d.validityDurationType, tpfile1d.validityDuration, &end_timebcd[0]);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vEndDateTime = tpTxnProductPassCompensate.passEndDateTime = toMoto(timestr2long(end_timebcd) + TIME2000);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vDuration = toMoto(tpfile1d.validityDuration);
	//validate 
	//travel start date-time
	memcpy(out_buf, "\x25\x0a", 2);
	xa_MinuteTolocaltime(&last_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngTravelsecond);
	//last used time
	lngLastsecond = lngTravelsecond + (tpfile15.lastDateTime * 60);
	long2timestr(lngLastsecond, &last_timebcd[0]);
	memcpy(out_buf, "\x4c\x14", 2);
	get_degrade_sensitive_mode(NULL, last_timebcd);
	if((chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, last_timebcd, tpfile1d.validityDurationType)) != 0)
		return chCode;
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpCPU.time_bcd, last_timebcd, 4) != 0))
	{
		tpfile15.journeyStatus = 0;
	}
	//validate area
	memcpy(out_buf, "\x4c\x15", 2);
	//if((chCode = xa_ValidateArea(out_buf, out_buf)) != 0)
	//	return chCode;
	card_to_location(tpfile1d.validityOrigin, &lngstation);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vOrigin = toMoto(lngstation);
	card_to_location(tpfile1d.validityDestination, &lngstation);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	//lock flag
	if(tpfile1d.productStatus != 1)
		return CE_LOCKED_TICKET;

	//metro entry,exit status check
	chRejectCode = CPU_TellEntry(tpfile15.journeyStatus, 0);
	if(cmd_buf[25] == 1)
	{//FEE area
		if(chRejectCode == 0)
		{//entry status
			//travel start station
			if(0 != (chCode = card_to_location(tpfile15.lastLocation, &lngLocation)))
				return chCode;
			tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			//overtime
			if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpCPU.time_bcd, 0, 0)))
			{
				//overtime/overfare
				if(cmd_buf[26] != 0x02)
					return CE_BADPARAM;
				if(lngTravelsecond == lngLastsecond)
				{//travel start time is the ENTRY time
					if((tpCPU.lowsecond - lngLastsecond) > 255 * 60)
					{//need change the travel start time
						cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
						xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
						xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
						xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
						xaFile15._File15.lastDateTime = 0;
					}else 
					{//only change the last date/time
						xaFile15._File15.lastDateTime = (tpCPU.lowsecond - lngLastsecond) / 60;
					}
				}else 
				{
					cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
					xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
					xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
					xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
					xaFile15._File15.lastDateTime = 0;
				}
			}
		}else
		{
			//update mode error
			if(cmd_buf[26] != 0x01)
				return CE_BADPARAM;
			//entry datetime
			cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
			xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
			xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
			xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
			xaFile15._File15.lastDateTime = 0;
			//entry station
			memcpy(&lngstation, &cmd_buf[27], 4);
			tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngstation);
			if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
				return chCode;
			xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
			xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xf;
			xaFile15._File15.origin_1 = (tpfile15.lastLocation >> 4) & 0xff;
			xaFile15._File15.origin = tpfile15.lastLocation & 0xf;
			
			xaFile15._File15.journeyStatus = 1;
			xaFile15._File15.totalPurchaseValue_1 = 0;
			xaFile15._File15.totalPurchaseValue = 0;
		}
	}else
	{//NON-Fee area
		if(chRejectCode == 0)
		{
			if(cmd_buf[26] != 0x03)
				return CE_BADPARAM;
			//travel start station
			if(0 != (chCode = card_to_location(tpfile15.lastLocation, &lngLocation)))
				return chCode;
			tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			xaFile15._File15.journeyStatus = 2;
		}else
		{
			return CE_BADPARAM;
		}
	}
	memcpy(cpu_15_data, xaFile15.buff, 13);
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
		return chCode;
	
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 5;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = 0;
	xaFile17._File17.remainingValue_2 = 0;
	xaFile17._File17.remainingValue = 0;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
#ifdef DEBUG_PRINT 
printf("xa_update_file_17\n");
#endif	
label_sz_rollback_main:
	//
	tpTxnProductPassCompensate.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	//mac
	cnt2 = cnt = sizeof(TxnProductPassCompensationFare_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPassCompensate.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPassCompensate.SysComHdr_val.reservedField, sh_mac_len - 36 - 20 - 4);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[ sh_mac_len - 36 - 20], &tpTxnProductPassCompensate.DevUdProductValidity_val.vStartDateTime, 36 + 20);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPassCompensate.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPassCompensate.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 0;

	ch_mac_sel = 4;
	sem_init(&g_samreturn, 0, 0);
	sem_post(&g_samcalwait);

	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.productId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_PERIOD;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	sem_wait(&g_samreturn);
	memcpy(&out_buf[39], tpTxnProductPassCompensate.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;

	(*out_len) += cnt;
	reader_status = XA_RW_IDLE;

#ifdef	DEBUG_PRINT
	PRINTK("CPU update:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif

	memset(cpu_1d_data,0x00,16);//20230714 
	memset(cpu_15_data,0x00,16);
	memset(xaFile1C.buff,0x00,16);
	//read the priod
	memcpy(out_buffEnd, "\x32\x4E", 2);
	if((endChCode = CPU_GetFiles1d(out_buffEnd)) == 0){
		if(beforeStartTransTime != tpfile1d.validityStartDateTime){
			xaFile1C._File1C.validityStartDateTime_1 = (beforeStartTransTime >> 16) & 0x3F;
			xaFile1C._File1C.validityStartDateTime_2 = (beforeStartTransTime >> 8) & 0xFF;
			xaFile1C._File1C.validityStartDateTime = beforeStartTransTime & 0xFF;
			memset(cpu_1d_data_end, 0x00, 16);
			memcpy(cpu_1d_data_end, xaFile1C.buff, 16);
			endChCode = xa_update_file_1d(cpu_1d_data_end, out_buffEnd);
		}
	}

	return CE_OK;
}

char xa_CPU_update_cnt(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100], blnPurchase;
unsigned char cpubuf[100], cpulen, Le;
unsigned char start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned long time2, lngHisecond1, lngLosecond, lngTravelsecond, lngLastsecond, lngLocation, lngstation;
char chCode, chmonth, chRejectCode;
unsigned long lngTranTimes, lngTranamount,lngtxnDatetime;
unsigned short cnt, cnt2, shTicketType;

	//record
	tpTxnProductMultirideCompensate.SysComHdr_val.formatVersion = toMoto(tpfile05.version);
	tpTxnProductMultirideCompensate.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
	tpTxnProductMultirideCompensate.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[6]);
	tpTxnProductMultirideCompensate.SysComHdr_val.udType = toMoto(3);
	tpTxnProductMultirideCompensate.SysComHdr_val.udSubtype = toMoto(120);
	
	tpTxnProductMultirideCompensate.SysProductCom_val.productIssuerId = tpTxnProductMultirideCompensate.SysCardCom_val.cardissuerId = toMoto(tpfile05.productIssuerId);
	tpTxnProductMultirideCompensate.SysProductCom_val.productSerialNumber = tpTxnProductMultirideCompensate.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
	tpTxnProductMultirideCompensate.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
	tpTxnProductMultirideCompensate.SysCardCom_val.cardLifeCycleCount = toMoto(tpfile05.lifecycleCount);
	
	tpTxnProductMultirideCompensate.SysAppCom_val.applicationPassengerType = toMoto(tpfile05.passengerType);
	tpTxnProductMultirideCompensate.SysProductCom_val.productType = toMoto(tpfile05.productId);
	//read multiride file
	if(0 != (chCode = CPU_GetFiles1c(out_buf)))
		return chCode;
	tpTxnProductMultirideCompensate.SysProductCom_val.productActionSequenceNumber = toMoto(tpfile1c.actionSequenceNumber);
	tpTxnProductMultirideCompensate.SysProductCom_val.Ptsn = toMoto(tpfile1c.transactionSequenceNumber);
	tpTxnProductMultirideCompensate.SysProductCom_val.invoicePrinted = toMoto(tpfile1c.invoicePrinted);
	//
	tpTxnProductMultirideCompensate.DevUdMultirideLavHdr_val.lavSamId = toMoto(tpfile1c.lavSamId);
	tpTxnProductMultirideCompensate.DevUdMultirideLavHdr_val.lavMethodOfPayment = toMoto(tpfile1c.lavPaymentMethod);
	//check the valid date and actived flag
	xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1c.validityStartDateTime, &lngHisecond1);
	if(tpfile1c.activated == 0)
	{
		if(memcmp(tpCPU.time_bcd, start_timebcd, 4) > 0)
			return CE_EXPIREDDATE;
		else
		{
			lngHisecond1 = tpCPU.lowsecond;
			//memcpy(start_timebcd, tpCPU.time_bcd, 7);
			//2018/8/21 13:00:00 更新激�????
			if(0 != (chCode = xa_CPU_active_cnt(start_timebcd, out_buf, out_len)))
				return chCode;
		}
	}
	tpTxnProductMultirideCompensate.DevUdProductValidity_val.vStartDateTime = toMoto(lngHisecond1 + TIME2000);
	xa_DurationTolocaltime(lngHisecond1, tpfile1c.validityDurationType, tpfile1c.validityDuration, &end_timebcd[0]);
	lngHisecond1 = timestr2long(&end_timebcd[1]);
	tpTxnProductMultirideCompensate.DevUdProductValidity_val.vEndDateTime = toMoto(lngHisecond1 + TIME2000);
	tpTxnProductMultirideCompensate.DevUdProductValidity_val.vDuration = toMoto(tpfile1c.validityDurationType);
	//validate 
	//travel start date-time
	memcpy(out_buf, "\x25\x0a", 2);
	xa_MinuteTolocaltime(&last_timebcd[0], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngTravelsecond);
	//last used time
	lngLastsecond = lngTravelsecond + (tpfile15.lastDateTime * 60);
	long2timestr(lngLastsecond, &last_timebcd[0]);
	memcpy(out_buf, "\x4c\x14", 2);
	get_degrade_sensitive_mode(NULL, last_timebcd);
	if((chCode = xa_TellDate(tpCPU.time_bcd, start_timebcd, end_timebcd, tpfile15.journeyStatus, last_timebcd, tpfile1c.validityDurationType)) != 0)
		return chCode;
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpCPU.time_bcd, last_timebcd, 4) != 0))
	{
		tpfile15.journeyStatus = 0;
	}
	//validate area
	memcpy(out_buf, "\x4c\x15", 2);
	card_to_location(tpfile1c.validityOrigin, &lngLocation);
	card_to_location(tpfile1c.validityDestination, &lngstation);
	tpTxnProductMultirideCompensate.DevUdProductValidity_val.vOrigin = toMoto(lngLocation);
	tpTxnProductMultirideCompensate.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	//if((chCode = xa_ValidateArea(out_buf, out_buf)) != 0)
	//	return chCode;
	
	//lock flag
	if(tpfile1c.productStatus != 1)
		return CE_LOCKED_TICKET;

	//metro entry,exit status check
	blnPurchase = 0;
	chRejectCode = CPU_TellEntry(tpfile15.journeyStatus, 0);
	memcpy(&lngTranamount, &cmd_buf[21], 4);
	
	tpTxnProductMultirideCompensate.SysFinDetails_val.paymentMethod = toMoto(cmd_buf[20]);
	if(cmd_buf[25] == 1)
	{//FEE area
		tpTxnProductMultirideCompensate.DevUdMultirideCommonHdr_val.numRides = toMoto(tpCPU.balance);
		tpTxnProductMultirideCompensate.DevUdMultirideCommonHdr_val.remainingRides = toMoto(tpCPU.balance);
		tpTxnProductMultirideCompensate.SysFinDetails_val.transactionValue = toMoto(lngTranamount);
		//tpTxnProductMultirideCompensate.SysFinDetails_val.paymentMethod = toMoto(1);
		tpTxnProductMultirideCompensate.SysFinDetails_val.partialTransactionValue = 0;
		if(chRejectCode == 0)
		{//entry status
			//travel start station
			if(0 != (chCode = card_to_location(tpfile15.lastLocation, &lngLocation)))
				return chCode;
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideCompensate.SysComHdr_val.deviceLocation;
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripPreviousLocation = toMoto(lngLocation);
			//overtime
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpCPU.time_bcd, 0, 0)))
				{
					if(cmd_buf[26] != 0x02)
						return CE_BADPARAM;
					if((cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
						return CE_BADPARAM;
					if(lngTravelsecond == lngLastsecond)
					{//travel start time is the ENTRY time
						if((tpCPU.lowsecond - lngLastsecond) > 255 * 60)
						{//need change the travel start time
							cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
							xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
							xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
							xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
							xaFile15._File15.lastDateTime = 0;
						}else 
						{//only change the last date/time
							xaFile15._File15.lastDateTime = (tpCPU.lowsecond - lngLastsecond) / 60;
						}
					}else 
					{
						cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
						xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
						xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
						xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
						xaFile15._File15.lastDateTime = 0;
					}
				}
			}
		}else
		{
			//update mode error
			if(cmd_buf[26] != 0x01)
				return CE_BADPARAM;
			//entry datetime
			cnt = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
			xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
			xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
			xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
			xaFile15._File15.lastDateTime = 0;
			//entry station
			memcpy(&lngLocation, &cmd_buf[27], 4);
			if(0 != (chCode = location_to_card(lngLocation, &tpfile15.lastLocation)))
				return chCode;
			xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
			xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xf;
			xaFile15._File15.origin_1 = (tpfile15.lastLocation >> 4) & 0xff;
			xaFile15._File15.origin = tpfile15.lastLocation & 0xf;
			
			xaFile15._File15.journeyStatus = 1;
			xaFile15._File15.totalPurchaseValue_1 = 0;
			xaFile15._File15.totalPurchaseValue = 0;
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideCompensate.SysComHdr_val.deviceLocation;
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripPreviousLocation = toMoto(lngLocation);
		}
	}else
	{//NON-Fee area
		if(chRejectCode == 0)
		{
			if(cmd_buf[26] != 0x03)
				return CE_BADPARAM;
			xaFile15._File15.journeyStatus = 2;
			//Pay card
			if( (cmd_buf[20] != 0x02) && (cmd_buf[20] != XA_PAYTYPE_CARD) )
				return CE_BADPARAM;
			if(lngTranamount > tpCPU.balance)
				return CE_BADPARAM;
			//travel start station
			if(0 != (chCode = card_to_location(tpfile15.lastLocation, &lngLocation)))
				return chCode;
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.passengerType = toMoto(tpfile05.passengerType);
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideCompensate.SysComHdr_val.deviceLocation;
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripPreviousLocation = toMoto(lngLocation);
			//last used time
			lngLastsecond = lngTravelsecond + (tpfile15.lastDateTime * 60);
			long2timestr(lngLastsecond, &last_timebcd[0]);
			tpCPU.tranamount = lngTranamount;
			tpTxnProductMultirideCompensate.DevUdMultirideCommonHdr_val.numRides = toMoto(lngTranamount);
			tpTxnProductMultirideCompensate.DevUdMultirideCommonHdr_val.remainingRides = toMoto(tpCPU.balance - lngTranamount);
			tpTxnProductMultirideCompensate.SysFinDetails_val.transactionValue = 0;
			tpTxnProductMultirideCompensate.SysFinDetails_val.paymentMethod = toMoto(2);
			tpTxnProductMultirideCompensate.SysFinDetails_val.partialTransactionValue = 0;
			blnPurchase = 0xFF;
		}else
		{
			return CE_BADPARAM;
		}
	}
	memcpy(cpu_15_data, xaFile15.buff, 13);
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
		return chCode;

	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 5;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = (tpCPU.balance >> 12) & 0x1F;
	xaFile17._File17.remainingValue_2 = (tpCPU.balance >> 4) & 0xFF;
	xaFile17._File17.remainingValue = tpCPU.balance & 0xF;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	
	if(blnPurchase)
	{
		memset(buf, 0x00, 8);
		memcpy(buf, &cpu_05_data[4], 4);
		if(0 != (chCode = CPU_init_for_purchase(1, tpCPU.tranamount, ch_cpu20_psam_id, buf, NULL, ch_cpu_mac_data)))
		{
			return chCode;
		}
		memcpy(out_buf, "\x46\x1d", 2);
		ch_sz_cpu_rollback = SZ_CPU_CAPP_1;
		if(CPU_debit_for_purchase(&ch_sz_cpu_rollback, sz_CPU20_ee_write, out_buf))
		{
			return CE_WRITE;
		}
		
	}
label_sz_rollback_main:
	//mac
	tpTxnProductMultirideCompensate.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductMultirideCompensationFare_t);
	sh_mac_len = cnt - 22;
	memcpy(ch_mac_data, &tpTxnProductMultirideCompensate.SysComHdr_val.formatVersion, 40);
	memcpy(&ch_mac_data[40], &tpTxnProductMultirideCompensate.SysComHdr_val.reservedField, sh_mac_len - 40 - 4);
	sh_mac_len -= 4;
	//bakup the TXN
	g_sha1txnsn = tpTxnProductMultirideCompensate.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductMultirideCompensate.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 0;

	ch_mac_sel = 4;
	sem_init(&g_samreturn, 0, 0);
	sem_post(&g_samcalwait);

	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.productId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_TIMES;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	sem_wait(&g_samreturn);
	memcpy(&out_buf[39], tpTxnProductMultirideCompensate.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;

	(*out_len) += cnt;
	reader_status = XA_RW_IDLE;

#ifdef	DEBUG_PRINT
	PRINTK("CPU update:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	return CE_OK;
}

/************************************
CPU inquire
************************************/
char xa_CPU_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[300], factor[60], des[60], deslen;
unsigned char cpubuf[100], cpulen, Le;
unsigned char cnt;
unsigned long time2, lngLocation, lngHisecond1, lngLosecond1, lngLogicID, lngcurLocation;
unsigned long lngSecStartLocation, lngSecEndLocation;
unsigned char chCode, chRejectCode, chZoneCode;
unsigned char chmonth, start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned short shlen, shFare, DurationType;
unsigned long lngOvertimePenalty, lngOverfarePenalty, lngsrcstation, lngPenalty;
unsigned char 	blnOverfare, blnOvertime;

	*out_len = 4;
	memcpy(out_buf, "\x53\x35\x4b\x01", 4);
	memcpy(tpCPU.time_bcd, &cmd_buf[9], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[10]);
	memcpy(out_buf, "\x53\x35\x00\x00", 4);
	//select 3f01
	memcpy(out_buf, "\x4b\x0c", 2);
	if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
		return chCode;
	memcpy(sfi_bak, "\x3f\x01", 2);

	//extern auth
	memcpy(out_buf, "\x4b\x10", 2);
	if((chCode = CPU_externauth(0, xa_metro_psam_index, cpu_05_data, out_buf)) != 0)
		return chCode;
	
	//read balance
	memcpy(out_buf, "\x4b\x11", 2);
	if((chCode = CPU_GetFiles15(out_buf)) != 0)
	{
		return chCode;
	}
	memcpy(out_buf, "\x4b\x12", 2);
	if((chCode = CPU_GetFiles1b(out_buf)) != 0)
	{//purse
		return chCode;
	}
	if((chCode = CPU_GetFiles1c(out_buf)) != 0)
	{//times
		return chCode;
	}
	if((chCode = CPU_GetFiles1d(out_buf)) != 0)
	{//period
		return chCode;
	}
	//
	CPU_GetFiles18(10, out_buf);

	get_degrade_mode(tpCPU.curstation);
	//return message
	chCode = CE_OK;
	*out_len = 96 + 22 + 1;
	memset(&out_buf[0], 0x00, ((*out_len) + 10 * 16));
	//phiycial type
	out_buf[0] = XA_MCPU_FAMILY;
	//length
	out_buf[1] = 96;
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
	//product type
	out_buf[22] = tpfile05.productCategory;
	//city code
	memcpy(&out_buf[25], &cpu_05_data[0], 2);
	//business code
	memcpy(&out_buf[27], &cpu_05_data[2], 2);
	//phycial code
	memset(&out_buf[29], 0x00, 7);
	//logicial code
	out_buf[36] = cpu_05_data[7]; out_buf[37] = cpu_05_data[6]; 
	out_buf[38] = cpu_05_data[5]; out_buf[39] = cpu_05_data[4];
	//reinitial times
	memcpy(&out_buf[40], &tpfile05.lifecycleCount, 2);
	//test flag
	out_buf[42] = tpfile05.testCard;
	//card initial code 
	out_buf[43] = tpfile05.cardissuerId;
	//initial date-4
	xa_monthtodate(tpfile05.cardBaseDateTime, &out_buf[44]);
	//initial patch
	memcpy(&out_buf[48], &tpfile05.cardBatchNumber, 2);
	//passenger type
	out_buf[50] = tpfile05.passengerType;
	//deposit
	memset(&out_buf[51] , 0x00, 4);	
	out_buf[51] = tpfile05.cardDepositValue;
	switch(tpfile05.productCategory)
	{
	case XA_FEETYPE_PERIOD:
	//period
		//ticket product type
		out_buf[23] = tpfile05.productId;
		out_buf[24] = 0;
		//product type
		out_buf[55] = XA_FEETYPE_PERIOD;
		//product status
		out_buf[58] = tpfile1d.productStatus;
		//actived flag
		out_buf[59] = tpfile1d.activated;
		//valid start station-4
		card_to_location(tpfile1d.validityOrigin, &lngLocation);
		lngSecStartLocation = lngLocation;
		memcpy(&out_buf[74], &lngLocation, 4);
		//valid end station-4
		card_to_location(tpfile1d.validityDestination, &lngLocation);
		lngSecEndLocation = lngLocation;
		memcpy(&out_buf[78], &lngLocation, 4);
		//balance 
		memset(&out_buf[82], 0x00, 4);
		//card status 
		out_buf[88] = tpfile15.cardStatus;
		//last valid/period/times used 
		out_buf[89] = 2;
		//valid start date
		//xa_daytodate(tpfile05.cardBaseDateTime, tpfile1d.validityStartDateTime, &lngHisecond1, &out_buf[60]);
		xa_MinuteTolocaltime(&out_buf[60], tpfile05.cardBaseDateTime, tpfile1d.validityStartDateTime, &lngHisecond1);
		//valid end date
		if(tpfile1d.activated == 0)
		{
			memcpy(&out_buf[67], &out_buf[60], 7);
			if(memcmp(tpCPU.time_bcd, &out_buf[60], 4) < 0)
				memcpy(&out_buf[60], tpCPU.time_bcd, 7);
		}else
		{
			xa_DurationTolocaltime(lngHisecond1, tpfile1d.validityDurationType, tpfile1d.validityDuration, &out_buf[67]);
			memset(&out_buf[71], 0x00, 3);
			lngLosecond1 = timestr2long(&out_buf[68]) - 1;
			long2timestr(lngLosecond1, &out_buf[67]);
		}
		DurationType = tpfile1d.validityDurationType;
		//travel start time
		xa_MinuteTolocaltime(&out_buf[91], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
		//travel start station
		card_to_location(tpfile15.lastLocation, &lngLocation);
		memcpy(&out_buf[98], &lngLocation, 4);
		//last used time
		lngHisecond1 += (tpfile15.lastDateTime * 60);
		long2timestr(lngHisecond1, &out_buf[106]);
		//last used Station
		memcpy(&out_buf[113], &lngLocation, 4);
		//last travel status 
		out_buf[117] = tpfile15.journeyStatus;
		//
		if((chRejectCode = UL_TellSysCard(tpfile05.productId, NULL)) != 0)
		{
			out_buf[3] = out_buf[2] = (unsigned char)chRejectCode;
			return chCode;
		}
		break;
	case XA_FEETYPE_TIMES:
	//multiride
		//ticket product type
		out_buf[23] = tpfile05.productId;
		out_buf[24] = 0;
		//product type
		out_buf[55] = XA_FEETYPE_TIMES;
		//product status
		out_buf[58] = tpfile1c.productStatus;
		//actived flag
		out_buf[59] = tpfile1c.activated;
		//valid start station-4
		card_to_location(tpfile1c.validityOrigin, &lngLocation);
		lngSecStartLocation = lngLocation;
		memcpy(&out_buf[74], &lngLocation, 4);
		//valid end station-4
		card_to_location(tpfile1c.validityDestination, &lngLocation);
		lngSecEndLocation = lngLocation;
		memcpy(&out_buf[78], &lngLocation, 4);
		//balance --4
		memcpy(&out_buf[82], &tpCPU.balance, 4);
		//card status 
		out_buf[88] = tpfile15.cardStatus;
		//last valid/period/times used 
		out_buf[89] = XA_FEETYPE_TIMES;
		//valid start date
		xa_MinuteTolocaltime(&out_buf[60], tpfile05.cardBaseDateTime, tpfile1c.validityStartDateTime, &lngHisecond1);
		//valid end date
		if(tpfile1c.activated == 0)
		{
			memcpy(&out_buf[67], &out_buf[60], 7);
			if(memcmp(tpCPU.time_bcd, &out_buf[60], 4) < 0)
				memcpy(&out_buf[60], tpCPU.time_bcd, 7);
		}else
		{
			xa_DurationTolocaltime(lngHisecond1, tpfile1c.validityDurationType, tpfile1c.validityDuration, &out_buf[67]);
			memset(&out_buf[71], 0x00, 3);
			lngLosecond1 = timestr2long(&out_buf[68]) - 1;
			long2timestr(lngLosecond1, &out_buf[67]);
		}
		DurationType = tpfile1c.validityDurationType;
		//travel start time-7
		xa_MinuteTolocaltime(&out_buf[91], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
		//travel start station
		card_to_location(tpfile15.lastLocation, &lngLocation);
		memcpy(&out_buf[98], &lngLocation, 4);
		//last used time
		lngHisecond1 += (tpfile15.lastDateTime * 60);
		long2timestr(lngHisecond1, &out_buf[106]);
		//last used Station
		memcpy(&out_buf[113], &lngLocation, 4);
		//last travel status 
		out_buf[117] = tpfile15.journeyStatus;
		//
		if((chRejectCode = UL_TellSysCard(tpfile05.productId, NULL)) != 0)
		{
			out_buf[3] = out_buf[2] = (unsigned char)chRejectCode;
			return chCode;
		}
		break;
	case XA_FEETYPE_VALUE:
	case 0:
		//ticket product type
		out_buf[23] = tpfile05.purseId;
		out_buf[24] = 0;
		//product type
		out_buf[55] = XA_FEETYPE_VALUE;
		//product status
		out_buf[58] = tpfile1b.productStatus;
		//actived flag
		out_buf[59] = tpfile1b.activated;
		//valid start station-4
		memset(&out_buf[74], 0x00, 4);
		//valid end station-4
		memset(&out_buf[78], 0x00, 4);
		//balance --4
		memcpy(&out_buf[82], &tpCPU.balance, 4);
		//card status 
		out_buf[88] = tpfile15.cardStatus;
		//last valid/period/times used 
		out_buf[89] = XA_FEETYPE_VALUE;
		//valid start date
		xa_daytodate(tpfile05.cardBaseDateTime, tpfile1b.validityStartDate, &lngHisecond1, &out_buf[60]);
		//
		memcpy(&out_buf[64], "\x23\x59\x59", 3);
		//valid end date
		if(tpfile1b.activated == 0)
		{
			memcpy(&out_buf[67], &out_buf[60], 7);
			if(memcmp(tpCPU.time_bcd, &out_buf[60], 4) < 0)
				memcpy(&out_buf[60], tpCPU.time_bcd, 7);
		}else
		{	
			xa_DurationTolocaltime(lngHisecond1, tpfile1b.validityDurationType, tpfile1b.validityDuration, &out_buf[67]);
			memset(&out_buf[71], 0x00, 3);
			lngLosecond1 = timestr2long(&out_buf[68]) - 1;
			long2timestr(lngLosecond1, &out_buf[67]);
		}
		DurationType = tpfile1b.validityDurationType;
		//travel start time-7
		xa_MinuteTolocaltime(&out_buf[91], tpfile05.cardBaseDateTime, tpfile15.startDateTime, &lngHisecond1);
		//travel start station
		card_to_location(tpfile15.lastLocation, &lngLocation);
		memcpy(&out_buf[98], &lngLocation, 4);
		//last used time
		lngHisecond1 += (tpfile15.lastDateTime * 60);
		long2timestr(lngHisecond1, &out_buf[106]);
		//last used Station
		memcpy(&out_buf[113], &lngLocation, 4);
		//last travel status 
		out_buf[117] = tpfile15.journeyStatus;
		//
		if((chRejectCode = UL_TellSysCard(tpfile05.purseId, NULL)) != 0)
		{
			out_buf[3] = out_buf[2] = (unsigned char)chRejectCode;
			return chCode;
		}
		break;
	default:
		out_buf[2] = out_buf[3] = CE_NON_FEETYPE;
		return chCode;
	}
	//sub ticket type
	out_buf[56] = 0;
	//issued operation id 
	out_buf[57] = tpTicketDef.ProductIssuer;

	//history
	out_buf[118] = 0;
	if(cmd_buf[22])
	{
		(*out_len) += 10 * 16;
		out_buf[118] = 0x0a;
		CPU_GetFiles17(9, &out_buf[119]);
		CPU_GetFiles1A(1, &out_buf[119 + 9 * 16]);
	}
#ifdef	DEBUG_PRINT
	PRINTK("CPU inquire:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	//product status:1-NOTLOCK;4,8,9,10-LOCK
	if(out_buf[58] != 1)
	{
		out_buf[2] = out_buf[3] = CE_LOCKED_TICKET;
#ifdef DEBUG_PRINT
		PRINTK("reject code lock %02x\n", out_buf[151]);
#endif
		return chCode;
	}
	//black list
	lngLogicID = ByteToLong(NULL, &cpu_05_data[4]);
	if(0 != (chCode = check_metro_Black_Lock(lngLogicID, 0xff, cmd_buf, buf, &shlen)))
	{
		out_buf[2] = out_buf[3] = chCode;
#ifdef DEBUG_PRINT
		PRINTK("reject code black %02x\n", out_buf[2]);
#endif
		return chCode;
	}
	//testing mode
	if((chRejectCode = CPU_TellTesting(tpCmdInit.test))!= 0)
	{
		out_buf[2] = out_buf[3] = chRejectCode;
		return chCode;
	}
	//lock status
	if(out_buf[88] != 1)
	{
		out_buf[2] = out_buf[3] = CE_CARDSTATUS;
		return chCode;
	}
	//票状�????
//	if(out_buf[59] == 0)
//	{//not active
//		if(memcmp(tpCPU.time_bcd, &out_buf[60], 4) > 0)
//			out_buf[2] = out_buf[3] = CE_EXPIREDDATE;
//		else
//			out_buf[2] = out_buf[3] = CE_NONACTIVED;
//#ifdef DEBUG_PRINT
//		PRINTK("reject code actived %02x\n", out_buf[2]);
//#endif
//		return chCode;
//	}
	//有效�????
	get_degrade_sensitive_mode(NULL, &out_buf[106]);
	if((chRejectCode = xa_TellDate(tpCPU.time_bcd, &out_buf[60], &out_buf[67], out_buf[117], &out_buf[106], DurationType)) != 0)
	{
		out_buf[2] = out_buf[3] = chRejectCode;
#ifdef DEBUG_PRINT
		PRINTK("reject code valid date %02x\n", out_buf[2]);
#endif
		return chCode;
	}
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpCPU.time_bcd, &out_buf[106], 4) != 0))
	{
		out_buf[117] = 0;
	}
	if((tpfile05.productCategory == 0) || (tpfile05.productCategory == XA_FEETYPE_VALUE))
	{
		if(tpCPU.balance <= 0)
		{
			out_buf[2] = out_buf[3] = CE_ENOUGH_BALANCE;
			return chCode;
		}
	}
	//metro entry,exit status
	blnOverfare = blnOvertime = 0;
	chRejectCode = CPU_TellEntry(out_buf[117], 0);
	if(cmd_buf[8] == 0x01)
	{//FEE AREA
		if(chRejectCode == 0)
		{//entry status
			//valid zone area
			//禁用区域、区段判�????
			//if((chRejectCode = xa_ValidateArea(&out_buf[74], &out_buf[78])) != 0)
			//{
			//	out_buf[2] = out_buf[3] = chRejectCode;
			//	return chCode;
			//}
			//Over fare
			if((tpfile05.productCategory == 0) || (tpfile05.productCategory == XA_FEETYPE_VALUE))
			{
				if(0 != (chRejectCode = UL_TellOverRide(tpCPU.time_bcd, tpfile15.origin, &tpCPU.curstation[0], tpfile15.lastLocation, tpfile15.journeyStatus, tpCPU.balance)))
				{
					if(chRejectCode == CE_OVERRIDE)
					{
						out_buf[2] = CE_OVERRIDE;
						blnOverfare = 0xff;
						lngOverfarePenalty = tpSysPrice.price - tpCPU.balance;
						memcpy(&out_buf[4], &lngOverfarePenalty, 4);
					}
					else
						out_buf[2] = chRejectCode;
				}
			}
			//overtime
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				//2017/12/28 14:09:49 超时时间判断使用最后使用时间，而不能是旅程起始时间
				if(0 != (chRejectCode = cal_overtime(&out_buf[106], tpCPU.time_bcd, 0, 0)))
				{
					out_buf[2] = chRejectCode;
					out_buf[3] = 0;
					blnOvertime = 0xff; 
					//penalty
					if(tpStationPrice.SJTNum == 0)
						lngOvertimePenalty = 500;
					else
						lngOvertimePenalty = tpStationPrice.SJTPrice[tpStationPrice.SJTNum - 1];
					memcpy(&out_buf[4], &lngOvertimePenalty, 4);
				}
			}
			if(blnOverfare && blnOvertime)
			{
				out_buf[2] = CE_OVERFARETIME;
				lngPenalty = lngOverfarePenalty + lngOvertimePenalty;
				memcpy(&out_buf[4], &lngPenalty, 4);
			}
		}else
		{
			//判断是否在区�????
			if( (tpfile05.productCategory == XA_FEETYPE_PERIOD ) || ( tpfile05.productCategory == XA_FEETYPE_TIMES) )
			{
				chZoneCode = xa_ValidateArea(tpCmdInit.curstation, lngSecStartLocation);
				if(	chZoneCode == 0)
				{
					out_buf[2] = chRejectCode;
					out_buf[3] = 0;
				}else
				{
					out_buf[2] = out_buf[3] = CE_ZONE;
				}
				
			}else
			{
				out_buf[2] = chRejectCode;
				out_buf[3] = 0;
			}
		}
	}else
	{//NON-FEE AREA
		if(chRejectCode == 0)
		{//entry status
			chCode = 0;
			chZoneCode = 0;
			if( (tpfile05.productCategory == XA_FEETYPE_PERIOD) || (tpfile05.productCategory == XA_FEETYPE_TIMES) )
			{
				chZoneCode = xa_ValidateArea(tpCmdInit.curstation, lngSecEndLocation);
			}
			if(chZoneCode != 0)
			{
				out_buf[2] = out_buf[3] = CE_ZONE;
			}else
			{
				out_buf[2] = 0;
				lngLosecond1 = timestr2long(&out_buf[107]);
				memset(&out_buf[4], 0x00, 4);
				memcpy(&lngLocation, &out_buf[113], 4);
				lngcurLocation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
				if((lngcurLocation == lngLocation) && (lngLosecond1 > tpCPU.lowsecond))
				{
					out_buf[3] = CE_FREE_UPDATE_ENTRY;
				}else if((lngcurLocation == lngLocation) && ((tpCPU.lowsecond - lngLosecond1) < 20 * 60))
				{
					out_buf[3] = CE_FREE_UPDATE_ENTRY;
				}else 
				{
					out_buf[3] = CE_FEE_UPDATE_ENTRY;
					if(tpfile05.productCategory == XA_FEETYPE_TIMES) 
					{
						out_buf[4] = 1;
					}
					if((tpfile05.productCategory == XA_FEETYPE_VALUE) || (tpfile05.productCategory == 0))
					{
						if(cmd_buf[17] == 1)
						{//according to the exit station calculate the price
							memcpy(&lngsrcstation, &cmd_buf[18], 4);
							if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngLocation, &shFare)))
								return chCode;
							//using the current time
							if(0 != (chCode = cal_fare_value(tpCPU.time_bcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
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
CPU add value before
************************************/
char sz_CPU_add_prepare(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[80], chmonth;
unsigned char cpubuf[100], cpulen, Le;
unsigned char cnt;
unsigned char chCode, chRejectCode;
unsigned long lngLoadvalue;

#ifdef DEBUG_PRINT
unsigned char localtime[7];
	PRINTK("\ncpu load prepare command is %02x and length is %02x:\n", cmd_buf[6], cmd_buf[5]);
	PRINTK("current station is %02x%02x device id is %02x%02x%02x%02x\n", cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	PRINTK("phycial ID %02x%02x%02x%02x %02x%02x%02x%02x\n", 
		cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20]);//,  cmd_buf[21], cmd_buf[22], cmd_buf[23], cmd_buf[24]);
	  
	PRINTK("chip type %02x ticket type %02x language ? device test mode %02x\n", cmd_buf[21], cmd_buf[22], cmd_buf[23]);//, cmd_buf[27], cmd_buf[28]);
	LocalDateTime2BCD(&cmd_buf[24], localtime);
	PRINTK("local date time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", localtime[0], localtime[1], localtime[2], localtime[3], localtime[4], localtime[5], localtime[6]);
	PRINTK("transaction amount:%02x%02x %02x%02x\n", cmd_buf[28], cmd_buf[29], cmd_buf[30], cmd_buf[31]);
	PRINTK("month added select:%d\n", cmd_buf[32]);
#endif
	*out_len = 4;
	/*memcpy(out_buf, "\x4d\x04\x00\x00", 4);
	memcpy(tpCPU.curtime, &cmd_buf[24], 4);
	if(!(sz_localtimeToSecond(&cmd_buf[24], &tpCPU.hisecond, &tpCPU.lowsecond)))
		return CE_COMMAND;
	LocalDateTime2BCD(&cmd_buf[24], tpCPU.time_bcd);
	sz_localtimeToDay(&cmd_buf[24], &tpCPU.days, &tpCPU.midsecond);
	if(0 != (chCode = check_station_id(&cmd_buf[7])))
		return chCode;
	//
	//must be set to zero for the add value correctly.
	ch_sz_cpu_rollback = 0;
	memcpy(out_buf, "\x4d\x05", 2);
	//read file 05 and 06
	//if((chCode = CPU_GetFiles05(out_buf)) != 0)
	//	return chCode;
	if((chCode = CPU_TellSysCard(cmd_buf[22])) != 0)
		return chCode;
	//transaction
	ByteToLong(&tpCPU.tranamount, &cmd_buf[28]);
	switch(tpTicketDef.TicketFamily)
	{
	case XA_FEETYPE_TIMES:
		//card status
		memcpy(out_buf, "\x4d\x07", 2);
		if(cpu_06_data[25] != SZ_CPU_STATUS_INIT)
		{
			return ERR_TICKET_STATUS;
		}
		if(tpTicketDef.AreaTicketFlag != 0)
			return ERR_FORBID_AREA;
		memcpy(sfi_bak, "\x3f\x01", 2);
		if(0 != (chCode = CPU_select_file(sfi_bak, 2, out_buf, NULL)))
			return chCode;
		//read balance & file 19
		memcpy(out_buf, "\x4d\x10", 2);
		if((chCode = CPU_GetFiles15(out_buf)) != 0)
		{
			return chCode;
		}
		//read file 14
		memcpy(out_buf, "\x4d\x11", 2);
		if((chCode = CPU_GetFiles14(out_buf)) != 0)
			return chCode;
		break;
	case SZ_FEETYPE_MONTH:
		//testing mode
		memcpy(out_buf, "\x4d\x06", 2);
		if((chCode = CPU_TellTesting(cmd_buf[23])) != 0)
			return chCode;
		//card status
		memcpy(out_buf, "\x4d\x07", 2);
		if(cpu_06_data[25] != SZ_CPU_STATUS_SALE)
		{
			return ERR_TICKET_STATUS;
		}
		//check whether the added times is not equal to the Ticket definition or not
		//if(tpTicketDef.MultiRideNumber != tpCPU.tranamount)
		//	return CE_COMMAND;
		//select month
		if((cmd_buf[32] % 2) == 0)
			memcpy(sfi_bak, "\x3f\x02", 2);
		else
			memcpy(sfi_bak, "\x3f\x03", 2);
		if(0 != (chCode = CPU_select_file(sfi_bak, 2, out_buf, NULL)))
			return chCode;
		//read balance & file 19
		memcpy(out_buf, "\x4d\x10", 2);
		if((chCode = CPU_GetFiles19(out_buf)) != 0)
		{
			return chCode;
		}
		//verify the balance whether is zero or not
		if(tpCPU.balance != 0)
			return ERR_NON_ZERO;
		//read file 14
		memcpy(out_buf, "\x4d\x11", 2);
		if((chCode = CPU_GetFiles14(out_buf)) != 0)
			return chCode;
		//validate the period to get startday and end day
		CPU_ValidatePeriod(cpu_06_data[6], tpCPU.days);
		tpCPU.enddate = tpCPU.days + tpTicketDef.Duration;
		break;
	case XA_FEETYPE_VALUE:
		//testing mode
		memcpy(out_buf, "\x4d\x06", 2);
		if((chCode = CPU_TellTesting(cmd_buf[23])) != 0)
			return chCode;
		//card status
		memcpy(out_buf, "\x4d\x07", 2);
		if(cpu_06_data[25] != SZ_CPU_STATUS_SALE)
		{
			return ERR_TICKET_STATUS;
		}
		//block flag
		memcpy(out_buf, "\x4d\x0a", 2);
		if(cpu_06_data[24] != 0)
			return CE_LOCKED_TICKET;
		//add value authorized
		memcpy(out_buf, "\x4d\x0b", 2);
		if(!tpTicketDef.AddValueAuthorized)
			return ERR_FORBID_LOAD;
		//add value integral times
		memcpy(out_buf, "\x4d\x0c", 2);
		if(tpTicketDef.MinAddValue != 0)
		{
			if((tpCPU.tranamount % tpTicketDef.MinAddValue) != 0)
				return ERR_ADDVALUE_TIMES;
		}
		//according the feetype select 3f01/3f02/3f03
		memcpy(out_buf, "\x4d\x0d", 2);
		memcpy(sfi_bak, "\x3f\x01", 2);
		if(0 != (chCode = CPU_select_file(sfi_bak, 2, out_buf, NULL)))
			return chCode;
		//read balance & file 19
		memcpy(out_buf, "\x4d\x10", 2);
		if((chCode = CPU_GetFiles19(out_buf)) != 0)
		{
			return chCode;
		}
		//read file 14
		memcpy(out_buf, "\x4d\x11", 2);
		if((chCode = CPU_GetFiles14(out_buf)) != 0)
			return chCode;
		//validate the period to get startday and end day
		CPU_ValidatePeriod(cpu_06_data[6], tpCPU.days);
		tpCPU.enddate = tpCPU.days + tpTicketDef.Duration;
		//verify balance+ addedamount
		memcpy(out_buf, "\x4d\x12", 2);
		//verify the max remaining value
		if(tpCPU.balance + tpCPU.tranamount > tpTicketDef.MaxRemainingValue)
			return ERR_OVERMAX_AMOUNT;
		break;
	default:
		return ERR_FORBID_LOAD;
	}
	//ticket definition
	//memcpy(out_buf, "\x4d\x08", 2);
	//if((chCode = CPU_TellSysCard(cpu_06_data[0])) != 0)
	//{
	//	return chCode;
	//}

	if(0 != (chCode = CPU_init_for_credit(tpCPU.tranamount, out_buf)))
		return chCode;
#ifdef DEBUG_PRINT
	PRINTK("balance:%02x%02x%02x%02x ", capp_init[0], capp_init[1], capp_init[2], capp_init[3]);
	PRINTK("inline sn:%02x%02x ", capp_init[4], capp_init[5]);
	PRINTK("key version %02x algorithm %02x \n", capp_init[6], capp_init[7]);
	PRINTK("random %02x%02x%02x%02x mac1 %02x%02x%02x%02x\n", capp_init[8], capp_init[9], capp_init[10], capp_init[11], capp_init[12], capp_init[13], capp_init[14], capp_init[15]);
#endif	

	*out_len = 32;
	//device id--change to the psam
	memcpy(&out_buf[0], &ch_cpu20_psam_id[2], 4);
	//phycial id-8
	memcpy(&out_buf[4], &cmd_buf[13], 8);
	//transaction amount-4
	LongToByte(tpCPU.tranamount, &out_buf[12]);
	//balance/sn/key ver/key algorithm/random/mac1
	memcpy(&out_buf[16], capp_init, 16);
	*/return CE_OK;
}
/************************************
CPU add value for value
************************************/
char xa_CPU_add(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
char chCode;

	memcpy(tpCPU.time_bcd, &cmd_buf[10], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[7]);
	//
	get_degrade_mode(tpCPU.curstation);
	//read file 05 and 06--had read in the polling card function
	memcpy(out_buf, "\x53\x38\x00\x00", 4);
	//if((chCode = CPU_GetFiles05(out_buf)) != 0)
	//	return chCode;
	//test mode
	memcpy(out_buf, "\x53\x38\x00\x01", 4);
	if((chCode = CPU_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//card status
	memcpy(out_buf, "\x53\x38\x47\x09", 4);
	//select file
	if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
		return chCode;
	memcpy(sfi_bak, "\x3f\x01", 2);
	//extern auth
	memcpy(out_buf, "\x53\x38\x48\x0e", 4);
	if((chCode = CPU_externauth(0, xa_metro_psam_index, cpu_05_data, out_buf)) != 0)
		return chCode;
	//read blance and file 15
	if((chCode = CPU_GetFiles15(out_buf)) != 0)
	{
		return chCode;
	}
	//not locked status
	if(tpfile15.cardStatus != 1)
		return CE_CARDSTATUS;
	
label_sz_rollback_main:
	memcpy(out_buf, "\x53\x33\x01\x07", 4);
	switch(tpfile05.productCategory)
	{
	case XA_FEETYPE_VALUE:
	case 0:
		break;
	case XA_FEETYPE_TIMES:
		return xa_CPU_add_cnt(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_PERIOD:
		break;
	default:
		return CE_NON_FEETYPE;
	}
	return CE_COMMAND;
}

char xa_CPU_add_cnt(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[90], cpurandom[50];
unsigned char cpubuf[100], Le;
unsigned char factor[20], des[20], start_timebcd[7], end_timebcd[7], entry_timebcd[7];;
unsigned char chCode, chRejectCode;
unsigned short tempdate, shTicketType, cnt, cnt2, cpulen;
long lngHisecond1, lngLosecond1, lngHisecond2, lngtxnDatetime;	

#ifdef DEBUG_PRINT
unsigned char localtime[7];
	PRINTK("\ncpu add dis command is %02x and length is %02x:\n", cmd_buf[6], cmd_buf[5]);
	PRINTK("current station is %02x%02x device id is %02x%02x%02x%02x\n", cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	PRINTK("phycial ID %02x%02x%02x%02x %02x%02x%02x%02x SN:%02x%02x%02x%02x\n", 
		cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20],  cmd_buf[21], cmd_buf[22], cmd_buf[23], cmd_buf[24]);
	  
	PRINTK("chip type %02x device test mode %02x\n", cmd_buf[25], cmd_buf[26]);//, cmd_buf[27], cmd_buf[28]);
	LocalDateTime2BCD(&cmd_buf[27], localtime);
	PRINTK("local date time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", localtime[0], localtime[1], localtime[2], localtime[3], localtime[4], localtime[5], localtime[6]);
	
	PRINTK("CCHS time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", cmd_buf[31], cmd_buf[32], cmd_buf[33], cmd_buf[34], cmd_buf[35], cmd_buf[36], cmd_buf[37]);
	PRINTK("mac2:%02x%02x %02x%02x\n", cmd_buf[38], cmd_buf[39], cmd_buf[40], cmd_buf[41]);
#endif
	*out_len = 2;
	/*
	//check whether goto the rollback place or not
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
	PRINTK("rollback %02x old phyical id", ch_sz_cpu_rollback);
	for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id[i]);
	PRINTK("  new phyical id ");
	for(i = 0; i < 8; i++) PRINTK("%02x", ch_cpu20_phyical_id_bak[i]);
#endif
	if((ch_sz_cpu_rollback != 0) && (memcmp(ch_cpu20_phyical_id, ch_cpu20_phyical_id_bak, 8) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_cpu_rollback);
#endif
		if(ch_sz_cpu_rollback == SZ_CPU_LOAD_1)
		{
			if((chRejectCode = CPU_gettransprove(0x02, sfi_bak, xa_metro_psam_index, cpu_05_data, out_buf)) == 0)
			{
				blncpuRollback = 1;
				//return 255;//maybe special return code for the application can rollback the add value
			}else if(chRejectCode == CE_READ)
				return CE_READ;
			else
				blncpuRollback = 0;
		}else
			//return 255;
			blncpuRollback = 1;
	}
	if(blncpuRollback)
	{
		if(ch_sz_cpu_rollback == SZ_CPU_LOAD_1)
		{//credit for load successfully
			goto label_sz_rollback_1;
		}else if(ch_sz_cpu_rollback == SZ_CPU_LOAD_2)//update file 14 failure
			goto label_sz_rollback_1;
		else if(ch_sz_cpu_rollback == SZ_CPU_LOAD_3)//update file 06 failure
			goto label_sz_rollback_2;
	}*/
	//read the multiride
	memcpy(out_buf, "\x32\x20", 2);
	if((chCode = CPU_GetFiles1c(out_buf)) != 0)
		return chCode;
	tpTxnProductMultirideEntry.SysProductCom_val.Ptsn = toMoto(tpfile1c.transactionSequenceNumber);
	//actived
	xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1c.validityStartDateTime, &lngHisecond1);
	tpTxnProductMultirideEntry.DevUdProductValidity_val.vStartDateTime = toMoto(lngHisecond1 + TIME2000);

	//
	tpTxnProductMultirideEntry.SysCardCom_val.cardActionSequenceNumber = toMoto(tpfile1c.actionSequenceNumber);
	tpTxnProductMultirideEntry.SysProductCom_val.productActionSequenceNumber = toMoto(tpfile1c.actionSequenceNumber);
	tpTxnProductMultirideEntry.SysProductCom_val.invoicePrinted = toMoto(tpfile1c.invoicePrinted);
	//
	memcpy(&tpCPU.tranamount, &cmd_buf[24], 4);
	if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6 , out_buf)) != 0)
		return chCode;
#ifdef DEBUG_PRINT
	PRINTK("tpCPU.tranamount: %d\n", tpCPU.tranamount);
#endif
	if(0 != (chCode = CPU_init_for_credit(tpCPU.tranamount, ch_cpu20_psam_id, out_buf)))
		return chCode;

	memset(factor, 00, 16);
	memcpy(factor, &cpu_05_data[4], 4);
	if(0 != CPU_load_mac2(factor, capp_init, tpCPU.tranamount, mac2, out_buf))
		return CE_MACERR;

	//credit for load
	memcpy(out_buf, "\x38\x26", 2);
	memcpy(buf, "\x80\x52\x00\x00\x0b", 5);
	memcpy(&buf[5], tpCPU.time_bcd, 7);
	memcpy(&buf[12], mac2, 4);
#ifdef DEBUG_PRINT
	PRINTK("credit mac2:");
	for(i = 0; i < 16; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x38\x27\x00\x00", 4);
	ret = mifpro_apdu(buf, 16, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_ADD_MOVED;
	}
#ifdef DEBUG_PRINT
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


label_cnt_update_17:
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 4;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = (tpCPU.balance >> 12) & 0x1F;
	xaFile17._File17.remainingValue_2 = (tpCPU.balance >> 4) & 0xFF;
	xaFile17._File17.remainingValue = tpCPU.balance & 0xF;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	//
	if(ch_sz_cpu_rollback != 0)
	{
		tpMCPUProtect[tpMCPUProtectIndex].rollBack = 0;
		memset(tpMCPUProtect[tpMCPUProtectIndex].phyicalID, 0x00, 8);
	}
	//
	tpTxnProductMultirideEntry.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductMultirideUseOnEntry_t);
	sh_mac_len = cnt - 22;
	memcpy(ch_mac_data, &tpTxnProductMultirideEntry.SysComHdr_val.formatVersion, 40);
	memcpy(&ch_mac_data[40], &tpTxnProductMultirideEntry.SysComHdr_val.reservedField, sh_mac_len - 40 - 4);
	sh_mac_len -= 4;
	//bakup the TXN
	g_sha1txnsn = tpTxnProductMultirideEntry.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductMultirideEntry.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;

label_sz_rollback_1:
	//UDSN added
	out_buf[0] = 1;
	//recycle
	out_buf[1] = 0;
	//black list
	out_buf[2] = 0;
	//ticket family
	out_buf[3] = XA_MCPU_FAMILY;
	//ticket type
	shTicketType = tpfile05.productId;
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	memcpy(&out_buf[6], &ch_cpu20_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);
	//lock flag
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_TIMES;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductMultirideAdd.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	//calculate the TAC
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);


		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
//	return CE_OK;
	
	return CE_COMMAND;
}

char CPU_load_mac2(unsigned char *logic_id, unsigned char *init_mac1_data, int transValue, char *mac2, unsigned char *out_buf)
{
unsigned char cpubuf[100], sambuf[100], samlen;
long i;

	//initial mac1
	memset(cpubuf, 0x00, 100);
	memcpy(cpubuf, "\xF0\x74\x00\x00\x24", 5);
	//用户卡应序列�????
	memcpy(&cpubuf[5], logic_id, 8);
	//伪随机数
	memcpy(&cpubuf[13], &init_mac1_data[8], 4);
	//电子存折或电子钱包交易序�????
	memcpy(&cpubuf[17], &init_mac1_data[4], 2);
	//电子存折或钱包交易余�????
	memcpy(&cpubuf[19], &init_mac1_data[0], 4);
	//交易金额
	LongToByte(transValue, &cpubuf[23]);
	//交易类型标识
	cpubuf[27] = 2;
	//充值密钥版本号
	cpubuf[28] = init_mac1_data[6];
	//充值密钥算法标�????
	cpubuf[29] = init_mac1_data[7];
	//交易日期
	memcpy(&cpubuf[30], tpCPU.time_bcd, 7);
	//mac1
	memcpy(&cpubuf[37], &init_mac1_data[12], 4);

#ifdef DEBUG_PRINT
	for(i = 0; i < 41; i++)
		PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if(0 != sam_apdu(xa_metro_psam_index, cpubuf, 41, sambuf, &samlen, 0, 0))
	{
#ifdef	DEBUG_PRINT
		PRINTK("init ret len %02x %02x%02x\n", samlen, sambuf[0], sambuf[1]);
#endif
		return CE_METROISAM;
	}

#ifdef	DEBUG_PRINT
	PRINTK("mac ret len %02x %02x%02x\n", samlen, sambuf[0], sambuf[1]);
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(cpubuf, "\x00\xc0\x00\x00", 4);
		cpubuf[4] = sambuf[1];
		if(sam_apdu(xa_metro_psam_index, cpubuf, 5, sambuf, &samlen, 0, 0) != 0)
		{
			return CE_METROISAM;
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
		return CE_METROISAM;
	}

	//
	memcpy(mac2, sambuf, 4);
	return 0;
}



#ifdef DEBUG_TEST
//read suzhou file 06
char sz_test_get_file(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len)
{
int ret;
unsigned char buf[40];
unsigned char cpubuf[80], cpulen, Le;
char chret;

	*out_len = 1;
	chret = 0xff;
	switch(cmd_buf[7])
	{
	case 0x06:
		if(0 == CPU_GetFiles05(out_buf))2013/8/16 10:02:38
		{
			memcpy(out_buf, cpu_06_data, 0x1a);
			chret = CE_OK;
			*out_len = 0x1a;
		}
		break;
	case 0x08:
		if(0 == CPU_GetFiles05(out_buf))
		{
			memcpy(out_buf, cpu_05_data, XA_CPU_05_LEN);
			chret = CE_OK;
			*out_len = XA_CPU_05_LEN;
		}
		break;
	case 0x19:
		if(cmd_buf[8] == 0)
		{
			memcpy(buf, "\x3f\x01", 2);
		}else if((cmd_buf[8] % 2) == 0)
			memcpy(buf, "\x3f\x02", 2);
		else
			memcpy(buf, "\x3f\x03", 2);
		if(0 != CPU_select_file(buf, 2, out_buf, NULL))
			break;
		if(0 == CPU_GetFiles15(out_buf))
		{
			chret = CE_OK;
		}
		break;
	default:
		chret = ERR_NOPARAMETER;
		break;
	}
	return chret;
}
//update suzhou file 
char sz_test_update_file(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len)
{
int ret;
unsigned char buf[140], cpurandom[16], des[50], factor[20];
unsigned char cpubuf[180], Le;
char chret;
unsigned short cpulen;

	chret = 0xff;
	*out_len = 1;
	switch(cmd_buf[7])
	{
	case 0x06:
		if(0 != CPU_externauth(1, xa_metro_psam_index, cpu_05_data, out_buf))
			break;;
		//line protected mac write-get random
		memcpy(out_buf, "\x40\x10", 2);
		memset(buf, 0x00, 40);
		memcpy(buf, "\x00\x84\x00\x00\x04", 5);
		if(0 != mifpro_apdu(buf, 5, cpubuf, &cpulen))
		{
#ifdef	DEBUG_PRINT
			PRINTK("get cpu card random failure\n");
#endif
			break;
		}
		if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
			break;
		memset(cpurandom, 0x00, 8);
		memcpy(cpurandom, cpubuf, 4);
		
		//udpate 06 file
		memset(buf, 0x00, 80);
		memcpy(&buf[8], "\x04\xd6\x86\x00", 4);
		buf[12] = SZ_CPU_06_LEN + 4;
		memcpy(&buf[13], &cmd_buf[8], SZ_CPU_06_LEN);

		memcpy(buf, cpurandom, 8);
		buf[5 + 8 + SZ_CPU_06_LEN] = 0x80;	
		//cal mac
		memset(factor, 0x00, 16);
		memcpy(factor, ch_cpu20_logic_id, 8);
		memcpy(&factor[8], "\x21\x50\x80", 3);
		if((ret = cpu_cal_protect_mac(xa_metro_psam_index, factor, 16, "\x45\x01", buf, 5 + 8 + SZ_CPU_06_LEN + 1, des)) != 0)
		{
#ifdef	DEBUG_PRINT
			PRINTK("line mac return %d\n", ret);
#endif
			return CE_METROPSAM;
		}
		memcpy(&buf[5 + 8 + SZ_CPU_06_LEN], des, 4);
		
		ret = mifpro_apdu(&buf[8], 5 + SZ_CPU_06_LEN + 4, cpubuf, &cpulen);
		if(ret != 0)
			break;
		if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))
			break;
		chret = CE_OK;
		break;
	case 0x08:
		out_buf[0] = 0x01;
		if(0 != CPU_externauth(1, xa_metro_psam_index, cpu_05_data, out_buf))
			break;
		//line protected mac write-get random
		memset(buf, 0x00, 40);
		memcpy(buf, "\x00\x84\x00\x00\x04", 5);
		out_buf[0] = 0x02;
		if(0 != mifpro_apdu(buf, 5, cpubuf, &cpulen))
		{
			break;
		}
		out_buf[0] = 0x03;
		if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
			break;
		memset(cpurandom, 0x00, 8);
		memcpy(cpurandom, cpubuf, 4);
		
		//udpate 08 file
		memset(buf, 0x00, 80);
		memcpy(&buf[8], "\x04\xd6\x88\x00", 4);
		buf[12] = SZ_CPU_08_LEN + 4;
		memcpy(&buf[13], &cmd_buf[8], SZ_CPU_08_LEN);

		memcpy(buf, cpurandom, 8);
		buf[5 + 8 + SZ_CPU_08_LEN] = 0x80;	
		//cal mac
		memset(factor, 0x00, 16);
		memcpy(factor, ch_cpu20_logic_id, 8);
		memcpy(&factor[8], "\x21\x50\x80", 3);
		out_buf[0] = 0x04;
		if((ret = cpu_cal_protect_mac(xa_metro_psam_index, factor, 16, "\x45\x01", buf, 5 + 8 + SZ_CPU_08_LEN + 8, des)) != 0)
		{
			break;
		}
		memcpy(&buf[5 + 8 + SZ_CPU_08_LEN], des, 4);
		out_buf[0] = 0x05;
		ret = mifpro_apdu(&buf[8], 5 + SZ_CPU_08_LEN + 4, cpubuf, &cpulen);
		if(ret != 0)
			break;
		out_buf[0] = 0x06;
		if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))
			break;
		chret = CE_OK;
		break;
	case 0x19:
		if(0 != CPU_init_for_capp(1, 0, ch_cpu20_psam_id, ch_cpu20_logic_id, "\x21\x50\x80", 3, ch_cpu_mac_data))
			break;
		if(0 != CPU_debit_for_capppurchase(buf, sz_CPU20_ee_write, out_buf))
			break;
		chret = CE_OK;
		break;
	}
	return chret;
}
#endif

char xa_update_file_15(unsigned char *file_buf, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], cpurandom[8];
unsigned char cpubuf[100], Le;
unsigned char chCode, chRejectCode;
unsigned char factor[20], des[80], deslen, time_bcd[7];
unsigned short cpulen;

	//verify pin
	//if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6, out_buf)) != 0)
	//	return chCode;
	//udpate 15 file
	memcpy(buf, "\x00\x84\x00\x00\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		PRINTK("get cpu card random failure\n");
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("get random :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x13", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memset(cpurandom, 0x00, 8);
	memcpy(cpurandom, cpubuf, 4);
	memcpy(out_buf, "\x44\x14", 2);
	memset(buf, 0x00, 80);
	memcpy(&buf[8], "\x04\xd6\x95\x00", 4);
	buf[12] = 16 + 4;
	//20260826 P1-1: use file_buf param so the 5 recovery blocks actually roll back (NULL keeps old behavior)
	memcpy(&buf[13], (file_buf == NULL) ? cpu_15_data : file_buf, 16);
	memcpy(buf, cpurandom, 8);
	buf[5 + 8 + 16] = 0x80;	
	//cal mac
	memset(factor, 0x00, 16);
	memcpy(factor, &cpu_05_data[4], 4);

	if((ret = cpu_cal_protect_mac(xa_metro_psam_index, factor, 8, "\x23\x01", buf, 5 + 8 + 16 + 3, des)) != 0)
	{
		PRINTK("line mac return %d\n", ret);
		return  CE_READ;
	}
	memcpy(&buf[5 + 8 + 16], des, 4);
	
#ifdef DEBUG_PRINT
	PRINTK("credit:");
	for(i = 8; i < 5 + 16 + 4 + 8; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x15", 2);
	ret = mifpro_apdu(&buf[8], 5 + 16 + 4, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("file 15:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x16", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))	
	{
		return CE_WRITE;
	}
	return 0;
}

char xa_update_file_17(unsigned char *file_buf, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], cpurandom[8];
unsigned char cpubuf[100], Le;
unsigned char chCode, chRejectCode;
unsigned char factor[20], des[80], deslen, time_bcd[7];
unsigned short cpulen;
	//
	//if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6, out_buf)) != 0)
	//	return chCode;
	//udpate 17 file
	memcpy(buf, "\x00\x84\x00\x00\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		PRINTK("get cpu card random failure\n");
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("get random :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x13", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memset(cpurandom, 0x00, 8);
	memcpy(cpurandom, cpubuf, 4);
	memcpy(out_buf, "\x44\x14", 2);
	memset(buf, 0x00, 80);
	memcpy(&buf[8], "\x04\xe2\x00\x00", 4);
	buf[11] = 0x17 << 3;
	buf[12] = XA_CPU_17_LEN + 4;
	memcpy(&buf[13], cpu_17_data, XA_CPU_17_LEN);
	memcpy(buf, cpurandom, 8);
	buf[5 + 8 + 16] = 0x80;	
	//cal mac
	memset(factor, 0x00, 16);
	memcpy(factor, &cpu_05_data[4], 4);

	if((ret = cpu_cal_protect_mac(xa_metro_psam_index, factor, 8, "\x23\x01", buf, 5 + 8 + XA_CPU_17_LEN + 3, des)) != 0)
	{
		PRINTK("line mac return %d\n", ret);
		return  CE_READ;
	}
	memcpy(&buf[5 + 8 + 16], des, 4);
	
#ifdef DEBUG_PRINT
	PRINTK("update17:");
	for(i = 8; i < 5 + 16 + 4 + 8; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x15", 2);
	ret = mifpro_apdu(&buf[8], 5 + XA_CPU_17_LEN + 4, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x16", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))	
	{
		return CE_WRITE;
	}
	return 0;
}

char xa_update_file_1b(unsigned char *file_buf, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], cpurandom[8];
unsigned char cpubuf[100], Le;
unsigned char chCode, chRejectCode;
unsigned char factor[20], des[80], deslen, time_bcd[7];
unsigned short cpulen;
	//
	//if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6, out_buf)) != 0)
	//	return chCode;
	//udpate 17 file
	memcpy(buf, "\x00\x84\x00\x00\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		PRINTK("get cpu card random failure\n");
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("get random :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x13", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memset(cpurandom, 0x00, 8);
	memcpy(cpurandom, cpubuf, 4);
	memcpy(out_buf, "\x44\x14", 2);
	memset(buf, 0x00, 80);
	memcpy(&buf[8], "\x04\xD6\x00\x00", 4);
	buf[10] = 0x1B | 0x80;
	buf[12] = XA_CPU_1B_LEN+ 4;
	memcpy(&buf[13], file_buf, XA_CPU_1B_LEN);
	memcpy(buf, cpurandom, 8);
	buf[5 + 8 + 16] = 0x80;	
	//cal mac
	memset(factor, 0x00, 16);
	memcpy(factor, &cpu_05_data[4], 4);

	if((ret = cpu_cal_protect_mac(xa_metro_psam_index, factor, 8, "\x23\x01", buf, 5 + 8 + XA_CPU_1B_LEN + 3, des)) != 0)
	{
		PRINTK("line mac return %d\n", ret);
		return  CE_READ;
	}
	memcpy(&buf[5 + 8 + 16], des, 4);
	
#ifdef DEBUG_PRINT
	PRINTK("update1C:");
	for(i = 8; i < 5 + 16 + 4 + 8; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x15", 2);
	ret = mifpro_apdu(&buf[8], 5 + XA_CPU_1B_LEN + 4, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x16", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))	
	{
		return CE_WRITE;
	}
	return 0;
}

char xa_update_file_1c(unsigned char *file_buf, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], cpurandom[8];
unsigned char cpubuf[100], Le;
unsigned char chCode, chRejectCode;
unsigned char factor[20], des[80], deslen, time_bcd[7];
unsigned short cpulen;
	//
	//if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6, out_buf)) != 0)
	//	return chCode;
	//udpate 17 file
	memcpy(buf, "\x00\x84\x00\x00\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		PRINTK("get cpu card random failure\n");
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("get random :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x13", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memset(cpurandom, 0x00, 8);
	memcpy(cpurandom, cpubuf, 4);
	memcpy(out_buf, "\x44\x14", 2);
	memset(buf, 0x00, 80);
	memcpy(&buf[8], "\x04\xD6\x00\x00", 4);
	buf[10] = 0x1C | 0x80;
	buf[12] = XA_CPU_1C_LEN + 4;
	//20260826 P1-1: use file_buf param so the 5 recovery blocks actually roll back (NULL keeps old behavior)
	memcpy(&buf[13], (file_buf == NULL) ? cpu_1c_data : file_buf, XA_CPU_1C_LEN);
	memcpy(buf, cpurandom, 8);
	buf[5 + 8 + 16] = 0x80;	
	//cal mac
	memset(factor, 0x00, 16);
	memcpy(factor, &cpu_05_data[4], 4);

	if((ret = cpu_cal_protect_mac(xa_metro_psam_index, factor, 8, "\x23\x01", buf, 5 + 8 + XA_CPU_1C_LEN + 3, des)) != 0)
	{
		PRINTK("line mac return %d\n", ret);
		return  CE_READ;
	}
	memcpy(&buf[5 + 8 + 16], des, 4);
	
#ifdef DEBUG_PRINT
	PRINTK("update1C:");
	for(i = 8; i < 5 + 16 + 4 + 8; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x15", 2);
	ret = mifpro_apdu(&buf[8], 5 + XA_CPU_1C_LEN + 4, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x16", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))	
	{
		return CE_WRITE;
	}
	return 0;
}

char xa_update_file_1d(unsigned char *file_buf, unsigned char *out_buf)
{
int ret, i;
unsigned char buf[100], cpurandom[8];
unsigned char cpubuf[100], Le;
unsigned char chCode, chRejectCode;
unsigned char factor[20], des[80], deslen, time_bcd[7];
unsigned short cpulen;
	//
	//if((chCode = CPU_VerifyPIN("\x31\x32\x33\x34\x35\x36", 6, out_buf)) != 0)
	//	return chCode;
	//udpate 17 file
	memcpy(buf, "\x00\x84\x00\x00\x04", 5);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		PRINTK("get cpu card random failure\n");
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("get random :");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x13", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_READ;
	}
	memset(cpurandom, 0x00, 8);
	memcpy(cpurandom, cpubuf, 4);
	memcpy(out_buf, "\x44\x14", 2);
	memset(buf, 0x00, 80);
	memcpy(&buf[8], "\x04\xd6\x00\x00", 4);
	buf[10] = 0x1D | 0x80;
	buf[12] = XA_CPU_1D_LEN + 4;
	//20260826 P1-1: use file_buf param so the 5 recovery blocks actually roll back (NULL keeps old behavior)
	memcpy(&buf[13], (file_buf == NULL) ? cpu_1d_data : file_buf, XA_CPU_1D_LEN);
	memcpy(buf, cpurandom, 8);
	buf[5 + 8 + 16] = 0x80;	
	//cal mac
	memset(factor, 0x00, 16);
	memcpy(factor, &cpu_05_data[4], 4);

	if((ret = cpu_cal_protect_mac(xa_metro_psam_index, factor, 8, "\x23\x01", buf, 5 + 8 + XA_CPU_1D_LEN + 3, des)) != 0)
	{
		PRINTK("line mac return %d\n", ret);
		return  CE_READ;
	}
	memcpy(&buf[5 + 8 + 16], des, 4);
	
#ifdef DEBUG_PRINT
	PRINTK("update1D:");
	for(i = 8; i < 5 + 16 + 4 + 8; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x15", 2);
	ret = mifpro_apdu(&buf[8], 5 + XA_CPU_1D_LEN + 4, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_WRITE;
	}
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\x44\x16", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0))	
	{
		return CE_WRITE;
	}
	return 0;
}

char xa_CPU_active(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
int ret, i;
unsigned char buf[100], start_timebcd[7];
unsigned char cpubuf[100], cpulen, Le;
unsigned char cnt;
unsigned char chCode, chRejectCode;
unsigned long lngHisecond1;

	*out_len = 4;
	memcpy(out_buf, "\x53\x3c\x00\x01", 4);
	//
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[7]);
	//
	get_degrade_mode(tpCPU.curstation);
	//test mode
	memcpy(out_buf, "\x32\x05", 2);
	if((chCode = CPU_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//card status
	memcpy(out_buf, "\x53\x3c\x00\x02", 4);
	//select file
	if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
		return chCode;
	memcpy(sfi_bak, "\x3f\x01", 2);
	//extern auth
	memcpy(out_buf, "\x53\x3c\x00\x0e", 4);
	if((chCode = CPU_externauth(0, xa_metro_psam_index, cpu_05_data, out_buf)) != 0)
		return chCode;
	//read blance and file 15
	memcpy(out_buf, "\x53\x3c\x00\x10", 4);
	if((chCode = CPU_GetFiles15(out_buf)) != 0)
	{
		return chCode;
	}
	tpCPU.tranamount = 0;

	memcpy(out_buf, "\x53\x3c\x00\x11", 4);
	switch(tpfile05.productCategory)
	{
	case XA_FEETYPE_VALUE:			//value
	case 0:
		//ticket definition
		memcpy(out_buf, "\x53\x3c\x00\x07", 4);
		if((chCode = CPU_TellSysCard(tpfile05.purseId)) != 0)
		{
			return chCode;
		}
		//read value
		memcpy(out_buf, "\x53\x3c\x00\x20", 4);
		if((chCode = CPU_GetFiles1b(out_buf)) != 0)
			return chCode;
		//
		if(tpfile1b.activated == 1)
			return CE_LOCKED_TICKET;
		xa_daytodate(tpfile05.cardBaseDateTime, tpfile1b.validityStartDate, &lngHisecond1, &start_timebcd[60]);
		return xa_CPU_active_dis(start_timebcd, out_buf, out_len);
	case XA_FEETYPE_TIMES:			//rides
		//ticket definition
		memcpy(out_buf, "\x53\x3c\x00\x07", 4);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		//read the multiride
		memcpy(out_buf, "\x53\x3c\x00\x20", 4);
		if((chCode = CPU_GetFiles1c(out_buf)) != 0)
			return chCode;
		//
		if(tpfile1c.activated == 1)
			return CE_LOCKED_TICKET;
		xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1c.validityStartDateTime, &lngHisecond1);
		return xa_CPU_active_cnt(start_timebcd, out_buf, out_len);
	case XA_FEETYPE_PERIOD:		//period
		//ticket definition
		memcpy(out_buf, "\x53\x3c\x00\x07", 4);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		//read the period
		memcpy(out_buf, "\x53\x3c\x00\x20", 4);
		if((chCode = CPU_GetFiles1d(out_buf)) != 0)
			return chCode;
		//
		if(tpfile1d.activated == 1)
			return CE_LOCKED_TICKET;
		xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1d.validityStartDateTime, &lngHisecond1);
		return xa_CPU_active_em(start_timebcd, out_buf, out_len);
	default:
		return CE_NON_FEETYPE;
	}
}


char xa_CPU_active_cnt(unsigned char *start_timebcd, unsigned char *out_buf, unsigned short *out_len)
{
unsigned long lngHisecond1;
unsigned char chCode;

	memcpy(out_buf, "\x53\x3c\x03\x21", 4);
	if(memcmp(tpCPU.time_bcd, start_timebcd, 4) > 0)
	{
		xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile1c.validityStartDateTime);
		//
		xaFile1C._File1C.validityStartDateTime_1 = (tpfile1c.validityStartDateTime >> 16) & 0x3F;
		xaFile1C._File1C.validityStartDateTime_2 = (tpfile1c.validityStartDateTime >> 8) & 0xFF;
		xaFile1C._File1C.validityStartDateTime = tpfile1c.validityStartDateTime & 0xFF;
		memcpy(cpu_1c_data, xaFile1C.buff, 16);
		//if(0 != (chCode = xa_update_file_1c(cpu_1c_data, out_buf)))
		//	return chCode;
#ifdef TESTLOG2022
			TestLog(buf2023,"CPU_entry_active_time error:",1);
			TestLog(cpu_1c_data,"FF:",16);
#endif
		return CE_EXPIREDDATE;
	}else
	{
		//memset();
		lngHisecond1 = tpCPU.lowsecond;
		memcpy(start_timebcd, tpCPU.time_bcd, 7);
		xa_localtimeToMinute(start_timebcd, tpfile05.cardBaseDateTime, &tpfile1c.validityStartDateTime);
		//
		xaFile1C._File1C.activated = tpfile1c.activated = 1;
		xaFile1C._File1C.validityStartDateTime_1 = (tpfile1c.validityStartDateTime >> 16) & 0x3F;
		xaFile1C._File1C.validityStartDateTime_2 = (tpfile1c.validityStartDateTime >> 8) & 0xFF;
		xaFile1C._File1C.validityStartDateTime = tpfile1c.validityStartDateTime & 0xFF;
		memcpy(cpu_1c_data, xaFile1C.buff, 16);
#ifdef TESTLOG2022
			TestLog(buf2023,"CPU_entry_active_writeback:",1);
			TestLog(cpu_1c_data,"FF:",16);
#endif
		memcpy(out_buf, "\x53\x3c\x03\x22", 4);

		if(0 != (chCode = xa_update_file_1c(cpu_1c_data, out_buf)))
			return chCode;
	}
	*out_len = 1;
	out_buf[0] = reader_status = XA_RW_IDLE;
	return CE_OK;
}

char xa_CPU_active_em(unsigned char *start_timebcd, unsigned char *out_buf, unsigned short *out_len)
{
unsigned long lngHisecond1;
unsigned char chCode;

	memcpy(out_buf, "\x32\x41", 2);
	if(memcmp(tpCPU.time_bcd, start_timebcd, 4) > 0)
	{
		xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile1d.validityStartDateTime);
		xaFile1C._File1C.validityStartDateTime_1 = (tpfile1d.validityStartDateTime >> 16) & 0x3F;
		xaFile1C._File1C.validityStartDateTime_2 = (tpfile1d.validityStartDateTime >> 8) & 0xFF;
		xaFile1C._File1C.validityStartDateTime = tpfile1d.validityStartDateTime & 0xFF;
		memcpy(cpu_1d_data, xaFile1C.buff, 16);
#ifdef TESTLOG2022
		TestLog(buf2023,"CPU_entry_active_em_time error:",1);
		TestLog(cpu_1d_data,"FF:",16);
#endif
		//if(0 != (chCode = xa_update_file_1d(cpu_1d_data, out_buf)))
		//	return chCode;
		return CE_EXPIREDDATE;
	}else
	{
		lngHisecond1 = tpCPU.lowsecond;
		memcpy(start_timebcd, tpCPU.time_bcd, 7);
		xa_localtimeToMinute(start_timebcd, tpfile05.cardBaseDateTime, &tpfile1d.validityStartDateTime);
		//
		memcpy(out_buf, "\x32\x42", 2);
		xaFile1C._File1C.activated = tpfile1d.activated = 1;
		xaFile1C._File1C.validityStartDateTime_1 = (tpfile1d.validityStartDateTime >> 16) & 0x3F;
		xaFile1C._File1C.validityStartDateTime_2 = (tpfile1d.validityStartDateTime >> 8) & 0xFF;
		xaFile1C._File1C.validityStartDateTime = tpfile1d.validityStartDateTime & 0xFF;
		memcpy(cpu_1d_data, xaFile1C.buff, 16);
#ifdef TESTLOG2022
			TestLog(buf2023,"CPU_entry_active_em_writeback:",1);
			TestLog(cpu_1d_data,"FF:",16);
#endif		
#ifdef DEBUG_PRINT_EM
	PRINTK("file 1d write back len:%02x,%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
			16, cpu_1d_data[0], cpu_1d_data[1], cpu_1d_data[2], cpu_1d_data[3],	cpu_1d_data[4], cpu_1d_data[5], cpu_1d_data[6], cpu_1d_data[7], cpu_1d_data[8], cpu_1d_data[9], cpu_1d_data[10], cpu_1d_data[11], cpu_1d_data[12], cpu_1d_data[13], cpu_1d_data[14], cpu_1d_data[15]);
#endif
		if(0 != (chCode = xa_update_file_1d(cpu_1d_data, out_buf)))
			return chCode;
	}
	*out_len = 1;
	out_buf[0] = reader_status = XA_RW_IDLE;
	return CE_OK;
}

char xa_CPU_active_dis(unsigned char *start_timebcd, unsigned char *out_buf, unsigned short *out_len)
{
unsigned long lngHisecond1;
unsigned char chCode;

	memcpy(out_buf, "\x32\x41", 2);
	if(memcmp(tpCPU.time_bcd, start_timebcd, 4) > 0)
		return CE_EXPIREDDATE;
	else
	{
		lngHisecond1 = tpCPU.lowsecond;
		memcpy(start_timebcd, tpCPU.time_bcd, 7);
		tpfile1b.validityStartDate = xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &lngHisecond1);
		//
		memcpy(out_buf, "\x32\x42", 2);
		xaFile1B._File1B.activated = tpfile1b.activated = 1;
		xaFile1B._File1B.validityStartDate_1 = (tpfile1b.validityStartDate >> 4) & 0xFF;
		xaFile1B._File1B.validityStartDate = tpfile1b.validityStartDate & 0xF;
		memcpy(cpu_1b_data, xaFile1B.buff, 16);
		if(0 != (chCode = xa_update_file_1b(cpu_1b_data, out_buf)))
			return chCode;
	}
	*out_len = 1;
	out_buf[0] = reader_status = XA_RW_IDLE;
	return CE_OK;
}


char xa_CPU_defer(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned long lngHisecond1;
unsigned char chCode;
unsigned char buf[100], start_timebcd[7];

	*out_len = 4;
	memcpy(out_buf, "\x53\x3d\x00\x01", 4);
	//
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	tpCPU.lowsecond = timestr2long(&cmd_buf[7]);
	//
	get_degrade_mode(tpCPU.curstation);
	//test mode
	memcpy(out_buf, "\x53\x3d\x00\x05", 4);
	if((chCode = CPU_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//card status
	memcpy(out_buf, "\x53\x3d\x00\x02", 4);
	//select file
	if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
		return chCode;
	memcpy(sfi_bak, "\x3f\x01", 2);
	//extern auth
	memcpy(out_buf, "\x53\x3d\x00\x0e", 4);
	if((chCode = CPU_externauth(0, xa_metro_psam_index, cpu_05_data, out_buf)) != 0)
		return chCode;
	//read blance and file 15
	memcpy(out_buf, "\x53\x3d\x00\x10", 4);
	if((chCode = CPU_GetFiles15(out_buf)) != 0)
	{
		return chCode;
	}
	tpCPU.tranamount = 0;

	memcpy(out_buf, "\x53\x3d\x00\x11", 4);
	switch(tpfile05.productCategory)
	{
	case XA_FEETYPE_VALUE:			//value
	case 0:
			//ticket definition
		memcpy(out_buf, "\x53\x3d\x00\x07", 4);
		if((chCode = CPU_TellSysCard(tpfile05.purseId)) != 0)
		{
			return chCode;
		}
		//read value
		memcpy(out_buf, "\x53\x3c\x00\x20", 4);
		if((chCode = CPU_GetFiles1b(out_buf)) != 0)
			return chCode;
		//
		if(tpfile1b.activated == 1)
			return CE_LOCKED_TICKET;
		xa_daytodate(tpfile05.cardBaseDateTime, tpfile1b.validityStartDate, &lngHisecond1, &start_timebcd[60]);
		
	case XA_FEETYPE_TIMES:			//rides
		//ticket definition
		memcpy(out_buf, "\x53\x3c\x00\x07", 4);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		//read the multiride
		memcpy(out_buf, "\x53\x3c\x00\x20", 4);
		if((chCode = CPU_GetFiles1c(out_buf)) != 0)
			return chCode;
		//
		if(tpfile1c.activated == 1)
			return CE_LOCKED_TICKET;
		xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1c.validityStartDateTime, &lngHisecond1);
		memcpy(out_buf, "\x53\x3d\x03\x21", 4);
		if(memcmp(tpCPU.time_bcd, start_timebcd, 4) > 0)
		{
			//20260826 P0-5: remove the "rewrite card then reject" defer date change.
			//Old code rewrote validityStartDateTime to the current time and wrote file1C here, then
			//fell through into the PERIOD branch and finally returned CE_NON_FEETYPE - card rewritten,
			//transaction rejected. Defer values must come from the backend as a complete correct 1C
			//record (incl. checksum, see P1-4) written via the repair command; the TP no longer
			//fabricates them on the spot.
			PRINTK("defer TIMES expired, no card write, logicID %08lx\n", ByteToLong(NULL, &cpu_05_data[4]));
			return CE_NON_FEETYPE;
		}else
		{
			return CE_NONACTIVED;
		}
		
	case XA_FEETYPE_PERIOD:		//period
		//ticket definition
		memcpy(out_buf, "\x53\x3c\x00\x07", 4);
		if((chCode = CPU_TellSysCard(tpfile05.productId)) != 0)
		{
			return chCode;
		}
		//read the period
		memcpy(out_buf, "\x53\x3c\x00\x20", 4);
		if((chCode = CPU_GetFiles1d(out_buf)) != 0)
			return chCode;
		//
		if(tpfile1d.activated == 1)
			return CE_LOCKED_TICKET;
		xa_MinuteTolocaltime(&start_timebcd[0], tpfile05.cardBaseDateTime, tpfile1d.validityStartDateTime, &lngHisecond1);
		
	default:
		return CE_NON_FEETYPE;
	}

	*out_len = 1;
	out_buf[0] = reader_status = XA_RW_IDLE;
	return CE_OK;
}

//分段1：独立黑名单卡参�????
//分段2：区段黑名单卡参�????
char check_metro_Black_Lock(unsigned long logicID, unsigned char blnBlock, unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned long lngBlacklist;
long lngCSCLowIndex, lngCSCHighIndex, lngCSCMidIndex;
unsigned char bytBlackCSCId[12], bytParaCSCId[12], blnBlacklist, des[8], blnSectionBlacklist;
int ret, i;
unsigned char 	buf[100], cpubuf[100], cpulen, cpurandom[8], factor[16], chCode;
unsigned short	cnt, cnt2;
unsigned long 	lngtxnDatetime;
unsigned long lngstation;

    blnBlacklist = blnSectionBlacklist = 0xFF;
    //use the binary method to find the BLACK No
    lngCSCLowIndex = 0;
    //20260826 P0-1: upper bound must be blacknum-1 (valid index range [0, blacknum-1]).
    //Old value blacknum let mid read past the list end into ghost slots when the card id is
    //larger than every entry; combined with P0-2's non-reset buffer this falsely matched and
    //silently locked normal cards (root of the 0x03/0x08 gates).
    lngCSCHighIndex = tpBlacklist1104.CardBlack_val.blacknum - 1;
//    PRINTK("lowindex %08x highindex %08x\n", lngCSCLowIndex, lngCSCHighIndex);
    if(tpBlacklist1104.CardBlack_val.blacknum != 0)
    {
//	    if(tpBlacklist1104.CardBlack_val.blacknum < 5)
//	    {
//	    	for(lngCSCLowIndex = 0; lngCSCLowIndex < lngCSCHighIndex; lngCSCLowIndex++)
//	    	{
//	    		if(tpBlacklist1104.CardBlack_val.CardBlacklist_val[lngCSCLowIndex].CardID == logicID)
//	    		{
//		            blnBlacklist = 0;
//		            tpfile15.cardStatus = tpBlacklist1104.CardBlack_val.CardBlacklist_val[lngCSCMidIndex].cardStatusCode;
//	    			break;
//	    		}
//	    	}
//	    	lngCSCLowIndex = lngCSCHighIndex + 1;
//	    }
	    //do
	    while(lngCSCLowIndex <= lngCSCHighIndex)
	    {
		    lngCSCMidIndex = (lngCSCLowIndex + lngCSCHighIndex) / 2;
		    //PRINTK("lowindex %08x midindex %08x highindex %08x\n", lngCSCLowIndex, lngCSCMidIndex, lngCSCHighIndex);
	        if(tpBlacklist1104.CardBlack_val.CardBlacklist_val[lngCSCMidIndex].CardID > logicID)
	        //要检索的卡号在前半部�????
	            lngCSCHighIndex = lngCSCMidIndex - 1;
	        else if(tpBlacklist1104.CardBlack_val.CardBlacklist_val[lngCSCMidIndex].CardID < logicID)
	        //要检索的卡号在后半部�????
	            lngCSCLowIndex = lngCSCMidIndex + 1;
	        else if(tpBlacklist1104.CardBlack_val.CardBlacklist_val[lngCSCMidIndex].CardID == logicID)
	        {
	        //
	            blnBlacklist = 0;
	            tpfile15.cardStatus = tpBlacklist1104.CardBlack_val.CardBlacklist_val[lngCSCMidIndex].cardStatusCode;
	            //20260826 P0-3: single-card hit must leave a log (card id + status), for backend attribution
	            PRINTK("CARD-BLACK-HIT logicID %08lx status %02x\n", logicID, tpfile15.cardStatus);
	            break;
	        }
	    }
	}
#ifdef	DEBUG_PRINT
	PRINTK("logicid %08x in %08x result %02x\n", logicID, tpBlacklist1104.CardBlack_val.blacknum, blnBlacklist);
#endif
    //use the binary method to find the BLACK No
    lngCSCLowIndex = 0;
    lngCSCHighIndex = tpBlacklist1104.SectionCardBlack_val.sectionnum;
    //PRINTK("lowindex %08x highindex %08x\n", lngCSCLowIndex, lngCSCHighIndex);
    if((tpBlacklist1104.SectionCardBlack_val.sectionnum != 0) && (blnBlacklist))
    {
    	for(lngCSCMidIndex = 0; lngCSCMidIndex < lngCSCHighIndex; lngCSCMidIndex++)
    	{
    		//20260826 P0-3: skip invalid ranges (Start==0 or Start>End), they would lock a whole id block
    		if((tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].StartCardID == 0)
    			|| (tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].StartCardID >
    			    tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].EndCardID))
    			continue;
    		if((logicID >= tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].StartCardID)
    			&& (logicID <= tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].EndCardID))
    		{
    			blnSectionBlacklist = 0;
    			tpfile15.cardStatus = tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].cardStatusCode;
    			//20260826 P0-3: section hit must leave a log (range + card id + status) for backend list-data audit
    			PRINTK("SECTION-BLACK-HIT logicID %08lx range %08lx-%08lx status %02x\n",
    				logicID,
    				(unsigned long)tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].StartCardID,
    				(unsigned long)tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].EndCardID,
    				tpBlacklist1104.SectionCardBlack_val.SectionCardBlacklist_val[lngCSCMidIndex].cardStatusCode);
    			break;
    		}
    	}
	}

#ifdef	DEBUG_PRINT
	PRINTK("in %08x result %02x\n", tpBlacklist1104.SectionCardBlack_val.sectionnum, blnSectionBlacklist);
#endif
	//
    if(blnBlacklist && blnSectionBlacklist)
    {//not blacklist
    	return 0;
    }
    //not block card only display message
    //20260826 P0-4: every hit must leave a log - card id, hit mechanism (single/section), status, station
    if(blnBlock)
    {
    	PRINTK("BLACK-HIT-NOWRITE logicID %08lx single %02x section %02x status %02x station %02x%02x\n",
    		logicID, blnBlacklist, blnSectionBlacklist, tpfile15.cardStatus,
    		tpCPU.curstation[0], tpCPU.curstation[1]);
    	return CE_BLACKLIST;
    }
	//
	//20260826 P0-4: write-lock path log (entry now passes 0xff; this path only runs if explicit
	//authorization is restored later)
	PRINTK("BLACK-LOCK-WRITE logicID %08lx single %02x section %02x status %02x station %02x%02x\n",
		logicID, blnBlacklist, blnSectionBlacklist, tpfile15.cardStatus,
		tpCPU.curstation[0], tpCPU.curstation[1]);
	tpTxnCardBlock.SysComHdr_val.formatVersion = toMoto(1);
	tpTxnCardBlock.SysComHdr_val.txnDateTime = toMoto(tpCPU.lowsecond + TIME2000 - ZONE8);
	tpTxnCardBlock.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]);
	tpTxnCardBlock.SysComHdr_val.udType = toMoto(1);
	tpTxnCardBlock.SysComHdr_val.udSubtype = toMoto(6);
	//
	tpTxnCardBlock.SysCardCom_val.cardissuerId = toMoto(1);
	tpTxnCardBlock.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_CPU);
	tpTxnCardBlock.SysCardCom_val.cardLifeCycleCount = 0xffff0000;
	tpTxnCardBlock.SysCardCom_val.cardActionSequenceNumber = 0;
	tpTxnCardBlock.SysCardCom_val.cardSerialNumber = (*(long *)&cpu_05_data[4]);
	//
	tpTxnCardBlock.reasonCode = toMoto(tpfile15.cardStatus);
	tpTxnCardBlock.SysSecurityHdr_val.keyVersion = toMoto(tpfile05.keySetNumber);
	//
	lngstation = 0x09000000 + (tpCPU.curstation[0] << 8) + tpCPU.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpfile15.lastLocation)))
		return chCode;
	
   	xaFile15._File15.cardStatus = tpfile15.cardStatus;
   	xa_localtimeToMinute(tpCPU.time_bcd, tpfile05.cardBaseDateTime, &tpfile15.startDateTime);
   	xaFile15._File15.startDateTime_1 = (tpfile15.startDateTime >> 16) & 0x3F;
   	xaFile15._File15.startDateTime_2 = (tpfile15.startDateTime >> 8) & 0xFF;
	xaFile15._File15.startDateTime = tpfile15.startDateTime & 0xFF;
	xaFile15._File15.lastLocation_1 = (tpfile15.lastLocation >> 4) & 0xFF;
	xaFile15._File15.lastLocation = tpfile15.lastLocation & 0xF;

	memcpy(cpu_15_data, xaFile15.buff, 13);
	if(0 != (chCode = xa_update_file_15(NULL, out_buf)))
		return chCode;
	
	//append record for 0017
	lngtxnDatetime = tpCPU.lowsecond + TIME2000 - ZONE8;
	xaFile17._File17.dateTime_1 = (lngtxnDatetime >> 24) & 0xFF;
	xaFile17._File17.dateTime_2 = (lngtxnDatetime >> 16) & 0xFF;
	xaFile17._File17.dateTime_3 = (lngtxnDatetime >> 8) & 0xFF;
	xaFile17._File17.dateTime = lngtxnDatetime & 0xFF;
	xaFile17._File17.serviceProviderId = tpCmdInit.participantid & 0xFF;
	xaFile17._File17.productIssuerId = tpTicketDef.ProductIssuer & 0x1F;
	xaFile17._File17.category = tpfile05.productCategory;
	xaFile17._File17.paymentMethod = 2;
	xaFile17._File17.transactionType = 7;
	xaFile17._File17.location_1 = (tpfile15.lastLocation >> 10) & 0x3;
	xaFile17._File17.location_2 = (tpfile15.lastLocation >> 2) & 0xFF;
	xaFile17._File17.location = tpfile15.lastLocation & 0x3;
	xaFile17._File17.productTypeId = tpfile05.productId;
	xaFile17._File17.value_1 = 0;
	xaFile17._File17.value_2 = 0;
	xaFile17._File17.value = 0;
	xaFile17._File17.remainingValue_1 = (tpCPU.balance >> 12) & 0x1F;
	xaFile17._File17.remainingValue_2 = (tpCPU.balance >> 4) & 0xFF;
	xaFile17._File17.remainingValue = tpCPU.balance & 0xF;
	xaFile17._File17.padding_1 = 0;
	xaFile17._File17.padding_2 = 0;
	xaFile17._File17.Padding = 0;
	memcpy(cpu_17_data, xaFile17.buff, 16);
	xa_update_file_17(NULL, out_buf);
	
	//generate the Record
	cnt2 = cnt = sizeof(TxnCardBlock_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnCardBlock.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnCardBlock.SysComHdr_val.reservedField, sh_mac_len - 40);

	g_sha1txnsn = tpTxnCardBlock.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_MCPU_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnCardBlock.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;
	ch_mac_sel = 4;
	sem_init(&g_samreturn, 0, 0);
	sem_post(&g_samcalwait);
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	cnt = sizeof(TxnCardBlock_t);
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnCardBlock.AFCHead_val.operatorid, cnt);
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	reader_status = XA_RW_RECORD;
	
	return CE_BLACKLIST;
}

/*
Metro ISAM External Authentication
*/
char xa_ext_auth_init(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char buf[100], sambuf[100], sambytes;
unsigned char chCode;
unsigned char factor[8], mac[8], maclen;
long 	i;	
	
#ifdef	DEBUG_PRINT
	PRINTK("external authentication-init command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("time %02x%02x-%02x-%02x %02x:%02x:%02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	PRINTK("SN:%02x%02x\n", cmd_buf[13], cmd_buf[14]);
#endif
	*out_len = 2;
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	//auth log-in or log-out
	//10.0.0.64
	//8188
	memcpy(buf, "\x00\x84\x00\x00\x04", 5);
	if(0 != sam_apdu(xa_metro_psam_index, buf, 5, sambuf, &sambytes, 0, 0))
		return CE_NOMETROSAM;
	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
		return CE_NOMETROSAM;
	//authentication requests
	memset(out_buf, 0x00, 34);
	out_buf[0] = 0x01;
	//8bytes random
	memcpy(&out_buf[1], sambuf, 4);
	memset(&out_buf[5], 0x00, 4);
	//6bytes ISAM
	memcpy(&out_buf[9], ch_cpu20_psam_id, 6);
	//version
	out_buf[15] = sam_version;
	//60C9CBB4
//	memcpy(buf, "\x00\xa4\x00\x00\x02\x3f\x00", 7);
//	if(0 != sam_apdu(xa_metro_psam_index, buf, 7, sambuf, &sambytes, 0, 0))
//		return CE_NOMETROSAM;
//	PRINTK("select 3f00 len %02x:", sambytes);
//	for(i = 0; i < sambytes; i++)
//		PRINTK("%02x\n", sambuf[i]);
//	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
//		return CE_NOMETROSAM;
	//018068774D00000000 000000000697 01 49B2CB51
	//memcpy(out_buf, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x91\xF2\x48\x54\x00\x00\x00\x00\x00\x00\x00\x00\x07\x00\x01\x80", 25);
//	memcpy(out_buf, "\xff\xff\xff\xff\xff\xff\xff\xff\x01\x91\xF2\x48\x54\x00\x00\x00\x00\x00\x00\x00\x00\x07\x00\x01\x80", 25);
//	memset(factor, 0xff, 8);
//	memcpy(factor, "\x00\x00\x00\x00\x00\x00\x07\x00", 8);
//	PRINTK("metro psam index %02x\n", xa_metro_psam_index);
//	if(0 != (chCode = cpu_cal_dcmk(xa_metro_psam_index, "\x08\x01", factor, 0, 5, out_buf, 32, mac, &maclen)))
//		return chCode;
	CmdWatchCalMac(16, out_buf, "\x00\x00\x00\x00\x00\x00\x00\x00", Metro_Transfer_key, mac, 0xff);
	*out_len = 20;
	memcpy(&out_buf[16], mac, 4);
#ifdef DEBUG_PRINT
	PRINTK("external init: %04x\n", *out_len);
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	return CE_OK;
}

char xa_ext_auth(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char buf[100], sambuf[100], sambytes;
unsigned char chCode;
long 	i;	
short cnt;
FILE	*fl;

#ifdef	DEBUG_PRINT
	PRINTK("longin command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	memcpy(&cnt, &cmd_buf[1], 2);
	PRINTK("external auth: %04x\n", cnt);
	for(i = 0; i < cnt; i++)
		PRINTK("%02x", cmd_buf[i + 6]);
	PRINTK("\n");
#endif
	*out_len = 4;
	switch(cmd_buf[6])
	{
	case 2:
		break;
	case 3:		//数据包为ＭＡＣ错响应
		//break;
	case 4:		//数据包为黑卡响应
		//break;
	case 5:		//数据包为认证结果
		//break;
	case 6:		//数据包为认证结果�????
	default:
		return CE_MACERR;
	}
	//extern authorization
	memcpy(buf, "\x00\x82\x00\x01\x08", 5);
	memcpy(&buf[5], &cmd_buf[7], 8);
	if(0 != sam_apdu(xa_metro_psam_index, buf, 13, sambuf, &sambytes, 0, 0))
		return CE_NOMETROSAM;
	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
		return CE_NOMETROSAM;
#ifdef DEBUG_PRINT
#endif	
	
	*out_len = 11;
	//auth amount
	memset(out_buf, 0x00, 4);
	//auth time
	memcpy(&out_buf[4], tpCPU.time_bcd, 7);
	return CE_OK;
}
