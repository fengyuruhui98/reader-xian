#include "linux2440lib.h"
#include "xa_error_code.h"
#include "bin_file_manage.h"
#include "xa_sam.h"
#include "serial.h"
#include "hh_cpu_operation.h"
#include "eeprom.h"
#include "xa_operation.h"
#include "time_tools.h"
#include "xa_tong_operation.h"
#include "xa_pboc_operation.h"
#include "tlv.h"

unsigned char AID[300], GPO[300];
unsigned char pboc_15_data[3][64];
unsigned char gpo[300];
unsigned char pboc_19_data[64];
struct TLVEntity *AIDtag, *PDOLtag, *CAPPtag, *GPOtag, *ATCtag, *AFLtag, AIPtag;
unsigned char AID_len, GPO_len;

extern unsigned char cput_15_data[30];

char pboc_read_fci(unsigned char *in_buf, unsigned char in_len, unsigned char *out_buf)
{
unsigned char buf[100], cpubuf[500], cpulen;
unsigned char i, chCode;
int ret, j;
unsigned short tagbuf[10], taglen;

#ifdef DEBUG_PRINT
	PRINTK("PPSE:");
	for(j = 0; j < in_len; j++)
		PRINTK("%02x", in_buf[j]);
	PRINTK("\n");
#endif
	free_tlv(&tlv_ppse);
	free_tlv(&tlv_aid);
	free_tlv(&tlv_gpo);
	//construct PPSE tlv	
	memset(&tlv_ppse, 0x00, sizeof(struct TLVEntity));
	tlv_ppse.UpArray = 1;
	construct_tlv(PPSE, PPSE_len, &tlv_ppse, 1);
	//get AID priority tvl :0087
	//get AID tlv::006F/00A5/BF0C/0061/004F
	tagbuf[0] = 0x6F;
	tagbuf[1] = 0xA5;
	tagbuf[2] = 0xBF0C;
	tagbuf[3] = 0x61;
	tagbuf[4] = 0x4F;
	AIDtag = search_tlv(tagbuf, 5, &tlv_ppse);
	if(AIDtag == NULL)
		return CE_READ;
	//select AID
	//memcpy(buf, "\xA0\x00\x00\x03\x33\x01\x01\x02", 8);
	memset(cpubuf, 0x00, 500);
	chCode = CPU_select_file(AIDtag->Value, AIDtag->Length, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_READ;
	memcpy(AID, cpubuf, cpulen);
	AID_len = cpulen;
	//construct AID tlv
	memset(&tlv_aid, 0x00, sizeof(struct TLVEntity));
	tlv_aid.UpArray = 1;
	construct_tlv(AID, AID_len, &tlv_aid, 1);
	// get PDOL tlv:006f/00a5/9f38
	tagbuf[0] = 0x6F;
	tagbuf[1] = 0xA5;
	tagbuf[2] = 0x9F38;
	PDOLtag = search_tlv(tagbuf, 3, &tlv_aid);
	if(PDOLtag == NULL)
		return CE_READ;
	//get CAPP tlv:006f/00a5/bf0c/df61
	tagbuf[0] = 0x6f;
	tagbuf[1] = 0xa5;
	tagbuf[2] = 0xbf0c;
	tagbuf[3] = 0xdf61;
	CAPPtag = search_tlv(tagbuf, 4, &tlv_aid);
	return 0;
}

/*
function:read the file 19
parameter:record :1 only reading record 1 or 3 for reading all
*/
char PBOC_GetFiles15(unsigned char record, unsigned char *out_buf)
{
int ret, i, j;
unsigned char buf[40];
unsigned char cpubuf[200], Le;
unsigned short cpulen;
	
	//read file 15 - variable file length
	memcpy(out_buf, "\xf0\x22", 2);
	memcpy(buf, "\x80\xb4\x00\x00\x02\x97\x10", 7);
	buf[3] = 0x15 << 3;
	for(i = 1; i <=record; i++)
	{
		buf[5] = i;
		ret = mifpro_apdu(buf, 7, cpubuf, &cpulen);
#ifdef DEBUG_PRINT
		PRINTK("read file 15:");
		for(j = 0; j < 7; j++) PRINTK("%02x", buf[j]);
		PRINTK("\n");
#endif
		if(ret != 0)
		{
			return CE_READ;
		}
		memcpy(out_buf, "\xf0\x23", 2);
#ifdef DEBUG_PRINT
		PRINTK("read file 15[%02x21]:len %d ", i, cpulen);
		for(j = 0; j < cpulen; j++) PRINTK("%02x", cpubuf[j]);
		PRINTK("\n");
#endif
		memcpy(out_buf, "\xf0\x24", 2);
		if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		{
			return CE_INVADLIDCARD;
		}
		memcpy(pboc_15_data[i], cpubuf, 64);
	}
#ifdef DEBUG_PRINT
#endif

	//read balance
	memcpy(out_buf, "\xfa\x20", 2);
	memcpy(buf, "\x80\xca\x9f\x79\x00", 5);
	memset(cpubuf, 0x00, 80);
	ret = mifpro_apdu(buf, 5, cpubuf, &cpulen);
	if(ret != 0)
	{
		return CE_READ;
	}
#ifdef DEBUG_PRINT
	PRINTK("read balance return len %d ",cpulen);
	for(i = 0; i < cpulen; i++)
		PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf0\x21", 2);
	if(cpulen < 3)
		return 	CE_INVADLIDCARD;
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
	{
		return CE_INVADLIDCARD;
	}
	tpCPU.balance = bcd2bin(cpubuf[4]) * 100000000 + bcd2bin(cpubuf[5]) * 1000000 + bcd2bin(cpubuf[6]) * 10000 + bcd2bin(cpubuf[7]) * 100 + bcd2bin(cpubuf[8]);
#ifdef DEBUG_PRINT
	PRINTK("cpu balance:%d\n", tpCPU.balance);
#endif
	return 0;
}

char PBOC_GetFiles19(unsigned char *out_buf)
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
#ifdef DEBUG_PRINT
	PRINTK("read balance return len %d %02x %02x %02x %02x ", cpulen, cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3]);
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

	//read file 19 - variable file length
	memcpy(out_buf, "\xf0\x22", 2);
	memcpy(buf, "\x00\xb2\x02\x00\x30", 5);
	buf[3] = (0x19 << 3) | 0x04;
	buf[4] = Le = 64;
