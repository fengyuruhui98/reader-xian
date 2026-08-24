#ifndef _GLOBAL_C_
#define _GLOBAL_C_
//start of file

#include "global.h"

//#define	DEBUG_HARDWARE		1

unsigned char bgLibInUseFlag = 0;
//RF
unsigned char blnRF_00_INIT = 0;
unsigned char blnRF_01_INIT = 0;
int  igLibFd;         
//EEPROM file
unsigned char EEPROM[8192];
unsigned char bgEEInUseFlag = 0xff;
FILE	*flEEprom;
int		fdEEprom;

//LED
int	flLEDRed = 0;
int flLEDGreen = 0;
unsigned char blnLEDRedUseFlag = 0xff, blnLEDGreenUseFlag = 0xff;
//3018
#ifdef	READER_3018
//EEPROM system 
	static const char *eeprom = "/sys/bus/i2c/devices/2-0050/eeprom";
	static const char *redled = "/sys/bus/platform/devices/leds-gpio/leds/red/brightness";
	static const char *greenled = "/sys/bus/platform/devices/leds-gpio/leds/green/brightness";
#else
//3030
//EEPROM system 
	static const char *eeprom = "/sys/bus/i2c/devices/2-0050/eeprom";
	static const char *redled = "/sys/class/leds/red/brightness";
	static const char *greenled = "/sys/class/leds/green/brightness";
#endif
static const char *ledon ="1";
static const char *ledoff ="0";

//watchdog
int	flWatchdog;

const char *
rfid_hexdump(const void *data, unsigned int len)
{
	static char string[1024];
	unsigned char *d = (unsigned char *) data;
	unsigned int i, left;

	string[0] = '\0';
	left = sizeof(string);
	for (i = 0; len--; i += 3) {
		if (i >= sizeof(string) -4)
			break;
		snprintf(string+i, 4, " %02x", *d++);
	}
	return string;
}


void delay_ms(int cnt)//???????????
{
	usleep(1000*cnt);
}

void delay_us(int cnt)//???????????
{
	usleep(cnt);
}

void ms_delay(int cnt)//???????????
{
	usleep(1000*cnt);
}

void us_delay(int cnt)//???????????
{
	usleep(cnt); 
}

UBYTE InitModule(void)
{
UBYTE	ver[200], len;

	if(bgLibInUseFlag) return 0;  //已经开启
	//	
	uart_init();
	//uart_open(SAM_UART_INDEX,115200);
	uart_open(SAM_UART_INDEX, 460800);
	//linux_sam_init();

	reader_get_version(ver, &len);
#ifdef	DEFINE_SUZHOU_XIAN
	printf("libLinux4418.so For suzhou or xian:%s", ver);
#else
	printf("libLinux4418.so For Dalian or ningbo:%s_%s %s\n", ver, __DATE__, __TIME__);
#endif		
	bgLibInUseFlag = 0xff;
	return 0;	
}

/*==============================================================================
函数：
功能：
================================================================================*/
UBYTE CleanUpModule(void)
{
	if(!bgLibInUseFlag)	return 0; 

	close(igLibFd);
	bgLibInUseFlag = 0;

	return 0;
}


UBYTE rf_reset(void)
{
	//用于再次重启射频芯片2018/12/12 16:13:43
	rf_select(0);
	rc_init();
	rf_select(1);
	rc_init();
	
	return 0;
}

void set_card_type(UBYTE card_type)
{
UBYTE	ret;

	ret = rc_select_op_type(card_type);
	
	return ;
}

UBYTE UL_Anticoll_Select(UBYTE * psnr)
{
UBYTE	ret;
UBYTE cardsnr1[5],cardsnr2[5], buf[5];

	ret = mcml_anticoll(cardsnr1);
	if(ret) return ret;
	
	ret = mcml_select(cardsnr1, buf);
	if(ret) return ret;
	
	psnr[7] = buf[0];
	
	ret = mcml_anticoll2(cardsnr2);
	if(ret) return ret;
		
	ret = mcml_select2(cardsnr2, buf);
	if(ret) return ret;

	memcpy(psnr, &cardsnr1[1], 3);
	memcpy(&psnr[3], cardsnr2, 4);
	
	return 0;
}

