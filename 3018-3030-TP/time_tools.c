#include <string.h>
#include <sys/select.h>
#include <stdio.h>

#include "time_tools.h"
#include "hh_cpu_operation.h"

//年份表
//起始为1，表示从2000-1-1的天数---20120217 change the days  from 2000-1-1
static unsigned short YearTable[100] = {
    0,  366,  731, 1096, 1461, 1827, 2192, 2557, 2922, 3288,
 3653, 4018, 4383, 4749, 5114, 5479, 5844, 6210, 6575, 6940,
 7305, 7671, 8036, 8401, 8766, 9132, 9497, 9862,10227,10593,
10958,11323,11688,12054,12419,12784,13149,13515,13880,14245,
14610,14976,15341,15706,16071,16437,16802,17167,17532,17898,
18263,18628,18993,19359,19724,20089,20454,20820,21185,21550,
21915,22281,22646,23011,23376,23742,24107,24472,24837,25203,
25568,25933,26298,26664,27029,27394,27759,28125,28490,28855,
29220,29586,29951,30316,30681,31047,31412,31777,32142,32508,
32873,33238,33603,33969,34334,34699,35064,35430,35795,36160
};

//非闰年
static unsigned short MonthTable1[12] = {
0,31,59,90,120,151,181,212,243,273,304,334
};
//闰年
static unsigned short MonthTable2[12] = {
0,31,60,91,121,152,182,213,244,274,305,335
};

/*
function:
	change the localdatetime_t struct to UTC second, the days from 1970-1-1 and the second from the mid-night.
parameter:
	*in_buf:localdatetime_t, 4bytes
	*lngSecond: UTC second, 4bytes
	*shDays: days, 2bytes
	*lngMidnightSecond: second, 4bytes
return:
	0: ok
	nozero:localdatetime_t invalid
*/
int sz_localtimeToSecond(unsigned char *in_buf, long *lngHISecond, long *lngLOSecond)
{
unsigned long tlong;
unsigned char datetimebcd[7];

	memset(datetimebcd, 0x00, 7);
	LocalDateTime2BCD(in_buf, datetimebcd);
	if(!time_chk_valid(datetimebcd))
		return 0;
	//
	tlong = (datestr2days(datetimebcd)) * 24 * 3600;
	tlong += ((unsigned long)bcd2bin(datetimebcd[4]) * 3600 + (unsigned long)bcd2bin(datetimebcd[5]) * 60 + bcd2bin(datetimebcd[6]));
	if(tlong > 0x7fffffff)
	{
		*lngHISecond = (long)(tlong - 0x7fffffff);
		*lngLOSecond = 0x7fffffff;
	}else
	{
		*lngHISecond = 0;
		*lngLOSecond = (long)tlong;
	}	
	return 1;
}

int sz_localtimeToDay(unsigned char *in_buf, unsigned short *shDays, unsigned long *lngMidnightSecond)
{
unsigned char datetimebcd[7];

	memset(datetimebcd, 0x00, 7);
	LocalDateTime2BCD(in_buf, datetimebcd);
	if(!time_chk_valid(datetimebcd))
		return 0;

	//days from 1970-1-1
	*shDays = datestr2days(datetimebcd);
	//second from mid-night
	*lngMidnightSecond = sz_get_seconds_since_midnight(datetimebcd);

	return 1;
}
/*
function:
return:
	0:sunday 1~6:mon-sat
*/
char DaysToWeek(unsigned short shDays)
{
	return ((shDays % 7) + WEEK1970) % 7;
}