#ifdef DEBUG_PRINT
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
#ifdef DEBUG_PRINT
	PRINTK("return:len%d ", Le);
	for(i = 0; i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xf0\x24", 2);
	if(cpulen != Le + 2)
	{
		return CE_INVADLIDCARD;
	}
	memcpy(pboc_19_data, cpubuf, Le);
	return 0;
}

char pboc_GPO(unsigned char *cur_timebcd, unsigned long tranamount, unsigned char cappflag, struct TLVEntity *tlv, unsigned char *out_buf)
{
unsigned char buf[300], cpubuf[300];
unsigned short tagbuf[20], taglen, cpulen;
unsigned char chCode;
long i;
	
	memcpy(out_buf, "\xF1\x01", 2);
	memset(buf, 0x00, 300);
	memcpy(buf, "\x80\xA8\x00\x00\x00\x83\x00", 7);
	cpulen = init_gpo(cur_timebcd, tranamount, cappflag, &buf[7], tlv);
	buf[4] = cpulen + 2;
	buf[6] = cpulen;
#ifdef DEBUG_PRINT
	PRINTK("GPO:");
	for(i = 0; i < cpulen + 5 + 2; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xF1\x02", 2);
	chCode = mifpro_apdu(buf, cpulen + 5 + 2, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_READ;
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
#endif
	memcpy(out_buf, "\xF1\x03", 2);
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
	memcpy(GPO, cpubuf, cpulen - 2);
	GPO_len = cpulen;
	//construct GPO	
	memset(&tlv_gpo, 0x00, sizeof(TLVEntity_t));
	tlv_gpo.UpArray = 1;
	construct_tlv(GPO, GPO_len, &tlv_gpo, 1);
	//get ATC tlv:0077/9f36
	memcpy(out_buf, "\xF1\x04", 2);
	tagbuf[0] = 0x77;
	tagbuf[1] = 0x9F36;
	ATCtag = search_tlv(tagbuf, 2, &tlv_gpo);
	if(ATCtag == NULL)
		return CE_READ;
	//get AFL tlv:0077/0094
	memcpy(out_buf, "\xF1\x05", 2);
	tagbuf[0] = 0x77;
	tagbuf[1] = 0x94;
	AFLtag = search_tlv(tagbuf, 2, &tlv_gpo);
	if(AFLtag == NULL)
		return CE_READ;

	return 0;
}

char pboc_update_capp_data(unsigned char psam_index, unsigned char *key, unsigned char *factor, unsigned char *in_buf, unsigned char in_len, unsigned char *out_buf)
{
unsigned char buf[300], cpubuf[300];
unsigned char chCode, callen;
long 	i;
unsigned short cpulen;

	//update capp data cache 
	memset(buf, 0x00, 300);
	memcpy(&buf[6], ATCtag->Value, 2);
	memcpy(&buf[8], "\x84\xde\x00\xA8\x44", 5);
	memcpy(&buf[13], in_buf, in_len);
	buf[13 + in_len] = 0x80;
	chCode = cpu_cal_dcmk(psam_index, key, factor, 8, 0x05, buf, ((13 + in_len) / 8 + 1) * 8, cpubuf, &callen);
	if(chCode != 0)
		return CE_METROPSAM;
	memcpy(&buf[13 + in_len], cpubuf, 4);
	//update capp data cache
#ifdef DEBUG_PRINT
	PRINTK("updatecappdata:");
	for(i = 8; i < 13 + in_len + 4; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	chCode = mifpro_apdu(&buf[8], 13 + in_len - 8 + 4, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_READ;
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
	return 0;
}

char pboc_read_record()
{
unsigned long i, j, k, ret;
unsigned char buf[300], cpubuf[300];
unsigned char chCode;
unsigned short cpulen;

	for(i = 0; i < AFLtag->Length / 4; i++)
	{
		memcpy(buf, "\x00\xb2\x00\x00\x00", 5);
		//file sfi
		buf[3] = AFLtag->Value[0 + i * 4]  | 0x4;
		for(j = AFLtag->Value[1 + i * 4]; j <= AFLtag->Value[2 + i * 4]; j++)
		{
			//record number
			buf[2] = j;
#ifdef DEBUG_PRINT
			PRINTK("readrecord:");
			for(k = 0; k < 5; k++) PRINTK("%02x", buf[k]);
			PRINTK("\n");
#endif
			chCode = mifpro_apdu(buf, 5, cpubuf, &cpulen);
#ifdef DEBUG_PRINT
			PRINTK("return:");
			for(k = 0; k < cpulen; k++) PRINTK("%02x", cpubuf[k]);
			PRINTK("\n");
#endif
			if(chCode != 0)
				return CE_WRITE;
			if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
				return CE_WRITE;
		}
	}
	return 0;
}
/************************************
pboc tong entry
************************************/
char xa_pboc_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode, chRejectCode, chFare, chEntryStatus;
unsigned char buf[300], factor[20], des[80], deslen;
unsigned char cpubuf[300], cpulen, Le, entry_time[4], last_timebcd[7];
//unsigned char cnt;
unsigned char status;
unsigned long time1,time2;
unsigned short tempdate, shTicketType, cnt;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
long lngCardBalance, ret, i, j;
unsigned short shDays;
unsigned long lngMidnightSecond;
YKTTerminal_t	tpPurchase;

	*out_len = 4;
	memcpy(out_buf, "\x32\x01\x00\x00", 4);
	//check whether rollback the last transation or not
	//if((ch_sz_pboc_rollback != 0) && (memcmp(ch_cput_phyical_id, ch_cput_phyical_id_bak, 8) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_pboc_rollback);
#endif
//		if((chRejectCode = CPUT_gettransprove(0x09, out_buf)) == 0)
//		{
//			blncputRollback = 1;
//			goto label_sz_city_rollback_1;
//		}else if(chRejectCode == CE_READ)
//			return CE_READ;
	}
	//clear the backup flag
	ch_sz_pboc_rollback = 0;
	//YKT purchase transaction record
	tpYKTTxnPurchase.udSubtype = 0x0C;
	tpYKTTxnPurchase.udType = 0x21;
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_cput_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, ch_cput_psam_sn, 8);
	//tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_cput_phyical_id[4]);
	tpYKTTxnPurchase.Enlocation = 0;
	tpYKTTxnPurchase.Entime = 0;
	tpYKTTxnPurchase.OrigEntime = 0;
	tpYKTTxnPurchase.OrigEnlocation = 0;
	tpYKTTxnPurchase.mode = 0;
	//
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	memcpy(tpYKTTxnPurchase.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnPurchase.TxnTime, &tpCPU.time_bcd[4], 3);
	
	get_degrade_mode(tpCPU.curstation);
	
	//select df01 and return file 15 information at the same time
	memcpy(out_buf, "\x71\x05", 2);
	if(0 != pboc_read_fci(PPSE, PPSE_len, out_buf))
		return CE_READ;
	if(0 != PBOC_GetFiles15(3, out_buf))
		return CE_READ;
	//
	//if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
	//	return chCode;
	//read file 15
	//memcpy(cput_15_data, out_buf, XA_CPUT_15_LEN);

label_sz_city_rollback_1:
	//clear rollback
	ch_sz_pboc_rollback = 0;
	//sn-2
	tpYKTTxnPurchase.CrdDebitCnt = ByteToShort(NULL, &capp_init[4]);
	//tacs-4
	tpYKTTxnPurchase.TAC = ByteToLong(NULL, tpCPU.tac);
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
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	//memcpy(&out_buf[6], &ch_cput_phyical_id[4], 4);
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
	PRINTK("PFBank Inquire:");
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
	*out_len = 1;
	return chCode;
}
/************************************
pboc tong entry
************************************/
char xa_pboc_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode, chRejectCode, chFare, chEntryStatus;
unsigned char buf[300], factor[20], des[80], deslen;
unsigned char cpubuf[300], cpulen, Le, entry_time[4], last_timebcd[7];
//unsigned char cnt;
unsigned char status;
unsigned long time1,time2;
unsigned short tempdate, shTicketType, cnt;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
long lngCardBalance, ret, i, j;
unsigned short shDays;
unsigned long lngMidnightSecond;
YKTTerminal_t	tpPurchase;

	*out_len = 4;
	memcpy(out_buf, "\x32\x01\x00\x00", 4);
	//check whether rollback the last transation or not
	//if((ch_sz_pboc_rollback != 0) && (memcmp(ch_cput_phyical_id, ch_cput_phyical_id_bak, 8) == 0))
	{
#ifdef DEBUG_PRINT
		PRINTK("need roll back %02x\n", ch_sz_pboc_rollback);
#endif
//		if((chRejectCode = CPUT_gettransprove(0x09, out_buf)) == 0)
//		{
//			blncputRollback = 1;
//			goto label_sz_city_rollback_1;
//		}else if(chRejectCode == CE_READ)
//			return CE_READ;
	}
	//clear the backup flag
	ch_sz_pboc_rollback = 0;
	//YKT purchase transaction record
	tpYKTTxnPurchase.udSubtype = 0x0C;
	tpYKTTxnPurchase.udType = 0x21;
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_cput_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, ch_cput_psam_sn, 8);
	//tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_cput_phyical_id[4]);
	tpYKTTxnPurchase.Enlocation = 0;
	tpYKTTxnPurchase.Entime = 0;
	tpYKTTxnPurchase.OrigEntime = 0;
	tpYKTTxnPurchase.OrigEnlocation = 0;
	tpYKTTxnPurchase.mode = 0;
	//
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	memcpy(tpYKTTxnPurchase.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnPurchase.TxnTime, &tpCPU.time_bcd[4], 3);
	
	get_degrade_mode(tpCPU.curstation);
	
	//select df01 and return file 15 information at the same time
	memcpy(out_buf, "\x71\x05", 2);
	if(0 != pboc_read_fci(PPSE, PPSE_len, out_buf))
		return CE_READ;
	//only read balance--PARA is zero
	if(0 != PBOC_GetFiles15(2, out_buf))
		return CE_READ;
	//GPO
	//memcpy(buf, "\x80\xA8\x00\x00\x24\x83\x22\x28\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x56\x00\x00\x00\x00\x00\x01\x56\x13\x05\x15\x00\x11\x22\x33\x44\x00", 41);
	//memcpy(buf, "\x80\xA8\x00\x00\x13\x83\x11\x28\x00\x00\x80\x00\x00\x00\x00\x00\x00\x11\x22\x33\x44\x01\x56\x00", 24);
	if(0 != (chCode = pboc_GPO(tpCPU.time_bcd, 0, 1, PDOLtag, out_buf)))
		return chCode;
	//Update the PuFa Bank 0121 record to EMPLOYEE
	pboc_15_data[1][19] = 0x90;
	if(0 != (chCode = pboc_update_capp_data(xa_pboc_psam_index, "\x28\x01", &pboc_15_data[1][9], pboc_15_data[1], 64, out_buf)))
		return chCode;