UBYTE UL_Page_Read(UBYTE addr, UBYTE *pReaddata)
{
UBYTE	oBuf[20], ret;

	ret = mcml_read(addr, oBuf);
	memcpy(pReaddata, oBuf, 16);
	
	return ret;
}

UBYTE UL_Page_Write(UBYTE addr, UBYTE *pWritedata)
{
UBYTE	iBuf[20], ret;

	memset(iBuf, 0x00, sizeof(iBuf));
	memcpy(iBuf, pWritedata, 4);

	//return mcml_write(addr, iBuf);
	return mcml_write_4bytes(addr, iBuf);
}

UBYTE mcml_request2(UBYTE request_type,UBYTE * atq)
{
	if( (!blnRF_00_INIT ) && (bgRfNow == 0) )
	{
		rc_init();
		blnRF_00_INIT = 0xff;
	}
	if( (!blnRF_01_INIT ) && (bgRfNow == 1) )
	{
		rc_init();
		blnRF_01_INIT = 0xff;
	}
	return mcml_request(request_type, atq);
}

UWORD rtc_rd_time(UBYTE *outbuf)
{
time_t lnglocaltime;
struct tm *tplocaltime;
long year;
unsigned char datTime;

	time(&lnglocaltime);
	tplocaltime = localtime(&lnglocaltime);

	year = tplocaltime->tm_year + 1900;
	
	outbuf[0] = (unsigned char)(year-2000);
	outbuf[0] = (outbuf[0]/10)*16 + (outbuf[0]%10);
	
	outbuf[1] = tplocaltime->tm_mon+1;
	outbuf[1] = (outbuf[1]/10)*16 + (outbuf[1]%10);
	
	outbuf[2] = tplocaltime->tm_mday;
	outbuf[2] = (outbuf[2]/10)*16 + (outbuf[2]%10);
	
	outbuf[3] = tplocaltime->tm_hour;
	outbuf[3] = (outbuf[3]/10)*16 + (outbuf[3]%10);
	
	outbuf[4] = tplocaltime->tm_min;
	outbuf[4] = (outbuf[4]/10)*16 + (outbuf[4]%10);
	
	outbuf[5] = tplocaltime->tm_sec;
	outbuf[5] = (outbuf[5]/10)*16 + (outbuf[5]%10);
	
#ifdef	DEBUG_HARDWARE
	printf("date %02x-%02x-%02x %02x:%02x:%02x\n", outbuf[0], outbuf[1], outbuf[2], outbuf[3], outbuf[4], outbuf[5]);
#endif

	return 0;
}

UWORD rtc_wr_time(UBYTE *inbuf)
{
struct timeval tv;
struct tm tplocaltime;
long year;

	tplocaltime.tm_year = (((inbuf[0]/16)*10) + (inbuf[0]%16)) + 2000 - 1900;

	tplocaltime.tm_mon = ((inbuf[1]/16)*10) + (inbuf[1]%16) - 1;

	tplocaltime.tm_mday = ((inbuf[2]/16)*10) + (inbuf[2]%16);

	tplocaltime.tm_hour = ((inbuf[3]/16)*10) + (inbuf[3]%16);

	tplocaltime.tm_min = ((inbuf[4]/16)*10) + (inbuf[4]%16);
	
	tplocaltime.tm_sec = ((inbuf[5]/16)*10) + (inbuf[5]%16);
	
	tv.tv_sec = mktime(&tplocaltime);
	tv.tv_usec = 0;
	
	settimeofday(&tv, NULL);
	
	return 0;
}

