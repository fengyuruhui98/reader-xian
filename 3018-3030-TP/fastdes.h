//fastdes.h

#ifndef _FASTDES_H_
#define _FASTDES_H_
//start of file

void des_encode(unsigned char *key, unsigned char *sr, unsigned char *dest);
void des_decode(unsigned char *key, unsigned char *sr, unsigned char *dest);
void des3_encode(unsigned char *key, unsigned char *sr, unsigned char *dest);
void des3_decode(unsigned char *key, unsigned char *sr, unsigned char *dest);

unsigned char WatchDiversity(unsigned char *pszMKKey,unsigned char *pszPID,unsigned char *pszSKKey,unsigned char bTriDes);
void CmdWatchCalMac(unsigned short nLenIn,unsigned char *pszBufIn,unsigned char *pszInitData,unsigned char *pszKey,unsigned char *pszMAC,unsigned char bTriDes);

//end of file
#endif


