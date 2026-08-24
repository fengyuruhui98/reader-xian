#define	SZ_WAIVER_NOMAL		0
#define	SZ_WAIVER_FAILURE	1
#define	SZ_WAIVER_ENTRY		2
#define	SZ_WAIVER_DATE		3
#define	SZ_WAIVER_FARE		5
#define	SZ_WAIVER_EMERGENCY	6

struct STATION_WAIVER_MODE{
	unsigned short waivermode_len;
	unsigned char *waivermode_val;
};
struct waiver_mode{
	unsigned char cur_sta_failure;
	unsigned char cur_sta_entry;
	unsigned char cur_sta_date;
	unsigned char cur_sta_fare;
	unsigned char cur_sta_emergency;
	
	unsigned char oth_sta_failure;
	unsigned char oth_sta_entry;
	unsigned char oth_sta_date;
	unsigned char oth_sta_fare;
	unsigned char oth_sta_emergency;
	
	unsigned long oth_entry_num;
	char *oth_entry_station;
	unsigned long sen_entry_num;
	char *sen_entry_station;
	
	unsigned char sen_sta_failure;
	unsigned char sen_sta_entry;
	unsigned char sen_sta_date;
	unsigned char sen_sta_fare;
	unsigned char sen_sta_emergency;
};

struct STATION_WAIVER_MODE tpStationWaiverMode;
struct waiver_mode	tpwaivermode;
