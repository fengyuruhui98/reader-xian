#include "xa_m1_operation.h"
#include "linux2440lib.h"
#include "xa_cpu20_operation.h"
#include "hh_cpu_operation.h"
#include "xa_error_code.h"
#include "xdr_file_manage.h"
#include "xa_operation.h"

unsigned char m1_block[128][16];
/*
function:
	*key:	M1 keyA or KeyB
	keyab: 	0 if *key is keyA , or non-zero
	type:	need request card again
	sectno: sector number
	block:	high-niddle stand for read all blocks and max block number is low-niddle, otherwise low-niddle stand for special block
*/
char sector_read(unsigned char *key, unsigned char keyab, unsigned char type, unsigned char sectno, unsigned char block, unsigned char *out_buf)
{
unsigned char chcode;
long i, j;
unsigned char cnt, buf[200];

	if(sectno > 63)
		return 0xff;
	
	cnt = 0;
	if(type == 1)
	{
loop:
		if(cnt > 3) return 0xff;
		cnt++;
		if(mcml_request2(PICC_REQALL, buf) != 0)
		{
			printf("requeset failure\n");
			goto loop;
		}
		if(0 != mcml_anticoll(buf))
			goto loop;
		if(memcmp(buf, &ch_m1_phyical_id[4], 5) != 0)
			goto loop;
		if(mcml_select(&ch_m1_phyical_id[4], (unsigned char *)&i) != 0)
		{
			printf("select card falure %02x %02x %02x %02x\n", ch_m1_phyical_id[4], ch_m1_phyical_id[5], ch_m1_phyical_id[6], ch_m1_phyical_id[7]);
			goto loop;
		}
		printf("request again success\n");
	}
	//
	if(0 != (chcode = mcml_load_key(0, keyab, sectno, key)))
	{
		printf("mcml load key failure\n");
		return chcode;
	}
	if(0 != (chcode = mcml_authentication(0, keyab, sectno)))
	{
		printf("mcml authentication failure\n");
		return chcode;
	}
	//only read special block
	if((block & 0xf0) == 0)
	{
		if(0 != (chcode = mcml_read(sectno * 4 + (block & 0xf), m1_block[sectno * 4 + (block & 0xf)])))
		{
			printf("mcml read failure %02x\n", chcode);
			return chcode;
		}
#ifdef DEBUG_PRINT
		printf("sect %d block %d:", sectno, sectno * 4 + (block & 0xf));
		for(j = 0; j < 16; j++)
			printf("%02x ", m1_block[sectno * 4 + (block & 0xf)][j]);
		printf("\n");
#endif
		return 0;
	}
	for(i = 0; i < (block & 0xf); i++)
	{
		if(0 != (chcode = mcml_read(sectno * 4 + i, m1_block[sectno * 4 + i])))
		{
			printf("mcml read failure %02x\n", chcode);
			return chcode;
		}
#ifdef DEBUG_PRINT
		printf("sect %d block %d:", sectno, sectno * 4 + i);
		for(j = 0; j < 16; j++)
			printf("%02x ", m1_block[sectno * 4 + i][j]);
		printf("\n");
#endif
	}
	
	return 0;
}

/*
function:
	auth:	need auth for write status
	*key:	keyB normal
	keyab:	zero stand for keyA
	blockno:write block number
	*in_data:16 bytes block written
*/
char block_write(unsigned char auth, unsigned char *key, unsigned char keyab, unsigned char blockno, unsigned char *in_data)
{
unsigned char chcode, i, j;
	
	if(blockno > 128)
		return 0xff;
	//write authoration
	if(auth)
	{
		if(0 != (chcode = mcml_load_key(0, 0x40, blockno / 4, key)))
		{
#ifdef DEBUG_PRINT
			printf("load key failure\n");
#endif
			return chcode;
		}
		if(0 != (chcode = mcml_authentication(0, 0x40, blockno / 4)))
		{
#ifdef DEBUG_PRINT
			printf("authenticate failure\n");
#endif
			return chcode;
		}
	}
	//
	if(0 != (chcode = mcml_write(blockno, in_data)))
	{
#ifdef DEBUG_PRINT
			printf("write data failure\n");
#endif
		return chcode;
	}
	
	return 0;
}

