#ifndef _GLOBAL_C_
#define _GLOBAL_C_
//start of file

#include "global.h"

//#define	DEBUG_HARDWARE

unsigned char bgLibInUseFlag = 0;
//RF
unsigned char blnRF_00_INIT = 0;
unsigned char blnRF_01_INIT = 0;
int  igLibFd;         
//EEPROM file
unsigned char EEPROM[8192];
unsigned char bgEEInUseFlag = 0xff;
FILE	*flEEprom;
//EEPROM system 
static const char *eeprom = "/sys/bus/i2c/devices/2-0050/eeprom";
int		fdEEprom;

//LED
int	flLEDRed = 0;
int flLEDGreen = 0;
unsigned char blnLEDRedUseFlag = 0xff, blnLEDGreenUseFlag = 0xff;
static const char *redled = "/sys/bus/platform/devices/leds-gpio/leds/red/brightness";
static const char *greenled = "/sys/bus/platform/devices/leds-gpio/leds/green/brightness";
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

	bgLibInUseFlag = 0;

	return 0;
}


UBYTE rf_reset(void)
{
	//用于再次重启射频芯片2018/12/12 16:13:43
	return 0;
}

void set_card_type(UBYTE card_type)
{
UBYTE	ret;

	
	return ;
}

UBYTE UL_Anticoll_Select(UBYTE * psnr)
{
UBYTE	ret;
UBYTE cardsnr1[5],cardsnr2[5], buf[5];

	
	memcpy(psnr, &cardsnr1[1], 3);
	memcpy(&psnr[3], cardsnr2, 4);
	
	return 0;
}

UBYTE UL_Page_Read(UBYTE addr, UBYTE *pReaddata)
{
UBYTE	oBuf[20], ret;

	memcpy(pReaddata, oBuf, 16);
	
	return ret;
}

UBYTE UL_Page_Write(UBYTE addr, UBYTE *pWritedata)
{
UBYTE	iBuf[20], ret;

	memset(iBuf, 0x00, sizeof(iBuf));
	memcpy(iBuf, pWritedata, 4);

	//return mcml_write_4bytes(addr, iBuf);
	return 0;
}

UBYTE mcml_request2(UBYTE request_type,UBYTE * atq)
{
UBYTE	req[2];

	if( (!blnRF_00_INIT ) && (bgRfNow == 0) )
	{
		blnRF_00_INIT = 0xff;
	}
	if( (!blnRF_01_INIT ) && (bgRfNow == 1) )
	{
		blnRF_01_INIT = 0xff;
	}
	//return mcml_request(request_type, atq);
		memcpy(atq, "\x08\x00", 2);
	if( (random() % 10) < 9)
		return 0xff;
	return 0;
}

UBYTE mcml_anticoll(UBYTE *snr)
{
UBYTE ret;

	
	memcpy(snr, "\x34\x1a\xe5\x19\\xd2", 5);
	return 0;
}


UBYTE mcml_select(UBYTE *snr,UBYTE *status)
{
UBYTE ret,buf[2];	

	//ret = rc_select(PICC_ANTICOLL1,snr,buf);
	*status=0x20;
	return 0;
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
// drivers/watchdog/nxp_wdt.c， 这个文件， 默认时间
//是 10 秒，如果想更改其它时间，通过修改该文件里面的
//CONFIG_NXP_WATCHDOG_DEFAULT_TIME 这个宏定义，或者编译内核时通过传递参
//数方式。
	//flWatchdog = open("/dev/watchdog", O_WRONLY);
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
	//ret = _mifpro_ats(in_cid, outbuf, &obytes);
	memcpy(outbuf, "\x10\x77\x33\xa0\x02\x86\x88\xff\x34\x1a\xe5\x19\x6e\x62\x73\x6d", 16);
	*outbytes = (UBYTE)16;
	
	return (UBYTE)0;
}




UBYTE mifpro_icmd(UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UWORD *outbytes)
{
UBYTE	ret;
UWORD	obytes;
char	i;

	obytes = 0;
	if(memcmp(inbuf, "\x00\xa4", 2) == 0)
		ret = cmd_00A4(inbuf, outbuf, outbytes);
	if(memcmp(inbuf, "\x00\xb2", 2) == 0)
		ret =  cmd_00B2(inbuf, outbuf, outbytes);
	if(memcmp(inbuf, "\x80\xa8", 2) == 0)
		ret =  cmd_80A8(inbuf, outbuf, outbytes);
	if(memcmp(inbuf, "\x80\xb4", 2) == 0)
		ret =  cmd_80B4(inbuf, outbuf, outbytes);
	if(memcmp(inbuf, "\x80\xca", 2) == 0)
		ret =  cmd_80CA(inbuf, outbuf, outbytes);
	if(memcmp(inbuf, "\x84\xde", 2) == 0)
		ret =  cmd_84DE(inbuf, outbuf, outbytes);
//	if(memcmp(inbuf, "", 2) == 0)
//		return cmd_(inbuf, outbuf, outbytes);
//	if(memcmp(inbuf, "", 2) == 0)
//		return cmd_(inbuf, outbuf, outbytes);

	//extendsPrintf("icmd :", inbuf, inbytes);
	//extendsPrintf("  resv:", outbuf, *outbytes);
		
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


UBYTE mcml_load_key(UBYTE keyset,UBYTE keyab, UBYTE sectno,UBYTE *buf)
{
	return 0;
}


UBYTE mcml_authentication(UBYTE keyset,UBYTE keyab,UBYTE sectno)
{
	return 0;
}


UBYTE mcml_read(UBYTE block,UBYTE *outbuf)
{
	return 0;
}

UBYTE mcml_write(UBYTE block,UBYTE *outbuf)
{
	return 0;
}

UBYTE mcml_decrement(UBYTE block, UDWORD value)
{
	return 0 ;
}

UBYTE mcml_increment(UBYTE block, UDWORD value)
{
	return 0;
}

UBYTE mcml_restore(UBYTE addr)
{
	return 0;
}

UBYTE mcml_transfer(UBYTE addr)
{
	return 0;
}

UBYTE mcml_halt(void)
{
	return 0;	
}

UBYTE sam_select(UBYTE index)
{
	return 0;
}

void sam_set(UBYTE index,UBYTE etu,UBYTE wait_etu)
{
	return ;
}


UBYTE sam_atr(UBYTE channel,UBYTE *outbuf,UBYTE *outbytes)
{
	*outbytes = 16;
	return  0;
}

int sam_pts(int channel,int ta1)
{
	return 0;
}

UBYTE sam_apdu(UBYTE channel, UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UBYTE *outbytes,UWORD timeout, UBYTE expectlen)
{
	memset(outbuf, 0x00, 50);
	memcpy(&outbuf[50], "\x90\x00", 2);
	*outbytes = 52;
	return 0;
}


void mcml_pwr_on(void)
{
	return ;
}

void mcml_pwr_off(void)
{
	return;
}


void rf_select(int channel)
{
	return ;
}

UBYTE mifpro_pps(UBYTE pps1,UBYTE *ppss)
{
	return 0;
}


UBYTE mifpro_set_speed(UBYTE tx_speed,UBYTE rx_speed)
{
	return 0;
}
#endif