/*
	memset(buf, 0x00, 300);
	memcpy(&buf[6], &gpo[20], 2);
	memcpy(&buf[8], "\x84\xde\x00\xA8\x44", 5);
	pboc_15_data[1][19] = 0x90;
	memcpy(&buf[13], pboc_15_data[1], 64);
	buf[77] = 0x80;
	chCode = cpu_cal_dcmk(xa_pboc_psam_index, "\x28\x01", &pboc_15_data[1][9], 8, 0x05, buf, 80, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_METROPSAM;
	memcpy(&buf[77], cpubuf, 4);
	//update capp data cache
#ifdef DEBUG_PRINT
	PRINTK("updatecappdata:");
	for(i = 8; i < 81; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	chCode = mifpro_apdu(&buf[8], 73, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_READ;
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
*/
/*
	memset(buf, 0x00, 300);
	memcpy(&buf[6], &gpo[20], 2);
	memcpy(&buf[8], "\x84\xde\x00\xA8\x44", 5);
	memcpy(&buf[13], pboc_15_data[2], 64);
	buf[77] = 0x80;
	chCode = cpu_cal_dcmk(xa_pboc_psam_index, "\x28\x02", &pboc_15_data[1][9], 8, 0x05, buf, 80, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_METROPSAM;
	memcpy(&buf[77], cpubuf, 4);
	//update capp data cache
#ifdef DEBUG_PRINT
	PRINTK("updatecappdata:");
	for(i = 8; i < 81; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	chCode = mifpro_apdu(&buf[8], 73, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_READ;
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;

	//update record 3
	memset(buf, 0x00, 300);
	memcpy(&buf[6], &gpo[20], 2);
	memcpy(&buf[8], "\x84\xde\x00\xA8\x44", 5);
	memcpy(&buf[13], pboc_15_data[3], 64);
	buf[77] = 0x80;
	chCode = cpu_cal_dcmk(xa_pboc_psam_index, "\x28\x03", &pboc_15_data[1][9], 8, 0x05, buf, 80, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_METROPSAM;
	memcpy(&buf[77], cpubuf, 4);
	//update capp data cache
#ifdef DEBUG_PRINT
	PRINTK("updatecappdata:");
	for(i = 8; i < 81; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	chCode = mifpro_apdu(&buf[8], 73, cpubuf, &cpulen);
	if(chCode != 0)
		return CE_READ;
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < cpulen; i++) PRINTK("%02x", cpubuf[i]);
	PRINTK("\n");
#endif
	if((cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00))
		return CE_INVADLIDCARD;
*/
	//read record
	if(0 != (chCode = pboc_read_record()))
		return chCode;
	

