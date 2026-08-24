#ifndef _MIFARE_H_
#define _MIFARE_H_
//start of file

#define 	KEYA		0
#define 	KEYB		0x40

uint8_t mcml_request(uint8_t req_code,uint8_t *atq);
uint8_t mcml_anticoll(uint8_t *snr);
uint8_t mcml_anticoll2(uint8_t *snr);
uint8_t mcml_select(uint8_t *snr,uint8_t *status);
uint8_t mcml_select2(uint8_t *snr,uint8_t *status);
uint8_t mcml_load_key(uint8_t keyset,uint8_t keyab, uint8_t sectno,uint8_t *buf);
uint8_t mcml_authentication(uint8_t keyset,uint8_t keyab,uint8_t sectno);
uint8_t mcml_read(uint8_t block,uint8_t *outbuf);
uint8_t mcml_write(uint8_t block,uint8_t *outbuf);
uint8_t mcml_increment(uint8_t addr, uint32_t value);
uint8_t mcml_decrement(uint8_t addr,uint32_t value);
uint8_t mcml_transfer(uint8_t addr);
uint8_t mcml_restore(uint8_t addr);
void mcml_pwr_off(void);
uint8_t mcml_halt(void);

uint8_t mcml_authentication2(uint8_t keyset,uint8_t keyab,uint8_t sectno);

uint8_t mcml_read_4bytes(uint8_t block,uint8_t *outbuf);
uint8_t mcml_write_4bytes(uint8_t block,uint8_t *outbuf);

//#define token_mcml_read(p1,p2)       mcml_read_4bytes(p1,p2)
//#define token_mcml_write(p1,p2)      mcml_write_4bytes(p1,p2)

//end of file
#endif