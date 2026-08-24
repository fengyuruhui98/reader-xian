#ifndef SZ_CPU20_OPERATION_H
#define SZ_CPU20_OPERATION_H

//#include "xdrBaseType.h"

#define SZ_CPU_STATUS_BLANK		0
#define	SZ_CPU_STATUS_INIT		1
#define SZ_CPU_STATUS_SALE		2
#define	SZ_CPU_STATUS_REFUND	3

#define	SZ_EDU_M1_TYPE			1
#define	SZ_CPU_METRO_TYPE		0
#define	SZ_CPU_TONG_TYPE		2

//defined in the suzhoutong operation file 
//#define	SZ_FEETYPE_SUZHOUCITY		7
//suzhou tong defined ticket type
#define	SUZHOUTONG_TICKET			150
//#define	DEBUG_PBOC				1


#define XA_CPU_05_LEN			32
#define XA_CPU_15_LEN			16
#define	XA_CPU_17_LEN			16
#define XA_CPU_1A_LEN			16
#define	XA_CPU_1B_LEN			16
#define XA_CPU_1C_LEN			16
#define	XA_CPU_1D_LEN			16
#define	XA_CPU_18_LEN			23



union
{
	struct 
	{//exclude the first 12bytes //after 12bytes
		unsigned	cardBatchNumber_1:3;			//10
		unsigned	testCard:1;						//1
		unsigned	cardissuerId:4;					//4

		unsigned	cardBaseDateTime_1:1;			//9
		unsigned	cardBatchNumber:7;

		unsigned	cardBaseDateTime:8;
			
		unsigned	checksum:8;						//8

		unsigned	keySetNumber:8;					//8

		unsigned	passengerType:4;				//4
		unsigned	language:1;						//1
		unsigned	version:3;						//3
			
		unsigned	cardDepositValue:8;				//8
			
		unsigned	lifecycleCount_1:7;				//10
		unsigned	rfu1:1;							//1
			
		unsigned	Padding:5;						//5
		unsigned	lifecycleCount:3;
			
		unsigned	productIssuerId:5;				//5
		unsigned	productCategory:3;				//3
			
		unsigned	productId_1:2;					//6
		unsigned	purseId:6;						//6
		
		unsigned	productSerialNumber_1:4;		//12
		unsigned	productId:4;
		
		unsigned	productSerialNumber:8;
			
		unsigned	productPurchaseValue_1:1;		//17
		unsigned	purseIssuerId:5;				//5
		unsigned	rfu2:2;							//2
		
		unsigned	productPurchaseValue_2:8;
		
		unsigned	productPurchaseValue:8;
			
		unsigned	rfu3_1:4;						//13
		unsigned	cardManufactureID:4;			//4
			
		unsigned	rfu3_2:8;
			
		unsigned	MaxRideOneDay_1:7;				//8
		unsigned	rfu3:1;
			
		unsigned	rfu4:7;							//7
		unsigned	MaxRideOneDay:1;
	}_File05;
	unsigned char buff[20];
}xaFile05;

struct File05
{
	char	cardissuerId;					//4
	char	testCard;						//1
	short	cardBatchNumber;				//10
	short	cardBaseDateTime;				//9
	unsigned char	checksum;				//8
	unsigned char	keySetNumber;			//
	char	version;						//3
	char	language;						//1
	char	passengerType;					//4
	unsigned char	cardDepositValue;		//8
	char	rfu1;							//1
	short	lifecycleCount;					//10
	char	Padding;						//5
	char	productCategory;				//3
	char	productIssuerId;				//5
	char	purseId;						//6
	char	productId;						//6
	short	productSerialNumber;			//12
	char	rfu2;							//2
	char	purseIssuerId;					//5
	long	productPurchaseValue;			//17
	char	cardManufactureID;				//4
	short	rfu3;							//13
	unsigned char	MaxRideOneDay;			//8
	char	rfu4;							//7
};
typedef struct File05	File05;


union
{
	struct
	{//exlude the last 3bytes
		unsigned	rfu1:4;
		unsigned	cardStatus:4;
			
		unsigned	startDateTime_1:6;
		unsigned	productId:1;
		unsigned	rfu2:1;
			
		unsigned	startDateTime_2:8;
		
		unsigned	startDateTime:8;
			
		unsigned	origin_1:8;
		
		unsigned	destination_1:4;
		unsigned	origin:4;
		
		unsigned	destination:8;
			
