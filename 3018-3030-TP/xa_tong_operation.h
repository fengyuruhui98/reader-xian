#ifndef SZ_TONG_OPERATION_H
#define SZ_TONG_OPERATION_H

//#include "xdrBaseType.h"
#include "binEOD.h"

#define	XA_CPU_STATUS_INIT		0

#define	SZ_FEETYPE_SUZHOUCITY		7

#define SUZHOU_TONG					1
//
#define SZ_CPUT_16_LEN			55
#define	SZ_CPUT_05_LEN			30
#define	XA_CPUT_18_LEN			23
#define	XA_CPUT_19_LEN			32
#define XA_CPUT_1A_LEN			31

char ch_cput_phyical_id[8], ch_cput_logic_id[8], ch_cput_ats[8];
char ch_cput_phyical_id_bak[8], ch_tong_code_bak;
char ch_sz_cput_rollback, blncputRollback;

struct cpu_tong
{
	unsigned char curtime[4];
	char time_bcd[7];
	unsigned long hisecond;
	unsigned long lowsecond;
	unsigned short days;
	unsigned long midsecond;
	
	long balance;
	long tranamount;
	long trantimes;
	short deposit;
	
	unsigned short cardsn;
	unsigned long transn;
	
	unsigned char curstation[4];
	unsigned char laststation[4];
	
	unsigned short startdate;
	unsigned short enddate;
	unsigned char tac[4];
};

struct auth_head
{
	unsigned char startFlag;			//始标识	固定，0xF0	HEX	1
	unsigned long length;				//消息报文头	报文长度	消息整体长度，即从开始标记到结尾标记的字节长度。	HEX	4
	unsigned long protocalFlag;			//协议标识	固定，0x01000000	HEX	4
	unsigned char protocalVersion;		//协议版本号	固定，0x01	HEX	1
	unsigned char formatVersion;		//数据格式版本号	固定，0x01	HEX	1
	unsigned char transfertype;			//数据传输类型	0：协议数据；4：FTP通知数据。	HEX	1
	unsigned char rfu1;					//预留	固定，0xFF	HEX	1
	unsigned char command;				//数据包唯一标识	传输命令编码	见编码规则，传输命令编码定义	HEX	1
	unsigned char curtime[7];			//发送时间戳	数据发送的时间：YYYYMMDDHH24MISS	BCD	7
	unsigned short sn;					//发送序列号	发送方发出数据包时由产生 初值为1，每次发送递增1，到达65535后归1。	HEX	2
	unsigned char acc[6];				//发送方标识	清算中心，0x640000010101；ACC，0x5F0000019804	HEX	6
	unsigned short rfu2;				//预留	固定，0x0000	HEX	2
	unsigned char packagetotal;			//分包总数	固定，0x01	HEX	1
	unsigned char packagesn;			//分包序号	固定，0x01	HEX	1
	unsigned long rfu3;					//预留	固定，0x00000000	HEX	4
	unsigned char crc;					//校验码算法	不采用，0x00；CRC32，0x01。	HEX	1
	unsigned char rfu4;					//预留	固定，0xFF	HEX	1
	//报文体	报文体内容		
	//校验数据	校验数据内容	HEX	4或0
	//结束标志	固定，0xFF	HEX	1
}__attribute__( ( packed, aligned(1) ) );

struct auth_in
{
	struct auth_head head;
	unsigned char MsgCode[2];			//1.		MsgCode	2，Hex	消息代码	需方正确定具体值0508
	unsigned char Unitid[4];			//2.		Unitid	4，BCD	单位号	71000301
	unsigned char TxnMode;				//3.		TxnMode	1，Hex	交易方式	0x00-脱机；0x01-联机
	unsigned char IsamId[8];			//4.		IsamId	8，BCD	ISAM卡号	15文件的8字节
	unsigned char Random[8];			//5.		Random	8，BCD	ISAM卡随机数	
	unsigned char ISAMAuthInfo[8];		//6.		ISAMAuthInfo	8，BCD	Isam卡授权信息	0
	unsigned char PosId[6];				//7.		PosId	6，BCD	设备号	一卡通清算中心统一分配的机具编号16文件的6字节
	unsigned char Termid[6];			//8.		Termid	6，BCD	终端编号	行业单位分配的机具编号？
	unsigned char Operid[8];			//9.		Oper id	8，BCD	操作员号 	
	unsigned char EDCardId[8];			//10.		EDCardId	8，BCD	操作主卡卡号	？
	unsigned char SettDate[4];			//11.		SettDate	4，BCD	结算日期	签到时，清算中心返回
	unsigned char BatchNo[3];			//12.		BatchNo	3，BCD	签到批次号	
	unsigned char SysDatetime[7];		//13.		SysDatetime	7，BCD	中心时间，可用于设备校正时间	YYYYMMDDhhmmss
	unsigned char ParamBit[16];			//14.		ParamBit	16，BCD	参数更新标志位	
	unsigned long LimiAmt;				//15.		LimitAmt	4，Hex	授权金额（单位：分，脱机）	
	unsigned char LimiTime[7];			//16.		LimitTime	7，BCD	授权时限（格式YYYYMMDDHHMMSS	
	unsigned char DivElement[8];		//17.		DivElement	8，HEX	ISAM卡认证MAC	
	unsigned char KeySet[16];			//18.		KeySet	16，HEX	ISAM卡随机数据产生密钥	
	unsigned char Reserved[10];			//19.		Reserved	10，BCD	保留域	
	unsigned short ResponseCode;		//20.		Response Code	2，Hex	交易应答码	
	unsigned char chk[4];				//
	unsigned char endFlag;				//
}__attribute__( ( packed, aligned(1) ) );

