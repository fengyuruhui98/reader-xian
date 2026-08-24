//linux2440libtest.c
//test suite for s3c2440 hardware lib under Linux 2.6 Kernel
//±‡÷∆£∫xux
// ±º‰£∫2010-10-21 8:57:12
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linux2440lib.h"
#include "xa_error_code.h"

char reader_test_eeprom(void)
{
int ret;
int i, j;
char inbuf[1000], outbuf[1000];
int eepromsize, eepromstep;

	eepromsize = 2048;
	eepromstep = 32;
#ifdef DEBUG_PRINT
	printf("\nEEPROM≤‚ ‘ø™ º");
#endif
	for(j = 0; j < (eepromsize / eepromstep); j++)
	{
		printf(".");
		memset(outbuf, 0, sizeof(outbuf));
		ret = ee_read(j * eepromstep, eepromstep, outbuf);
		if (ret)
		{
#ifdef DEBUG_PRINT
			printf("\n                                                Err:∂¡EEPROM ß∞‹,add=%x!", j * eepromstep);
#endif
			return 0xff;
  		}
 
		for(i = 0; i < eepromstep; i++)  inbuf[i] = (i % 256);
		ret = ee_write(j * eepromstep, eepromstep, inbuf);
		if (ret)
		{
#ifdef DEBUG_PRINT
			printf("\n                                                Err:–¥EEPROM ß∞‹,add=%x!", j * eepromstep);
#endif
  			return 0xff;
  		}
  
		memset(outbuf, 0, sizeof(outbuf));
		ret = ee_read(j * eepromstep, eepromstep, outbuf);
		if(ret)
		{
#ifdef DEBUG_PRINT
			printf("\n                                                Err:∂¡EEPROM ß∞‹,add=%x!", j * eepromstep);
#endif
  			return 0xff;
  		}

		if (memcmp(inbuf, outbuf, eepromstep) != 0)  
			printf("\n          Err:EEPROM≤‚ ‘ ß∞‹!");
	}

#ifdef DEBUG_PRINT
	printf("\nOK:EEPROM≤‚ ‘≥…π¶!");
#endif
	return ERR_OK;	
}	

char reader_test_rtc(char write_control, unsigned char *time_bcd, unsigned char *out_len)
{
int ret;
char inbuf[100];

	if(write_control)
	{
		memcpy(inbuf, &time_bcd[1], 6);
		ret = rtc_wr_time(inbuf);
		if (ret)
		{
#ifdef DEBUG_PRINT
			printf("\n                                                Err:–¥ ±÷” ß∞‹!");
#endif
	  		return  0xff;
	  	}
#ifdef DEBUG_PRINT
		printf("\nOK:–¥ ±÷”≥…π¶ %02x%02x%02x %02x:%02x:%02x.",inbuf[0],inbuf[1],inbuf[2],inbuf[3],inbuf[4],inbuf[5]);
#endif
	}

	memset(inbuf,0,sizeof(inbuf));
	ret = rtc_rd_time(inbuf);
	if (ret)
	{
#ifdef DEBUG_PRINT
		printf("\n                                                Err:∂¡ ±÷” ß∞‹!");
#endif
  		return 0xff;
	}
#ifdef DEBUG_PRINT
	printf("\nOK:∂¡ ±÷”≥…π¶ %02x-%02x-%02x %02x:%02x:%02x.",inbuf[0],inbuf[1],inbuf[2],inbuf[3],inbuf[4],inbuf[5]);
#endif
	if(write_control)
	{
		if(memcmp(inbuf, &time_bcd[1], 5) == 0)
			return ERR_OK;	
		else
			return 0xff;
	}
	*out_len = 7;
	time_bcd[0] = 0x20;
	memcpy(&time_bcd[1], inbuf, 6);
	return ERR_OK;
}

char reader_test_littlesam(char channel, unsigned char *out_buf, unsigned char *out_len)
{
int ret;
int i;
int samindex;
char outbuf[100];
unsigned char outbytes;

	*out_len = 1;
	samindex = channel;
	sam_set(samindex, SAM_ETU_93, 4);

	ret = sam_select(samindex);
	if (ret)
	{
#ifdef DEBUG_PRINT
		printf("\n    Err:—°‘ÒSAM%02dø®◊˘ ß∞‹!", samindex);
#endif
  		return 0xff;
  	}

	ret = sam_atr(samindex, outbuf, &outbytes);
	if(ret)
	{
#ifdef DEBUG_PRINT
		printf("\n                Err:SAM%02d∏¥Œª ß∞‹!",samindex);
#endif
		out_buf[0] = 0x10;
  		return 0xff;
  	}
  
#ifdef DEBUG_PRINT
	printf("\nSAM%02d return :", samindex);  
	for(i = 0; i < outbytes; i++)  printf("%02x ", (unsigned char)outbuf[i]); 
#endif
	if (outbuf[0] == 0x3b) 
	{
#ifdef DEBUG_PRINT
		printf("\nOK:SAM%02d≤‚ ‘≥…π¶!", samindex);
#endif
  	}else
  	{
#ifdef DEBUG_PRINT
  		printf("\n            Err:SAM%02d≤‚ ‘ ß∞‹!", samindex);
#endif
  		out_buf[0] = 0x11;
		return 0xff;
	}
	*out_len = 0;
	return ERR_OK;
}	