UWORD ee_read(UWORD addr,UWORD bytes,UBYTE *outbuf)
{
int i;
int ret, len;

	InitModule();
	if(bgEEInUseFlag)
	{
		fdEEprom = open(eeprom, O_RDWR | O_SYNC);
		if( fdEEprom < 0)
			return 0xff;
			
		bgEEInUseFlag = 0;
		len = ret = read(fdEEprom, EEPROM, sizeof(EEPROM));
		while(len != sizeof(EEPROM))
		{
			ret = read(fdEEprom, &EEPROM[len], sizeof(EEPROM) - len);
			len += ret;
#ifdef	DEBUG_HARDWARE
			printf("read return %d len is %d\n", ret, len);
#endif
		}
	}
	memcpy(outbuf, &EEPROM[addr], bytes);
#ifdef	DEBUG_HARDWARE
	printf("Read EEPROM:");
	for(i = addr; i < addr + bytes; i++)
	{
		printf("%02x", EEPROM[i]);
		if((i % 16) == 0) printf("\n");
	}
	printf("\n");
#endif

	return 0;
}

UWORD ee_write(UWORD addr,UWORD bytes,UBYTE *inbuf)
{
int i;
int ret, len;

	InitModule();
	if(bgEEInUseFlag)
	{
		//flEEprom = fopen("./eeprom", "wb+");
		//if(flEEprom == NULL)
		//	return 0xff;
		
		fdEEprom = open(eeprom, O_RDWR | O_SYNC);
		if(fdEEprom < 0)
			return 0xff;
			
		bgEEInUseFlag = 0;
		//fread(EEPROM, sizeof(EEPROM), 1, flEEprom);
		len = ret = read(fdEEprom, EEPROM, sizeof(EEPROM));
		while(len != sizeof(EEPROM))
		{
			ret = read(fdEEprom, &EEPROM[len], sizeof(EEPROM) - len);
			len += ret;
#ifdef	DEBUG_HARDWARE
			printf("read return %d len = %d\n", ret, len);
#endif
		}
	}
	memcpy(&EEPROM[addr], inbuf, bytes);
#ifdef	DEBUG_HARDWARE
	printf("Write EEPROM:");
	for(i = addr; i < addr + bytes; i++)
	{
		printf("%02x", EEPROM[i]);
		if((i % 16) == 0) printf("\n");
	}
	printf("\n");
#endif
	if( fdEEprom > 0)
	{
		lseek(fdEEprom, addr, SEEK_SET);
		ret = write(fdEEprom, &EEPROM[addr], bytes);
	}
#ifdef	DEBUG_HARDWARE
	printf("Write Read EEPROM:");
	for(i = addr; i < addr + bytes; i++)
	{
		printf("%02x", EEPROM[i]);
		if((i % 16) == 0) printf("\n");
	}
	printf("\n");
#endif
	return 0;
}

UWORD watchdog_init(UBYTE type, UBYTE timeout)
{
unsigned char food = 1; 
// drivers/watchdog/nxp_wdt.c， 这个文件， 默认时间
//是 10 秒，如果想更改其它时间，通过修改该文件里面的
//CONFIG_NXP_WATCHDOG_DEFAULT_TIME 这个宏定义，或者编译内核时通过传递参
//数方式。
//3030强制系统重启
//echo 1 > /proc/sys/kernel/sysrq
//echo b > /proc/sysrq-trigger	
//强制关机
//echo 1 > /proc/sys/kernel/sysrq
//echo o > /proc/sysrq-trigger
//return 0;
	if(type == WATCHDOG_STOP)
	{
		if(flWatchdog > 0) 
		{
			write(flWatchdog, &food, 1);   
			close(flWatchdog);
		}
		return 0;
	}
	flWatchdog = open("/dev/watchdog", O_WRONLY);
	if(flWatchdog == -1) 
	{       
#ifdef	DEBUG_HARDWARE
		printf("\n!!! FAILED to open /dev/watchdog, errno: %s\n", strerror(errno));
#endif
		return 1;
	}  	
	return 0;
}

