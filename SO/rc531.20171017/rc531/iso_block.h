#ifndef _ISO_BLOCK_H_
#define _ISO_BLOCK_H_
//start of file

typedef void mifpro_icmd_func_type(void);    
extern mifpro_icmd_func_type *mifpro_icmd_func_call_back;

//Definitions made in this part of ISO/IEC 14443:
//RATS (11100000)b
//PPS (1101xxxx)b
//I-block (00xxxxxx)b (not (00xxx101)b)
//R-block (10xxxxxx)b (not (1001xxxx)b)
//S-block (11xxxxxx)b (not (1110xxxx)b and not (1101xxxx)b)

#define RATS_BLOCK  0xe0
#define PPS_BLOCK   0xd0

#define I_BLOCK          0x02
#define I_BLOCK_CHAIN    0x12
#define I_BLOCK_NO_CHAIN 0x02
#define I_BLOCK_MASK     0xf2

#define R_BLOCK      0xA2
#define R_BLOCK_ACK  0xa2
#define R_BLOCK_NAK  0xb2
#define R_BLOCK_MASK 0xe6
#define S_BLOCK      0xc2
#define S_BLOCK_MASK 0xc7

#define WTX_BLOCK_MASK 0xf7
#define WTX_BLOCK      0xf2

#define ATS_TIME_OUT         10   //10ms
#define DESELECT_TIME_OUT    5 

#define rc_is_in_receive()   ((rc_read_byte(REG_RC500_PRIMARY_STATUS)&0x70) == 0x70)

extern uint8_t bgCID;
extern uint8_t bgPCB;
extern uint8_t bgCIDFlag;
extern uint16_t wgFWT;
extern uint8_t bgSFGI;
extern uint8_t bgWTX;
//
uint16_t mifpro_ats(uint8_t cid,uint8_t *obuf,uint16_t *obytes);
uint16_t iso_block_get_fwt(uint8_t fwt);
uint16_t mifpro_deselect(uint8_t *outbuf);
uint16_t mifpro_wtx(uint8_t *outbuf);
uint16_t mifpro_noack(uint8_t *outbuf);
uint16_t mifpro_ack(uint8_t *outbuf);
void pcb_reverse(void);
uint16_t mifpro_icmd_chain(uint16_t len,uint8_t *inbuf,uint8_t *outbuf);
uint16_t mifpro_icmd_nochain(uint16_t len,uint8_t *inbuf,uint8_t *outbuf);
uint8_t mifpro_icmd(uint8_t *ibuf,uint16_t ibytes,uint8_t *obuf,uint16_t *obytes);
//
void  iso_block_set_time_out(uint8_t cnt_5ms);
uint16_t iso_block_transceve(uint8_t *inbuf,uint16_t inbytes,uint8_t *outbuf,uint16_t *outbytes,uint16_t timeout);

uint8_t mifpro_pps(uint8_t pps1,uint8_t *ppss);
void mifpro_set_speed(uint8_t tx_speed,uint8_t rx_speed);

//void mifpro_icmd_call_back_set(mifpro_icmd_func_type *p);

//end of file
#endif