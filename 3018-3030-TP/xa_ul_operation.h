#ifndef SZ_UL_OPERATION_H
#define SZ_UL_OPERATION_H

#define SZ_SJT_1			1		//
//product ID
#define	XA_SJT_PURSE		1		//purse ticket
#define	XA_SJT_PERIOD		2		//period
#define	XA_SJT_RIDES		3		//multirides

//card status
#define XA_SJT_CARD_RFU		0		//rfu
#define XA_SJT_CARD_NOMAL	1		//not-locking
#define XA_SJT_CARD_LOCK	2		//lock
#define XA_SJT_CARD_RECYCLE	3		//recycled card

char ch_ul_phyical_id[8], ch_ul_logic_id[8];
char ch_ul_phyical_id_bak[8];
char ch_sz_sjt_rollback, blnsjtRollback;

union
{
	struct 
	{
		//page 4
		unsigned	lifecycleCount_1:6; 	//10
		unsigned	Version:2; 				//2

		unsigned	cardBaseDataTime_1:1;	//9
		unsigned	keySetNumber:3; 		//3
		unsigned	lifecycleCount:4;
		
		unsigned	cardBaseDataTime:8;
			
		unsigned	cardBatchNumber_2:6;
		unsigned	cardBatchNumber_1:2;	//10
		//page 5
		unsigned	productType_1:5;		//6
		unsigned	testMode:1;				//1
		unsigned	cardBatchNumber:2;
			
		unsigned	purchaseValue_1:2;		//17
		unsigned	productID:2;			//2
		unsigned	passengerType:3;		//3
		unsigned	productType:1;		
		
		unsigned	purchaseValue_2:8;

		unsigned	rfu1_1:1;				//17
		unsigned	purchaseValue:7;
		//page 6
		unsigned	rfu1_2:8;
			
		unsigned	rfu1:8;
			
		unsigned	validityDuration_1:4;	//7
		unsigned	DurationType:2;					//2
		unsigned	lavPaymentMethod:2;		//2
		
		unsigned	productIssuerId:5;		//5
		unsigned	validityDuration:3;
	}_StaticZone;
	unsigned char buff[12];
}xaStaticZone;

union
{
	struct 
	{
		//page 8
		unsigned	transactionSequenceNumber_1:5;	//8
		unsigned	cardStatus:2;					//2
		unsigned	activeStatus:1;					//1

		unsigned	startDateTime_1:4;				//22
		unsigned	rfu1:1;							//1
		unsigned	transactionSequenceNumber:3;
		
		unsigned	startDateTime_2:8;
			
		unsigned	startDateTime_3:8;
		//page 9
		unsigned	lastDateTime_1:6;				//8
		unsigned	startDateTime:2;
		
		unsigned	lastLocation_1:6;				//12
		unsigned	lastDateTime:2;
			
		unsigned	transfersTaken:2;		//2
		unsigned	lastLocation:6;
			
		unsigned	validityStartDate_1:5;	//12
		unsigned	status:3;				//3
		//page 10
		unsigned	remainingValue_1:1;		//17
		unsigned	validityStartDate:7;
			
		unsigned	remainingValue_2:8;
			
		unsigned	remainingValue:8;
			
		unsigned	totalPurchaseValue_1:7;	//15
		unsigned	activated:1;			//1
		//page 11
		unsigned	totalPurchaseValue:8;
			
		unsigned	origin_1:8;				//12
		
		unsigned	mac2_1:4;				//12
		unsigned	origin:4;

		unsigned	mac2:8;	
	}_DynamicZone;
	unsigned char buff[16];
}xaDynamicZonePurse;

union
{
	struct 
	{
		unsigned	transactionSequenceNumber_1:5;	//8
		unsigned	cardStatus:2;					//2
		unsigned	activeStatus:1;					//1

		unsigned	startDateTime_1:4;				//22
		unsigned	rfu1:1;							//1
		unsigned	transactionSequenceNumber:3;
		
		unsigned	startDateTime_2:8;
			
		unsigned	startDateTime_3:8;
		
		unsigned	lastDateTime_1:6;				//8
		unsigned	startDateTime:2;
		
		unsigned	lastLocation_1:6;		//12
		unsigned	lastDateTime:2;
			
		unsigned	transfersTaken:2;		//2
		unsigned	lastLocation:6;
			
