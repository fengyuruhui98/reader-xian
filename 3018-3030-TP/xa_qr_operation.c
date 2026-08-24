#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "linux2440lib.h"
#include "xa_error_code.h"
#include "bin_file_manage.h"
#include "xa_sam.h"
#include "serial.h"
#include "hh_cpu_operation.h"
#include "eeprom.h"
#include "xa_operation.h"
#include "time_tools.h"
#include "xa_qr_operation.h"

const unsigned char *iACC_id = "UCITY.NBMETRO.iTPS.QRFACTORY";
unsigned char *iACC_id_len = "\x00\xE0";
const unsigned char *plat_id = "UCITY.NBMETRO.iTPS.QRFACTORY";
unsigned char *plat_id_len = "\x00\xE0";


const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char * base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
int i, j;
unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int base64_decode( const char * base64, unsigned char * bindata )
{
int i, j;
unsigned char k;
unsigned char temp[4];

#ifdef	DEBUG_PRINT
	PRINTK("base64 src: %s\n", base64);
#endif
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}

unsigned short qr_send_recv(int fd, short *out_len)
{
struct timeval timeout;
fd_set readfd;
long lreadlen, ret, i;
unsigned char fStat, blnTimeout, recv_buf[8196];
unsigned char buff[600];

	if(fd <= 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("no opening comm:%d timeout %d\n", fd, qr_timeout);
#endif
		return 0;
	}

	//clear the data received but not read
	//tcflush(fd, TCIFLUSH);
	memset(recv_buf, 0x00, sizeof(recv_buf));
	memset(qr_info, 0x00, sizeof(qr_info));
	//
	lreadlen = 0;
	blnTimeout = 0xff;
	//
	FD_ZERO(&readfd);
	FD_SET(fd, &readfd);
	timeout.tv_sec = 0;
	timeout.tv_usec = qr_timeout * 1000;
	ret = select(fd + 1, &readfd, NULL, NULL, &timeout);
	if(ret > 0)
	{
		do
		{
			fStat = readcom(fd, buff, 64);
			if(fStat)
			{
				memcpy(&recv_buf[lreadlen], buff, fStat);
				lreadlen += fStat;
			}
			if(lreadlen > 500)
			{
#ifdef	DEBUG_PRINT
				PRINTK("qr read data len to long:\n", lreadlen);
#endif
				tcflush(fd, TCIFLUSH);
				return 0;
			}
			FD_ZERO(&readfd);
			FD_SET(fd, &readfd);
			timeout.tv_sec = 0;
			timeout.tv_usec = 9000;
			ret = select(fd + 1, &readfd, NULL, NULL, &timeout);
			if(ret > 0)
			{
				continue;
			}else
			{
#ifdef	DEBUG_PRINT
				PRINTK("qr read data len %d:\n", lreadlen);
				for(i = 0; i < lreadlen; i++)
					PRINTK("%02x", recv_buf[i]);
				PRINTK("\n");
#endif
				if(memcmp(recv_buf, "\xff\xff", 2) != 0)
				{//说明报文头不对
#ifdef	DEBUG_PRINT
					PRINTK("qr read data package format error\n");
#endif
					return 0;
				}
				blnTimeout = 0;
				memcpy(qr_info, &recv_buf[2], lreadlen - 4);
				lreadlen -= 4;
				//recv_buf[lreadlen - 2] = 0x00;
				//lreadlen = base64_decode(&recv_buf[2], qr_info);
#ifdef	DEBUG_PRINT
				PRINTK("qr decode data len %d:\n", lreadlen);
				for(i = 0; i < lreadlen; i++)
					PRINTK("%02x", qr_info[i]);
				PRINTK("\n");
#endif
				return lreadlen;
			}
		}while(blnTimeout);
	}else if(ret == 0)
	{
#ifdef	DEBUG_PRINT
		PRINTK("qr (%d) read data timeout: %d\n", fd, qr_timeout);
#endif
		;
	}else
	{
#ifdef	DEBUG_PRINT
		PRINTK("qr select failure\n");
#endif
		;
	}
	return 0;
}