char cal_sh_m1_key()
{
unsigned char buf[200], sambuf[100], samlen;
char i;
	//cal multi sector keyA 
	//80 fc kid1 kid2 length + DATA(20 00，cardsn(4) B1[8]-[13](6) index)
	//card sn- 4: 06 A2 07 10
	//B1(8)-(13)- 6: 4F 00 A9 22 75 D6
	//B2：00 10 01 04 03 03 03 20 21 27 22 28 30 31 32 33
	//cal key A1、A2 、A3、A4、A7、A8
	
	memcpy(buf, "\x80\xfc\x00\x01\x12\x20\x00", 7); 
	//card sn
	memcpy(&buf[7], &ch_m1_phyical_id[4], 4);
	//issued sn low 2bytes and card mac
	memcpy(&buf[11], &m1_block[1][8], 6);
	//keya1
	buf[17] = m1_block[2][1];
	//keya2
	buf[18] = m1_block[2][2];
	//keya3
	buf[19] = m1_block[2][3];
	//keya4
	buf[20] = m1_block[2][4];
	//keya7
	buf[21] = m1_block[2][7];
	//keya8
	buf[22] = m1_block[2][8];
#ifdef DEBUG_1_PRINT
	printf("psam cal key:");
	for(i = 0; i < 5 + 18; i++) printf("%02x", buf[i]);
	printf("\n");
#endif
	if(sam_apdu(sh_psam_index, buf, 5 + 18, sambuf, &samlen, 0, 0) != 0)
	{
		printf("cal key failure\n");
		return -5;
	}
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(sh_psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
		/*if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
			return -7;
		memcpy(mac, sambuf, 4);
		return 0;*/
	}
	if(samlen != 0x24 + 2)
		return -6;
	
#ifdef DEBUG_1_PRINT
	printf(" key:");
	for(i = 0; i < 0x24 + 2; i = i + 6) 
		printf("%02x%02x%02x%02x%02x%02x ", sambuf[i], sambuf[i + 1], sambuf[i + 2], sambuf[i + 3], sambuf[i + 4], sambuf[i + 5]);
	printf("\n");
#endif
	//
	memcpy(m1_key[1], sambuf, 6);
	memcpy(m1_key[2], &sambuf[6], 6);
	memcpy(m1_key[3], &sambuf[12], 6);
	memcpy(m1_key[4], &sambuf[18], 6);
	memcpy(m1_key[5], m1_key[4], 6);
	memcpy(m1_key[6], m1_key[4], 6);
	memcpy(m1_key[7], &sambuf[24], 6);
	memcpy(m1_key[8], &sambuf[30], 6);

#ifdef DEBUG_1_PRINT
	printf(" key:");
	for(i = 0; i < 8; i++) 
		printf("%02x%02x%02x%02x%02x%02x \n", m1_key[i][0], m1_key[i][1], m1_key[i][2], m1_key[i][3], m1_key[i][4], m1_key[i][5]);
#endif
}