		unsigned	validityStartDateTime_1:5;	//22
		unsigned	status:3;				//3
			
		unsigned	validityStartDateTime_2:8;
			
		unsigned	validityStartDateTime_3:8;
		
		unsigned	validityOrigin_1:7;		//12
		unsigned	validityStartDateTime:1;
			
		unsigned	validityDestination_1:3;			//12
		unsigned	validityOrigin:5;
		
		unsigned	validityDestination_2:8;
		
		unsigned	rfu_1:6;				//10
		unsigned	activated:1;			//1
		unsigned	validityDestination:1;

		unsigned	mac2_1:4;				//12
		unsigned	rfu:4;

		unsigned	mac2:8;	
	}_DynamicZone;
	unsigned char buff[16];
}xaDynamicZonePeriod;

union
{
	struct 
	{
		unsigned	transactionSequenceNumber_1:5;	//8
		unsigned	cardStatus:2;					//2
		unsigned	activeStatus:1;					//1

		unsigned	startDateTime_1:4;				//22
		unsigned	rfu1:1;							//1
		unsigned	transactionSequenceNumber:3;
		
		unsigned	startDateTime_2:8;
			
		unsigned	startDateTime_3:8;
		
		unsigned	lastDateTime_1:6;				//8
		unsigned	startDateTime:2;
		
		unsigned	lastLocation_1:6;		//12
		unsigned	lastDateTime:2;
			
		unsigned	transfersTaken:2;		//2
		unsigned	lastLocation:6;
			
		unsigned	validityStartDateTime_1:5;	//22
		unsigned	status:3;				//3
			
		unsigned	validityStartDateTime_2:8;
			
		unsigned	validityStartDateTime_3:8;
			
		unsigned	remainingRides_1:7;		//8
		unsigned	validityStartDateTime:1;
			
		unsigned	validityOrigin_1:7;		//12
		unsigned	remainingRides:1;
			
		unsigned	validityDestination_1:3;	//12
		unsigned	validityOrigin:5;
			
		unsigned	validityDestination_2:8;
		
		unsigned	mac2_1:4;				//12
		unsigned	rfu:1;				//1
		unsigned	totalValueUsed:1;		//1
		unsigned	activated:1;			//1
		unsigned	validityDestination:1;

		unsigned	mac2:8;	
	}_DynamicZone;
	unsigned char buff[16];
}xaDynamicZoneMultiride;

static int Len_ST[] = {2, 10, 3, 9, 10, 1, 6, 3, 2, 17, 17, 2, 2, 7, 5};
struct StaticZone
{
	char	Version; 			//2
	short	lifecycleCount; 	//10
	char	keySetNumber; 		//3
	short	cardBaseDataTime;	//9
	short	cardBatchNumber;	//10
	char	testMode;			//1
	char	productType;		//6
	char	passengerType;		//3
	char	productID;			//2
	long	purchaseValue;		//17
	long	rfu1;				//17
	char	lavPaymentMethod;	//2
	char	DurationType;		//2
	short	validityDuration;	//7
	char	productIssuerId;	//5
};
typedef	struct StaticZone	StaticZone;

static int Len_DZP[] = {1, 2, 8, 1, 22, 8, 12, 2, 3, 12, 17, 1, 15, 12};
struct DynamicZonePurse
{
	char	activeStatus;		//1
	char	cardStatus;			//2
	unsigned char	transactionSequenceNumber;	//8
	char	rfu1;				//1
	long	startDateTime;		//22
	unsigned char	lastDateTime;				//8
	short	lastLocation;		//12
	char	transfersTaken;		//2
	char	status;				//3
	short	validityStartDate;	//12
	long	remainingValue;		//17
	char	activated;			//1
	short	totalPurchaseValue;	//15
	short	origin;				//12
	short	mac2;				//12
};
typedef	struct DynamicZonePurse	DynamicZonePurse;

static int Len_DZR[] = {1, 2, 8, 1, 22, 8, 12, 2, 3, 22, 12, 12, 1, 10};
struct DynamicZonePeriod
{
	char	activeStatus;		//1
	char	cardStatus;			//2
	unsigned char	transactionSequenceNumber;	//8
	char	rfu1;				//1
	long	startDateTime;		//22
	unsigned char	lastDateTime;				//8
	short	lastLocation;		//12
	char	transfersTaken;		//2
	char	status;				//3
	long	validityStartDateTime;	//22
	short	validityOrigin;		//12
	short	validityDestination;			//12
	char	activated;			//1
	short	rfu;				//10
	short	mac2;				//12
};
typedef	struct DynamicZonePeriod	DynamicZonePeriod;

