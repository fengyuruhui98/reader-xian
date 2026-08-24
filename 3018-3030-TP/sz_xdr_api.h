#ifndef SZ_XDR_API_H
# define SZ_XDR_API_H

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef	__ANDROID__
#include <iconv.h>
#endif

typedef unsigned long xdr_ver32;

int code_convert(char *from_charset, char * to_charset, char * inbuf, int inlen, char *outbuf, int outlen);
unsigned char *ShortToByte(short sh_in, unsigned char *out_buf);
unsigned char *LongToByte(long lng_in, unsigned char *out_buf);
short ByteToShort(short *sh_in, unsigned char *out_buf);
long ByteToLong(long *lng_in, unsigned char *out_buf);
long toMoto(long lng_in);
long sz_round(long x);
int FileisExist(char *dirname, char *postfix, char *strRtn);

#endif