/*
function:
	calculate the month number from now to the card issued day.
parameter:
	*in_buf:current time (bcd)
	issued_days:issued days
	*months: month number, 4bytes
return:
	0 or more than 0: months
	less than 0:
*/
long sz_cal_month(unsigned char *in_buf, unsigned short issued_days, long *months)
{
unsigned long lngmonths;
unsigned char datetimebcd[7], chmonth1, chmonth2;
unsigned short shyear1, shyear2;

	memcpy(datetimebcd, in_buf, 7);
	shyear1 = bcd2bin(datetimebcd[0]) * 100;
	shyear1 += bcd2bin(datetimebcd[1]);
	chmonth1 = bcd2bin(datetimebcd[2]);
//PRINTK("now year %d month %d ", shyear1, chmonth1);	
	days2datestr(issued_days - DAY2000, datetimebcd);
	shyear2 = bcd2bin(datetimebcd[0]) * 100;
	shyear2 += bcd2bin(datetimebcd[1]);
	chmonth2 = bcd2bin(datetimebcd[2]);
//PRINTK(" old year %d month %d \n", shyear2, chmonth2);		
	lngmonths = (shyear1 - shyear2) * 12;
	lngmonths += (chmonth1 - chmonth2);
	*months = lngmonths;
	return lngmonths;
}

/*=====================================================================================
函数：bcd2bin
功能：
=======================================================================================*/
unsigned char bcd2bin(unsigned char inbyte)
{

	return ((inbyte / 0x10) * 10 + inbyte % 0x10);	
}	


/*=====================================================================================
函数：bin2bcd
功能：
=======================================================================================*/
unsigned char bin2bcd(unsigned char inbyte)
{
	inbyte = inbyte % 100;	
	
	return ((inbyte / 10) * 16 + inbyte % 10);	
}	

/*==============================================================
函数:datestr2days
功能：将日期串转换成以1970起始的天数
buf:  CCYYMMDD
=================================================================*/
unsigned short datestr2days(unsigned char *date)
{
unsigned short tint, tyear;
unsigned char ch;

	//年
	tyear = bcd2bin(date[0]) * 100 + bcd2bin(date[1]);
	//ch = bcd2bin(date[1]);
	//ch = tyear - 1970;
	ch = tyear - 2000;
	tint = YearTable[ch];
	//月
	if(ch%4 == 0)
	{	//闰年
		ch = bcd2bin(date[2]);
		tint += MonthTable2[ch-1];
	}else
	{	//非闰年
		ch = bcd2bin(date[2]);
		tint += MonthTable1[ch-1];
	}
	//日  
	ch = bcd2bin(date[3]);
	tint += (unsigned short)(ch - 1);

	//return tint - 1;
	return tint + DAY2000;
}


/*==============================================================
函数:days2datestr
功能：
=================================================================*/
void  days2datestr(unsigned short indays, unsigned char *out_datestr)
{
unsigned char i;

	out_datestr[0] = 0x20;

	//年
	for(i = 0; i < 100; i++){
	  if(indays < YearTable[i + 1]) break;
	}
	out_datestr[1] = bin2bcd(i);  //年
	indays -= (unsigned short)YearTable[i];

	//月日
	if((i % 4) == 0){  //闰年
		for(i = 0; i < 11; i++){
			if(indays < MonthTable2[i + 1]) break;
		}
		//月
		out_datestr[2] = bin2bcd(i + 1);
		//日
		indays -= MonthTable2[i];
		out_datestr[3] = bin2bcd(indays + 1);
  	}
	else{             //非闰年
		for(i = 0; i < 11; i++){
			if(indays < MonthTable1[i + 1]) break;
     		}
		//月
		out_datestr[2] = bin2bcd(i + 1);
		//日
		indays -= MonthTable1[i];
		out_datestr[3] = bin2bcd(indays + 1);
	}

	return;
}  


