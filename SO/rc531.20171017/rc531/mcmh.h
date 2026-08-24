#ifndef _MCMH_H_
#define _MCMH_H_
//start of file

#define MCMH_MAX_TRY    3
#define MCMH_MAX_SECTOR 64

extern uint8_t gKeyA[MCMH_MAX_SECTOR][6],gKeyB[MCMH_MAX_SECTOR][6];

uint8_t mcmh_get_cardsnr(uint8_t *cardsnr);
uint8_t mcmh_read(uint8_t block, uint8_t *outbuf,uint8_t op_type,uint8_t key_type);
uint8_t mcmh_read_simple(uint8_t block, uint8_t *outbuf,uint8_t op_type,uint8_t key_type);
uint8_t mcmh_write(uint8_t block, uint8_t *inbuf,uint8_t op_type,uint8_t key_type);
uint8_t mcmh_transfer(uint8_t block, uint8_t op_type,uint8_t key_type);
uint8_t mcmh_restore(uint8_t block, uint8_t op_type,uint8_t key_type);
uint8_t mcmh_increment(uint8_t block, uint32_t value,uint8_t op_type,uint8_t key_type);
uint8_t mcmh_decrement(uint8_t block, uint32_t value,uint8_t op_type,uint8_t key_type);
uint8_t mcmh_set_key(uint8_t sector,uint8_t key_type,uint8_t *inbuf);
uint8_t mcmh_get_key(uint8_t sector,uint8_t key_type,uint8_t *outbuf);

//end of file
#endif
