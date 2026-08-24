//rc_op.h

#ifndef _RC_OP_H_
#define _RC_OP_H_
//start of file
#include "rc_base.h"


//#define REQ_TIME_OUT    3      //1ms
//#define SEL_TIME_OUT    14     //2ms
//#define AUTH_TIME_OUT   20     //3ms
//#define HALT_TIME_OUT   7      //1ms
//#define READ_TIME_OUT   20     //3ms
//#define WRITE1_TIME_OUT 7      //1ms
//#define WRITE2_TIME_OUT 100    //15ms
//#define VALUE1_TIME_OUT 14     //1ms
//#define VALUE2_TIME_OUT 20     //3ms
//#define VALUE3_TIME_OUT 100    //15ms
//
//#define ANTICOLL_TIME_OUT    67     //10ms


//预定义------------------------------------------------------------------------
#define REQ_TIME_OUT    40 //7      //3-->1ms,20-->3ms,2012/8/22 17:34:55
#define SEL_TIME_OUT    14     //2ms
#define AUTH_TIME_OUT   20     //3ms
#define HALT_TIME_OUT   7      //1ms
#define READ_TIME_OUT   20     //3ms
#define WRITE1_TIME_OUT 7      //1ms
#define WRITE2_TIME_OUT 100    //15ms
#define VALUE1_TIME_OUT 14     //1ms
#define VALUE2_TIME_OUT 40     //3ms
#define VALUE3_TIME_OUT 100    //15ms

#define ANTICOLL_TIME_OUT    67     //10ms


#define NORMAL_IRQ_MASK   (BIT_RXI|BIT_TIMERI)
#define NORMAL_ERR_MASK   (BIT_RXI|BIT_TIMERI)
#define AUTH_IRQ_MASK     (BIT_IDLEI|BIT_TIMERI|BIT_ERRI)


#define m4_init()    rc_iso14443_typeb_init()
#define m1_init()    rc_iso14443_typea_init()

extern uint8_t bgIsoType;   //0:type a 1:type b


#define ISO14443A_M1_TYPE    0        
#define ISO14443A_SH_TYPE    1
#define ISO14443B_M4_TYPE    2
#define ISO15693_ICODE1_TYPE 3

//变量---------------------------------------------------------------------------
void rc_init(void);
void rc_iso14443_typeb_init(void);
void rc_iso14443_typea_init(void);
void rc_wait_irq(void);
//uint8_t rc_rece_bits(void);
uint16_t rc_rece_bits(void);
uint8_t rc_send_cmd(uint8_t *inbuf,uint8_t inbytes,uint8_t irq_mask);
uint8_t rc_request(uint8_t req_code,uint8_t *atq);
uint8_t rc_anticoll(uint8_t sel_code,uint8_t bcnt,uint8_t *snr); 
uint8_t rc_select(uint8_t sel_code,uint8_t *snr,uint8_t *sak);
uint8_t rc_read(uint8_t block,uint8_t *outbuf);
uint8_t rc_read_4bytes(uint8_t block,uint8_t *outbuf);
uint8_t rc_write(uint8_t block,uint8_t *inbuf);
uint8_t rc_write_4bytes(uint8_t block,uint8_t *inbuf);
uint16_t rc_crc_a(uint8_t *inbuf,uint8_t inbytes);
uint8_t rc_auth(uint8_t keyAB,uint8_t sector);
uint8_t rc_halta(void);
uint8_t rc_select_op_type(uint8_t op_type);
uint8_t rc_auth2(uint8_t keyAB,uint8_t sector);
uint8_t rc_value_op0(uint8_t op_mode,uint8_t sr_block,uint8_t *value);
uint8_t rc_transfer(uint8_t dest_block);

//2013/8/27 9:33:29
void rc_set_speed(uint8_t tx_speed,uint8_t rx_speed);
uint8_t rc_pps(uint8_t cid,uint8_t pps1,uint8_t *ppss);

//end of file
#endif