static int Len_DZM[] = {1, 2, 8, 1, 22, 8, 12, 2, 3, 22, 8, 12, 12, 1, 1, 1};
struct DynamicZoneMultiride
{
	char	activeStatus;		//1
	char	cardStatus;			//2
	unsigned char	transactionSequenceNumber;	//8
	char	rfu1;				//1
	long	startDateTime;		//22
	unsigned char	lastDateTime;				//8
	short	lastLocation;		//12
	char	transfersTaken;		//2
	char	status;				//3
	long	validityStartDateTime;	//22
	unsigned char	remainingRides;		//8
	short	validityOrigin;		//12
	short	validityDestination;	//12
	char	activated;			//1
	char	totalValueUsed;		//1
	char	rfu;				//1
	short	mac2;				//12
};
typedef	struct DynamicZoneMultiride	DynamicZoneMultiride;


struct sjt
{
	unsigned char curtime[4];
	char time_bcd[7];
	unsigned long hisecond;
	unsigned long lowsecond;
	unsigned short days;
	unsigned long midsecond;
	
	long balance;
	long tranamount;
	
	unsigned short cardsn;
	unsigned long transn;
	
	unsigned char curstation[4];
	unsigned char laststation[4];
	unsigned char trantype;
	//
	unsigned char	activeReadpage;
	unsigned char	activeWritepage;
	//analyze sjt info--static
	char version;
	char keyversion;
	unsigned long basedate;
	char testflag;
	char tickettype;
	char passengertype;
	char productid;
	char validityduration;
	//
	char cardstatus;
	unsigned long startdate;
	unsigned long lastdate;
	unsigned long lnglaststation;
	char travelstatus;
	//
	unsigned long validstartdate;
	long remaining;
	short validorigin;
	short validdestin;
	char activated;
	short totalused;
	short mac2;
};

StaticZone 	tpSZ;
DynamicZonePurse	tpDZpurse;
DynamicZonePeriod	tpDZperiod;
DynamicZoneMultiride	tpDZmultiride;

struct sjt tpSJT;

short	gDebugStep;

//¹ºÆ±·½Ê½
#define Type_DestStation 	0 		//0£¬Ä¿±êÕ¾¹ºÆ±
#define Type_TransValue  	1 		//1,½ð¶î¹ºÆ±

#define Type_Exit_Ticket  	0x1 	//exit ticket

#define	FLAG_RECYCLE		1
#define FLAG_NONRECYCLE		0

char standard_tocken_read(unsigned char blockno, unsigned char *out_buf);
void sz_ul_ee_read();
void sz_ul_ee_write();

char read_tocken_otp_info(unsigned char *ul_page);
char read_tocken_info(void);

char xa_ul_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_entry_purse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_entry_multiride(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_entry_period(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_exit_purse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_exit_multiride(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_exit_period(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_sale(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_update_purse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_update_multiride(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_update_period(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_reverse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_ul_reverse_purse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);

unsigned char UL_TellSysCard(unsigned short tickettype, unsigned char *in_localtime);
char UL_CalEntryMAC(char *entryMAC);
char UL_CalExitMAC(char *exitMAC);
char UL_CalInitMAC(unsigned char activepage, short *cal_mac2, unsigned char *out_buf);
char UL_CalIssueMAC(char *issueMAC);
char UL_TellCallback(void);
char UL_TellDate(unsigned char *cur_timebcd, unsigned char *start_timebcd, unsigned char *end_timebcd);
char UL_TellEntryMAC(char entryMAC, unsigned char mode_check);
char UL_TellExitMAC(char *exitMAC);
char UL_TellInitMAC(unsigned char *initMAC);
char UL_TellIssueMAC(char *issueMAC);
char UL_TellOverRide(unsigned char *cur_timebcd, unsigned short srcstation, unsigned char *desstation, unsigned short laststation, unsigned char journeyStatus, long balance);
char UL_TellOverTime(unsigned char *entrytime, unsigned char *curtime, unsigned char mileclass);
char UL_TellTesting(unsigned char chTestMode);

#endif
