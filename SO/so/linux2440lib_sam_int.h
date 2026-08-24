//sam_int.h

#ifndef _SAM_INT_H_
#define _SAM_INT_H_
//start of file

extern int bpgApduExpectLen;

//预定义-------------------------------------------------------------------------------
#define MAX_SAM_INDEX   8
#define SAM_ETU_31        (256-(32/2))    //115200,32
#define SAM_ETU_62        (256-(62/2))    //57600,62
#define SAM_ETU_93        (256-(93/2))    //38400
#define SAM_ETU_372       (256-(372/2))   //9600

// 
//邓建华  17:23:26
//SAM 脚：FF FF pin option 
//邓建华  17:23:37
#define SAM_PIN_COMSEL  0
#define SAM_PIN_PWRCTL  1
#define SAM_PIN_RST0    2
#define SAM_PIN_RST1    3 


//函数---------------------------------------------------------------------------------
void sam_set(UBYTE index,UBYTE etu,UBYTE wait_etu);
int  sam_atr0(UBYTE index,UBYTE *outbuf,UBYTE *outbytes);
int  sam_apdu0(UBYTE index,UBYTE *inbuf,UBYTE inbytes,UBYTE *outbuf,UBYTE *outbytes);
int  sam_pps0(UBYTE index,UBYTE ta1,UBYTE *obuf,UBYTE *obytes);
void mcu1_select(void);
void mcu2_select(void);
void mcu1_reset(void);
void mcu2_reset(void);
void mcu_powerctrl_set(void);
void mcu_powerctrl_clr(void);
int  sam_get_ver(UBYTE index,UBYTE *obuf,UBYTE *obytes);

int  sam_apdu0_ext(UBYTE index,UBYTE expectlen,UBYTE *ibuf,UBYTE ibytes,UBYTE *obuf,UBYTE *obytes);

#ifdef _TEST_SAM_
int  sam_apdu_send(UBYTE index,UBYTE *ibuf,UBYTE ibytes);
int  sam_apdu_receive(UBYTE *obuf,UBYTE *obytes);
#endif

//2014/1/14 11:01:46
int  sam_apdu1(UBYTE index,UBYTE expectlen, UBYTE *ibuf,UBYTE ibytes,UBYTE *obuf,UBYTE *obytes);
long samDeleteDLE(unsigned char *pbytData, unsigned char *lrc, int intLength);



//end of file
#endif


