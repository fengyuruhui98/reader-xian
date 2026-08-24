#ifndef	XA_OPERATION_H
#define	XA_OPERATION_H
#include <semaphore.h>

#include "binEOD.h"

sem_t g_sha1wait;
unsigned long g_sha1txnsn;
short sh_mac_len;
char ch_mac_data[1000], ch_tac_data[4];

//#define	DEBUG_PBOC				1

#define	XA_FEETYPE_VALUE		1
#define	XA_FEETYPE_PERIOD		2
#define	XA_FEETYPE_TIMES		3
#define	SZ_FEETYPE_MONTH		0

#define	XA_CARD_PHYSICAL_UL		3
#define	XA_CARD_PHYSICAL_CPU	2

//支付方式常量定义
#define XA_PAYTYPE_MONEY		10
#define XA_PAYTYPE_CARD			11
#define	XA_PAYTYPE_ALIPAY		12
#define	XA_PAYTYPE_WEPAY		13
#define	XA_PAYTYPE_UNION		14


struct CMD_INQUIRE_t
{
	unsigned char	check;
	unsigned char	function;
	unsigned char	feearea;
	unsigned char	curtime[7];
	unsigned char	antenna;
	unsigned char	flag;
	unsigned long	exitstation;
	unsigned char	history;
}__attribute__( ( packed, aligned(1) ) );
typedef struct CMD_INQUIRE_t	CMD_INQUIRE_t;

struct CMD_INIT
{
	unsigned long	timeout;
	unsigned char	deviceID[4];
	unsigned short	hardtype;
	unsigned long	curstation;
	unsigned long	participantid;
	unsigned char	operation_day[4];
	unsigned char	natural_day[4];
	unsigned char	test;
	unsigned short	mode;
	unsigned char	operationid[3];
	unsigned char	transfer;
	unsigned char	tputype;
	unsigned short	maxtimes;
	unsigned short	waittime;
	unsigned short	rewrite;
	unsigned char	antenna;
	unsigned long	rfu;
}__attribute__( ( packed, aligned(1) ) );
typedef	struct CMD_INIT		CMD_INIT_t;

struct CMD_SALE
{
	unsigned char UDSN[4];		//3	UDSN 	4	BIN	ACC定义的每台SLE唯一的UD或交易序号。从1开始，依次累加，范围1～4294967295。达到最大值后从1开始。如果重置需通知上位系统。传输时转换成INTEL序。
	unsigned char curtime[7];	//4	交易时间	7	BCD	交易发生的日期时间
	unsigned char family;		//5	卡片种类	1	BIN	0x01：预留；0x02：预留；0x11：一票通CPU卡片；0x12：一票通UL卡
	unsigned char product;		//6	车票产品类型	1	BIN	1：钱包产品 2：定期产品3：计次产品
	unsigned char tickettype[2];	//7	车票产品种类	2	BIN	传输时转换成INTEL序。
					//	0x03：单程票
					//	0x01：出站票
					//	0x05：预制单程票
	unsigned char 	subtype[2];	//8	车票产品子类型	2	BIN	传输时转换成INTEL序。
	unsigned char 	saleType;	//9	售票方式	1	BIN	0x01：表示按照票价来售票；0x02：表示按照起止站来售票
	unsigned char 	passenge;	//10	乘客类型	1	BIN	ACC标准规定的乘客的类别0x01
	unsigned long 	price;		//11	售票金额	4	BIN	售票总金额。传输时转换成INTEL序。
	unsigned long 	srcStation;	//12	起点站（或区段）站码	4	BIN	按照起止站售票时填入的起始车站位置信息 。传输时转换成INTEL序。
	unsigned long 	desStation;	//13	终点站（或区段）站码	4	BIN	按照起止站售票时填入的终止车站位置信息。传输时转换成INTEL序。
	unsigned short 	times;		//14	计次类车票使用次数	2	BIN	非计次类车票，该字段填0 。传输时转换成INTEL序。
	unsigned long  	duration;	//15	有效期的时间长度	4	BIN	售卡时写入票卡的有效期的时间长度。售卡时写入票卡的有效期的时间长度，一票通卡的有效期的时间长度由CD控制， 不受该参数确认，详见产品参数中的Duration，ValidityOrigin。传输时转换成INTEL序。
				//16	支付方式	1	BIN	0x01 现金支付补票金额（兼容）；0x02 卡内扣除补票金额（兼容），0x0A 现金支付，0x0B 卡内扣款，0x0C 支付宝，0x0D 微信，0x0E 银联
}__attribute__( ( packed, aligned(1) ) );
typedef	struct CMD_SALE	CMD_SALE_t;

struct SYS_PRICE
{
	unsigned long minPrice;
	unsigned long maxPrice;
	unsigned long price;
};
typedef struct SYS_PRICE	SYS_PRICE_t;