/*==============================================================
函数:timestr4to6
功能：
=================================================================*/
void  timestr4to6(unsigned char *timestr4, unsigned char *timestr6)
{
unsigned long tlong;
unsigned char ch;

	*((char *)&tlong + 3) = timestr4[0];
	*((char *)&tlong + 2) = timestr4[1];
	*((char *)&tlong + 1) = timestr4[2];
	*((char *)&tlong + 0) = timestr4[3];

	//分
	ch = *((char *)&tlong + 0);
	ch = ch & 0x3f;
	timestr6[5] = bin2bcd(ch);
	tlong = tlong >> 6;
	//时
	ch = *((char *)&tlong + 0);
	ch = ch & 0x1f;
	timestr6[4] = bin2bcd(ch);
	tlong = tlong >> 5;
	//日
	ch = *((char *)&tlong + 0);
	ch = ch & 0x1f;
	timestr6[3] = bin2bcd(ch);
	tlong = tlong >> 5;
	//月 
	ch = *((char *)&tlong + 0);
	ch = ch & 0x0f;
	timestr6[2] = bin2bcd(ch);
	tlong = tlong >> 4;
	//年
	ch = *((char *)&tlong + 0);
	ch = ch&0xff;
	timestr6[1] = bin2bcd(ch);

	timestr6[0] = 0x20;

	return;
}  

/*==============================================================
函数:timestr6to4
功能：CCYYMMDDHHMM -> hex压缩串
=================================================================*/
void  timestr6to4(unsigned char *timestr6, unsigned char *timestr4)
{
unsigned long tlong;

	tlong = (unsigned char)bcd2bin(timestr6[1]);
	tlong = tlong << 4;
	tlong += (unsigned char)bcd2bin(timestr6[2]);
	tlong = tlong << 5;
	tlong += (unsigned char)bcd2bin(timestr6[3]);
	tlong = tlong << 5;
	tlong += (unsigned char)bcd2bin(timestr6[4]);
	tlong = tlong << 6;
	tlong += (unsigned char)bcd2bin(timestr6[5]);

	timestr4[0] = *((char *)&tlong + 3);
	timestr4[1] = *((char *)&tlong + 2);
	timestr4[2] = *((char *)&tlong + 1);
	timestr4[3] = *((char *)&tlong + 0);

	return;
}


/*==============================================================
函数:
功能:
=================================================================*/
void timestr4todate(unsigned char *timestr4)
{
unsigned char buf[6];

	timestr4to6(timestr4, buf);
	memcpy((char *)timestr4, (char *)buf, 4);
	return;
}

/*==============================================================
函数:timestr6to4
功能:YYYYMMDDHHMM
=================================================================*/
unsigned short  time_chk_valid(unsigned char *timestr6)
{
unsigned char i, high, low;
unsigned char buf[7];
unsigned char month[12] = {31,29,31,30,31,30,31,31,30,31,30,31};

	//PRINTK("%0x%02x-%02x-%02x %02x\n", timestr6[0], timestr6[1], timestr6[2], timestr6[3], timestr6[4], timestr6[5]);
	//BCD检查
	for(i = 0; i < 6; i++)
	{
		high = timestr6[i] / 16;
		if(high > 9) goto label_err;
		low = timestr6[i] % 16;
		if(low > 9) goto label_err;
		buf[i] = high * 10 + low;
	}

	//CC
	if(buf[0] != 20) goto label_err;
	//MM
	if((buf[2] > 12) ||(buf[2] == 0)) goto label_err;
	if((buf[2] == 2) && (buf[3] == 29))
	{
		if(buf[1]%4 != 0) goto label_err;  //非闰年
	}
	//DD
	if(buf[3] > month[buf[2]-1]) goto label_err;
	if(buf[3] == 0) goto label_err;
	//HH
	if(buf[4] > 23) goto label_err;
	//MM
	if(buf[5] > 59) goto label_err;

	return 1;

//非法
label_err:
	return 0;  
}  