/*======================================================================
函数：
功能：PSAM初始化
========================================================================*/
int ResetSHSam(void)
{
unsigned char chret, retry, i;
unsigned char sambuf[257]; 
unsigned char sambytes;
unsigned char buf[257];
unsigned char inlen;
	
	i = 7;
	{
		if(sam_select(i) != 0)
		//	continue;
			printf("select suzhou metro sam error \n");
		//
		sam_set(i, SAM_ETU_93, 4);
		for(retry = 0; retry < 3; retry++)
		{
		  	if((chret = sam_atr(i, sambuf, &sambytes)) != 0)
		  	{
		  		printf("sz-metro sam atr return %02x\n", chret);
		    	continue;
		    }
		    //终端信息文件－终端机编号
		 	/*memcpy(buf,"\x00\xb0\x96\x00\x06",5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 8)) != 0)
		  	{
		  		printf("sam apdu read file 16 return %02x\n", chret);
			    continue;
	    	}	
		  	if(sambytes != 8)
		  	{
		  		printf("file 16 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			    continue;
	  		}*/
		  	memcpy(ch_sh_psam_id, sambuf, 6);
		  	printf("sh sam id :%02x %02x %02x %02x %02x %02x\n", ch_sh_psam_id[0], ch_sh_psam_id[1], ch_sh_psam_id[2], ch_sh_psam_id[3], 
		  			ch_sh_psam_id[4], ch_sh_psam_id[5]);
			
	  		memcpy(buf,"\x00\xa4\x00\x00\x02\x10\x03",7);
		  	if(sam_apdu(i, buf, 7, sambuf, &sambytes, 0, 2) != 0)
		  	{
		  		printf("sam apdu select file 1003 return %02x %02x\n", sambuf[0], sambuf[1]);
			    continue;
	    	}	
		  	if((sambuf[0] != 0x61) && (sambuf[0] != 0x90))
		  	{
			//    goto label_oldpsam;
				continue;
	  		}
	  		//终端信息文件－终端机编号
		  	memcpy(buf,"\x00\xb0\x97\x00\x01", 5);
	  		if(sam_apdu(i, buf, 5, sambuf, &sambytes, 0, 3) != 0)
	  		{
	  			printf("sam apdu read file 17 failure %02x %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
		    	continue;
		    }	
		    printf("sam apdu read file 17 return %02x %02x\n", sambytes, sambuf[0]);
		    if(sambytes != 3)
	  		{
		    	continue;
	  		}
			
			sh_psam_index = i;
			break;
		}
	}
}
/********************************************************
函数：lrc2_gen
功能：产生校验
*********************************************************/
unsigned char lrc_gen2(unsigned char *buf, unsigned short bytes)
{
unsigned short i;

	buf[bytes] = 0;
	for(i = 0; i < bytes; i++) 
		buf[bytes] ^= buf[i];

	return buf[bytes];
}

/********************************************************
函数：lrc2_chk
功能：检查校验是否正确
*********************************************************/
unsigned char lrc_chk2(unsigned char *in_buf, unsigned short in_len)
{
unsigned short i;
unsigned char lrc;

	lrc = 0;
	for(i = 0; i < in_len; i++) 
		lrc ^= in_buf[i];

	if(lrc != in_buf[in_len]) 
		return 0xff;
	
	return 0;
}

char sz_M1_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len)
{
int ret, i;
unsigned char buf[100];
unsigned char cpubuf[100], cpulen, Le;
unsigned char cnt;
unsigned char chCode, chRejectCode;
unsigned short	shyear;

#ifdef DEBUG_PRINT
unsigned char localtime[7];
	printf("\ncpu entry command is %02x and length is %02x:\n", cmd_buf[6], cmd_buf[5]);
	printf("current station is %02x%02x device id is %02x%02x%02x%02x\n", cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	printf("current station mode %02x device test mode %02x phycial ID %02x%02x%02x%02x %02x%02x%02x%02x chip type ??\n", 
		cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22]);
	
	printf("SN:%02x%02x%02x%02x\n", cmd_buf[23], cmd_buf[24], cmd_buf[25], cmd_buf[26]);
	LocalDateTime2BCD(&cmd_buf[27], localtime);
	printf("local date time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", localtime[0], localtime[1], localtime[2], localtime[3], localtime[4], localtime[5], localtime[6]);
#endif
	*out_len = 4;
	memcpy(out_buf, "\x46\x85\x00\x00", 4);
	//check whether rollback the last transation or not
	memcpy(tpCPU.curtime, &cmd_buf[27], 4);
	if(!(sz_localtimeToSecond(&cmd_buf[27], &tpCPU.hisecond, &tpCPU.lowsecond)))
		return CE_COMMAND;
	LocalDateTime2BCD(&cmd_buf[27], tpCPU.time_bcd);
	sz_localtimeToDay(&cmd_buf[27], &tpCPU.days, &tpCPU.midsecond);
	if(0 != (chCode = check_station_id(&cmd_buf[7])))
		return chCode;
	//
	memcpy(tpCPU.curstation, &cmd_buf[7], 2);
	memcpy(&tpCPU.curstation[2], &cmd_buf[11], 2);
	//read sector 9 get the valid date 
	memcpy(out_buf, "\x46\x86\x00\x00", 4);
	if(0 != (chCode = sector_read(m1_key[9], 0, 0, 9, 0, out_buf)))
		return CE_READ;
	//check the valid date
	ByteToShort(&shyear, m1_block[36]);
	buf[0] = bin2bcd(shyear / 100);
	buf[1] = bin2bcd(shyear % 100);
	buf[2] = bin2bcd(m1_block[36][2]);
	buf[3] = bin2bcd(m1_block[36][3]);
	memset(&buf[4], 0x00, 3);
	memcpy(out_buf, "\x46\x87\x00\x00", 4);
	if(!time_chk_valid(buf))
		return CE_EXPIREDDATE;
	memcpy(out_buf, "\x46\x88\x00\x00", 4);
	if(memcmp(tpCPU.time_bcd, buf, 4) > 0)
		return CE_EXPIREDDATE;
	//check the card type
	memcpy(out_buf, "\x46\x89\x00\x00", 4);
	if((chCode = CPU_TellSysCard(0x80 + m1_block[36][5])) != 0)
	{
		return chCode;
	}
	//read metro sector
	memcpy(out_buf, "\x46\x8a\x00\x00", 4);
	if(0 != (chCode = sector_read(m1_key[SZ_E_EDU_SECTOR], 1, 0, SZ_E_EDU_SECTOR, 0x13, out_buf)))
		return CE_READ;
	//metro entry/exit status
	if(m1_block[SZ_E_EDU_B2][0] == 0x01)
	{
		chCode = CE_FREE_UPDATE_ENTRY;
		goto label_refuse_to_entry;
	}
	//entry block S10B40
	memcpy(&m1_block[SZ_E_EDU_B0][0], tpCPU.curstation, 4);
	memcpy(&m1_block[SZ_E_EDU_B0][4], &tpCPU.time_bcd[1], 6);
	memset(&m1_block[SZ_E_EDU_B0][10], 0x00, 5);
	lrc_gen2(m1_block[SZ_E_EDU_B0], 16);
	memcpy(out_buf, "\x46\x8b\x00\x00", 4);
	if(0 != block_write(0, m1_key[SZ_E_EDU_SECTOR], 1, SZ_E_EDU_B0, m1_block[SZ_E_EDU_B0]))
		return CE_WRITE;
	//status block 66
	m1_block[SZ_E_EDU_B2][0] = 0x01;
	m1_block[SZ_E_EDU_B2][1] = 0x00;
	m1_block[SZ_E_EDU_B2][2] = 0x00;
	memset(&m1_block[SZ_E_EDU_B2][3], 0x00, 14);
	lrc_gen2(m1_block[SZ_E_EDU_B2], 16);
	memcpy(out_buf, "\x46\x8c\x00\x00", 4);
	if(0 != block_write(0, m1_key[SZ_E_EDU_SECTOR], 1, SZ_E_EDU_B2, m1_block[SZ_E_EDU_B2]))
		return CE_WRITE;
	//
	*out_len = 46;
	ch_sz_cpu_rollback = 0;
	//logic id-8
	memset(out_buf, 0x00, 8);
	memcpy(&out_buf[4], &ch_m1_phyical_id[4], 4);
	//ticket type-1
	out_buf[8] = 0x80 + m1_block[36][5];
	//sn-2
	memset(&out_buf[9], 0x00, 2);
	//transaction date-4
	memcpy(&out_buf[11], tpCPU.curtime, 4);
	//start valid date-2
	ShortToByte(0, &out_buf[15]);
	//end valid date -2
	ShortToByte(0, &out_buf[17]);
	//sam id-4
	memset(&out_buf[19], 0x00, 4);
	//balance
	LongToByte(0, &out_buf[23]);
	LongToByte(0, &out_buf[27]);
	//area flag
	out_buf[31] = 0;
	//zone id & section sta 1 & section sta 2
	memset(&out_buf[32], 0x00, 5);
	//tacs
	memset(&out_buf[37], 0x00, 4);
	out_buf[41] = XA_FEETYPE_VALUE;
	//sam sn
	memset(&out_buf[42], 0x00, 4);
	return CE_OK;
label_refuse_to_entry:	
	m1_block[SZ_E_EDU_B2][1] = CE_FREE_UPDATE_ENTRY;
	if(0 != block_write(0, m1_key[SZ_E_EDU_SECTOR], 1, SZ_E_EDU_B2, m1_block[SZ_E_EDU_B2]))
		return CE_WRITE;
	//
	*out_len = 47;
	ch_sz_cpu_rollback = 0;
	//logic id-8
	memset(out_buf, 0x00, 8);
	//ticket type-1
	out_buf[8] = 0x80 + m1_block[36][5];
	//sn-2
	memset(&out_buf[9], 0x00, 2);
	//transaction date-4
	memcpy(&out_buf[11], tpCPU.curtime, 4);
	//start valid date-2
	ShortToByte(tpCPU.startdate, &out_buf[15]);
	//end valid date -2
	ShortToByte(0, &out_buf[17]);
	//sam id-4
	memset(&out_buf[19], 0x00, 4);
	//balance-4
	LongToByte(0, &out_buf[23]);
	//times-4
	memset(&out_buf[27], 0x00, 4);
	//update station-2
	memset(&out_buf[31], 0x00, 2);
	//area flag-1
	out_buf[33] = 0;
	//zone id & section sta 1 & section sta 2-5
	memset(&out_buf[34], 0x00, 5);
	//tacs-4
	memset(&out_buf[39], 0x00, 4);
	//sam sn-4
	memset(&out_buf[43], 0x00, 4);
	return chCode;
}


