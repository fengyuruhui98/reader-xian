#include <stdio.h>
#include "failure.h"

short ByteToShort(short *sh_in, unsigned char *out_buf)
{
union {
	short sh_union;
	unsigned char ch_union[2];
}sh_chUnion;

	sh_chUnion.ch_union[1] = out_buf[0];
	sh_chUnion.ch_union[0] = out_buf[1];
	*sh_in = sh_chUnion.sh_union;

	return *sh_in;
}

unsigned char xdrFileManage(unsigned char chcmd, unsigned char *cmd_buf, int cmd_len, unsigned char *out_buf, unsigned char *out_len)
{
FILE	*fl;
unsigned long lngbyte4, i;
unsigned short shbyte2, mode_days;
char  *filebuf, filepath[200], chfile[40], chprefix[20];
int	ret;

	switch(chcmd)
	{
	case 0x06:
		ByteToShort((short *)&shbyte2, &cmd_buf[7]);
		tpStationWaiverMode.waivermode_len = shbyte2;
		printf("%02x%02x lenght is %02x\n", cmd_buf[7], cmd_buf[8], shbyte2);
		if(tpStationWaiverMode.waivermode_val != NULL)
		{
			free(tpStationWaiverMode.waivermode_val);
			tpStationWaiverMode.waivermode_val = NULL;
		}
		if(tpStationWaiverMode.waivermode_len != 0)
			tpStationWaiverMode.waivermode_val = malloc((long)tpStationWaiverMode.waivermode_len * 3);
		if((tpStationWaiverMode.waivermode_val == NULL) && (tpStationWaiverMode.waivermode_len != 0))
		{
			*out_len = 0;
		}else
		{
			if(tpStationWaiverMode.waivermode_len != 0)
				memcpy(tpStationWaiverMode.waivermode_val, &cmd_buf[9], (long)tpStationWaiverMode.waivermode_len * 3);
			for(i=0; i <tpStationWaiverMode.waivermode_len; i++ )
			{
				printf("%02x%02x %02x \n", tpStationWaiverMode.waivermode_val[i*3+0], tpStationWaiverMode.waivermode_val[i*3+1], tpStationWaiverMode.waivermode_val[i*3+2]);
			}
			fl = fopen("./para/waivermode", "w+");
				fwrite(&tpStationWaiverMode.waivermode_len, 1, 2, fl);
				fwrite(tpStationWaiverMode.waivermode_val, 1, (long)tpStationWaiverMode.waivermode_len * 3, fl);
			fclose(fl); 
			*out_len = 0;
		}
	}
}

void get_degrade_mode(unsigned char *src_station)
{
unsigned int	i;
char filename[100], filepath[200];
unsigned long lngbyte4;
unsigned char	curstation[2];

	memset(&tpwaivermode.cur_sta_failure, 0x00, 10);
	for(i = 0; i < tpStationWaiverMode.waivermode_len; i++)
	{
		switch(tpStationWaiverMode.waivermode_val[i * 3 + 2])
		{
		case SZ_WAIVER_FAILURE:
			if(memcmp(src_station, &tpStationWaiverMode.waivermode_val[i * 3], 2) == 0)
				if(!tpwaivermode.cur_sta_failure) tpwaivermode.cur_sta_failure = 0xff;
			else
				if(!tpwaivermode.oth_sta_failure) tpwaivermode.oth_sta_failure = 0xff;
			break;
		case SZ_WAIVER_ENTRY:
			if(memcmp(src_station, &tpStationWaiverMode.waivermode_val[i * 3], 2) == 0)
				if(!tpwaivermode.cur_sta_entry) tpwaivermode.cur_sta_entry = 0xff;
			else
			{
				if(!tpwaivermode.oth_sta_entry) tpwaivermode.oth_sta_entry = 0xff;
				tpwaivermode.oth_entry_num += 1;
			}
			break;
		case SZ_WAIVER_DATE:
			printf("cur date %02x other date %02x\n",tpwaivermode.cur_sta_date, tpwaivermode.oth_sta_date);
			if(memcmp(src_station, &tpStationWaiverMode.waivermode_val[i * 3], 2) == 0)
			{
				if(!tpwaivermode.cur_sta_date) tpwaivermode.cur_sta_date = 0xff;
			}else
			{
				if(!tpwaivermode.oth_sta_date) {tpwaivermode.oth_sta_date = 0xff;}
			}
			printf("cur date %02x other date %02x\n",tpwaivermode.cur_sta_date, tpwaivermode.oth_sta_date);
			break;
		case SZ_WAIVER_FARE:
			if(memcmp(src_station, &tpStationWaiverMode.waivermode_val[i * 3], 2) == 0)
				if(!tpwaivermode.cur_sta_fare) tpwaivermode.cur_sta_fare = 0xff;
			else
				if(!tpwaivermode.oth_sta_fare) tpwaivermode.oth_sta_fare = 0xff;
			break;
		case SZ_WAIVER_EMERGENCY:
			if(memcmp(src_station, &tpStationWaiverMode.waivermode_val[i * 3], 2) == 0)
				if(!tpwaivermode.cur_sta_emergency) tpwaivermode.cur_sta_emergency = 0xff;
			else
				if(!tpwaivermode.oth_sta_emergency) tpwaivermode.oth_sta_emergency = 0xff;
			break;
		default:
			break;
		}
	}
	//mode list
/*	ByteToShort(&curstation, src_station);
	for(i = 0; i < EodWaiverDateMasterConfig.StationModeInfo.StationModeInfo_len; i++)
	{
		switch(EodWaiverDateInfo[i].ModeCode)
		{
		case SZ_WAIVER_FAILURE:
			if(EodWaiverDateInfo[i].StationID == curstation)
				tpwaivermode.cur_sta_failure = 0xff;
			break;
		case SZ_WAIVER_FARE:
			if(EodWaiverDateInfo[i].StationID == curstation)
				tpwaivermode.cur_sta_fare = 0xff;
			break;
		}
	}*/
	//printf("cur date %02x other date %02x\n",tpwaivermode.cur_sta_date, tpwaivermode.oth_sta_date);
	return ;
}


int main()
{
unsigned char chtemp[1000], out_buf[200];
unsigned char	src_station[2];
	
	memcpy(chtemp, "\x03\x00\x01\x00\x01\x0F\x06\x00\x04\x01\x41\x01\x01\x51\x03\x01\x52\x02\x01\x53\x05\xEE\x9B\x04", 24);
	xdrFileManage(chtemp[6], chtemp, 24, out_buf, out_buf);
	//file_waivermode();
	memcpy(src_station, "\x1\x40", 2);
	get_degrade_mode(src_station);
	return 0;
}