/*==============================================================
函数:check the bcd date format
功能：
=================================================================*/
unsigned short  date_chk_valid(unsigned char *datestr4)
{
unsigned char i,high,low;
unsigned char buf[5];
unsigned char month[12] = {31,29,31,30,31,30,31,31,30,31,30,31};

	//BCD检查
	for(i = 0; i < 4; i++)
	{
		high = datestr4[i] / 16;
		if(high > 9) goto label_err;
		low = datestr4[i] % 16;
		if(low > 9) goto label_err;
		buf[i] = high * 10 + low;
	}

	//CC
	if(buf[0] != 20) goto label_err;
	//MM
	if((buf[2] > 12) ||(buf[2] == 0)) goto label_err;
	if((buf[2] == 2) && (buf[3] == 29)){
		if(buf[1]%4 != 0) goto label_err;  //非闰年
	}
	//DD
	if(buf[3] > month[buf[2]-1]) goto label_err;
	if(buf[3] == 0) goto label_err;

	return 1;
//非法
label_err:
	return 0;  
}

/*==============================================================
函数:timestr2long
功能：将时间串转换成以2000起始的秒数
	buf:  year(1bcd),month,date,hour,minute,second
=================================================================*/
unsigned long timestr2long(unsigned char *buf)
{
unsigned long tlong;
unsigned char ch, ch1;

	//year
	ch = bcd2bin(buf[0]);
	tlong = (unsigned long)((unsigned short)YearTable[ch]);
	//month
	ch1 = bcd2bin(buf[1]);
	if(ch%4 == 0)
		tlong += (unsigned long)((unsigned short)MonthTable2[ch1-1]);
	else
		tlong += (unsigned long)((unsigned short)MonthTable1[ch1-1]);
	//day
	ch = bcd2bin(buf[2]);
	tlong += (unsigned long)((unsigned char)(ch-1));

	//to hour
	tlong *= 24;
	ch = bcd2bin(buf[3]);
	tlong += (unsigned long)((unsigned char)ch);
	//to minute
	tlong *= 60;
	ch = bcd2bin(buf[4]);
	tlong += (unsigned long)((unsigned char)ch);
	//to second
	tlong *= 60;
	ch = bcd2bin(buf[5]);
	tlong += (unsigned long)((unsigned char)ch);

	return tlong;
}

/*====================================================================
函数:long2timestr
功能：将以2000起始的秒数转换成时间串
buf:  20year(1bcd),month,date,hour,minute,second-20130205
======================================================================*/
void long2timestr(unsigned long longt,unsigned char *buf)
{
unsigned long seconds;
unsigned short  tint,days;
unsigned char i;

	tint = (unsigned short)((unsigned long)longt/(unsigned long)86400L);
	
	buf[0] = 0x20;
	for(i = 0; i < 100; i++){
		if(tint < YearTable[i+1]) break;
	}
	buf[1] = bin2bcd(i);  //年
	days = (unsigned short)tint - (unsigned short)YearTable[i];

	seconds = longt%86400L;

	if(i%4 == 0){  //闰年
		for(i=0;i<12;i++){
			if(days < MonthTable2[i+1]) break;
			else if(i==11) break;
		}
		//月
		buf[2] = bin2bcd(i+1);
		//日
		i = days-MonthTable2[i];
		buf[3] = bin2bcd(i+1);
	}
	else{             //非闰年
		for(i=0;i<12;i++){
			if(days < MonthTable1[i+1]) break;
			else if(i==11) break;
		}
		//月
		buf[2] = bin2bcd(i+1);
		//日
		i = days-MonthTable1[i];
		buf[3] = bin2bcd(i+1);
	}
	//时
	i = (unsigned char)((unsigned long)seconds/(unsigned long)3600L);
	buf[4] = bin2bcd(i);
	//分
	tint = (unsigned short)((unsigned long)seconds%(unsigned long)3600L);
	i = (unsigned char)((unsigned short)tint/(unsigned short)60);
	buf[5] = bin2bcd(i);
	//秒
	i = (unsigned char)((unsigned short)tint%(unsigned short)60);
	buf[6] = bin2bcd(i);

	return;
}