char sz_M1_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len)
{
int ret, i;
unsigned char buf[100], entry_time[4];
unsigned char cpubuf[100], cpulen, chFare;
unsigned char cnt, chCode, chRejectCode;
unsigned short	shyear;

#ifdef DEBUG_PRINT
unsigned char localtime[7];
	printf("\ncpu exit command is %02x and length is %02x:\n", cmd_buf[6], cmd_buf[5]);
	printf("current station is %02x%02x device id is %02x%02x%02x%02x\n", cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	printf("current station mode %02x device test mode %02x phycial ID %02x%02x%02x%02x%02x%02x%02x%02x chip type ??\n", 
		cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22]);
	
	printf("SN:%02x%02x%02x%02x ", cmd_buf[23], cmd_buf[24], cmd_buf[25], cmd_buf[26]);
	LocalDateTime2BCD(&cmd_buf[27], localtime);
	printf("local date time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", localtime[0], localtime[1], localtime[2], localtime[3], localtime[4], localtime[5], localtime[6]);
#endif
	*out_len = 4;
	memcpy(out_buf, "\x47\x85\x00\x00", 4);
	memcpy(tpCPU.curtime, &cmd_buf[27], 4);
	if(!(sz_localtimeToSecond(&cmd_buf[27], &tpCPU.hisecond, &tpCPU.lowsecond)))
		return CE_COMMAND;
	LocalDateTime2BCD(&cmd_buf[27], tpCPU.time_bcd);
	sz_localtimeToDay(&cmd_buf[27], &tpCPU.days, &tpCPU.midsecond);
	if(0 != (chCode = check_station_id(&cmd_buf[7])))
		return chCode;
	
	//
	memcpy(tpCPU.curstation, &cmd_buf[7], 2);
	memcpy(&tpCPU.curstation[2], &cmd_buf[11], 2);
	//
	memcpy(out_buf, "\x47\x86\x00\x00", 4);
	if(0 != (chCode = sector_read(m1_key[9], 0, 0, 9, 0, out_buf)))
		return CE_READ;
	//check the valid date
	ByteToShort(&shyear, m1_block[36]);
	buf[0] = bin2bcd(shyear / 100);
	buf[1] = bin2bcd(shyear % 100);
	buf[2] = bin2bcd(m1_block[36][2]);
	buf[3] = bin2bcd(m1_block[36][3]);
	memset(&buf[4], 0x00, 3);
	memcpy(out_buf, "\x47\x87\x00\x00", 4);
	if(!time_chk_valid(buf))
		return CE_EXPIREDDATE;
	memcpy(out_buf, "\x47\x88\x00\x00", 4);
	if(memcmp(tpCPU.time_bcd, buf, 4) > 0)
		return CE_EXPIREDDATE;
	//check the card type
	memcpy(out_buf, "\x47\x89\x00\x00", 4);
	if((chCode = CPU_TellSysCard(0x80 + m1_block[36][5])) != 0)
	{
		return chCode;
	}
	//read metro sector
	memcpy(out_buf, "\x47\x8a\x00\x00", 4);
	if(0 != (chCode = sector_read(m1_key[SZ_E_EDU_SECTOR], 1, 0, SZ_E_EDU_SECTOR, 0x13, out_buf)))
		return CE_READ;
	//metro entry/exit status
	if(m1_block[SZ_E_EDU_B2][0] != 0x01)
	{
		chCode = CE_NO_ENTRY;
		goto label_refuse_to_exit;
	}
	//calculate the price
	if(0 != (chCode = cal_station_fare(m1_block[SZ_E_EDU_B0], &cmd_buf[7], &chFare)))
		return chCode;
	if(0 != (chCode = cal_fare_value(&cmd_buf[27], 0x80 + m1_block[36][5], chFare, 1, &tpCPU.tranamount)))
		return chCode;
	//check the over-time
	BCD2LocalDateTime(&m1_block[SZ_E_EDU_B0][3], entry_time);
	if(m1_block[SZ_E_EDU_B2][2] != 0x33)
	{
		memcpy(out_buf, "\x72\x11", 2);
		if((chCode = cal_overtime(entry_time, tpCPU.curtime, chFare, SZ_WAIVER_NOMAL)) != 0)
		{
			if(chCode == CE_OVERTIME)
				goto label_refuse_to_exit;
			else
				return chCode;
		}
	}
	//exit block 65
	memcpy(&m1_block[SZ_E_EDU_B1][0], tpCPU.curstation, 4);
	memcpy(&m1_block[SZ_E_EDU_B1][4], &tpCPU.time_bcd[1], 6);
	ShortToByte((unsigned short)tpCPU.tranamount, &m1_block[SZ_E_EDU_B1][10]);
	memset(&m1_block[SZ_E_EDU_B1][12], 0x00, 3);
	lrc_gen2(m1_block[SZ_E_EDU_B1], 15);
	memcpy(out_buf, "\x47\x8b\x00\x00", 4);
	if(0 != block_write(0, m1_key[SZ_E_EDU_SECTOR], 1, SZ_E_EDU_B1, m1_block[SZ_E_EDU_B1]))
		return CE_WRITE;
	//status block 66
	m1_block[SZ_E_EDU_B2][0] = 0x00;
	m1_block[SZ_E_EDU_B2][1] = 0x00;
	m1_block[SZ_E_EDU_B2][2] = 0x00;
	memset(&m1_block[SZ_E_EDU_B2][3], 0x00, 14);
	lrc_gen2(m1_block[SZ_E_EDU_B2], 16);
	memcpy(out_buf, "\x47\x8c\x00\x00", 4);
	if(0 != block_write(0, m1_key[SZ_E_EDU_SECTOR], 1, SZ_E_EDU_B2, m1_block[SZ_E_EDU_B2]))
		return CE_WRITE;
	//
	*out_len = 60;
	ch_sz_cpu_rollback = 0;
	//logic id-8
	memset(&out_buf[0], 0x00, 8);
	memcpy(&out_buf[4], &ch_m1_phyical_id[4], 4);
	//ticket type-1
	out_buf[8] = 0x80 + m1_block[36][5]; 
	//card sn-2
	memset(&out_buf[9], 0x00, 2);
	//transaction time-4
	memcpy(&out_buf[11], tpCPU.curtime, 4);
	//start date-2
	ShortToByte(0, &out_buf[15]);
	//end date-2
	ShortToByte(0, &out_buf[17]);
	//sam id-4
	memset(&out_buf[19], 0x00, 4);
	//entry station-2
	memcpy(&out_buf[23], &m1_block[SZ_E_EDU_B0][0], 2);
	//entry time-4
	BCD2LocalDateTime(&m1_block[SZ_E_EDU_B0][3], entry_time);
	memcpy(&out_buf[25], entry_time, 4);
	//LocalDateTime2BCD(entry_time, localtime);
	//printf("local date time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", localtime[0], localtime[1], localtime[2], localtime[3], localtime[4], localtime[5], localtime[6]);
	//transactin amount-4
	LongToByte(tpCPU.tranamount, &out_buf[29]);
	//balance after
	LongToByte(0, &out_buf[33]);
	//transaction times-4
	//times after-4
	memset(&out_buf[37], 0x00, 8);
	//area flag-1
	out_buf[45] = 0;
	//zone id-1
	//sec-sta1-2
	//sec-sta2-2
	memset(&out_buf[46], 0x00, 5);
	//tac-4
	memset(&out_buf[51], 0x00, 4);
	out_buf[55] = XA_FEETYPE_VALUE;
	//sam sn-4
	memset(&out_buf[56], 0x00, 4);
	return CE_OK;
label_refuse_to_exit:
	m1_block[SZ_E_EDU_B2][1] = chCode;
	if(0 != block_write(0, m1_key[SZ_E_EDU_SECTOR], 1, SZ_E_EDU_B2, m1_block[SZ_E_EDU_B2]))
		return CE_WRITE;
	//
	*out_len = 47;
	ch_sz_cpu_rollback = 0;
	//logic id-8
	memset(&out_buf[0], 0x00, 8);
	//ticket type-1
	out_buf[8] = 0x80 + m1_block[36][5]; 
	//card sn-2
	memset(&out_buf[9], 0x00, 2);
	//transaction time-4
	memcpy(&out_buf[11], tpCPU.curtime, 4);
	//start date-2
	ShortToByte(0, &out_buf[15]);
	//end date-2
	ShortToByte(0, &out_buf[17]);
	//sam id-4
	memset(&out_buf[19], 0x00, 4);
	//balance-4
	LongToByte(0, &out_buf[23]);
	//times-4
	memset(&out_buf[27], 0x00, 4);
	//update station-4
	memset(&out_buf[31], 0x00, 2);
	//area flag-1
	out_buf[33] = 0;
	//zone id-1/sec-sta1-2/sec-sta2-2
	memset(&out_buf[34], 0x00, 5);
	//tac-4
	memset(&out_buf[39], 0x00, 4);
	//sam sn-4
	memset(&out_buf[43], 0x00, 4);
	
	return chCode;
}

