#ifndef BIN_FILE_MANAGE
#define BIN_FILE_MANAGE

#include "binEOD.h"
#include "xa_operation.h"

//test
//#define	DEBUG_TICKET		0
//#define	DEBUG_1_PRINT			1

#define	KM_RW_STOP			0xff
#define	KM_RW_IDLE			0x00
#define	KM_RW_SEARCH		0x01
#define	KM_RW_TRANSACTION	0x02
#define	KM_RW_RECORD		0x04

#define	SZ_DOWNLOAD_PARA	0
#define SZ_DOWNLOAD_TP		1
#define	SZ_DOWNLOAD_MODEL	2

#define	KM_WAIVER_NOMAL		0x00
#define	KM_WAIVER_EMERGENCY	0x01
#define	KM_WAIVER_ENTRY		0x02
#define	KM_WAIVER_DATE		0x04
#define	KM_WAIVER_TIME		0x08
#define	KM_WAIVER_FAILURE	0x10
#define	KM_WAIVER_FARE		0x20

long	tp_ver, wdmc_ver;

struct EOD_DOWNLOAD{
	unsigned short 	filetype;
	unsigned char 	filepath[200];
	unsigned char 	filename[41];
	unsigned long	filelen;
	unsigned char 	md5[16];

	long		curFilelen;		//have sended file len
	unsigned short	totalFrame;		//total frame number
	unsigned short	curFrame;		//current frame
};

#define	XA_WAIVER_LEN		20
struct STATION_WAIVER_MODE{
	unsigned long waivermode_len;
	unsigned char *waivermode_val;
};
struct waiver_mode{
	unsigned char cur_sta_failure;
	unsigned char cur_sta_entry;
	unsigned char cur_sta_date;
	unsigned char cur_sta_time;
	unsigned char cur_sta_fare;
	unsigned char cur_sta_emergency;
	unsigned char cur_sta_exit;
	
	unsigned char oth_sta_failure;
	unsigned char oth_sta_entry;
	unsigned char oth_sta_date;
	unsigned char oth_sta_time;
	unsigned char oth_sta_fare;
	unsigned char oth_sta_emergency;
	unsigned char oth_sta_exit;
	
	unsigned char sen_sta_failure;
	unsigned char sen_sta_entry;
	unsigned char sen_sta_date;
	unsigned char sen_sta_time;
	unsigned char sen_sta_fare;
	unsigned char sen_sta_emergency;
	unsigned char sen_sta_exit;

	unsigned long oth_entry_num;
	char *oth_entry_station;
	unsigned long sen_entry_num;
	char *sen_entry_station;
	
}__attribute__( ( packed, aligned(1) ) );

PACC_1101	tpSystem1101;
PACC_1102	tpBusiness1102;
PACC_1104	tpBlacklist1104;
PACC_1105	tpProduct1105;
PACC_1106	tpLocation1106;
PACC_1107	tpCalendar1107;
PACC_1108	tpFareTable1108;
PACC_1109	tpSaleTable1109;

PACC_1103	tpPACC1103;
PACC_1097	tpLCC1097;
PACC_1002	tpLCC1002;

YKT_1901	tpBlacklist1901;
YKT_1912	tpCard1912;
YKT_1913	tpProperty1913;
YKT_1914	tpLoad1914;
YKT_1919	tpTerminal1919;
YKT_1920	tpContinue1920;

JTB_1931_t	tpBlacklist1931;
JTB_1932_t	tpWhite1932;
JTB_1933_t	tpProperty1933;
JTB_1934_t	tpTerminal1934;
JTB_1935_t	tpPreferential1935;
JTB_1938_t	tpLoad1938;
JTB_1939_t	tpServer1939;

LCC_3021_t	tpSensitive3021;

Product_t	tpTicketDef;
Product_t	tpSJTTicketDef;

unsigned short	TxnUdType;

unsigned short 		para_type;						//记录交易记录消息类型码

struct EOD_DOWNLOAD eod_download;
struct STATION_WAIVER_MODE tpStationWaiverMode;
struct waiver_mode	tpwaivermode;

int active_eod_file(char * filename, long *lngver, char ver_len);
int active_tp_file(char * filename, long *lngver);

char check_station_id(unsigned char *station_id);
int cal_fare_time(unsigned short faretier, unsigned long *lngovertime);
char cal_overtime(unsigned char *entrytime, unsigned char *curtime, unsigned char mileclass, unsigned char station_mode);
//int cal_fare_value(unsigned char *srcstation_id, unsigned char *curtime,void *ticket, unsigned char *faretier, unsigned short *lngFareValue);
int fast_cal_fare(unsigned short shValue, unsigned char *faretier);
//int get_ticket_para(unsigned char chTickettype, void *td);
char CheckTicketOvertime(unsigned char *issue_bcd, unsigned char *curr_bcd, unsigned char ticketType);

void get_cur_para_ver(unsigned char *eod_type, long eod_ver, unsigned char *eod_validdate, unsigned char *out_buf);
void get_temp_para_ver(unsigned char *eod_file, short eod_type, unsigned char *eod_num, char eod_ctrl, unsigned char *out_buf);


unsigned short ee_write_last_record(char ticket_family, char flag,  unsigned char *in_buf, unsigned short in_len);
unsigned short ee_read_last_record(unsigned short ud_addr, unsigned char *out_buf, unsigned short *out_len, char upload_control);

unsigned char binFileManage(unsigned short shcmd, unsigned char *cmd_buf, int cmd_len, unsigned char *out_buf, unsigned short *out_len);

long PACC_1101_SystemParameter();
long PACC_1102_Business();
long PACC_1104_Blacklist();
long PACC_1105_Product();
long PACC_1106_Location();
long PACC_1107_Calendar();
long PACC_1108_FareTable();

long YKT_1901_Blacklist();
long YKT_1912_Card();
long YKT_1913_Property();
long YKT_1914_Load();
long YKT_1919_Terminal();
long YKT_1920_Continue();

long JTB_1931_Blacklist();
long JTB_1932_White();
long JTB_1933_Property();
long JTB_1934_Terminal();
long JTB_1935_Preferential();
long JTB_1938_Load();
long JTB_1939_Server();

long LCC_3021_Sensitive();

long LCC_1002();
long LCC_1097();
long LCC_1103();

int cal_station_fare(unsigned short FareCodeTableId, unsigned long srcstation_id, unsigned long desstation_id, unsigned short *shFare);
int cal_fare_value(unsigned char *curtime, Product_t *ticket, unsigned short farecode, unsigned char passenger, SYS_PRICE_t *pPrice);

int cal_station_multi_fare(unsigned short FareCodeTableId, unsigned long srcstation_id);
void get_degrade_sensitive_mode(unsigned char *src_station, unsigned char *cur_timebcd);
void get_degrade_mode(unsigned char *src_station);

int check_same_station(unsigned long curstation_id, unsigned short cardstation_code);
int card_to_location(short cardstation, unsigned long *sysstation);

#endif