char reader_test_antenna(char channel, unsigned char *out_buf, unsigned char *out_len)
{
int ret;
int i;
int try_times;
char inbuf[100], outbuf[100], chret, atslen;

#ifdef DEBUG_PRINT
	printf("\nantenna test-->");
	if(channel)  printf("channel 1.");
	else printf("channel 0.");
#endif
	*out_len = 1;
	rf_select(channel);
	mcml_pwr_on();
	try_times = 2;
	set_timeout(5000);
	for(i = 0; i < try_times; i++)
	{
		//request
		ret = mcml_request2(PICC_REQALL, outbuf);
		if(ret)
		{
#ifdef DEBUG_PRINT
			printf("      Err: request! %d", i);
#endif
			out_buf[0] = 0x01;
			set_timeout(3000);
  			continue;//return;
  		}
	
#ifdef DEBUG_PRINT
		printf("request");
#endif
		//according to the request result
		if((outbuf[0] == 0x44) && (outbuf[1] == 0x00))
		{//single ticket card
			ret = mcml_anticoll(outbuf);
			if (ret)
			{
#ifdef DEBUG_PRINT
				printf("       Err:anticoll!");
#endif
	  			out_buf[0] = 0x02;
	  			continue; //return;
	  		}

#ifdef DEBUG_PRINT
			printf(" anticoll");
#endif
	  		memcpy(inbuf, outbuf, 5);
			ret = mcml_select(inbuf, outbuf);
			if (ret)
			{
#ifdef DEBUG_PRINT
				printf("      Err:select!");
#endif
	  			out_buf[0] = 0x03;
	  			continue; //return;
	  		}
#ifdef DEBUG_PRINT
			printf(" select");
#endif
		}else if((outbuf[0] == 0x04) && (outbuf[1] == 0x00))
		{//M1
			ret = mcml_anticoll(outbuf);
			if (ret)
			{
#ifdef DEBUG_PRINT
				printf("       Err:anticoll!");
#endif
	  			out_buf[0] = 0x02;
	  			continue; //return;
	  		}

#ifdef DEBUG_PRINT
			printf(" anticoll");
#endif
	  		memcpy(inbuf, outbuf, 5);
			ret = mcml_select(inbuf, outbuf);
			if (ret)
			{
#ifdef DEBUG_PRINT
				printf("      Err:select!");
#endif
	  			out_buf[0] = 0x03;
	  			continue; //return;
	  		}
#ifdef DEBUG_PRINT
			printf(" select");
#endif
		}else
		{//cpu
			ret = mcml_anticoll(outbuf);
			if (ret)
			{
#ifdef DEBUG_PRINT
				printf("       Err:anticoll!");
#endif
	  			out_buf[0] = 0x02;
	  			continue; //return;
	  		}

#ifdef DEBUG_PRINT
			printf(" anticoll");
#endif
	  		memcpy(inbuf, outbuf, 5);
			ret = mcml_select(inbuf, outbuf);
			if (ret)
			{
#ifdef DEBUG_PRINT
				printf("      Err:select!");
#endif
	  			out_buf[0] = 0x03;
	  			continue; //return;
	  		}
#ifdef DEBUG_PRINT
			printf(" select");
#endif
			//ats
			ret = mifpro_ats(0, inbuf, &atslen);
			if(ret != 0)
			{
#ifdef DEBUG_PRINT
				printf("      Err:ats!");
#endif
	 			continue;
			}
#ifdef DEBUG_PRINT
			printf(" ats");
#endif
			//select 3f 00
			memcpy(inbuf, "\x00\xa4\x00\x00\x02\x3f\x00", 7);
			ret = mifpro_icmd(inbuf, 7, outbuf, &atslen);
			if(ret != 0)
			{
#ifdef	DEBUG_PRINT
				printf("     Err:select 3f 00\n");
#endif
				continue;
			}
#ifdef DEBUG_PRINT
			printf(" select 3f00\n");
#endif
		}
  		break;
 	} 
	if(i >= try_times)
	{
		mcml_pwr_off();
		return 0xff;
	}
	
#ifdef DEBUG_PRINT
	printf("-- ok");
#endif
	out_buf[0] = 0;
	return ERR_OK;
}	

int reader_hard_test(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len)
{

	switch(cmd_buf[7])
	{
	case 1:		//antenna
		return reader_test_antenna(cmd_buf[8], out_buf, out_len);
		break;
	case 2:		//psam
		return reader_test_littlesam(cmd_buf[8], out_buf, out_len);
		break;
	case 3:		//eeprom
		return reader_test_eeprom();
		break;
	case 4:		//rtc
		return reader_test_rtc(1, &cmd_buf[8], out_len);
		break;
	case 5:		//LED
		if(cmd_buf[8] == 0)//red
		{
			if(cmd_buf[9] == 1)
				rled(LED_ON);
			else
				rled(LED_OFF);
		}
		else
		{
			if(cmd_buf[9] == 1)
				gled(LED_ON);
			else
				gled(LED_OFF);
		}
		break;
	default:
		*out_len = 0;
		return ERR_NOPARAMETER;
	}
}