/************************************
CPU inquire
************************************/
char sz_M1_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len)
{
int ret;
unsigned char buf[100], factor[60], des[60], i, deslen;
unsigned char cpubuf[100], cpulen, Le;
unsigned char cnt, entry_time[4];
unsigned long time2;
unsigned char chCode, chRejectCode;
unsigned char chmonth;
unsigned short	shyear;

#ifdef DEBUG_PRINT
unsigned char localtime[7];
	printf("\ncpu inquire command is %02x and length is %02x:\n", cmd_buf[6], cmd_buf[5]);
	printf("current station is %02x%02x device id is %02x%02x%02x%02x\n", cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	printf("current station mode ?? device test mode ?? phycial ID %02x%02x%02x%02x%02x%02x%02x%02x chip type ??\n", 
		cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21]);
	
	//printf("SN:%02x%02x%02x%02x\n", cmd_buf[23], cmd_buf[24], cmd_buf[25], cmd_buf[26]);
	LocalDateTime2BCD(&cmd_buf[22], localtime);
	printf("local date time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", localtime[0], localtime[1], localtime[2], localtime[3], localtime[4], localtime[5], localtime[6]);
#endif
	*out_len = 4;
	memcpy(out_buf, "\x4b\x81\x00\x00", 4);
	memcpy(tpCPU.curtime, &cmd_buf[22], 4);
	if(!(sz_localtimeToSecond(&cmd_buf[22], &tpCPU.hisecond, &tpCPU.lowsecond)))
		return CE_COMMAND;
	LocalDateTime2BCD(&cmd_buf[22], tpCPU.time_bcd);
	sz_localtimeToDay(&cmd_buf[22], &tpCPU.days, &tpCPU.midsecond);
	if(0 != (chCode = check_station_id(&cmd_buf[7])))
		return chCode;
	//
	memcpy(tpCPU.curstation, &cmd_buf[7], 2);
	memcpy(&tpCPU.curstation[2], &cmd_buf[11], 2);
	//read sector 9 get the valid date 
	memcpy(out_buf, "\x4b\x86\x00\x00", 4);
	if(0 != (chCode = sector_read(m1_key[9], 0, 0, 9, 0, out_buf)))
		return CE_READ;
	//read metro sector
	memcpy(out_buf, "\x4b\x8a\x00\x00", 4);
	if(0 != (chCode = sector_read(m1_key[SZ_E_EDU_SECTOR], 1, 0, SZ_E_EDU_SECTOR, 0x13, out_buf)))
		return CE_READ;
	
	//
	chCode = CE_OK;
	*out_len = 153;
	//file 05 发行文件-logic id
	memset(&out_buf[0], 0x00, 8);
	memcpy(&out_buf[4], &ch_m1_phyical_id[4], 4);
	//file 05 - ticket face type
	out_buf[8] = 0;
	//file 06 发售文件 - ticket type
	out_buf[9] = 0x80 + m1_block[36][5];
	memset(&out_buf[10], 0x00, 20);
	//ticket family
	out_buf[30] = XA_FEETYPE_VALUE;
	out_buf[31] = 0;
	out_buf[32] = 2;
	//file 07 持卡人信息文件
	memset(&out_buf[33], 0x00, 40);
	//employee 员工文件
	memset(&out_buf[73], 0x00, 19);
	//balance 余额
	memset(&out_buf[92], 0x00, 4);
	//file 19 地铁交易复合文件
	//memcpy(&out_buf[96], &cpu_19_data[4], 34);
	memset(&out_buf[96], 0x00, 34);
	//entry station
	memcpy(&out_buf[96], &m1_block[SZ_E_EDU_B0][0], 4);
	//entry time
	BCD2LocalDateTime(&m1_block[SZ_E_EDU_B0][3], entry_time);
	memcpy(&out_buf[100], entry_time, 4);
	//entry/exit status
	if(m1_block[SZ_E_EDU_B2][0] == 0x01)
		out_buf[107] = 1;
	//update status
	if(m1_block[SZ_E_EDU_B2][2] == 0x33)
		out_buf[117] = 1;//m1_block[SZ_E_EDU_B2][2];
	//exit station
	memcpy(&out_buf[120], &m1_block[SZ_E_EDU_B1][0], 4);
	//exit time
	BCD2LocalDateTime(&m1_block[SZ_E_EDU_B1][3], entry_time);
	memcpy(&out_buf[124], entry_time, 4);
	
	//file 14 钱包信息文件
	memset(&out_buf[130], 0x00, 21);
	//可否允许更新
	out_buf[152] = 0;
	//check the valid date
	ByteToShort(&shyear, m1_block[36]);
	buf[0] = bin2bcd(shyear / 100);
	buf[1] = bin2bcd(shyear % 100);
	buf[2] = bin2bcd(m1_block[36][2]);
	buf[3] = bin2bcd(m1_block[36][3]);
	memset(&buf[4], 0x00, 3);
	if(!time_chk_valid(buf))
	{
		out_buf[151] = CE_EXPIREDDATE;
		return chCode;
	}
	if(memcmp(tpCPU.time_bcd, buf, 4) > 0)
	{
		out_buf[151] =  CE_EXPIREDDATE;
		return chCode;
	}
	//check the card type
	if((chRejectCode = CPU_TellSysCard(0x80 + m1_block[36][5])) != 0)
	{
		out_buf[151] = chRejectCode;
		return chCode;
	}
	//
	switch(m1_block[SZ_E_EDU_B2][1])
	{
	case 5:
	case 6:
	case 13:
	case 18:
	case 28:
		out_buf[151] = m1_block[SZ_E_EDU_B2][1];
		out_buf[152] = 1;
		break;
	default:
		out_buf[151] = CE_OK;
	}
	return chCode;
}