label_sz_city_rollback_1:
	//clear rollback
	ch_sz_pboc_rollback = 0;
	//sn-2
	tpYKTTxnPurchase.CrdDebitCnt = ByteToShort(NULL, &capp_init[4]);
	//tacs-4
	tpYKTTxnPurchase.TAC = ByteToLong(NULL, tpCPU.tac);
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
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	//memcpy(&out_buf[6], &ch_cput_phyical_id[4], 4);
	//before balance
	memcpy(&out_buf[10], &tpCPU.balance, 4);
	//after balance
	memcpy(&out_buf[14], &tpCPU.balance, 4);//tpYKTTxnPurchase.AftBalance, 4);
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
	*out_len = 1;
	return chCode;
}
/************************************
pboc tong exit
************************************/
char xa_pboc_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode, chRejectCode, chExitStatus;
unsigned char buf[500], factor[20], des[80], deslen;
unsigned char cpubuf[500], cpulen, Le;
//unsigned char cnt;
unsigned char status, entry_station[2], entry_time[7];
unsigned long time1,time2, cnt, lngsrcstation;
unsigned short tempdate, shTicketType, shFare;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
long lngCardBalance, ret, i, j;
unsigned short shDays;
unsigned long lngMidnightSecond;

	*out_len = 4;
	memcpy(out_buf, "\x33\x01\x00\x00", 4);
	//check whether rollback the last transation or not