		unsigned	lastDateTime:8;
			
		unsigned	totalPurchaseValue_1:8;
			
		unsigned	totalPurchaseValue:8;
			
		unsigned	lastLocation_1:8;
		
		unsigned	rfu3_1:2;
		unsigned	transfersTaken:2;
		unsigned	lastLocation:4;
		
		unsigned	padding:3;
		unsigned	journeyStatus:3;
		unsigned	rfu3:2;
	}_File15;
	unsigned char	buff[13];
}xaFile15;

struct	File15
{
	char	cardStatus;				//4
	char	rfu1;					//4
	char	rfu2;					//1
	char	productId;				//1
	long	startDateTime;			//22
	short	origin;					//12
	short	destination;			//12
	unsigned char	lastDateTime;	//8
	unsigned short	totalPurchaseValue;		//16
	short	lastLocation;			//12
	char	transfersTaken;			//2
	char	rfu3;					//4
	char	journeyStatus;			//3
};
typedef	struct File15	File15;

union
{
	struct
	{//exclude the last 5bytes
		unsigned	actionSequenceNumber:4;
		unsigned	productStatus:4;
			
		unsigned	transactionSequenceNumber_1:8;
		
		unsigned	transactionSequenceNumber:8;
			
		unsigned	lavSamId_1:8;
		
		unsigned	lavSamId:8;
			
		unsigned	validityStartDate_1:8;
		
		unsigned	activated:1;
		unsigned	lavPaymentMethod:3;
		unsigned	validityStartDate:4;
			
		unsigned	lavValue_1:8;
		
		unsigned	lavValue_2:8;
		
		unsigned	validityDuration:7;
		unsigned	lavValue:1;
			
		unsigned	padding:5;
		unsigned	invoicePrinted:1;
		unsigned	validityDurationType:2;
	}_File1B;
	unsigned char	buff[11];
}xaFile1B;

struct File1B
{
	char	productStatus;			//4
	char	actionSequenceNumber;	//4
	unsigned short	transactionSequenceNumber;	//16
	unsigned short	lavSamId;			//16
	short	validityStartDate;			//12
	char	lavPaymentMethod;			//3
	char	activated;					//1
	long	lavValue;					//17
	char	validityDuration;			//7
	char	validityDurationType;		//2
	char	invoicePrinted;				//1
};
typedef	struct 	File1B	File1B;

union
{
	struct
	{
		unsigned	actionSequenceNumber:4;
		unsigned	productStatus:4;
		
		unsigned	validityStartDateTime_1:6;
		unsigned	invoicePrinted:1;
		unsigned	activated:1;
		
		unsigned	validityStartDateTime_2:8;
		
		unsigned	validityStartDateTime:8;
		
		unsigned	validityOrigin_1:8;
		
		unsigned	validityDestination_1:4;
		unsigned	validityOrigin:4;
			
		unsigned	validityDestination:8;
		
		unsigned	transactionSequenceNumber_1:8;
		
		unsigned	transactionSequenceNumber:8;
			
		unsigned	lavSamId_1:8;
		
		unsigned	lavSamId:8;
			
		unsigned	lavValue_1:8;
			
		unsigned	lavValue_2:8;
			
		unsigned	validityDuration_1:2;
		unsigned	validityDurationType:2;
		unsigned	lavPaymentMethod:3;
		unsigned	lavValue:1;
		
		unsigned	padding:3;
		unsigned	validityDuration:5;
		
		unsigned	checksum:8;
	}_File1C;
	unsigned char buff[16];
}xaFile1C;

struct File1C
{
	char	productStatus;				//4
	char	actionSequenceNumber;		//4
	char	activated;					//1
	char	invoicePrinted;				//1
	long	validityStartDateTime;		//22
	short	validityOrigin;				//12
	short	validityDestination;		//12
	unsigned short	transactionSequenceNumber;		//16
	unsigned short	lavSamId;			//16
	long	lavValue;					//17
	char	lavPaymentMethod;			//3
	char	validityDurationType;		//2
	short	validityDuration;			//7
	char	padding;					//3
	char	checksum;					//8
};
typedef	struct File1C	File1C;

union
{
	struct
	{
		unsigned	dateTime_1:8; 		//交易时间 32
		unsigned	dateTime_2:8;
		unsigned	dateTime_3:8;
		unsigned	dateTime:8;
		
		unsigned	serviceProviderId:8;	 	//服务商ID 8

