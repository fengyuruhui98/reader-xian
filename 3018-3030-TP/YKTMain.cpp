#include <windows.h>
#include <stdio.h>
#include<ctime>
#include "CommBase.h"
#include "DCR4000Comm.h"
#include "YKTMain.h"
//#include "DesEnc.h"
#include "StringOper.h"
#include "ShortCom.h"

#define uchar unsigned char


#pragma comment(lib, "DCR4000Comm.lib")
//#pragma comment(lib, "DesEnc.lib")
#pragma comment(lib, "UtilStrOper.lib")
#pragma comment(lib, "ShortCom.lib")

void HexStrToByte(char* source, char* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;
    
    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return;
}

void BytesToHexStr(BYTE *SouByte, int Len, BYTE *DesStr)
{

	int i;
	BYTE HighByte, LowByte;
	
	for (i = 0; i < Len; i++)
	{
		HighByte = SouByte[i] >> 4;
		LowByte  = SouByte[i] & 0x0f ;
	    
		HighByte += 0x30;
		if (HighByte > 0x39) DesStr[i * 2] = HighByte + 0x07;
		else DesStr[i * 2] = HighByte;

		LowByte += 0x30;
		if (LowByte > 0x39) DesStr[i * 2 + 1] = LowByte + 0x07;
		else DesStr[i * 2 + 1] = LowByte;
	}
}

void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;

    for (i = 0; i < sourceLen; i++)
    {
        highByte = source[i] >> 4;
        lowByte = source[i] & 0x0f ;

        highByte += 0x30;

        if (highByte > 0x39)
                dest[i * 2] = highByte + 0x07;
        else
                dest[i * 2] = highByte;

        lowByte += 0x30;
        if (lowByte > 0x39)
            dest[i * 2 + 1] = lowByte + 0x07;
        else
            dest[i * 2 + 1] = lowByte;
    }
    return;
}

