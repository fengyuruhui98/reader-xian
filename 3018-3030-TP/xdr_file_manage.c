#include <stdio.h>
#include <stdlib.h>

#include "xdr_file_manage.h"
#include "bin_file_manage.h"
#include "md5.h"
#include "time_tools.h"
#include "xa_error_code.h"

unsigned char xdrFileManage(unsigned char chcmd, unsigned char *cmd_buf, int cmd_len, unsigned char *out_buf, unsigned char *out_len)
{
unsigned char chEodnum, chFramelen;
unsigned char chret, filename[100], cache_file[150];
FILE	*fl;
unsigned long lngbyte4, i;
unsigned short shbyte2, mode_days;
char  *filebuf, filepath[200], chfile[40], chprefix[20];
int	ret;

	switch(chcmd)
	{
	case 0xC3:		//get the TP version
		LongToByte(tp_ver, &out_buf[0]);
		chret = ERR_OK;
		*out_len = 4;
		break;
	case 0xC7:		//download the EOD file and TP program.
		ByteToShort((short *)&shbyte2, &cmd_buf[3]);	//current frame number
		chFramelen = cmd_buf[5];
		if(shbyte2 == 1)		//first frame
		{
			eod_download.filelen = ByteToLong((long *)&lngbyte4, &cmd_buf[48]);
			memset(eod_download.filename, 0x00, 41);
			memcpy(eod_download.filename, &cmd_buf[8], 40);
			memcpy(eod_download.md5, &cmd_buf[52], 16);
			eod_download.curFrame = shbyte2;
			eod_download.curFilelen = 0;
			eod_download.filetype = cmd_buf[7];
			ByteToShort((short *)&eod_download.totalFrame, &cmd_buf[1]);
			if(eod_download.filetype == 0)
			{
				eod_download.filename[0] = toupper(eod_download.filename[0]);
				eod_download.filename[1] = toupper(eod_download.filename[1]);
				eod_download.filename[2] = toupper(eod_download.filename[2]);
				eod_download.filename[3] = toupper(eod_download.filename[3]);
				sprintf(eod_download.filepath, "./cache/%s", eod_download.filename);
			}else if(eod_download.filetype == 1)
			{
				sprintf(eod_download.filename, "xian");
				sprintf(eod_download.filepath, "./prognew/%s", eod_download.filename);
			}else if(eod_download.filetype == 2)
			{
				for(i = 0; i < 26; i++) eod_download.filename[i] = toupper(eod_download.filename[i]);
				sprintf(eod_download.filepath, "./cache/%s", eod_download.filename);
			}else
			{
				chret = 205;
				memcpy(&out_buf[0], &cmd_buf[3], 2);
				memcpy(&out_buf[2], &cmd_buf[1], 2);
				*out_len = 4;
				break;
			}
			//first check whether the file name in the cache direcotry or not or delete it
			memset(cache_file, 0x00, 150);
			memset(filename, 0x00, 100);
			memcpy(cache_file, eod_download.filename, 9);
			if(0 == FileisExist("./cache/", cache_file, filename))
			{
				sprintf(cache_file, "./cache/%s", filename);
				remove(cache_file);
			}
			fl = fopen(eod_download.filepath, "w+");
			fclose(fl);
			chret = 21;
			memcpy(&out_buf[0], &cmd_buf[3], 2);
			memcpy(&out_buf[2], &cmd_buf[1], 2);
			*out_len = 4;
#ifdef DEBUG_PRINT			
			PRINTK("first frame filelen:%d totalframe:%d curframe:%d filename:%s\n", eod_download.filelen, eod_download.totalFrame, shbyte2, eod_download.filename);
#endif
		}else if (shbyte2 != eod_download.totalFrame)
		{
			//first check the frame continuous
			if(shbyte2 != eod_download.curFrame + 1)
			{
				chret = 200;	//not continuous frame
				memcpy(&out_buf[0], &cmd_buf[3], 2);
				memcpy(&out_buf[2], &cmd_buf[1], 2);
				*out_len = 4;
				break;
			}
			eod_download.curFrame = shbyte2;
			//check the total frame number
			ByteToShort((short *)&shbyte2, &cmd_buf[1]);
			if(shbyte2 != eod_download.totalFrame)
			{
				chret = 201;	//not continuous frame
				memcpy(&out_buf[0], &cmd_buf[3], 2);
				memcpy(&out_buf[2], &cmd_buf[1], 2);
				*out_len = 4;
				break;
			}
			//record the file and it's length, 
			eod_download.curFilelen += (chFramelen - 1);
			//sprintf(filepath, "./prognew/%s", eod_download.filename);
			fl = fopen(eod_download.filepath, "a+");
				fseek(fl, 0, SEEK_END);
				fwrite(&cmd_buf[7], 1, chFramelen - 1 , fl);
			fclose(fl);
			chret = 21;
			memcpy(&out_buf[0], &cmd_buf[3], 2);
			memcpy(&out_buf[2], &cmd_buf[1], 2);
			*out_len = 4;
		}else
		{
			//first check the frame continuous
			if(shbyte2 != eod_download.curFrame + 1)
			{
				chret = 200;	//not continuous frame
				memcpy(&out_buf[0], &cmd_buf[3], 2);
				memcpy(&out_buf[2], &cmd_buf[1], 2);
				*out_len = 4;
				break;
			}
			//check the file length
			eod_download.curFilelen += (chFramelen - 1);
			if(eod_download.filelen != eod_download.curFilelen)
			{
				chret = 203;
				memcpy(&out_buf[0], &cmd_buf[3], 2);
				memcpy(&out_buf[2], &cmd_buf[1], 2);
				*out_len = 4;
				break;
			}
			//sprintf(filepath, "./prognew/%s", eod_download.filename);
			fl = fopen(eod_download.filepath, "a+");
				fseek(fl, 0, SEEK_END);
				fwrite(&cmd_buf[7], 1, chFramelen -1 , fl);
			fclose(fl);
			//check the md5
			fl = fopen(eod_download.filepath, "r+");
				fseek(fl, 0, SEEK_END);
				lngbyte4 = ftell(fl);
				fseek(fl, 0, SEEK_SET);
				filebuf = (char *)malloc(lngbyte4);
				fread(filebuf, 1, lngbyte4, fl);
			md5_str((unsigned char *)filebuf, lngbyte4, out_buf);
			free(filebuf);
			if(memcmp(eod_download.md5, out_buf, 16) != 0)
			{
				chret = 204;
				memcpy(&out_buf[0], &cmd_buf[3], 2);
				memcpy(&out_buf[2], &cmd_buf[1], 2);
				*out_len = 4;
				break;
			}
			//new file ok
			if(eod_download.filetype == SZ_DOWNLOAD_TP)
			{
				fseek(fl, 0, SEEK_END);
				fwrite(eod_download.md5, 1, 16, fl);
			}
			fclose(fl);
			memset(chprefix, 0x00, 20);
			memcpy(chprefix, eod_download.filename, 9);
			if(0 == FileisExist("./paranew/", chprefix, chfile))
			{
				sprintf(filepath, "./paranew/%s", chfile);
				remove(filepath);
			}
			sprintf(filepath, "./paranew/%s", eod_download.filename);
			if(eod_download.filetype != SZ_DOWNLOAD_TP)
				rename(eod_download.filepath, filepath); 
			chret = ERR_OK;
			memcpy(&out_buf[0], &cmd_buf[3], 2);
			memcpy(&out_buf[2], &cmd_buf[1], 2);
			*out_len = 4;
		}
		break;
	case 0xC8:
		chret = 204;
		*out_len = 0;
		if (0 != FileisExist("./prognew/", NULL, filename))
			break;
		sprintf(filepath, "./prognew/%s", filename);
		fl = fopen(filepath, "r+");
			fseek(fl, 0, SEEK_END);
			lngbyte4 = ftell(fl);
			if((lngbyte4 - 16) <= 0)
			{
				fclose(fl);
				return chret;
			}
			fseek(fl, 0, SEEK_SET);
			filebuf = (char *)malloc(lngbyte4);
			fread(filebuf, 1, lngbyte4, fl);
		fclose(fl);
		md5_str((unsigned char *)filebuf, lngbyte4 - 16, out_buf);
		if(memcmp(out_buf, &filebuf[lngbyte4 - 16], 16) == 0)
		{
			chret = ERR_OK;
			*out_len = 0;
			active_tp_file(filename, NULL);
		}
		free(filebuf);
		break;
	}
	return chret;
}
