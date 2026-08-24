#include <string.h>
#include <stdio.h>

#include "xa_func.h"
#include "linux2440lib.h"

#include "bin_file_manage.h"
#include "xa_operation.h"
#include "xa_ul_operation.h"
#include "xa_cpu20_operation.h"
#include "xa_tong_operation.h"
#include "xa_transport_operation.h"
#include "hh_cpu_operation.h"
#include "xa_m1_operation.h"
#include "xa_error_code.h"
#include "xa_sam.h"
#include "eeprom.h"
#include "time_tools.h"

//#define DEBUG_PRINT_POLL 1 //20230705
//#define DEBUG_PRINT_POLLWRONG 1 //20230706

extern unsigned char cpu_05_data[29];
extern unsigned char cpu_06_data[30];
extern unsigned char transport_15_data[30];		//公共应用信息文件

/*===============================================================================================
函数：szmt_rm_polling_card
功能：寻卡指令
=================================================================================================*/
unsigned char xa_polling_card(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
char 	i, chCode, chUID[50], chAntenna;
unsigned char buf[400], cpubuf[400], atslen;
int ret;
unsigned char 	ee_buf[20], ul_page;

struct timeval tvBegin;
struct timeval tvNow;
int nReduce = 0;
struct timespec req;

	memset(out_buf, 0x00, 11);
	//
	mcml_pwr_off();
	//mifpro_deselect(0, buf, &atslen);
	if(cmd_buf[6] == 0x00)
	{
		chAntenna = 1;
		rf_select(0x00);
	}else if(cmd_buf[6] == 0x01)
	{
		chAntenna = 2;
		rf_select(0x01);
	}
	//usleep(5000);
	mcml_pwr_on();
/*	
	gettimeofday(&tvBegin, NULL);
	set_timeout(5000);
	gettimeofday(&tvNow, NULL);
	nReduce = (tvNow.tv_sec - tvBegin.tv_sec) * 1000000 + tvNow.tv_usec - tvBegin.tv_usec;
	PRINTK("select 5ms test %d reduce is %dus\n", nReduce, nReduce - 5000);

	gettimeofday(&tvBegin, NULL);
	req.tv_sec = 0;
	req.tv_nsec = 5 * 1000000;
	nanosleep(&req, NULL);
	gettimeofday(&tvNow, NULL);
	nReduce = (tvNow.tv_sec - tvBegin.tv_sec) * 1000000 + tvNow.tv_usec - tvBegin.tv_usec;
	PRINTK("nanosleep 5ms test %d reduce is %dus\n", nReduce, nReduce - 5000);
*/
	*out_len = 4;
	for(i = 0; i < 6; i++)
	{
		if(mcml_request2(PICC_REQALL, buf) == 0)
		{
#ifdef DEBUG_PRINT_POLL
		PRINTK("##################  mcml_request2 == 0 RIGHT!!#############\n");
#endif	
			break;
		}
		//usleep(15000);
#ifdef DEBUG_PRINT_POLL
		PRINTK("##################  mcml_request2 != 0 WRONG!!#############\n");
#endif	
		if(i>=3)
		{
			mcml_pwr_off();
			set_timeout(5000);
			mcml_pwr_on();
		}

		set_timeout(3000);
	}
	memcpy(out_buf, "\x31\x01\x00\x00", 4);
	if(i >= 6)
	{
#ifdef DEBUG_PRINT_POLL
		PRINTK("i >= 6 request card return %02x %02x i %02x\n", buf[0], buf[1], i);
#endif
		//mcml_pwr_off();
		reader_status = XA_RW_IDLE;
		return CE_NOCARD;
	}
#ifdef DEBUG_PRINT_POLL
	PRINTK("request card return %02x %02x i %02x\n", buf[0], buf[1], i);
#endif
	memcpy(out_buf, "\x31\x02", 2);
	memcpy(&out_buf[2], buf, 2);
	//if((buf[0] == 0x44) && (buf[1] == 0x00))	//单程票
	//according to the UID size bit frame:b8b7=00 4 bytes UID,b8b7-01:7bytes UID,b8b7=10:10bytes UID
	if((buf[0] & 0xC0) == 0x40)
	{
		set_card_type(ISO14443A_M1_TYPE);
		//mcml_anticoll2(buf);
#ifdef DEBUG_PRINT_POLL
		PRINTK("request card return sjt %02x %02x i %02x\n", buf[0], buf[1], i);
#endif
		memcpy(out_buf, "\x31\x10", 2);
		ret = UL_Anticoll_Select(cpubuf);
		if(ret != 0)
		{
#ifdef DEBUG_PRINT_POLL
			PRINTK("sjt anticoll return =%d 0x%0x\n", ret, ret);
#endif
#ifdef DEBUG_PRINT_POLLWRONG
			PRINTK("***************sjt anticoll wrong*****************\n");
#endif
			mcml_pwr_off();
			if( ret & 0x80)
				return CE_NOCARD;
			else
				return CE_MULTI_TICKET;
		}
#ifdef DEBUG_PRINT_POLL
		PRINTK("sjt select %02x %02x %02x %02x %02x %02x %02x\n", cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3], 
					cpubuf[4], cpubuf[5], cpubuf[6], cpubuf[7]);
#endif
		ch_ul_phyical_id[0] = 0x00;//单程票
		memcpy(&ch_ul_phyical_id[1], cpubuf, 7);
		
		if(memcmp(xa_metro_psam_sfi, "\x2f\x01", 2) != 0)
		{
			ch_mac_sel = 5;
			sem_post(&g_samcalwait);
			/*
			memcpy(buf, "\x00\xa4\x00\x00\x02\x10\x02", 7);
			if(sam_apdu(xa_metro_psam_index, buf, 7, cpubuf, &atslen, 0, 0) != 0)
				return CE_METROPSAM;
	
			if(cpubuf[0] != 0x61)
				return CE_METROPSAM;
			memcpy(xa_metro_psam_sfi, "\x10\x02", 2);*/
		}/*else
			sem_post(&g_samreturn);*/
		memcpy(out_buf, "\x31\x11", 2);
		if((chCode = read_tocken_otp_info(&ul_page)) != 0)
			return chCode;
		//
		out_buf[0] = XA_SJT_FAMILY;
		memcpy(&out_buf[1], ch_ul_phyical_id, 8);
		out_buf[9] = chAntenna;//cmd_buf[6];
		//
		if(tpTicketDef.CanBeRecycled)
			out_buf[10] = 0x03;

		if(0 == ee_read(EE_UL_BACKUP, 9, ee_buf))
		{
			if((ee_buf[0] != 0) && (memcmp(ch_ul_phyical_id, &ee_buf[1], 8) == 0))
				sz_ul_ee_read();
		}
		reader_status = XA_RW_READ;
		xa_ticket_family = XA_SJT_FAMILY;
		blnsjtRollback = 0;
		*out_len = 10 + 1 + 2;
		return CE_OK;
	}
	else if((buf[0] == 0x00) && (buf[1] == 0x00))
	{
		mcml_pwr_off();
		set_timeout(2000);
#ifdef DEBUG_PRINT_POLLWRONG
			PRINTK("***************sjt CE_NOCARD wrong*****************\n");
#endif
		return CE_NOCARD;
	}else
	//else if((buf[0] == 0x04) && (buf[1] == 0x00))
	{
#ifdef DEBUG_PRINT_POLLWRONG
	PRINTK("\n***********request return %02x %02x i %02x\n", buf[0], buf[1], i);
#endif
#ifdef DEBUG_PRINT_POLLWRONG
			PRINTK("***************sjt mcml_anticoll1 wrong*****************\n");
#endif
		memcpy(out_buf, "\x31\x30", 2);
		ret = mcml_anticoll(cpubuf);
		if(ret != 0)
		{
			mcml_pwr_off();
			if( ret & 0x80)
			{

				return CE_NOCARD;
			}	
			else
			{

				return CE_MULTI_TICKET;
			}
				
		}
#ifdef DEBUG_PRINT_POLL
	PRINTK("\nanticoll  %02x %02x %02x %02x %02x\n", cpubuf[0], cpubuf[1], cpubuf[2], cpubuf[3], cpubuf[4]);
#endif
		memcpy(chUID, cpubuf, 5);
		//
		memcpy(out_buf, "\x31\x31", 2);
		ret = mcml_select(cpubuf, &i);
		if(ret != 0)
		{
			mcml_pwr_off();
#ifdef DEBUG_PRINT_POLLWRONG
			PRINTK("***************sjt mcml_anticoll2 wrong*****************\n");
#endif
			return CE_NOCARD;
		}
#ifdef DEBUG_PRINT_POLL
		PRINTK("\nselect return  %02x\n",  i);
#endif
//		if((ret = mcml_halt()) != 0)
//			return CE_NOCARD;
//		memcpy(out_buf, "\x10\x09", 2);
//#ifdef DEBUG_PRINT
//		PRINTK("\nhalt ok\n");
//#endif
//		for(i = 0; i < 3; i++)
//		{
//			if(mcml_request2(PICC_REQALL, buf) == 0)
//				break;
//		}
//#ifdef DEBUG_PRINT
//		PRINTK("\nrequest stardard again ok i = %d\n", i);
//#endif
//		if(i < 3)
//			return CE_MULTI_TICKET;
//		memcpy(out_buf, "\x10\x0A", 2);
//		for(i = 0; i < 3; i++)
//		{
//			if(mcml_request2(PICC_REQALL, buf) == 0)
//				break;
//		}
//#ifdef DEBUG_PRINT
//		PRINTK("\nrequest all again ok\n");
//#endif
//		memcpy(out_buf, "\x10\x0B", 2);
//		ret = mcml_anticoll(cpubuf);
//		if(ret != 0)
//		{
//			return CE_NOCARD;
//		}
//		memcpy(out_buf, "\x10\x11", 2);
//		ret = mcml_select(cpubuf, &i);
//		if(ret != 0)
//		{
//			return CE_NOCARD;
//		}
		//according to the SAK: b6=1 have to ats command for cpu card, otherwise is for m1
		if((i & 0x24) == 0x20)
		{
			set_card_type(ISO14443A_M1_TYPE);
			memset(ch_cpu20_phyical_id, 0x00, 8);
			memset(ch_cput_phyical_id, 0x00, 8);
			memset(ch_transport_phyical_id, 0x00, 8);
			memcpy(out_buf, "\x31\x32", 2);
			//ret = mifpro_ats(0, cpubuf, &atslen);
			ret = mifpro_ats(0x00, cpubuf, &atslen);
			if(ret != 0)
			{
				mcml_pwr_off();
				//delay_ms(5);
				*out_len = 2;
#ifdef DEBUG_PRINT_POLLWRONG
			PRINTK("***************sjt mifpro_ats wrong*****************\n");
#endif
				return CE_NOCARD;
			}
#ifdef DEBUG_PRINT_POLL
			PRINTK("ats:");
			for(i = 0; i < atslen; i++) PRINTK("%02x ", cpubuf[i]);
			PRINTK("\n");
#endif		
			memcpy(out_buf, "\x31\x33", 2);
			if(atslen <= 2)
			{
				mcml_pwr_off();
#ifdef DEBUG_PRINT_POLLWRONG
			PRINTK("***************sjt atslen <= 2 wrong*****************\n");
#endif
				return CE_READ;
			}
			//
			if(atslen > 8)
				memcpy(ch_cput_ats, &cpubuf[atslen - 8], 8);
			memcpy(out_buf, "\x31\x34", 2);
			//
			if( 0 != (chCode = CPU_GetFiles05(buf)) )
			{
				*out_len = 2;
				mcml_pwr_off();
#ifdef DEBUG_PRINT_POLLWRONG
			PRINTK("***************sjt CPU_GetFiles05 wrong*****************\n");
#endif
				return chCode;
			}else
			{
				memcpy(out_buf, "\x31\x35", 2);
				if(xa_ticket_family == XA_MCPU_FAMILY)
				{//xian metro cpu card
					ch_metro_edu_type = SZ_CPU_METRO_TYPE;
					memcpy(&ch_cpu20_phyical_id[4], chUID, 4);
					
					out_buf[0] = XA_MCPU_FAMILY;
					memcpy(&out_buf[1], ch_cpu20_phyical_id, 8);
					ch_sz_cpu_rollback = 0;
					for(i = 0; i < 10; i++)
					{
#ifdef	DEBUG_PRINT_POLL
						PRINTK("roll id %02x%02x%02x%02x back code %02x\n", tpMCPUProtect[i].phyicalID[4], tpMCPUProtect[i].phyicalID[5], tpMCPUProtect[i].phyicalID[6], tpMCPUProtect[i].phyicalID[7], tpMCPUProtect[i].rollBack);
#endif
						if((memcmp(ch_cpu20_phyical_id, tpMCPUProtect[i].phyicalID, 8) == 0) && (tpMCPUProtect[i].rollBack != 0))
						{
							ch_sz_cpu_rollback = tpMCPUProtect[i].rollBack;
							tpMCPUProtectIndex = i;
							break;
						}
					}
					out_buf[9] = chAntenna;
					xa_ticket_family = XA_MCPU_FAMILY;
					blncpuRollback = 0;
				}
				else if(xa_ticket_family == XA_TRANSPORT_FAMILY)
				{//deal with JTB
					ch_transport_phyical_id[0] = buf[0];
					memcpy(&ch_transport_phyical_id[4], chUID, 4);
					
					out_buf[0] = XA_TRANSPORT_FAMILY;
					memcpy(&out_buf[1], ch_transport_phyical_id, 8);
					out_buf[9] = chAntenna;
					ch_sz_transport_rollback = 0;
					for(i = 0; i < 10; i++)
					{
#ifdef	DEBUG_PRINT_POLL
						PRINTK("roll id %02x%02x%02x%02x back code %02x\n", tpTransportProtect[i].phyicalID[4], tpTransportProtect[i].phyicalID[5], tpTransportProtect[i].phyicalID[6], tpTransportProtect[i].phyicalID[7], tpTransportProtect[i].rollBack);
#endif
						if((memcmp(ch_transport_phyical_id, tpTransportProtect[i].phyicalID, 8) == 0) && (tpTransportProtect[i].rollBack != 0))
						{
							ch_sz_transport_rollback = tpTransportProtect[i].rollBack;
							tpTransportProtectIndex = i;
							break;
						}
					}
				}else 
				{//xian one-through card card
					ch_metro_edu_type = SZ_CPU_TONG_TYPE;
					ch_cput_phyical_id[0] = buf[0];
					memcpy(&ch_cput_phyical_id[4], chUID, 4);
					
					out_buf[0] = XA_CITY_FAMILY;
					memcpy(&out_buf[1], ch_cput_phyical_id, 8);
					out_buf[9] = chAntenna;
					ch_sz_cput_rollback = 0;
					for(i = 0; i < 10; i++)
					{
#ifdef	DEBUG_PRINT_POLL
						PRINTK("roll id %02x%02x%02x%02x back code %02x\n", tpXACPUProtect[i].phyicalID[4], tpXACPUProtect[i].phyicalID[5], tpXACPUProtect[i].phyicalID[6], tpXACPUProtect[i].phyicalID[7], tpXACPUProtect[i].rollBack);
#endif
						if((memcmp(ch_cput_phyical_id, tpXACPUProtect[i].phyicalID, 8) == 0) && (tpXACPUProtect[i].rollBack != 0))
						{
							ch_sz_cput_rollback = tpXACPUProtect[i].rollBack;
							tpXACPUProtectIndex = i;
							break;
						}
					}
					//if(0 == ee_read(EE_CITY_BACKUP, 9, ee_buf))
					//{
					//	if((ee_buf[0] !=0) && (memcmp(ch_cput_phyical_id, &ee_buf[1], 8) == 0))
					//		sz_tong_ee_read();
					//}
					blncputRollback = 0;
					xa_ticket_family = XA_CITY_FAMILY;
				}
			}
			reader_status = XA_RW_READ;
			*out_len = 10 + 1;
			return CE_OK;
		}
		else
		{//M1
#ifdef DEBUG_PRINT_POLLWRONG
			PRINTK("***************sjt M1 wrong*****************\n");
#endif
		
			return CE_NOCARD;
		}
	}
}