struct auth_out
{
	struct auth_head head;
	unsigned char MsgCode[2];			//1.		MsgCode	2，Hex	消息代码	需方正确定具体值
	unsigned char Unitid[4];			//2.		Unitid	4，BCD	单位号	
	unsigned char TxnMode;				//3.		TxnMode	1，Hex	交易方式	0x00-脱机；0x01-联机
	unsigned char IsamId[8];			//4.		IsamId	8，BCD	ISAM卡号	
	unsigned char PosId[6];				//5.		PosId	6，BCD	设备号	一卡通清算中心统一分配的机具编号
	unsigned char Termid[6];			//6.		Termid	6，BCD	终端编号	行业单位分配的机具编号
	unsigned char Operid[8];			//7.		Oper id	8，BCD	操作员号 	
	unsigned char SettDate[4];			//8.		SettDate	4，BCD	结算日期	YYYYMMDD，上次签到时授权中心返回的结算日期
	unsigned char BatchNo[3];			//9.		BatchNo	3，BCD	签到批次号	签到时获取到的批次号
	unsigned long TotalSvNum;			//10.		TotalSvNum	4，Hex	本批次累计售卡充资交易总笔数	
	unsigned long TotalSvAmt;			//11.		TotalSvAmt	4，Hex	本批次累计充资交易总金额（分）	
	unsigned long TotalSaleDep;			//12.		TotalSaleDep	4，Hex	本批次累计售卡交易押金总金额（分）	
	unsigned char Reserved[10];			//13.		Reserved	10，BCD	保留域	
	unsigned short ResponseCode;		//14.		Response Code	2，Hex	交易应答码	
	unsigned char chk[4];				//
	unsigned char endFlag;				//
}__attribute__( ( packed, aligned(1) ) );

struct cpu_tong tpCPUtong;
struct auth_in tpauthLogin;
struct auth_out tpauthLogout;


void sz_tong_ee_write(unsigned char sn_bak);

void sz_timetype2bcd(unsigned char *timetype, unsigned char *timebcd, unsigned char flag);
void sz_CPU20_ee_write(unsigned char sn_bak);
void xa_hex2bcd(unsigned char *bcd, unsigned char *hex);

char CPUT_GetFiles05(unsigned char *out_buf);
char CPUT_GetFiles08(unsigned char *out_buf);
char CPUT_GetFiles14(unsigned char *out_buf);
char CPUT_GetFiles19(unsigned char *out_buf);

char CPUT_ValidatePeriod(unsigned char durationmode, unsigned short shDays);
char CPUT_TellOverTime(unsigned char *entrytime, unsigned char *curtime, unsigned char mileclass);
char CPUT_ValidateArea(unsigned char *out_buf);
char CPUT_ValidateEmArea(char metrostatus, unsigned char *out_buf);
char CPUT_TellTesting(unsigned char chTestMode);
char CPUT_TellEntry(unsigned char *expecting_status, unsigned char mode_check);
char check_YKT_Black_Lock(unsigned char *city, unsigned char *business, unsigned char *SN, unsigned char blnBlock, unsigned char *out_buf, unsigned short *out_len);

char xa_CPU_sale(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_tong_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_tong_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_tong_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_tong_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);

char xa_update_city_15(unsigned char *file_buf, unsigned char *out_buf);
int get_load_ticket(unsigned char subtype, unsigned char maintype, YKTLoad_t *td);
int get_purchase_ticket(unsigned char subtype, unsigned char maintype, YKTTerminal_t *td);


char CPUT_VerifyPIN(unsigned char p2, unsigned char *in_buf, unsigned char in_len, unsigned char *out_buf);

char CPUT_gettransprove(unsigned char transtypeid, unsigned char *out_buf);
char CPUT_externauth(char extern_auth_type, unsigned short shkey, unsigned char cardky, unsigned char *out_buf);

void calPin(char *pszCardsn, char *pin);
char xa_auth_login_init(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_auth_logout_init(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_auth_login(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_auth_logout(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_load_mac2(unsigned char *mac2, unsigned char *out_buf);

#endif