#ifdef DEBUG_PRINT
//		PRINTK("need roll back %02x id %02x%02x%02x%02x bak %02x%02x%02x%02x\n", ch_sz_cput_rollback, ch_cput_phyical_id[4], ch_cput_phyical_id[5], ch_cput_phyical_id[6], ch_cput_phyical_id[7]
//					, ch_cput_phyical_id_bak[4], ch_cput_phyical_id_bak[5], ch_cput_phyical_id_bak[6], ch_cput_phyical_id_bak[7]);
#endif
//	if((ch_sz_cput_rollback != 0) && (memcmp(ch_cput_phyical_id, ch_cput_phyical_id_bak, 8) == 0))
//	{
//		if((chRejectCode = CPUT_gettransprove(0x09, out_buf)) == 0)
//		{
//			blncputRollback = 0xff;
//			goto label_sz_city_rollback_1;
//		}else if(chRejectCode == CE_READ)
//			return CE_READ;
//	}
	//
	ch_sz_pboc_rollback = 0;

	tpYKTTxnPurchase.udSubtype = 0x0D;
	tpYKTTxnPurchase.udType = 0x21;
	memcpy(&tpYKTTxnPurchase.LocalTxnSeq, &cmd_buf[13], 4);
	memcpy(tpYKTTxnPurchase.PosId, ch_cput_psam_id, 6);
	memcpy(tpYKTTxnPurchase.SamId, ch_cput_psam_sn, 8);
	//tpYKTTxnPurchase.CardCsn = ByteToLong(NULL, &ch_cput_phyical_id[4]);
	tpYKTTxnPurchase.OrigEntime = 0;
	tpYKTTxnPurchase.OrigEnlocation = 0;
	tpYKTTxnPurchase.mode = 0;
	//
	memcpy(tpCPU.time_bcd, &cmd_buf[6], 7);
	lngMidnightSecond = timestr2long(&cmd_buf[7]);
	LongToByte(lngMidnightSecond, tpCPU.curtime);
	memcpy(tpYKTTxnPurchase.TxnDate, tpCPU.time_bcd, 4);
	memcpy(tpYKTTxnPurchase.TxnTime, &tpCPU.time_bcd[4], 3);

	get_degrade_mode(tpCPU.curstation);
	//select df01 and return file 15 information at the same time
	memcpy(out_buf, "\x71\x05", 2);
	if(0 != pboc_read_fci(PPSE, PPSE_len, out_buf))
		return CE_READ;
	if(0 != PBOC_GetFiles15(0, out_buf))
		return CE_READ;

	//GPO
	//memcpy(buf, "\x80\xA8\x00\x00\x24\x83\x22\x28\x00\x00\x80\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x56\x00\x00\x00\x00\x00\x01\x56\x13\x05\x15\x00\x11\x22\x33\x44\x00", 41);
	//memcpy(buf, "\x80\xA8\x00\x00\x13\x83\x11\x28\x00\x00\x80\x00\x00\x00\x00\x00\x01\x11\x22\x33\x44\x01\x56\x00", 24);
	if(0 != (chCode = pboc_GPO(tpCPU.time_bcd, 1, 0, PDOLtag, out_buf)))
		return chCode;

	//read record
	if(0 != (chCode = pboc_read_record()))
		return chCode;

	tpYKTTxnPurchase.AftBalance = tpCPU.balance - 1;