/*===============================================================================================
函数：szmt_rm_rf_on_off
功能：苏州地铁指令-射频选择与开关
=================================================================================================*/
char sz_rf_on_off(unsigned char *in_buf, unsigned char *out_buf, unsigned char *out_len)
{
	*out_len = 2;
	memcpy(out_buf, "\x02\x01", 2);
/*
	if(inbuf[1]&0x02)  rf_set(RF1);
		else rf_set(RF0);
  if(inbuf[1]&0x01)  mcml_pwr_off();
  	else mcml_pwr_on();
  	
  outbuf[0] = oldcmd;
	outbuf[1] = SZMT_OP_OK;
	*outbytes = 2;
	return;  

label_err:
	szmt_bom_func_err(outbuf,outbytes);		
	return;*/
	return 21;
}

char xa_founder_polling(unsigned char *cmd_buf)
{
unsigned char buf[100];

	if(cmd_buf[6] == 0x02)
	{//only A polling (default antenna)
		rf_select(0x00);
		if(mcml_request2(PICC_REQALL, buf) == 0)
		{
			cmd_buf[6] = 0x00;
			return 0;
		}
	}else if(cmd_buf[6] == 0x03)
	{//only B polling 
		rf_select(0x01);
		if(mcml_request2(PICC_REQALL, buf) == 0)
		{
			cmd_buf[6] = 0x01;
			return 0;
		}
	}else
	{//ALL
		rf_select(0x00);
		if(mcml_request2(PICC_REQALL, buf) == 0)
		{
			cmd_buf[6] = 0x00;
			return 0;
		}
		rf_select(0x01);
		if(mcml_request2(PICC_REQALL, buf) == 0)
		{
			cmd_buf[6] = 0x01;
			return 0;
		}
	}
	
	return CE_NOCARD;
}