/*====================================================================
函数:get_month_end_date
功能：
======================================================================*/
void get_month_end_date(unsigned char *now_date,unsigned char *month_end_date)
{
unsigned char year,month,day;	
#ifdef _EMU_WIN_
AnsiString str;
int i;
#endif

	memcpy(month_end_date,now_date,3);
	year = bcd2bin(now_date[1]);
	month = bcd2bin(now_date[2]);
	if(month==2){
		if((year%4)==0) day=0x29;
		else day=0x28;
		goto label_end;		 
	}
	switch(month){
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		day = 0x31;
		break;
	default:
		day = 0x30;
		break;
	}

label_end:
	month_end_date[3] = day;
#ifdef _EMU_WIN_
	str.PRINTK("get_month_end_date:date=");
	for(i=0;i<4;i++) str.cat_printf("%02X",(unsigned char)now_date[i]);
	str.cat_printf("\nend date:");
	for(i=0;i<4;i++) str.cat_printf("%02X",(unsigned char)month_end_date[i]);
	debug_show(str,clPurple);
#endif
	return;
}


/*====================================================================
函数:time_wr_diff
功能：
======================================================================*/
#ifdef _EMU_WIN_
void time_wr_diff(int diff)
{
TIniFile *ini_file;
AnsiString file_name,str;

	file_name = ExtractFilePath(Application->ExeName)+"skj100.ini";

	ini_file = new TIniFile(file_name);

	str.PRINTK("%d",diff);
	ini_file->WriteString("时间调校","差值",str);

	delete ini_file;
	return;  
}  
#endif

/*====================================================================
函数:time_rd_diff
功能：
======================================================================*/
#ifdef _EMU_WIN_
int  time_rd_diff(void)
{
TIniFile *ini_file;
AnsiString file_name,str;
int diff;

	file_name = ExtractFilePath(Application->ExeName)+"skj100.ini";

	if(!FileExists(file_name)){
		return 0;
	}

	ini_file = new TIniFile(file_name);

	str = ini_file->ReadString("时间调校","差值",0);
	sscanf(str.c_str(),"%d",&diff);

	delete ini_file;
	return diff;  
}
#endif

//苏州读卡器
//参见数据字典中LocalDateTime_t定义
//B31-B26：年(有效值为0到63；表示为2000年到2064年)
//B25-B22：月（有效值为1到12；表示为1月到12月；0、13、14、15为非法值）
//B21-B17：日（有效值为1到31；表示为1日到31日；0为非法值）
//B16-B12：时（有效值为0到23；表示为0点到23日；24到31为非法值）
//B11-B6：分（有效值为0到59；表示为0分到59分；60到63为非法值）
//B5-B0：秒（有效值为0到59；表示为0秒到59秒；60到63为非法值）
//这样能表示64年，需要在全局参数中定义一个起始的基准年
/*==============================================================
函数: LocalDateTime2BCD
功能：将4字节LOCALDATETIME日期时间转换为7字节BCD时间CCYYMMDDHHMMSS
localdatetime:  XXXXXXXX
datetimebcd:  CCYYMMDDHHMMSS
=================================================================*/
void LocalDateTime2BCD(unsigned char *localdatetime, unsigned char *datetimebcd)
{
unsigned long tlong;

	*((char *)&tlong + 3) = localdatetime[0];
	*((char *)&tlong + 2) = localdatetime[1];
	*((char *)&tlong + 1) = localdatetime[2];
	*((char *)&tlong + 0) = localdatetime[3];

	datetimebcd[0] = 0x20;
	datetimebcd[1] = bin2bcd((tlong & 0xfc000000) >> 26);
	datetimebcd[2] = bin2bcd((tlong & 0x03c00000) >> 22);
	datetimebcd[3] = bin2bcd((tlong & 0x003e0000) >> 17);
	datetimebcd[4] = bin2bcd((tlong & 0x0001f000) >> 12);
	datetimebcd[5] = bin2bcd((tlong & 0x00000fc0) >> 6);		
	datetimebcd[6] = bin2bcd((tlong & 0x0000003f) >> 0);

	return;
}

