#ifndef TIME_TOOLS_H
#define TIME_TOOLS_H


#define  TIME2000   0x386d4380L		//from 1970-1-1 beijing Zone
#define  DAY2000	10957			//from 1970-1-1
#define	 DAY2006	2192			//from 2000-1-1
#define  WEEK1970	4
#define	 WEEK2000	6
#define  DAY1970	25569			//from 1900-1-1 and the day is 1 from 1900-1-1 to 1900-1-1
#define	 ZONE8		28800

//º¯Êý
unsigned short datestr2days(unsigned char *date);
void days2datestr(unsigned short indays,unsigned char *out_datestr);
void  timestr4to6(unsigned char *timestr4,unsigned char *timestr6);
void  timestr6to4(unsigned char *timestr6,unsigned char *timestr4);
unsigned short  time_chk_valid(unsigned char *timestr6);
unsigned short  date_chk_valid(unsigned char *datestr4);
void timestr4todate(unsigned char *timestr4);
unsigned long timestr2long(unsigned char *buf);
void long2timestr(unsigned long longt,unsigned char *buf);
void get_month_end_date(unsigned char *now_date,unsigned char *month_end_date);

unsigned char bcd2bin(unsigned char inbyte);
unsigned char bin2bcd(unsigned char inbyte);

void LocalDateTime2BCD(unsigned char *localdatetime,unsigned char *datetimebcd);
void BCD2LocalDateTime(unsigned char *datetimebcd,unsigned char *localdatetime);
unsigned short BCD2Date16(unsigned char *datetimebcd);
void Date162BCD(unsigned short Date16,unsigned char *datetimebcd);
unsigned long sz_get_seconds_since_midnight(unsigned char *datetimebcd);

int sz_localtimeToSecond(unsigned char *in_buf, long *lngHISecond, long *lngLOSecond);
int sz_localtimeToDay(unsigned char *in_buf, unsigned short *shDays, unsigned long *lngMidnightSecond);
char DaysToWeek(unsigned short shDays);

void set_timeout(long delay_time);

void xa_daytodate(short cardBaseDataTime, short day, unsigned long *second, unsigned char *date);
unsigned short xa_localtimeToMinute(unsigned char *date, short cardBaseDataTime, unsigned long *startDateTime);
long xa_MinuteTolocaltime(unsigned char *date, short cardBaseDataTime, long minute, unsigned long *startDateTime);
long xa_DurationTolocaltime(unsigned long basesecond, char durationtype, short duration, unsigned char *validdate);

#endif