char xa_pboc_select(unsigned char *inbuf, unsigned char inlen, unsigned char *ticketFamily)
{
int ret;
unsigned char i, buf[100], j;
unsigned char cpubuf[300];
unsigned char out_buf[1000];
unsigned short out_len, cpulen;

	free_tlv(&tlv_ppse);
	
	memset(&tlv_ppse, 0x00, sizeof(struct TLVEntity));
	tlv_ppse.UpArray = 1;
	construct_tlv(inbuf, inlen, &tlv_ppse, 1);
	//
	tag_bf0c = search_map_tlv(0xBF0C, &tlv_ppse);
	if(tag_bf0c == NULL)
		return CE_INVADLIDCARD;
	
	for(i = 0; i < tag_bf0c->ArraySize; i++)
	{
		if(tag_bf0c->Sub_TLVEntity[i].Tag == 0x61)
		{
			if(tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Tag == 0x4F)
			{
				if(memcmp("\xA0\x00\x00\x06\x32\x01\x01\x05", tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value, 8) == 0)
				{
					*ticketFamily = XA_TRANSPORT_FAMILY;
					break;
				}
			}
		}
	}
	if(i >= tag_bf0c->ArraySize)
		return CE_INVADLIDCARD;
//		memcpy(&transport_15_data[0], "\x02\x01\x79\x10\xff\xff\xff\xff", 8);
//		memcpy(&transport_15_data[12], "\x48\x00\x09\x99\x92\x92\x46\x46", 8);
//		check_JTB_Black_Lock(&transport_15_data[0], &transport_15_data[12], 0, out_buf, &out_len);
	//
	memcpy(buf, "\x00\xa4\x04\x00\x08", 5);
	buf[4] = tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Length;
	memcpy(&buf[5], tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value, tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Length);
	ret = mifpro_apdu(buf, 5 + buf[4], cpubuf, &cpulen);
#ifdef	DEBUG_PRINT
	PRINTK("select %02X%02X%02x%02x%02x%02x%02x%02x\n", tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[0], tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[1], tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[2], tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[3], tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[4],
				tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[5], tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[6], tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[7], tag_bf0c->Sub_TLVEntity[i].Sub_TLVEntity[0].Value[8]);
	for(j = 0; j < cpulen; j++)
		PRINTK("%02x", cpubuf[j]);
	PRINTK("\n");
#endif
	if(ret)
		return CE_READ;
	if( ((cpubuf[cpulen - 2] == 0x6A) && (cpubuf[cpulen - 1] == 0x81)) || ((cpubuf[cpulen - 2] == 0x93) && (cpubuf[cpulen - 1] == 0x03)) || ((cpubuf[cpulen - 2] == 0x62) && (cpubuf[cpulen - 1] == 0x83)))
	{
		return CE_LOCKED_TICKET;
	}
	if( (cpubuf[cpulen - 2] != 0x90) || (cpubuf[cpulen - 1] != 0x00) )
	{
		return CE_READ;
	}
	memset(&tlv_aid, 0x00, sizeof(struct TLVEntity));
	tlv_aid.UpArray = 1;
	construct_tlv(cpubuf, cpulen - 2, &tlv_aid, 1);
	fci_9f0c = search_map_tlv(0x9f0c, &tlv_aid);
	if(fci_9f0c == NULL)
	{
		if( 0 != Transport_GetFiles15(&buf[1]) )
			return CE_READ;
	}else
	{
		memcpy(transport_15_data, fci_9f0c->Value, fci_9f0c->Length);
#ifdef DEBUG_PRINT
		PRINTK("file 15 len :%02x, logicid: %02x %02x %02x %02x %02x %02x %02x %02x appindex:%02x app ver:%02x \n", fci_9f0c->Length, transport_15_data[0], transport_15_data[1], transport_15_data[2], transport_15_data[3],
				transport_15_data[4], transport_15_data[5], transport_15_data[6], transport_15_data[7], transport_15_data[8], transport_15_data[9]);
		PRINTK("app sn: %02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
				 transport_15_data[10], transport_15_data[11], transport_15_data[12], transport_15_data[13], transport_15_data[14], transport_15_data[15], transport_15_data[16], transport_15_data[17], transport_15_data[18], transport_15_data[19]);
		PRINTK(" app startdate %02x%02x%02x%02x valid date %02x%02x%02x%02x fci %02x%02x\n", 
				transport_15_data[20], transport_15_data[21], transport_15_data[22], transport_15_data[23], transport_15_data[24], transport_15_data[25], transport_15_data[26], transport_15_data[27], transport_15_data[28], transport_15_data[29]);
#endif
	}
	memcpy(ch_transport_logic_id, &transport_15_data[10], 10);
	return 0;
}