/************************************
CPU update
************************************/
char sz_M1_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len)
{
int ret, i;
unsigned char buf[100];
unsigned char cpubuf[100], cpulen, Le;
unsigned char cnt;
unsigned long time2;
char chCode, chmonth, chRejectCode;
long lngTranTimes;
unsigned short	shyear;

#ifdef DEBUG_PRINT
unsigned char localtime[7];
	printf("\ncpu update command is %02x and length is %02x:\n", cmd_buf[6], cmd_buf[5]);
	printf("current station is %02x%02x device id is %02x%02x%02x%02x\n", cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	printf("phycial ID %02x%02x%02x%02x %02x%02x%02x%02x SN:%02x%02x%02x%02x\n", 
		cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22], cmd_buf[23], cmd_buf[24]);
	  
	printf("update amount:%02x%02x%02x%02x chip type %02x ticket type %02x language %02x device test mode %02x\n", 
		cmd_buf[25], cmd_buf[26], cmd_buf[27], cmd_buf[28], cmd_buf[29], cmd_buf[30], cmd_buf[31], cmd_buf[32]);
	LocalDateTime2BCD(&cmd_buf[33], localtime);
	printf("local date time:%02x%02x-%02x-%02x %02x:%02x:%02x\n", localtime[0], localtime[1], localtime[2], localtime[3], localtime[4], localtime[5], localtime[6]);
	printf("entry station:%02x%02x\n", cmd_buf[37], cmd_buf[38]);
#endif
	*out_len = 4;
	//check whether rollback the last transation or not
	memcpy(out_buf, "\x4c\x81\x00\x00", 4);
	memcpy(tpCPU.curtime, &cmd_buf[33], 4);
	if(!(sz_localtimeToSecond(&cmd_buf[33], &tpCPU.hisecond, &tpCPU.lowsecond)))
		return CE_COMMAND;
	LocalDateTime2BCD(&cmd_buf[33], tpCPU.time_bcd);
	sz_localtimeToDay(&cmd_buf[33], &tpCPU.days, &tpCPU.midsecond);
	if(0 != (chCode = check_station_id(&cmd_buf[7])))
		return chCode;
	//
	memcpy(tpCPU.curstation, &cmd_buf[7], 2);
	memcpy(&tpCPU.curstation[2], &cmd_buf[11], 2);
	//read sector 9 get the valid date 
	memcpy(out_buf, "\x4c\x86\x00\x00", 4);
	if(0 != (chCode = sector_read(m1_key[9], 0, 0, 9, 0, out_buf)))
		return CE_READ;
	//read metro sector
	memcpy(out_buf, "\x4c\x8a\x00\x00", 4);
	if(0 != (chCode = sector_read(m1_key[SZ_E_EDU_SECTOR], 1, 0, SZ_E_EDU_SECTOR, 0x13, out_buf)))
		return CE_READ;
	
	//
	//check the valid date
	ByteToShort(&shyear, m1_block[36]);
	buf[0] = bin2bcd(shyear / 100);
	buf[1] = bin2bcd(shyear % 100);
	buf[2] = bin2bcd(m1_block[36][2]);
	buf[3] = bin2bcd(m1_block[36][3]);
	memset(&buf[4], 0x00, 3);
	memcpy(out_buf, "\x46\x87\x00\x00", 4);
	if(!time_chk_valid(buf))
	{
		return CE_EXPIREDDATE;
	}
	memcpy(out_buf, "\x46\x88\x00\x00", 4);
	if(memcmp(tpCPU.time_bcd, buf, 4) > 0)
	{
		return CE_EXPIREDDATE;
	}
	//check the card type
	memcpy(out_buf, "\x46\x89\x00\x00", 4);
	if((chRejectCode = CPU_TellSysCard(0x80 + m1_block[36][5])) != 0)
	{
		return chCode;
	}
	//
	switch(m1_block[SZ_E_EDU_B2][1])
	{
	case 5:		//no exit station
	case 6:
		m1_block[SZ_E_EDU_B2][0] = 0;
		m1_block[SZ_E_EDU_B2][1] = 0;
		m1_block[SZ_E_EDU_B2][2] = 0;
		break;
	case 13:	//overtime
		m1_block[SZ_E_EDU_B2][1] = 0;
		m1_block[SZ_E_EDU_B2][2] = 0x33;
		break;
	case 18:	//no entry staiton
		memcpy(&m1_block[SZ_E_EDU_B0][0], &cmd_buf[37], 2);
		memset(&m1_block[SZ_E_EDU_B0][2], 0x00, 2);
		memcpy(&m1_block[SZ_E_EDU_B0][4], &tpCPU.time_bcd[1], 6);
		lrc_gen2(m1_block[SZ_E_EDU_B0], 15);
		if(0 != block_write(0, m1_key[SZ_E_EDU_SECTOR], 1, SZ_E_EDU_B0, m1_block[SZ_E_EDU_B0]))
			return CE_WRITE;
		m1_block[SZ_E_EDU_B2][0] = 0x01;
		m1_block[SZ_E_EDU_B2][1] = 0;
		m1_block[SZ_E_EDU_B2][2] = 0;
		break;
	case 28:	//over fare
		m1_block[SZ_E_EDU_B2][1] = 0;
		break;
	default:
		return ERR_NOT_UPDATE;
	}
	//
	if(0 != block_write(0, m1_key[SZ_E_EDU_SECTOR], 1, SZ_E_EDU_B2, m1_block[SZ_E_EDU_B2]))
		return CE_WRITE;
	//
	*out_len = 35;
	ch_sz_cpu_rollback = 0;
	//logic id-8
	memset(&out_buf[0], 0x00, 8);
	memcpy(&out_buf[4], &ch_m1_phyical_id[4], 4);
	//ticket type-1
	out_buf[8] = 0x80 + m1_block[36][5];
	//sn-2
	memset(&out_buf[9], 0x00, 2);
	//transaction time-4
	memcpy(&out_buf[11], tpCPU.curtime, 4);
	//start date-2
	ShortToByte(0, &out_buf[15]);
	//end date-2
	ShortToByte(0, &out_buf[17]);
	//sam id-4
	memset(&out_buf[19], 0x00, 4);
	//update amount-4
	memcpy(&out_buf[23], &cmd_buf[25], 4);
	//tac-4
	memset(&out_buf[27], 0x00, 4);
	//sam sn-4
	memset(&out_buf[31], 0x00, 4);
	return CE_OK;
}