		unsigned	productIssuerId:5; 			//产品发行商ID 5
		unsigned	category:3;				 	//交易种类 3

		unsigned	paymentMethod:3; 			//付款方式 3
		unsigned	transactionType:5; 			//交易类型 5

		unsigned	location_1:2; 				//交易地点 12
		unsigned	productTypeId:6; 			//产品类型ID 6
		
		unsigned	location_2:8;
		
		unsigned	value_1:6; 					//交易金额或者数量 17
		unsigned	location:2;
			
		unsigned	value_2:8;

		unsigned	remainingValue_1:5; 		//剩余金额或者数量 17
		unsigned	value:3;			
		
		unsigned	remainingValue_2:8;
			
		unsigned	padding_1:4; 			//预留 20
		unsigned	remainingValue:4;
		
		unsigned	padding_2:8;
			
		unsigned	Padding:8;
	}_File17;
	unsigned char 	buff[16];
}xaFile17;

struct File17
{
	unsigned long dateTime;
	unsigned char serviceProviderId;
	unsigned char category;
	unsigned char productIssuerId;
	unsigned char transactionType;
	unsigned char paymentMethod;
	unsigned char productTypeId;
	unsigned short location;
	unsigned long value;
	unsigned long remainingValue;
	unsigned long padding;
};
typedef struct File17		File17;

File05	tpfile05;
File15	tpfile15;
File1B	tpfile1b;
File1C	tpfile1c;
File1C	tpfile1d;
File17	tpfile17;

char ch_metro_edu_type;					//metro cpu card or education m1 card
char ch_cpu20_phyical_id[8], ch_cpu20_logic_id[8];
char ch_cpu20_phyical_id_bak[8], chCode_bak, sfi_bak[2];
unsigned char blnNeedReadfile14;
char ch_sz_cpu_rollback, blncpuRollback, cur_sfi[2];
unsigned char Metro_Transfer_key[16];


void sz_timetype2bcd(unsigned char *timetype, unsigned char *timebcd, unsigned char flag);
void sz_CPU20_ee_write(unsigned char sn_bak);
void sz_CPU20_ee_read();
void xa_MCPU_rollback_write(unsigned char *record, unsigned short len);


char CPU_select_file(char *sfi, char len, unsigned char *out_buf, unsigned char *out_len);

char CPU_GetFiles05(unsigned char *out_buf);
char CPU_GetFiles08(unsigned char *out_buf);
char CPU_GetFiles14(unsigned char *out_buf);
char CPU_GetFiles19(unsigned char *out_buf);
char CPU_GetFiles15(unsigned char *out_buf);
char CPU_GetFiles1b(unsigned char *out_buf);
char CPU_GetFiles1c(unsigned char *out_buf);
char CPU_GetFiles1d(unsigned char *out_buf);

//char CPU_ValidatePeriod(unsigned char durationmode, Date2_t shDays);
char CPU_TellOverTime(unsigned char *entrytime, unsigned char *curtime, unsigned char mileclass);
char CPU_ValidateEmArea(char metrostatus, unsigned char *out_buf);
char CPU_TellTesting(unsigned char chTestMode);
char CPU_TellSysCard(unsigned short tickettype);
char CPU_load_mac2(unsigned char *logic_id, unsigned char *init_mac1_data, int transValue, char *mac2, unsigned char *out_buf);

char xa_update_file_15(unsigned char *file_buf, unsigned char *out_buf);
char xa_update_file_17(unsigned char *file_buf, unsigned char *out_buf);
char xa_update_file_1c(unsigned char *file_buf, unsigned char *out_buf);
char xa_update_file_1d(unsigned char *file_buf, unsigned char *out_buf);

char check_metro_Black_Lock(unsigned long logicID, unsigned char blnBlock, unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char sz_CPU_sale_dis(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char sz_CPU_sale_dis_person(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_entry_dis(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_entry_cnt(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_entry_em(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_exit_dis(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_exit_cnt(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_exit_em(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_update_dis(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_update_em(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_update_cnt(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_active(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_active_cnt(unsigned char *start_timebcd, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_active_em(unsigned char *start_timebcd, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_active_dis(unsigned char *start_timebcd, unsigned char *out_buf, unsigned short *out_len);

char sz_CPU_add_prepare(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_add(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_CPU_add_cnt(unsigned char *cmd_buff, unsigned char *out_buf, unsigned short *out_len);

void TestLog(unsigned char *buf,unsigned char *string, unsigned char len);
#endif