/*==============================================================
函数: BCD2LocalDateTime
功能：将7字节BCD时间CCYYMMDDHHMMSS转换为4字节LOCALDATETIME日期时间
datetimebcd:  CCYYMMDDHHMMSS
localdatetime:  XXXXXXXX
=================================================================*/
void BCD2LocalDateTime(unsigned char *datetimebcd, unsigned char *localdatetime)
{
unsigned long tlong;

	tlong = 0;
//默认世纪为0x20
	tlong |= ((bcd2bin(datetimebcd[1]) & 0x3f) << 26);
	tlong |= ((bcd2bin(datetimebcd[2]) & 0x0f) << 22);
	tlong |= ((bcd2bin(datetimebcd[3]) & 0x1f) << 17);
	tlong |= ((bcd2bin(datetimebcd[4]) & 0x1f) << 12);
	tlong |= ((bcd2bin(datetimebcd[5]) & 0x3f) << 6);
	tlong |= ((bcd2bin(datetimebcd[6]) & 0x3f) << 0);

	localdatetime[0] = *((char *)&tlong + 3);
	localdatetime[1] = *((char *)&tlong + 2);
	localdatetime[2] = *((char *)&tlong + 1);
	localdatetime[3] = *((char *)&tlong + 0);

	return;
}

// 本地时间是指在格林威治时间的基础上，同时考虑地区时钟的差异。
// 例如北京为UTC+8:00时区。在计算本地时间时，各个操作系统都是可以设置本地时差的，所以各操作系统也提供了如何获取本地时间的方法。
// 但是由于ACC/AFC系统庞大，所涉及到的终端设备众多，为了强化系统时间同步，所有本地时间的获取需先取得UTC时间，
// 再计算北京时区差异的值，自行计算本地时间。
// 禁止直接获操作系统提供的本地时间。
// 这样做的好处可以使得所有设备共享时钟服务器，获得相同的本地时间。
// 而与本地时区设置的值无关。
// 0：1970年1月1日
// 1：1970年1月2日
/*==============================================================
函数: Date162BCD
功能：将2字节Date16天数转换为CCYYMMDD
Date16:  XX XX
datetimebcd:  CCYYMMDD
=================================================================*/
void Date162BCD(unsigned short Date16, unsigned char *datetimebcd)
{
unsigned long tlong;
unsigned char buf[20];

	tlong = Date16 * 24 * 3600;
	tlong -= TIME2000;
	long2timestr(tlong, &buf[0]);

	memcpy(datetimebcd, buf, 4);

	return;
}

/*================================================================
 BCD2Date16
 BCD时间转换为1970/1/1到现在的天数
=================================================================*/
unsigned short BCD2Date16(unsigned char *datetimebcd)
{
unsigned short days;
unsigned short year;
unsigned char mon,day;

	year = bcd2bin(datetimebcd[0]) * 100 + bcd2bin(datetimebcd[1]);
	mon = bcd2bin(datetimebcd[2]);
	day = bcd2bin(datetimebcd[3]);

	if (0 >= (int)(mon -= 2)) 
	{
        	mon += 12;
        	year -= 1;
	}
 
	days =year/4 - year/100 + year/400 +(year-1)*365 +30*mon - 30 + (mon+mon/6)/2 + day-1 + 59- 719162;

	return days;
}

//从BCD格式日期时间中取从0点以来的秒数
unsigned long sz_get_seconds_since_midnight(unsigned char *datetimebcd)
{
unsigned long tlong;

	tlong = 0;
	tlong += (unsigned long)bcd2bin(datetimebcd[4]) * 3600;
	tlong += (unsigned long)bcd2bin(datetimebcd[5]) * 60;
	tlong += bcd2bin(datetimebcd[6]);
	
	return tlong;	
}