struct STATION_PRICE
{
	unsigned long *SJTPrice;
	unsigned long *SJTFareCode;
	unsigned long SJTNum;
};
typedef struct STATION_PRICE	STATION_PRICE_t;

#define	MCPU_ROLL_CHECK_15		1
#define MCPU_ROLL_BEFORE_ED		2
#define MCPU_ROLL_AFTER_ED		3

struct MUL_PROTECT
{
	unsigned char rollBack;
	unsigned char phyicalID[8];
	unsigned long usecond;
	unsigned long tranAmount;
	unsigned long balance;
	unsigned char ul_page[64];
	unsigned char rec_buf[1000];
	unsigned short rec_len;
}__attribute__( ( packed, aligned(1) ) );
typedef struct MUL_PROTECT		MUL_PROTECT_t;

struct MCPU_PROTECT
{
	unsigned char rollBack;
	unsigned char phyicalID[8];
	unsigned long usecond;
	unsigned long tranAmount;
	unsigned long balance;
	unsigned char capp_init[19];
	unsigned char rec_buf[1000];
	unsigned short rec_len;
}__attribute__( ( packed, aligned(1) ) );
typedef struct MCPU_PROTECT		MCPU_PROTECT_t;

struct XACPU_PROTECT
{
	unsigned char rollBack;
	unsigned char phyicalID[8];
	unsigned char logicID[10];
	unsigned long usecond;
	unsigned char time_bcd[7];
	unsigned long tranAmount;
	unsigned long balance;
	unsigned char capp_init[19];
	unsigned char sam_sn[4];
	unsigned char rec_buf[1000];
	unsigned short rec_len;
}__attribute__( ( packed, aligned(1) ) );
typedef struct XACPU_PROTECT		XACPU_PROTECT_t;

CMD_INQUIRE_t	tpCmdInquire;
CMD_INIT_t		tpCmdInit;
SYS_PRICE_t		tpSysPrice;
STATION_PRICE_t	tpStationPrice;

MCPU_PROTECT_t	tpMCPUProtect[10];
unsigned char 	tpMCPUProtectIndex;
unsigned char 	tpMCPUPointer;

XACPU_PROTECT_t	tpXACPUProtect[10];
unsigned char 	tpXACPUProtectIndex;
unsigned char 	tpXACPUPointer;

XACPU_PROTECT_t	tpTransportProtect[10];
unsigned char 	tpTransportProtectIndex;
unsigned char 	tpTransportPointer;

TxnProductMultirideUseOnEntry_t	tpTxnProductMultirideEntry;	
TxnProductPassUseOnEntry_t		tpTxnProductPassEntry;
TxnProductPurseUseOnEntry_t		tpTxnProductPurseEntry;
TxnProductMultirideUseOnExit_t	tpTxnProductMultirideExit;
TxnProductPassUseOnExit_t		tpTxnProductPassExit;
TxnProductPurseUseOnExit_t		tpTxnProductPurseExit;
TxnProductMultirideIssue_t		tpTxnProductMultirideIssue;
TxnProductPassIssue_t			tpTxnProductPassIssue;
TxnProductPurseIssue_t			tpTxnProductPurseIssue;
TxnProductMultirideExitTicketIssue_t	tpTxnProductExitIssue;
TxnProductMultirideCompensationFare_t	tpTxnProductMultirideCompensate;
TxnProductPassCompensationFare_t		tpTxnProductPassCompensate;
TxnProductPurseCompensationFare_t		tpTxnProductPurseCompensate;
TxnProductPurseRefund_t					tpTxnProductPurseRefund;
TxnProductPurseIssueReverse_t			tpTxnproductPurseReverse;
TxnEventBlacklistCardRequest_t			tpTxnEventBlacklistRequest;
TxnCardBlock_t							tpTxnCardBlock;
TxnProductMultirideAdd_t				tpTxnProductMultirideAdd;
TxnProductPassAdd_t						tpTxnProductPassAdd;		
TxnProductPurseAdd_t					tpTxnProductPurseAdd;

YKTTxnPurchase_t	tpYKTTxnPurchase;
YKTTxnLoad_t		tpYKTTxnLoad;
JTBTxnPurchaseEx_t	tpJTBTxnPurchaseEx;
JTBTxnPurchaseExII_t	tpJTBTxnPurchaseExII;


char xa_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_sale(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_getud(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_add(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);

char xa_TellDate(unsigned char *cur_timebcd, unsigned char *start_timebcd, unsigned char *end_timebcd, unsigned char travelstatus, unsigned char *maystatus, unsigned char DurationType);
char xa_ValidateArea(unsigned long curstation, unsigned long zonestation);

char xa_station_all_price();
char xa_station_to_station_price();

#endif