UWORD watchdog(void)
{
static unsigned char food = 0;       
	 
	if(flWatchdog > 0) 
	{       
		write(flWatchdog, &food, 1);       
	}
	
	return 0;
}

UBYTE reader_get_version(UBYTE *version, UBYTE *len)
{
char ver[100]={0};

	sprintf(ver, "HHJT.%s.3M", __DATE__);
	*len = strlen(ver);
	memcpy(version, ver, (*len));
	return 0;
}

void rled(UBYTE option)
{
	if( blnLEDRedUseFlag )
	{
		flLEDRed = open(redled, O_RDWR);
		if(  (flLEDRed) <= 0)
		{
			return ;
		}
		blnLEDRedUseFlag = 0;
	}
	
	
	if( flLEDRed > 0 )
	{
		if(option == LED_ON)
			write(flLEDRed, ledon, strlen(ledon) );
		else
			write(flLEDRed, ledoff, strlen(ledoff) );
	}
	return ;
}


void gled(UBYTE option)
{
	if( blnLEDGreenUseFlag )
	{
		flLEDGreen = open(greenled, O_RDWR);
		if( flLEDGreen <= 0 )
		{
			return ;
		}
		blnLEDGreenUseFlag = 0;
	}

	if( flLEDGreen > 0 )
	{
		if(option == LED_ON)
			write(flLEDGreen, ledon, strlen(ledon));
		else
			write(flLEDGreen, ledoff, strlen(ledoff));
	}
	return ;
}

void rgled(UBYTE option)
{
	if( blnLEDRedUseFlag )
	{
		flLEDRed = open(redled, O_RDWR);
		if( flLEDRed <= 0 )
		{
			return ;
		}
		blnLEDRedUseFlag = 0;
	}
	if( blnLEDGreenUseFlag )
	{
		flLEDGreen = open(greenled, O_RDWR);
		if( flLEDGreen <= 0 )
		{
			return ;
		}
		blnLEDGreenUseFlag = 0;
	}
	if( flLEDRed > 0)
	{
		if(option == LED_ON)
			write(flLEDRed, ledon, strlen(ledon));
		else
			write(flLEDRed, ledoff, strlen(ledoff));
	}
	
	if( flLEDGreen  > 0 )
	{
		if(option == LED_ON)
			write(flLEDGreen, ledon, strlen(ledon));
		else
			write(flLEDGreen, ledoff, strlen(ledoff));
	}

	return ;
}

UBYTE mifpro_ats(UBYTE in_cid,UBYTE *outbuf,UBYTE *outbytes)
{
UWORD	obytes;
UWORD	ret;

#ifdef DEBUG_CLOCK
clock_t start;
	//
	start = clock();
	printf("enter rats clock %d\n" ,start);
#endif	
	ret = _mifpro_ats(in_cid, outbuf, &obytes);
	
	*outbytes = (UBYTE)obytes;
	
	return (UBYTE)ret;
}

#ifdef	DEFINE_SUZHOU_XIAN
UBYTE mifpro_icmd(UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UBYTE *outbytes)
#else
UBYTE mifpro_icmd(UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UWORD *outbytes)
#endif
{
UBYTE	ret;
UWORD	obytes;
char	i;

	obytes = 0;
	i = ret = _mifpro_icmd(inbuf, inbytes, outbuf, &obytes);
	
//#ifdef	DEBUG_HARDWARE
	//extendsPrintf("icmd :", inbuf, inbytes);
	//extendsPrintf("  resv:", outbuf, obytes);
//#endif
	*outbytes = obytes;
	return ret;	
}

void extendsPrintf(unsigned char *msg, unsigned char *inbuf, int in_len)
{
int i;

	printf("%s : ", msg);
	for(i = 0; i < in_len; i++)
		printf("%02X", inbuf[i]);	
	printf("\n");
}

#endif