int _stdcall SysLogIn(int Com, unsigned long Baud, uchar *YKTS_IP, int YKTS_Port,uchar *POSID,uchar *OPERID,uchar *OPERPWD,uchar *CorpID,uchar *SysDatetime, int *LimitAmt, uchar *LimitTime,uchar  *ParamUpdateFlag)
{
	/* 函数参数句柄 */
	HANDLE phComm  = INVALID_HANDLE_VALUE;
	/* 单位号 */
	char szCorpID[9] = {0};
	/* DAT函数应答码 */
	ushort nRet = 0;
	/* ISAM卡ATR */
	uchar ucSAMATR[128]={0};
	/* ISAM卡ATR长度 */
	short Length = 0;
	/* APDU指令 */
	uchar szAPDU[128] = {0};
	/* APDU指令返回可见 */
	uchar szResp[128] = {0};
	/* APDU指令返回不可见 */
	char cszResp[128] = {0};
	/* APDU指令返回长度 */
	ushort rLen = 0;
	/* 签到报文返回 */
	uchar szSysLogInResp[2048]={0};
	/* 签到报文体 */
	char szAuthUpstr[512] = {0};
	/* 签到报文 */
	char szAuthUp[1024] = {0};
	/* 签到报文返回不可见 */
	char szAuthUpRev[1024] = {0};
	/* 签到报文不可见 */
	char szAuthUpByte[1024] = {0};
	/* MAC */
	char szMac[9] = {0};
	/* MAC不可见 */
	char szMacHEX[9] = {0};
	/* 16位SAMId */
	char szSAMIdBCD[17] ={0};
	/* CHAR型端口号*/
	char szPort[6]={0};
	/* CHAR型IP地址 */
	char szIP[48]={0};
	/* 12位POSID */
	char szhPOSID[13]={0};
	/* 16位操作员ID */
	char szhOPERID[17]={0};
	/* 16位操作员密码 */
	char szhOPERPWD[17]={0};
	/* 批次号 */
	char szBatchNo[7]={0};
	/* 16位SAMId不可见 */
	uchar szSAMId[13] ={0};
	/* 终端机设备号 */
	char szTermid[13] ={0};
	/* 随机数 */
	char Random[17] = {0};
	/* 应答码 */
	char szCode[5] = {0};
	char szLimitAmt[9] = {0};
	char szLimitAmtHEX[9] = {0};
	/* 报文应答码 */
	char szRevCode[5]={0};
	/* 报文应答码未转 */
	char szRevCodeHEX[5]={0};
	int Amt = 0;
	int nLen = 0;
	int sLen = 0;
	int Ret = 0;
	sprintf(szPort, "%d", YKTS_Port);
	char szLocalTime[15] = {0};
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);
	_snprintf(szLocalTime, sizeof(szLocalTime), "%4d%02d%02d%02d%02d%02d", 
		    		stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond);

	nLen=12 - strlen((const char *)POSID);
	if(nLen>0)
	{
		memset(szhPOSID, 0, sizeof(szhPOSID));
		memcpy(szhPOSID,"000000000000",12);
		memcpy(szhPOSID+nLen,POSID,12-nLen);
	}else
	{
		return -8;
	}
	
	nLen=0;
	nLen=16 - strlen((const char *)OPERID);
	if(nLen>0)
	{
		memset(szhOPERID, 0, sizeof(szhOPERID));
		memcpy(szhOPERID,"0000000000000000",16);
		memcpy(szhOPERID+nLen,OPERID,16-nLen);
	}else
	{
		return -9;
	}
	
	nLen=0;
	nLen=16 - strlen((const char *)OPERPWD);
	if(nLen>0)
	{
		memset(szhOPERPWD, 0, sizeof(szhOPERPWD));
		memcpy(szhOPERPWD,"0000000000000000",16);
		memcpy(szhOPERPWD+nLen,OPERPWD,16-nLen);
	}else
	{
		return -10;
	}
	
	nLen=0;
	memset(szCorpID, 0, sizeof(szCorpID));
	ByteToHexStr(CorpID,szCorpID,4);
	
	if(0 != (nRet= sReaderStart(Com,Baud,&phComm)))
	{
		sReaderStop(phComm);
		
        return nRet;
	}

	if(0 != (nRet= sSelectCard(2,phComm)))
	{
		sReaderStop(phComm);

        return nRet;
	}

	if(0 != (nRet= sISamReadATR(ucSAMATR,&Length,phComm)))
	{
		sReaderStop(phComm);

        return nRet;
	}
	//选择15文件
	if(0 != (nRet= sISamAPDU(7,(const uchar *)"\x00\xA4\x00\x00\x02\x00\x15",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);
        return nRet;
	}
	//读取15文件的前14个字节
	memset(szResp, 0, sizeof(szResp));
	if(0 != (nRet= sISamAPDU(5,(const uchar *)"\x00\xB0\x00\x00\x0E",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);

        return nRet;
	}

	memset(szSAMIdBCD, 0, sizeof(szSAMIdBCD));
	BytesToHexStr((unsigned char *)szResp,rLen,(unsigned char *)cszResp);
	memcpy(szSAMIdBCD,cszResp+4,16);
	//选择16文件
	if(0 != (nRet= sISamAPDU(7,(const uchar *)"\x00\xA4\x00\x00\x02\x00\x16",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);

        return nRet;
	}
	//读取前6个字节
	memset(szResp, 0, sizeof(szResp));
	memset(cszResp, 0, sizeof(cszResp));
	if(0 != (nRet= sISamAPDU(5,(const uchar *)"\x00\xB0\x00\x00\x06",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);
		
        return nRet;
	}

	memset(szTermid, 0, sizeof(szTermid));
	BytesToHexStr((unsigned char *)szResp,rLen,(unsigned char *)cszResp);
	memcpy(szTermid,cszResp,12);
	//选择1001 ADF
	if(0 != (nRet= sISamAPDU(7,(const uchar *)"\x00\xA4\x00\x00\x02\x10\x01",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);

        return nRet;
	}
	//取随机数
	memset(szResp, 0, sizeof(szResp));
	memset(cszResp, 0, sizeof(cszResp));
	if(0 != (nRet= sISamAPDU(5,(const uchar *)"\x00\x84\x00\x00\x08",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);
		
        return nRet;
	}

	memset(Random, 0, sizeof(Random));
	BytesToHexStr((unsigned char *)szResp,rLen,(unsigned char *)cszResp);
	memcpy(Random,cszResp,16);

	memset(szAuthUpstr, 0, sizeof(szAuthUpstr));
	//消息代码 HEX 4
	memcpy(szAuthUpstr,"0508",4);
	//单位号 BCD 8
	memcpy(szAuthUpstr+4,szCorpID,8);
	//交易方式 HEX 2
	memcpy(szAuthUpstr+12,"00",2);
	//ISAM卡号 BCD 8
	memcpy(szAuthUpstr+14,szSAMIdBCD,16);
	//ISAM卡随机数
	memcpy(szAuthUpstr+30,Random,16);
	//Isam卡授权信息
	memcpy(szAuthUpstr+46,"0000000000000000",16);
	//设备号
	memcpy(szAuthUpstr+62,szTermid,12);
	//终端编号
	memcpy(szAuthUpstr+74,szhPOSID,12);
	//操作员号
	memcpy(szAuthUpstr+86,szhOPERID,16);
	//操作主卡卡号
	memcpy(szAuthUpstr+102,szhOPERPWD,16);
	//结算日期
	memcpy(szAuthUpstr+118,"00000000",8);
	//签到批次号
	memcpy(szAuthUpstr+126,"000000",6);
	//中心时间，可用于设备校正时间
	memcpy(szAuthUpstr+132,"00000000000000",14);
	//参数更新标志位
	memcpy(szAuthUpstr+146,"00000000000000000000000000000000",32);
	//授权金额（单位：分，脱机）
	memcpy(szAuthUpstr+178,"00000000",8);
	//授权时限（格式YYYYMMDDHHMMSS）
	memcpy(szAuthUpstr+186,"00000000000000",14);
	//ISAM卡加密因子
	memcpy(szAuthUpstr+200,"0000000000000000",16);
	//ISAM卡随机数据产生密钥
	memcpy(szAuthUpstr+216,"00000000000000000000000000000000",32);
	//保留域
	memcpy(szAuthUpstr+248,"00000000000000000000",20);
	//交易应答码
	memcpy(szAuthUpstr+268,"0000",4);

	sLen = strlen(szAuthUpstr)/2 + 44;
	memset(szAuthUp, 0, sizeof(szAuthUp));
	memcpy(szAuthUp,"F0b400000001000000010100FF03",28);
	memcpy(szAuthUp+28,szLocalTime,14);
	memcpy(szAuthUp+42,"00015F0000010101000001010000000000FF",36);
	memcpy(szAuthUp+78,szAuthUpstr,272);
	memcpy(szAuthUp+350,"FFFFFFFFFF",10);

	HexStrToByte(szAuthUp,szAuthUpByte,360);
	
	if(0 !=(nRet= SendMsgSocket((const char *)YKTS_IP,(const char *)szPort,szAuthUpByte,szSysLogInResp,&sLen,&nLen,180)))
	{
		sReaderStop(phComm);
		
		return nRet;
	}

	ByteToHexStr(szSysLogInResp,szAuthUpRev,nLen);
	memset(szRevCode, 0, sizeof(szRevCode));
	memcpy(szRevCode,szAuthUpRev+346,4);

	if(0 ==memcmp("0000", szRevCode, 4)){
		memset(szMac, 0, sizeof(szMac));
		memset(szAPDU, 0, sizeof(szAPDU));
		memset(szResp, 0, sizeof(szResp));
		//外部认证
		memcpy(szAPDU, "\x00\x82\x00\x01\x08\x00\x00\x00\x00\x00\x00\x00\x00", 13);
		memcpy(szMac,szSysLogInResp+139,8);
		if( 8==strlen(szMac)){
			szAPDU[5] = szMac[0];
			szAPDU[6] = szMac[1];
			szAPDU[7] = szMac[2];
			szAPDU[8] = szMac[3];
			szAPDU[9] = szMac[4];
			szAPDU[10] = szMac[5];
			szAPDU[11] = szMac[6];
			szAPDU[12] = szMac[7];
			if(0 !=(nRet= sISamAPDU(13,(const uchar *)szAPDU,&rLen,szResp,phComm)))
			{
				sReaderStop(phComm);
				return nRet;
			}

			memset(szCode, 0, sizeof(szCode));
			BytesToHexStr((unsigned char *)szResp,rLen,(unsigned char *)cszResp);
			memcpy(szCode,cszResp,4);

			if(0 !=memcmp("9000", szCode, 4))
			{
				sscanf(szCode, "%X", &Ret);
				sReaderStop(phComm);
				return Ret;
			}
				memset(SysDatetime, 0, sizeof(SysDatetime));
				memcpy(SysDatetime,szAuthUpRev+210,14);
				memset(szLimitAmt, 0, sizeof(szLimitAmt));
				memcpy(szLimitAmt,szAuthUpRev+256,8);
				memset(szLimitAmtHEX, 0, sizeof(szLimitAmtHEX));
				memcpy(szLimitAmtHEX,szLimitAmt+6,2);
				memcpy(szLimitAmtHEX+2,szLimitAmt+4,2);
				memcpy(szLimitAmtHEX+4,szLimitAmt+2,2);
				memcpy(szLimitAmtHEX+6,szLimitAmt,2);
				sscanf(szLimitAmtHEX, "%X", LimitAmt);
				memset(LimitTime, 0, sizeof(LimitTime));
				memcpy(LimitTime,szAuthUpRev+264,14);
				memset(szBatchNo, 0, sizeof(szBatchNo));
		
				memcpy(szBatchNo,szSysLogInResp+102,3);
		
				if(0 != (nRet= sYKYSetLimite((unsigned char *)szTermid,(unsigned char *)SysDatetime,(unsigned long)LimitAmt,(unsigned char *)LimitTime,(unsigned char *)szBatchNo,phComm)))
				{
					sReaderStop(phComm);
					return nRet;
				}

				if(0 != (nRet= sReaderStop(phComm)))
				{
					return nRet;
				}

				return 0;
		}
		
		sReaderStop(phComm);
		return -11;
		
		
	}else{
		
		sReaderStop(phComm);
		memset(szRevCodeHEX, 0, sizeof(szRevCodeHEX));
		memcpy(szRevCodeHEX,szRevCode+2,2);
		memcpy(szRevCodeHEX+2,szRevCode,2);
		sscanf(szRevCodeHEX, "%X", &Ret);
		return Ret;
	}
}


int _stdcall SysLogOut(int Com, unsigned long Baud, uchar *YKTS_IP, int  YKTS_Port,uchar *POSID,uchar *OPERID,uchar *OPERPWD,uchar *AuditData,uchar *CorpID,uchar *ParamUpdateFlag)
{
	ushort nRet = 0;
	uchar szAuditData[49]={0};
	uchar szAuditDataHEX[49]={0};
	HANDLE phComm  = INVALID_HANDLE_VALUE;
	char szAuthDown[1024] = {0};
	char szAuthDownstr[512] = {0};
	char szLocalTime[15] = {0};
	uchar ucSAMATR[128]={0};
	short Length = 0;
	uchar szAPDU[128] = {0};
	uchar szResp[128] = {0};
	char cszResp[128] = {0};
	uchar cResp[128] = {0};
	char szRevCode[5]={0};
	char szRevCodeHEX[5]={0};
	ushort rLen = 0;
	char szSysDatetime[28]={0};
	uchar szSysLogOutResp[2048]={0};
	char szAuthDownRev[1024] = {0};
	char szAuthDownByte[1024] = {0};
	char szSAMIdBCD[17] ={0};
	char szPort[6]={0};
	char szIP[48]={0};
	char szPOSID[20]={0};
	char szhPOSID[13]={0};
	char szOPERID[17]={0};
	char szhOPERID[17]={0};
	char szOPERPWD[17]={0};
	char szhOPERPWD[17]={0};
	char szCorpID[9]={0};
	uchar szSAMId[13] ={0};
	char szTermid[13] ={0};
	int Ret = 0;
	int nLen = 0;
	int sLen = 0;
	sprintf(szPort, "%d", YKTS_Port);
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);
	_snprintf(szLocalTime, sizeof(szLocalTime), "%4d%02d%02d%02d%02d%02d", 
		    		stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond);
	
	nLen=12 - strlen((const char *)POSID);
	if(nLen>0)
	{
		memset(szhPOSID, 0, sizeof(szhPOSID));
		memcpy(szhPOSID,"000000000000",12);
		memcpy(szhPOSID+nLen,POSID,12-nLen);
	}else
	{
		return -8;
	}
	
	nLen=0;
	nLen=16 - strlen((const char *)OPERID);
	if(nLen>0)
	{
		memset(szhOPERID, 0, sizeof(szhOPERID));
		memcpy(szhOPERID,"0000000000000000",16);
		memcpy(szhOPERID+nLen,OPERID,16-nLen);
	}else
	{
		return -9;
	}
	
	nLen=0;
	nLen=16 - strlen((const char *)OPERPWD);
	if(nLen>0)
	{
		memset(szhOPERPWD, 0, sizeof(szhOPERPWD));
		memcpy(szhOPERPWD,"0000000000000000",16);
		memcpy(szhOPERPWD+nLen,OPERPWD,16-nLen);
	}else
	{
		return -10;
	}

	nLen=0;

	ByteToHexStr(CorpID,szCorpID,8);

	if(0 != (nRet= sReaderStart(Com,Baud,&phComm)))
	{
		sReaderStop(phComm);

        return nRet;
	}

	if(0 != (nRet= sSelectCard(2,phComm)))
	{
		sReaderStop(phComm);

        return nRet;
	}

	if(0 != (nRet= sISamReadATR(ucSAMATR,&Length,phComm)))
	{
		sReaderStop(phComm);

        return nRet;
	}
	//选择15文件
	if(0 != (nRet= sISamAPDU(7,(const uchar *)"\x00\xA4\x00\x00\x02\x00\x15",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);
        return nRet;
	}
	//读取15文件前14个字节
	memset(szResp, 0, sizeof(szResp));
	if(0 != (nRet= sISamAPDU(5,(const uchar *)"\x00\xB0\x00\x00\x0E",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);
        return nRet;
	}
	memset(szSAMIdBCD, 0, sizeof(szSAMIdBCD));
	memset(szSAMId, 0, sizeof(szSAMId));
	BytesToHexStr((unsigned char *)szResp,rLen,(unsigned char *)cszResp);
	memcpy(szSAMIdBCD,cszResp+4,16);
	memcpy(szSAMId,cszResp+8,12);
	//选择16文件
	if(0 != (nRet= sISamAPDU(7,(const uchar *)"\x00\xA4\x00\x00\x02\x00\x16",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);
        return nRet;
	}
	//读取16文件的前6个字节
	memset(szResp, 0, sizeof(szResp));
	memset(cszResp, 0, sizeof(cszResp));
	if(0 != (nRet= sISamAPDU(5,(const uchar *)"\x00\xB0\x00\x00\x06",&rLen,szResp,phComm)))
	{
		sReaderStop(phComm);
        return nRet;
	}

	memset(szTermid, 0, sizeof(szTermid));
	BytesToHexStr((unsigned char *)szResp,rLen,(unsigned char *)cszResp);
	memcpy(szTermid,cszResp,12);

	if(0 != (nRet= sYKYReadLimit((unsigned char *)szTermid,(unsigned char *)szAuditDataHEX,phComm)))
	{
		sReaderStop(phComm);
		return nRet;
	}
	memset(szAuditData, 0, sizeof(szAuditData));
	ByteToHexStr(szAuditDataHEX,(char *)szAuditData,24);

	memset(szAuthDownstr, 0, sizeof(szAuthDownstr));
	//消息代码 HEX 4
	memcpy(szAuthDownstr,"0608",4);
	//单位号 BCD 8
	memcpy(szAuthDownstr+4,szCorpID,8);
	//交易方式 HEX 2
	memcpy(szAuthDownstr+12,"00",2);
	//ISAM卡号 BCD 8
	memcpy(szAuthDownstr+14,szSAMIdBCD,16);
	//设备号
	memcpy(szAuthDownstr+30,szTermid,12);
	//终端编号
	memcpy(szAuthDownstr+42,szhPOSID,12);
	//操作员号
	memcpy(szAuthDownstr+54,szhOPERID,16);
	//结算日期
	memcpy(szAuthDownstr+70,"00000000",8);
	//签到批次号
	memcpy(szAuthDownstr+78,"000000",6);
	//TotalSvNum TotalSvAmt TotalSaleDep
	memcpy(szAuthDownstr+84,szAuditData,24);
	//保留域
	memcpy(szAuthDownstr+108,"00000000000000000000",20);
	//交易应答码
	memcpy(szAuthDownstr+128,"0000",4);
	
	memset(szAuthDown, 0, sizeof(szAuthDown));
	memcpy(szAuthDown,"F06E00000001000000010100FF03",28);
	memcpy(szAuthDown+28,szLocalTime,14);
	memcpy(szAuthDown+42,"00015F0000010101000001010000000000FF",36);
	memcpy(szAuthDown+78,szAuthDownstr,132);
	memcpy(szAuthDown+210,"FFFFFFFFFF",10);
	sLen = strlen(szAuthDownstr)/2 + 44;
	HexStrToByte(szAuthDown,szAuthDownByte,220);

	if(0 !=(nRet= SendMsgSocket((const char *)YKTS_IP,(const char *)szPort,szAuthDownByte,szSysLogOutResp,&sLen,&nLen,180)))
	{
		sReaderStop(phComm);
		return nRet;
	}
	ByteToHexStr(szSysLogOutResp,szAuthDownRev,nLen);
	
	memset(szRevCode, 0, sizeof(szRevCode));
	memcpy(szRevCode,szAuthDownRev+206,4);

	if(0 ==memcmp("0000", szRevCode, 4)){

		if(0 != (nRet= sReaderStop(phComm)))
		{
			return nRet;
		}

		return 0;
	}else{
		sReaderStop(phComm);
		memset(szRevCodeHEX, 0, sizeof(szRevCodeHEX));
		memcpy(szRevCodeHEX,szRevCode+2,2);
		memcpy(szRevCodeHEX+2,szRevCode,2);
		sscanf(szRevCodeHEX, "%X", &Ret);
		return Ret;
	}
	
}