/*
function:YYYYMMDDHHMMSS
	YYYYMM01~YYYYMM31
*/
void sz_get_month_day(unsigned char *datetimebcd, short *firstdays, short *enddays)
{
char mon_first_day[7], mon_end_day[7];
short shyear;

	//the first days of the month
	memcpy(mon_first_day, datetimebcd, 7);
	//the first day of the month 
	mon_first_day[3] = 0x01;
	*firstdays = datestr2days(mon_first_day);
	//the end days of the month
	mon_first_day[2] += 1;
	if(mon_first_day[2] >= 0x13)
	{
		mon_first_day[2] = 1;
		shyear = bcd2bin(mon_first_day[0]) * 100 + bcd2bin(mon_first_day[1]) + 1;
		mon_first_day[0] = bin2bcd(shyear / 100);
		mon_first_day[1] = bin2bcd(shyear % 100);
	}
	*enddays = datestr2days(mon_first_day);
	*enddays -= 1;
}

/*==============================================================
function: second2days
	change the second from 1970-1-1 0:0:0 to the days from 1970-1-1
parameter:
return:
=================================================================*/
unsigned short second2days(long hisecond, long lowsecond, unsigned short *days)
{
unsigned long hidays, himodsec, lowdays, lowmodsec;
unsigned short ret_days;

	//
	hidays = hisecond / (24 * 3600);
	himodsec = hisecond % (24 * 3600);
	
	lowdays = lowsecond / (24 * 3600);
	lowmodsec = lowsecond % (24 * 3600);
	
	ret_days = hidays + lowdays + (himodsec + lowmodsec) % (24 * 3600);
	if(days != NULL)
		*days = ret_days;
	
	return ret_days;	
}

/*
cardBaseDataTime: month from 2006-1-1
*/
void xa_monthtodate(short cardBaseDataTime, unsigned char *date)
{
unsigned short tyear;

	//get days from cardBaseDataTime
	tyear = 2006 + cardBaseDataTime / 12;
	date[0] = bin2bcd(tyear / 100);
	date[1] = bin2bcd(tyear % 100);
	
	date[2] = bin2bcd((cardBaseDataTime % 12) + 1);
	
	date[3] = 1;
	
	return ;
}

short xa_datetomonth(short *cardBaseDataTime, unsigned char *date)
{
unsigned short tyear;

	//get days from cardBaseDataTime
	tyear = bcd2bin(date[0]) * 100 + bcd2bin(date[1]);
	
	*cardBaseDataTime = (tyear - 2006) * 12 + (bcd2bin(date[2]) - 1);
	
	return (*cardBaseDataTime);
}
/*
cardBaseDataTime: month from 2006-1-1
second:from 2000-1-1
*/
void xa_daytodate(short cardBaseDataTime, short day, unsigned long *second, unsigned char *date)
{
unsigned short tyear;

	//
	memset(date, 0x00, 7);
	//year
	tyear = 2006 + cardBaseDataTime / 12;
	date[0] = bin2bcd(tyear / 100);
	date[1] = bin2bcd(tyear % 100);
	//month
	date[2] = bin2bcd((cardBaseDataTime % 12) + 1);
	//day
	date[3] = 1;
	//PRINTK("date:%02x%02x-%02x-%02x\n", date[0], date[1], date[2], date[3]);
	//
	*second = (unsigned long)day * 24 * 3600;
	//PRINTK("day second %d ", *second);
	(*second) += timestr2long(&date[1]);
	//PRINTK("and date %d\n", *second);
	//
	long2timestr((*second), &date[0]);
	//PRINTK("date:%02x%02x-%02x-%02x\n", date[0], date[1], date[2], date[3]);
	
	return ;
}