/*   shanghai employee
	if(pboc_15_data[1][19] != 0x90)
		return CE_INVADLIDCARD;
	chCode = CPU_select_file("\x41\x50\x50\x4D\x45\x43\x30\x32", 8, out_buf, &out_len);
	if(chCode != 0)
		return CE_READ;
		tpCPU.EDorEP = CPU_ED;
		tpCPU.thread_mac1 = 6;
		tpCPU.thread_mac2 = 7;
		tpCPU.capp_type = 9;
		tpCPU.capp_len = 0x1c;
		tpCPU.sz_psam_index = xa_pboc_psam_index;

	chCode = CPUT_GetFiles15(out_buf);
	if(chCode != 0)
		return CE_READ;
	chCode = PBOC_GetFiles19(out_buf);
	if(chCode != 0)
		return CE_READ;
	//
	tpCPU.tranamount = 0;
	if(0 != (chCode = CPU_init_for_capp(1, tpCPU.tranamount, ch_pboc_psam_id, &cput_15_data[12], buf, 3, ch_cpu_mac_data)))
	{
		return chCode;
	}
	ret = CPU_update_capp(1, 0x19, pboc_19_data[0], 64, pboc_19_data, 0);
	if(ret != 0)
	{
		return CE_WRITE;
	}

	if(0 != CPU_debit_for_capppurchase(&ch_sz_cput_rollback, sz_tong_ee_write, out_buf))
	{
#ifdef DEBUG_PRINT
		PRINTK("debit failure %02x\n", ch_sz_cput_rollback);
#endif		
		return CE_WRITE;
	}
*/
/*	
	//select 3f01
	memcpy(out_buf, "\x72\x01", 2);
	//if(0 != (chCode = CPU_select_file("\x3f\x01", 2, out_buf, NULL)))
	//	return chCode;
	//read file 15
	//memcpy(cput_15_data, out_buf, XA_CPUT_15_LEN);
	memcpy(ch_cput_logic_id, &cput_15_data[12], 8);
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
	case 0x03:	//old man
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
			return CE_EXPIREDDATE;
		memcpy(tpYKTTxnPurchase.Validday, &cput_21_data[60], 4);
		break;
	case 0x04:	//Student
		if(memcmp(tpCPU.time_bcd, &cput_21_data[60], 4) > 0)
			cput_21_data[0] = 0x01;
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
	if(0 != (chCode = CPUT_TellEntry(&chExitStatus)))
	{
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
			if(chCode == CE_OVERTIME)
				goto label_refuse_to_city_exit;
			else
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
	memcpy(out_buf, "\x72\x15", 2);
	//if current station is set to fare mode
	//if(tpwaivermode.cur_sta_fare)
	//	tpCPU.tranamount = Eod04->ExpressIssueFare.ExpressIssueFare_val[0].FareAmount;
	if((tpCPU.balance - tpCPU.tranamount) < (long)tpTicketDef.MinRemainingValue)
	{
		//memcpy(out_buf, "\x71\x16", 2);
		//if(cpu_19_data[25] == 2)
		//	tpCPU.tranamount = 0;
		//else
		//{
			chCode = CE_ENOUGH_BALANCE;
			goto label_refuse_to_city_exit;
		//}
	}	
	//debit for capp
	tpYKTTxnPurchase.OrigAmt = tpYKTTxnPurchase.TxnAmt = tpCPU.tranamount;
	tpYKTTxnPurchase.Entime = timestr2long(&entry_time[1]) + TIME2000;
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
	//status
	cpu_19_data[21] = 0x02;
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
	ch_sz_cput_rollback = SZ_CPU_CAPP_1;
	if(0 != CPU_debit_for_capppurchase(&ch_sz_cput_rollback, sz_tong_ee_write, out_buf))
	{
#ifdef DEBUG_PRINT
		PRINTK("debit failure %02x\n", ch_sz_cput_rollback);
#endif		
		return CE_WRITE;
	}
	tpYKTTxnPurchase.AftBalance = tpCPU.balance - tpCPU.tranamount;

#ifdef DEBUG_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, out_buf, 2);
#endif
*/
label_sz_city_rollback_1:
	ch_sz_pboc_rollback = 0;

	*out_len = 51;
	//card sn-2
	tpYKTTxnPurchase.CrdDebitCnt = ByteToShort(NULL, &capp_init[4]);
	//tac-4
	tpYKTTxnPurchase.TAC = ByteToLong(NULL, tpCPU.tac);
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
	memcpy(&out_buf[4], &shTicketType, 2);
	//logic card sn
	//memcpy(&out_buf[6], &ch_cput_phyical_id[4], 4);
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
	*out_len = 2;
	return chCode;
	
}