unsigned char xa_polling_qr(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char valid_date[7];
unsigned long i;
	
	//
	out_buf[0] = XA_QR_FAMILY;
	//卡片物理ID
	memset(&out_buf[1], 0x00, 8);
	//天线标志
	out_buf[9] = 0x00;
	//回收标志
	out_buf[10] = 0x00;;
	//二维码长度
	out_buf[11] = (unsigned char)qr_len;
	out_buf[12] = (unsigned char)(qr_len >> 8);
	//二维码
	memcpy(&out_buf[13], qr_info, qr_len);

#ifdef	DEBUG_PRINT
	PRINTK("qr_len %02x qr_info data \n", qr_len);
	for(i = 0; i < qr_len; i++)
		PRINTK("%02x", qr_info[i]);
	PRINTK("\n");
#endif
	*out_len = 11 + 2 + qr_len;

	return CE_OK;
}


//unsigned short dl_qr_logic(unsigned char *cmd_buf)
//{
//unsigned short shCode;
//unsigned char metro_type, blnPAY;
//unsigned char buf[100], factor[16], mac[16];
//unsigned char cpubuf[300], Le;
//unsigned short cpulen;
//int ret, i;
//unsigned char Xa[32], Ya[32];
//
//	if((memcmp(dl_psam_adf, "\x10\x01", 2) != 0) && (dl_metro_psam_status == 0))
//	{
//		if(0 != sam_select_file(dl_metro_psam_index, "\x10\x01", 2, buf))
//			return RC_NOPSAM;
//		memcpy(dl_psam_adf, "\x10\x01", 2);
//	}
//	//发码平台验签
//	//1、	801A450110 + 8字节 用户标识 +8字节 城市分散代码（目前固定为：1160FF0000000000）
//	//2、	80FA050030 + 8字节 0000000000000000 + 8字节 二维码凭证号 + 8字节 移动应用标识 + 4字节 移动应用机构代码
//	//				8字节 用户标识 + 4字节 二维码生成时间 + 2字节 二维码有效时间 + 2字节 行业使用范围 + 补4字节，0x80000000
//	memcpy(factor, tpQR_Info_val.UserID, 8);
//	memcpy(&factor[8], "\x11\x60\xFF\x00\x00\x00\x00\x00", 8);
//	memset(buf, 0x00, 100);
//	memcpy(&buf[8], tpQR_Info_val.PlatformID, 8);
//	memcpy(&buf[16], tpQR_Info_val.AppID, 8);
//	memcpy(&buf[24], tpQR_Info_val.Appcode, 4);
//	memcpy(&buf[28], tpQR_Info_val.UserID, 8);
//	LongToByte(tpQR_Info_val.GenerateQRtime, &buf[36]);
//	ShortToByte(tpQR_Info_val.qr_Delay, &buf[40]);
//	memcpy(&buf[42], tpQR_Info_val.limitBusiness, 2);
//	memcpy(&buf[44], "\x80\x00\x00\x00", 4);
//	if(0 != cpu_cal_dcmk(dl_metro_psam_index, "\x45\x01", factor, 16, 0x05, buf, 48, mac, &Le) )
//		return RC_LINEMAC;
//	if( memcmp(mac, tpQR_Info_val.PlatMAC, 4) != 0)
//		return RC_PSAMKEY;
//	//用户行程验签
//	//1、	801A440110  + 8字节 二维码凭证号 +8字节 城市分散代码（目前固定为：1160FF0000000000）
//	//2、	80FA050020 + 8字节 0000000000000000 + 1字节 行业自定义数据长度 + 1字节 凭证类型 + 1字节 进出站使用限制
//	//				8字节 站点限制 + 8字节 行程单号 + 补5字节，0x8000000000
//	memcpy(factor, tpQR_Info_val.PlatformID, 8);
//	memcpy(&factor[8], "\x11\x60\xFF\x00\x00\x00\x00\x00", 8);
//	memset(buf, 0x00, 100);
//	buf[8] = tpQR_Info_val.BusinessLen;
//	buf[9] = tpQR_Info_val.IDType;
//	buf[10] = tpQR_Info_val.limitEntryExit;
//	memcpy(&buf[11], tpQR_Info_val.limitStation, 8);
//	memcpy(&buf[19], tpQR_Info_val.Travel, 8);
//	memcpy(&buf[27], "\x80\x00\x00\x00\x00", 5);
//	if(0 != cpu_cal_dcmk(dl_metro_psam_index, "\x44\x01", factor, 16, 0x05, buf, 32, mac, &Le) )
//		return RC_LINEMAC;
//	if( memcmp(mac, tpQR_Info_val.UserMAC, 4) != 0)
//		return RC_PSAM;
//	
//	//
//	memcpy(&tpQR1005.qrVersion, &tpQR_Info_val.Plat, 6);
//	memcpy(tpQR1005.qrPlatcode, tpQR_Info_val.Appcode, 4);
//	memcpy(tpQR1005.qrLimitbusiness, tpQR_Info_val.limitBusiness, 2);
//	tpQR1005.qrLimittype = tpQR_Info_val.limitEntryExit;
//	memcpy(tpQR1005.qrLimitStation, tpQR_Info_val.limitStation, 8);
//	memcpy(tpQR1005.qrPlatMAC, tpQR_Info_val.PlatMAC, 8);
//	memcpy(tpQR1005.qrAPPid, tpQR_Info_val.AppID, 8);
//	
//	return 0;
//}
//
//unsigned short qr_TellValidateArea()
//{
//char chCode;
//unsigned int	i, j;
//unsigned long lngCurLineMap, lngCardLineMap;
//unsigned char chForbidStation[2][2];
//
//	//no forid area
//	if(0 == (memcmp(tpQR_Info_val.limitStation, "\x00\x00\x00\x00\x00\x00\x00\x00", 8)))
//		return 0;
//	//
//	for(i = 0; i < 4; i++)
//	{
//		if(memcmp(&tpQR_Info_val.limitStation[i * 2], tpCPU.curstation, 2) == 0)
//			return 0;
//	}
//	//
//		
//	return RC_FORBIDZONE;
//}
//
//unsigned short dl_qr_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
//{
//unsigned short shCode, shRejectCode;
//unsigned char buf[300], factor[24], des[80], deslen, chFare, chEntryStatus;
//unsigned char cpubuf[300], cpulen, Le, entry_time[4], last_timebcd[7];
//unsigned char status, metro_type;
//unsigned long time1,time2;
//unsigned short tempdate, shTicketType, cnt;
//long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
//long lngCardBalance, ret, i, j;
//unsigned short shDays;
//unsigned long lngMidnightSecond;
//
//	
//
//label_nb_inquire_error:
//	
//	out_buf[0] = 1;
//	//antenna
//	out_buf[1] = 3;
//	//
//	out_buf[2] = DL_QR_FAMILY;
//	
//	//
//	if((shCode = get_ticket_para(BUSINESS_QR, tpQR_Info_val.IDType, &tpTicketDef)) != 0)
//		return shCode;
//	*out_len = 29 + 23;
//
//	reader_status = DL_RW_READ;
//	return RC_OK;
//}
//
//
///************************************
//qr entry
//************************************/
//unsigned short dl_qr_entry_prepare(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
//{
//unsigned short shCode, shRejectCode, shRefuseCode;
//unsigned char buf[300], factor[24], des[80], deslen, chFare, chEntryStatus;
//unsigned char cpubuf[400], Le, entry_time[4], last_timebcd[7];
//unsigned short cpulen;
////unsigned char cnt;
//unsigned char status, history_record_pointer, metro_type;
//unsigned long time1,time2;
//unsigned short tempdate, shTicketType, cnt, shTranAmount, shBonus;
//long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
//long lngCardBalance, ret, i, j;
//unsigned short shDays, shSpecialYear, shCurYear;
//unsigned long lngMidnightSecond;
//
//	
//	tpQR1005.transType = TX_ENTRY;
//	memcpy(tpQR1005.transTime, tpCPU.time_bcd, 7);
//	//
//	if( (shCode = dl_qr_logic(cmd_buf)) != 0)
//		return shCode;	
//	if( memcmp(tpCPU.time_bcd, tpQR1005.qrValiddate, 7) > 0 )
//		return RC_OVERDATE;
//	if( memcmp(tpCPU.time_bcd, tpQR1005.qrGentime, 7) < 0 )
//		return RC_OVERDATE;
//	//
//	if((shCode = get_ticket_para(BUSINESS_QR, tpQR_Info_val.IDType, &tpTicketDef)) != 0)
//		return shCode;
//	tpQR1005.ticketType[0] = (unsigned char)(tpTicketDef.ticketType >> 8);
//	tpQR1005.ticketType[1] = (unsigned char)tpTicketDef.ticketType;
//	tpQR1005.feeType = tpTicketDef.feeType;
//	tpQR1005.payType = DL_PAYTYPE_QR;
//	memset(tpQR1005.transAmount, 0x00, 4);
//	
//	//
//	if(0 != (shCode = qr_TellValidateArea()) )
//		return shCode;
//	//
//	if( (tpQR_Info_val.limitEntryExit != 0x01) && (tpQR_Info_val.limitEntryExit != 0x02) )
//		return RC_INVALID;
//	if(tpQR_Info_val.limitEntryExit == 0x02)
//		return RC_NO_ENTRY;
//	//
//	memcpy(&tpQR1005.transSN[0], &cmd_buf[30], 4);
//	memcpy(&tpQR1005.transSN[4], &cmd_buf[37], 4);
//	memcpy(tpQR1005.qrID, tpQR_Info_val.PlatformID, 8);
//	memcpy(tpQR1005.qrTravel, tpQR_Info_val.Travel, 8);
//	tpQR1005.qrBusiness = 0x01;
//	memcpy(tpQR1005.qrUserID, tpQR_Info_val.UserID, 8);
//	tpQR1005.qrUserType = tpQR_Info_val.IDType;
//	lngMidnightSecond = tpQR_Info_val.GenerateQRtime + TIME2017;
//	long2timestr(lngMidnightSecond, tpQR1005.qrGentime);
//	tpQR1005.qrRFU2 =  0x00;
//	tpQR1005.line = tpCmdInit.deviceID[0];
//	memcpy(tpQR1005.station, &tpCmdInit.deviceID[0], 2);
//	tpQR1005.qrEntryExit = 0x01;
//	//不支持联机方式
//	return dl_qr_entry(cmd_buf, out_buf, out_len);
//	
//	*out_len = 48;
//
//	//命令代码	HEX	1	0xB1
//	memcpy(&out_buf[0], "\xb1", 1);
//	//后续数据长度	HEX	2	后序数据长度，0x002D
//	out_buf[1] = 0x00;
//	out_buf[2] = 0x2D;
//	//终端交易流水号	HEX	8	设备下发的终端交易流水号
//	memcpy(&out_buf[3], &cmd_buf[30], 4);
//	memcpy(&out_buf[7], &cmd_buf[37], 4);
//	//二维码凭证号	HEX	8	扫描到的二维码的二维码凭证号
//	memcpy(&out_buf[11], tpQR_Info_val.PlatformID, 8);
//	//移动应用ID	HEX	8	扫描到的二维码上的移动应用ID 
//	memcpy(&out_buf[19], tpQR_Info_val.AppID, 8);
//	//业务标识	HEX	1	0x01：地铁应用；
//	tpQR1005.qrBusiness = out_buf[27] = 0x01;
//	//用户标识	HEX	8	扫描到的二维码上的用户标识
//	memcpy(&out_buf[28], tpQR_Info_val.UserID, 8);
//	//凭证生成时间	HEX	7	扫描到的二维码上的二维码生成时间，格式：YYYYMMDDhhmmss
//	long2timestr(lngMidnightSecond, &out_buf[36]);
//	//RFU	HEX	1	预留备用信息，暂定填写0x00
//	out_buf[43] = 0x00;
//	//线路号	HEX	1	当前设备所属线路号
//	out_buf[44] = tpCmdInit.deviceID[0];
//	//站点号	HEX	2	当前设备所属站点号
//	memcpy(&out_buf[45], &tpCmdInit.deviceID[0], 2);
//	//进出站标识	HEX	1	0x01：进站；0x02：出站；
//	out_buf[47] = 0x01;
//	//
//	memcpy(&cmd_buf[11], "\x90\x91", 2);
//	return RC_OK;
//}
///************************************
//qr entry
//************************************/
//unsigned short dl_qr_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
//{
//unsigned short shCode, shRejectCode, shRefuseCode;
//unsigned char buf[300], factor[24], des[80], deslen, chFare, chEntryStatus;
//unsigned char cpubuf[400], Le, entry_time[4], last_timebcd[7];
//unsigned short cpulen;
////unsigned char cnt;
//unsigned char status, history_record_pointer, metro_type;
//unsigned long time1,time2;
//unsigned short tempdate, shTicketType, cnt, shTranAmount, shBonus;
//long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
//long lngCardBalance, ret, i, j;
//unsigned short shDays, shSpecialYear, shCurYear;
//unsigned long lngMidnightSecond;
//
//	LongToByte(HardSN, tpQR1005.crwSN);
//	HardSN += 1;
//	write_eeprom_sn(HardSN);
//
//	//recycle - 1
//	out_buf[4] = 0;
//	//phyical type - 1
//	out_buf[5] = DL_QR_FAMILY;
//	//ticket type -	2
//	out_buf[6] = tpQR1005.ticketType[0];
//	out_buf[7] = tpQR1005.ticketType[1];
//	//ticket serial number - 6
//	memcpy(&out_buf[8], &tpQR1005.qrUserID[2], 6);
//	//fee type - 1
//	out_buf[14] = tpQR1005.feeType;
//	//valid date - 4
//	memcpy(&out_buf[15], tpQR1005.qrValiddate, 4);
//	//交易金额	-	4
//	memset(&out_buf[19], 0x00, 4);
//	//交易后余额/余次-	4
//	LongToByte(0, &out_buf[23]);
//	//RFU - 9
//	memset(&out_buf[27], 0x00, 9);
//	
//	*out_len = 37 + 3 + sizeof(QR_1005_t);
//	//UD number
//	out_buf[36] = 0x01;
//	//UD type
//	out_buf[37] = 0x05;
//	//UD length
//	ShortToByte(sizeof(QR_1005_t), &out_buf[38]);
//	memcpy(&out_buf[40], &tpQR1005.deviceType, sizeof(QR_1005_t));
//	
//	memcpy(&cmd_buf[11], last_deal_command, 2);
//	return RC_OK;
//}
///************************************
//qr exit
//************************************/
//unsigned short dl_qr_exit_prepare(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
//{
//unsigned short shCode, shRejectCode, shRefuseCode;
//unsigned char buf[500], factor[20], des[80], deslen, chExitStatus;
//unsigned char cpubuf[500], Le, history_record_pointer, chFare;
//unsigned short cpulen;
//unsigned char status, entry_station[2], last_timebcd[7], metro_type;
//unsigned long time1,time2, cnt, lngsrcstation;
//unsigned short tempdate, shTicketType, shFare, shTranAmount, shBonus;
//long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
//long lngCardBalance, ret, i, j;
//unsigned short shDays;
//unsigned long lngMidnightSecond;
//
//	
//	tpQR1005.transType = TX_EXIT;
//	memcpy(tpQR1005.transTime, tpCPU.time_bcd, 7);
//	//
//	if( (shCode = dl_qr_logic(cmd_buf)) != 0)
//		return shCode;	
//	if( memcmp(tpCPU.time_bcd, tpQR1005.qrValiddate, 7) > 0 )
//		return RC_OVERDATE;
//	if( memcmp(tpCPU.time_bcd, tpQR1005.qrGentime, 7) < 0 )
//		return RC_OVERDATE;
//	//
//	if((shCode = get_ticket_para(BUSINESS_QR, tpQR_Info_val.IDType, &tpTicketDef)) != 0)
//		return shCode;
//	tpQR1005.ticketType[0] = (unsigned char)(tpTicketDef.ticketType >> 8);
//	tpQR1005.ticketType[1] = (unsigned char)tpTicketDef.ticketType;
//	tpQR1005.feeType = tpTicketDef.feeType;
//	tpQR1005.payType = DL_PAYTYPE_QR;
//	memset(tpQR1005.transAmount, 0x00, 4);
//	//
//	if(0 != (shCode = qr_TellValidateArea()) )
//		return shCode;
//	//
//	if( (tpQR_Info_val.limitEntryExit != 0x01) && (tpQR_Info_val.limitEntryExit != 0x02) )
//		return RC_INVALID;
//	if(tpQR_Info_val.limitEntryExit == 0x01)
//		return RC_NO_ENTRY;
//	//
//	memcpy(&tpQR1005.transSN[0], &cmd_buf[30], 4);
//	memcpy(&tpQR1005.transSN[4], &cmd_buf[37], 4);
//	memcpy(tpQR1005.qrID, tpQR_Info_val.PlatformID, 8);
//	memcpy(tpQR1005.qrTravel, tpQR_Info_val.Travel, 8);
//	tpQR1005.qrBusiness = 0x01;
//	memcpy(tpQR1005.qrUserID, tpQR_Info_val.UserID, 8);
//	tpQR1005.qrUserType = tpQR_Info_val.IDType;
//	lngMidnightSecond = tpQR_Info_val.GenerateQRtime + TIME2017;
//	long2timestr(lngMidnightSecond, tpQR1005.qrGentime);
//	tpQR1005.qrRFU2 =  0x00;
//	tpQR1005.line = tpCmdInit.deviceID[0];
//	memcpy(tpQR1005.station, &tpCmdInit.deviceID[0], 2);
//	tpQR1005.qrEntryExit = 0x02;
//	//不支持联机方式
//	return dl_qr_exit(cmd_buf, out_buf, out_len);
//	
//	*out_len = 48;
//
//	//命令代码	HEX	1	0xB1
//	memcpy(&out_buf[0], "\xb1", 1);
//	//后续数据长度	HEX	2	后序数据长度，0x002D
//	out_buf[1] = 0x00;
//	out_buf[2] = 0x2D;
//	//终端交易流水号	HEX	8	设备下发的终端交易流水号
//	memcpy(&out_buf[3], &cmd_buf[30], 4);
//	memcpy(&out_buf[7], &cmd_buf[37], 4);
//	//二维码凭证号	HEX	8	扫描到的二维码的二维码凭证号
//	memcpy(&out_buf[11], tpQR_Info_val.PlatformID, 8);
//	//移动应用ID	HEX	8	扫描到的二维码上的移动应用ID 
//	memcpy(&out_buf[19], tpQR_Info_val.AppID, 8);
//	//业务标识	HEX	1	0x01：地铁应用；
//	out_buf[27] = 0x01;
//	//用户标识	HEX	8	扫描到的二维码上的用户标识
//	memcpy(&out_buf[28], tpQR_Info_val.UserID, 8);
//	//凭证生成时间	HEX	7	扫描到的二维码上的二维码生成时间，格式：YYYYMMDDhhmmss
//	long2timestr(lngMidnightSecond, &out_buf[36]);
//	//RFU	HEX	1	预留备用信息，暂定填写0x00
//	out_buf[43] = 0x00;
//	//线路号	HEX	1	当前设备所属线路号
//	out_buf[44] = tpCmdInit.deviceID[0];
//	//站点号	HEX	2	当前设备所属站点号
//	memcpy(&out_buf[45], &tpCmdInit.deviceID[0], 2);
//	//进出站标识	HEX	1	0x01：进站；0x02：出站；
//	out_buf[47] = 0x02;
//	//
//	memcpy(&cmd_buf[11], "\x90\x91", 2);
//	return RC_OK;
//}
///************************************
//qr exit
//************************************/
//unsigned short dl_qr_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
//{
//unsigned short shCode, shRejectCode, shRefuseCode;
//unsigned char buf[500], factor[20], des[80], deslen, chExitStatus;
//unsigned char cpubuf[500], Le, history_record_pointer, chFare;
//unsigned short cpulen;
//unsigned char status, entry_station[2], last_timebcd[7], metro_type;
//unsigned long time1,time2, cnt, lngsrcstation;
//unsigned short tempdate, shTicketType, shFare, shTranAmount, shBonus;
//long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
//long lngCardBalance, ret, i, j;
//unsigned short shDays;
//unsigned long lngMidnightSecond;
//
//	
//	//recycle - 1
//	out_buf[4] = 0;
//	//phyical type - 1
//	out_buf[5] = DL_QR_FAMILY;
//	//ticket type -	2
//	out_buf[6] = tpQR1005.ticketType[0];
//	out_buf[7] = tpQR1005.ticketType[1];
//	//ticket serial number - 6
//	memcpy(&out_buf[8], &tpQR1005.qrUserID[2], 6);
//	//fee type - 1
//	out_buf[14] = tpQR1005.feeType;
//	//valid date - 4
//	memcpy(&out_buf[15], tpQR1005.qrValiddate, 4);
//	//交易金额	-	4
//	memset(&out_buf[19], 0x00, 4);
//	//交易后余额/余次-	4
//	LongToByte(0, &out_buf[23]);
//	//RFU - 9
//	memset(&out_buf[27], 0x00, 9);
//	
//	*out_len = 37 + 3 + sizeof(QR_1005_t);
//	//UD number
//	out_buf[36] = 0x01;
//	//UD type
//	out_buf[37] = 0x05;
//	//UD length
//	ShortToByte(sizeof(QR_1005_t), &out_buf[38]);
//	memcpy(&out_buf[40], &tpQR1005.deviceType, sizeof(QR_1005_t));
//	
//	return RC_OK;
//}
///************************************
//qr update
//************************************/
//unsigned short dl_qr_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
//{
//unsigned short shCode, shRejectCode, shRefuseCode;
//unsigned char buf[500], factor[20], des[80], deslen, chExitStatus;
//unsigned char cpubuf[500], Le, history_record_pointer, chFare;
//unsigned short cpulen;
//unsigned char status, entry_station[2], last_timebcd[7], metro_type;
//unsigned long time1,time2, cnt, lngsrcstation;
//unsigned short tempdate, shTicketType, shFare, shTranAmount, shBonus;
//long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
//long lngCardBalance, ret, i, j;
//unsigned short shDays;
//unsigned long lngMidnightSecond;
//
//	
//	return RC_OK;
//
//}