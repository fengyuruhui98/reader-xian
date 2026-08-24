#include <stdlib.h>

#include "sz_xdr_api.h"
#include "hh_cpu_operation.h"

/*
function:
parameter:
	*dirname:directory name
	*postfix:the filename may a part of filename
	*strRtn:the full filename
return:
	2:can't open the directory
	1:can't find the filename in the direcotry
	0:ok,find file name and 
*/
int FileisExist(char *dirname, char *postfix, char *strRtn)
{
int           rtn;
struct dirent *dirp;
DIR           *dp;
  
  	rtn = 1;
  	/* -------- 打开错误数据目录 ---------- */
//  	PRINTK("dir the file exist %s, fix is %s\n", dirname, postfix);
	if((dp = opendir(dirname)) == NULL)
	{
		return 2;
	}
	/* -------- 读取错误数据目录 ---------- */
	while((dirp = readdir(dp)) != NULL)
	{
		if(strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..") )
		{
			if(postfix != NULL)
			{
				if(strstr(dirp->d_name, postfix) != NULL)
				{
					rtn = 0;
					strcpy(strRtn, dirp->d_name);
				}
				else
					continue;
			}
			else
			{
				rtn = 0;
				strcpy(strRtn, dirp->d_name);
			}
			closedir(dp);
			return rtn;
		}
	}
	/* -------- 关闭错误数据目录 ---------- */
	closedir(dp);
  	
  	return rtn;
}

/*
function:
parameter:
	*from_charset:from char set
	*to_charset:to char set
	*strRtn:input buffer from source
	*inlen: intput buffer length
	*outbuf:output buffer to destionation
	*outlen: max output buffer length
return:
	-1:can't open or malloc memory failure or convert failure
	0:ok,conver from source char set to destionation char set
*/
int code_convert(char *from_charset, char * to_charset, char * inbuf, int inlen, char *outbuf, int outlen)
{
#ifndef __ANDROID__ 
iconv_t cd;
char *to_inbuf = NULL;
int i , len_in, len_out;

	cd = iconv_open(to_charset, from_charset);
	if(cd == 0)
		return -1;
	memset(outbuf, 0x00, outlen);
	to_inbuf = (char *)malloc(sizeof(short) * inlen);
	if(to_inbuf == NULL)
	{
		iconv_close(cd);
		return -1;
	}
	memcpy(to_inbuf, inbuf, inlen * 2);
	if(-1 == iconv(cd, &to_inbuf, &inlen, &outbuf, &outlen))
	{
		PRINTK("\n%s\n", strerror(errno));
		iconv_close(cd);
		return -1;
	}

	iconv_close(cd);
#endif
	return 0;
}

/*
function:
parameter:
return:
*/
unsigned char *ShortToByte(short sh_in, unsigned char *out_buf)
{
union {
	short sh_union;
	unsigned char ch_union[2];
}sh_chUnion;

	sh_chUnion.sh_union = sh_in;
	out_buf[0] = sh_chUnion.ch_union[1];
	out_buf[1] = sh_chUnion.ch_union[0];

	return out_buf;
}

unsigned char *LongToByte(long lng_in, unsigned char *out_buf)
{
union {
	long lng_union;
	unsigned char ch_union[4];
}lng_chUnion;

	lng_chUnion.lng_union = lng_in;
	out_buf[0] = lng_chUnion.ch_union[3];
	out_buf[1] = lng_chUnion.ch_union[2];
	out_buf[2] = lng_chUnion.ch_union[1];
	out_buf[3] = lng_chUnion.ch_union[0];

	return out_buf;
}

/*
function:change the two bytes of big endian to short 
parameter:
return:the little endian short
*/
short ByteToShort(short *sh_in, unsigned char *out_buf)
{
union {
	short sh_union;
	unsigned char ch_union[2];
}sh_chUnion;

	sh_chUnion.ch_union[1] = out_buf[0];
	sh_chUnion.ch_union[0] = out_buf[1];
	if(sh_in != NULL)
		*sh_in = sh_chUnion.sh_union;

	return sh_chUnion.sh_union;
}

long ByteToLong(long *lng_in, unsigned char *out_buf)
{
union {
	long lng_union;
	unsigned char ch_union[4];
}lng_chUnion;

	lng_chUnion.ch_union[3] = out_buf[0];
	lng_chUnion.ch_union[2] = out_buf[1];
	lng_chUnion.ch_union[1] = out_buf[2];
	lng_chUnion.ch_union[0] = out_buf[3];
//	PRINTK("addr %08x\n", lng_in);
//	lng_in[0] = ch_union[0];
//	lng_in[1] = ch_union[1];
//	lng_in[2] = ch_union[2];
//	lng_in[3] = ch_union[3];
	if(lng_in != NULL)
		*lng_in = lng_chUnion.lng_union;
	return lng_chUnion.lng_union;
}

/*
function:realize the round function
*/
long sz_round(long x)
{
long lngintegral;
float y;

	y = x / 100.0;
	lngintegral = (long)y;
	y -= (float)lngintegral;
	y += 0.5;
	lngintegral += (long)y;
	
	lngintegral *= 100;
	return lngintegral;
}

/*
function:
*/
long toMoto(long lng_in)
{
unsigned char buf[4], temp;
long	lngtemp;

	lngtemp	= lng_in;
	memcpy(buf, &lngtemp, 4);
	temp = buf[0];
	buf[0] = buf[3];
	buf[3] = temp;
	temp = buf[1];
	buf[1] = buf[2];
	buf[2] = temp;

//#ifdef	DEBUG_PRINT
//	PRINTK("input %08x output %08x\n", lng_in, (*(long *)buf));
//#endif
	memcpy(&lngtemp, buf, 4);
	return lngtemp;
	
	//return (*(long *)buf);
}

void printInfo(unsigned char *inbuf, unsigned char len)
{
unsigned char i;

	PRINTK("=====\n");
	for(i = 0; i < len; i++)
		PRINTK("%02x ", inbuf[i]);
	PRINTK("\n");
}