/*
date:current date/time -7 bytes BCD
cardBaseDataTime: month from 2006-1-1
startdatetime: minute from cardBaseDataTime
return: day from basetime
*/
unsigned short xa_localtimeToMinute(unsigned char *date, short cardBaseDataTime, unsigned long *startDateTime)
{
unsigned short tint, tyear;
unsigned char ch;
unsigned short	dayfrombase;

	//get days from current date/time
	//年
	tyear = bcd2bin(date[0]) * 100 + bcd2bin(date[1]);
	//ch = bcd2bin(date[1]);
	//ch = tyear - 1970;
	ch = tyear - 2000;
	tint = YearTable[ch];
	//月
	if(ch%4 == 0)
	{	//闰年
		ch = bcd2bin(date[2]);
		tint += MonthTable2[ch-1];
	}else
	{	//非闰年
		ch = bcd2bin(date[2]);
		tint += MonthTable1[ch-1];
	}
	//日  
	ch = bcd2bin(date[3]);
	tint += (unsigned short)(ch - 1);
	//PRINTK("from 2000-1-1 day is %04x ", tint);
	
	//get days from cardBaseDataTime
	tyear = 2006 + cardBaseDataTime / 12 - 2000;
	dayfrombase = YearTable[tyear];
	if(tyear % 4 == 0)
	{
		dayfrombase += MonthTable2[cardBaseDataTime % 12];
	}else
	{
		dayfrombase += MonthTable1[cardBaseDataTime % 12];
	}
	//PRINTK("from 2006-1-1 day is %04x \n", dayfrombase);
	//
	dayfrombase = tint - dayfrombase;
	//calculate minute from BaseDataTime
	*startDateTime = (dayfrombase) * 24 * 60 + bcd2bin(date[4]) * 60 + bcd2bin(date[5]);
	
	return dayfrombase;
}

/*
date:current date/time -7 bytes BCD
cardBaseDataTime: month from 2006-1-1
startdatetime: minute from cardBaseDataTime
	
*/
long xa_MinuteTolocaltime(unsigned char *date, short cardBaseDataTime, long minute, unsigned long *startDateTime)
{
unsigned short tint, tyear;
unsigned char ch;
unsigned short	dayfrombase;

	xa_daytodate(cardBaseDataTime, 0, startDateTime, date);
	//
	*startDateTime += (minute * 60);
	//
	long2timestr(*startDateTime, date);
	return *startDateTime;
}

/*
date:current date/time -7 bytes BCD
cardBaseDataTime: month from 2006-1-1
startdatetime: minute from cardBaseDataTime
validdate: 7 bytes bcd
*/
long xa_DurationTolocaltime(unsigned long basesecond, char durationtype, short duration, unsigned char *validdate)
{
unsigned long lngduration;
short shMonths;
unsigned char date[7];

	lngduration = 0;
	switch(durationtype)
	{
	case 0:		//minute
		lngduration = duration * 60;
		lngduration += basesecond;
		//
		long2timestr(lngduration, validdate);
		break;
	case 1:		//hour
		lngduration = duration * 3600;
		lngduration += basesecond;
		//
		long2timestr(lngduration, validdate);
		break;
	case 2:		//days
		lngduration = duration * 24 * 3600;
		lngduration += basesecond;
		//
		long2timestr(lngduration, validdate);
		memset(&validdate[4], 0x00, 3);
		break;
	case 3:		//calendar month
		long2timestr(basesecond, date);
		if(date[3] < 0x16)
			basesecond += (31 * 24 * 3600);
		else
			basesecond += (16 * 24 * 3600);
		long2timestr(basesecond, date);

		xa_datetomonth(&shMonths, date);
		shMonths += duration;
		xa_monthtodate(shMonths, validdate);
		memset(&validdate[4], 0x00, 3);
		break;
	}
#ifdef	DEBUG_PRINT
	PRINTK("basesecond %d durationtype %d duration %d validdate %02x%02x-%02x-%02x\n", basesecond, durationtype, duration, validdate[0], validdate[1], validdate[2], validdate[3]);
#endif
	return lngduration;
}


void set_timeout(long delay_time)
{
struct timeval timeout;
fd_set readfd;

	FD_ZERO(&readfd);
	FD_SET(1, &readfd);
	timeout.tv_sec = 0;
	timeout.tv_usec = delay_time;
	select(0, NULL, NULL, NULL, &timeout);
	
}