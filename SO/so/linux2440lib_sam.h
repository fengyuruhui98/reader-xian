//linux2440lib_sam.h

#ifndef _LINUX2440LIB_SAM_H_
#define _LINUX2440LIB_SAM_H_
//start of file


//º¯Êý
UBYTE sam_select(UBYTE index);
UBYTE sam_atr(UBYTE channel,UBYTE *outbuf,UBYTE *outbytes);
int sam_pts(int channel,int ta1);
UBYTE sam_apdu(UBYTE channel, UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UBYTE *outbytes,UWORD timeout, UBYTE expectlen);
UBYTE sam_apdu_ext(UBYTE channel, UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UBYTE *outbytes,UWORD timeout, UBYTE expectlen);
//
UBYTE linux_sam_select(UBYTE sam_index);
UBYTE linux_sam_set_speed(UBYTE sam_index,UBYTE speed);
UBYTE linux_sam_atr(UBYTE *outbuf,UBYTE *outbytes);
UBYTE linux_sam_apdu(UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UBYTE *outbytes);
int linux_sam_pps(UBYTE index,UBYTE ta1,UBYTE *obuf,UBYTE *obytes);

//end of file
#endif

