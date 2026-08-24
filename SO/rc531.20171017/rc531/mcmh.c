#ifndef _MCMH_C_
#define _MCMH_C_
//start of file
#include "global.h"




uint8_t gKeyA[MCMH_MAX_SECTOR][6],gKeyB[MCMH_MAX_SECTOR][6];


/*=======================================================================================
函数：
功能：
=========================================================================================*/
uint8_t mcmh_get_cardsnr(uint8_t *cardsnr)
{
uint8_t buf[5],buf2[2];
uint8_t i;
uint8_t ret;

for(i=0;i<3;i++){
  if(mcml_request(PICC_REQSTD,buf) != 0) continue;
  if(mcml_anticoll(buf) != 0) continue;
  if(mcml_select(buf,buf2) != 0) continue;
  memcpy(cardsnr,buf,5);
  memcpy(gThisCardSnr,buf,5);
  ret = 0;
  goto label_exit;
  }
  
ret = (uint8_t)-1;	

label_exit:
return ret;	
}	


/*========================================================================================
函数：mcmh_read
功能：
==========================================================================================*/
uint8_t mcmh_read(uint8_t block, uint8_t *outbuf,uint8_t op_type,uint8_t key_type)
{
uint8_t cnt;
uint8_t buf[2];
//uint8_t ret;

cnt = 0;
if(op_type == 2) goto label_op;

label_loop:
cnt++;
if(cnt > MCMH_MAX_TRY) return (uint8_t)-1;
//request	
if(mcml_request(PICC_REQSTD,buf) == 0) goto label_sel;
goto label_loop;
//select
label_sel:
if(mcml_select(gThisCardSnr,buf) != 0) goto label_loop;
//load key	
if(key_type==0){
    if(mcml_load_key(0,KEYA,block/4,gKeyA[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYA,block/4)) goto label_loop;	
    }
else{
    if(mcml_load_key(0,KEYB,block/4,gKeyB[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYB,block/4)) goto label_loop;	
	  }    

label_op:
if(mcml_read(block,outbuf) != 0) goto label_loop;
return 0;	

}


uint8_t mcmh_read_simple(uint8_t block, uint8_t *outbuf,uint8_t op_type,uint8_t key_type)
{
uint8_t cnt;
uint8_t buf[2];
//uint8_t ret;

cnt = 0;
if(op_type == 2) goto label_op;

label_loop:
cnt++;
if(cnt > MCMH_MAX_TRY) return (uint8_t)-1;
//request	
if(mcml_request(PICC_REQSTD,buf) == 0) goto label_sel;
goto label_loop;
//select
label_sel:
if(mcml_select(gThisCardSnr,buf) != 0) goto label_loop;
//load key	
if(key_type==0){
    if(mcml_load_key(0,KEYA,block/4,gKeyA[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYA,block/4)) goto label_loop;	
    }
else{
    if(mcml_load_key(0,KEYB,block/4,gKeyB[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYB,block/4)) goto label_loop;	
	  }    

label_op:
if(mcml_read(block,outbuf) != 0) return (uint8_t)-1;
return 0;	

}


/*========================================================================================
函数：mcmh_write
功能：
==========================================================================================*/
uint8_t mcmh_write(uint8_t block, uint8_t *inbuf,uint8_t op_type,uint8_t key_type)
{
uint8_t cnt;
uint8_t buf[2];

cnt = 0;
if(op_type == 2) goto label_op;

label_loop:
cnt++;
if(cnt > MCMH_MAX_TRY) return (uint8_t)-1;
//request	
if(mcml_request(PICC_REQSTD,buf) == 0) goto label_sel;
goto label_loop;
//select
label_sel:
if(mcml_select(gThisCardSnr,buf) != 0) goto label_loop;
//load key	
if(key_type==0){
    if(mcml_load_key(0,KEYA,block/4,gKeyA[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYA,block/4)) goto label_loop;	
    }
else{
    if(mcml_load_key(0,KEYB,block/4,gKeyB[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYB,block/4)) goto label_loop;	
	  }    

label_op:
if(mcml_write(block,inbuf) != 0) goto label_loop;
return 0;	
}


/*========================================================================================
函数：
功能：
==========================================================================================*/
uint8_t mcmh_decrement(uint8_t block, uint32_t value,uint8_t op_type,uint8_t key_type)
{
uint8_t cnt;
uint8_t buf[2];

cnt = 0;
if(op_type == 2) goto label_op;

label_loop:
cnt++;
if(cnt > MCMH_MAX_TRY) return (uint8_t)-1;
//request	
if(mcml_request(PICC_REQSTD,buf) == 0) goto label_sel;
goto label_loop;
//select
label_sel:
if(mcml_select(gThisCardSnr,buf) != 0) goto label_loop;
//load key	
if(key_type==0){
    if(mcml_load_key(0,KEYA,block/4,gKeyA[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYA,block/4)) goto label_loop;	
    }
else{
    if(mcml_load_key(0,KEYB,block/4,gKeyB[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYB,block/4)) goto label_loop;	
	  }    

label_op:
if(mcml_decrement(block,value) != 0) goto label_loop;
return 0;	
}


/*========================================================================================
函数：mcmh_increment
功能：
==========================================================================================*/
uint8_t mcmh_increment(uint8_t block, uint32_t value,uint8_t op_type,uint8_t key_type)
{
uint8_t cnt;
uint8_t buf[2];

cnt = 0;
if(op_type == 2) goto label_op;

label_loop:
cnt++;
if(cnt > MCMH_MAX_TRY) return (uint8_t)-1;
//request	
if(mcml_request(PICC_REQSTD,buf) == 0) goto label_sel;
goto label_loop;
//select
label_sel:
if(mcml_select(gThisCardSnr,buf) != 0) goto label_loop;
//load key	
if(key_type==0){
    if(mcml_load_key(0,KEYA,block/4,gKeyA[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYA,block/4)) goto label_loop;	
    }
else{
    if(mcml_load_key(0,KEYB,block/4,gKeyB[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYB,block/4)) goto label_loop;	
	  }    

label_op:
if(mcml_increment(block,value) != 0) goto label_loop;
return 0;	
}


/*========================================================================================
函数：mcmh_restore
功能：
==========================================================================================*/
uint8_t mcmh_restore(uint8_t block, uint8_t op_type,uint8_t key_type)
{
uint8_t cnt;
uint8_t buf[2];

cnt = 0;
if(op_type == 2) goto label_op;

label_loop:
cnt++;
if(cnt > MCMH_MAX_TRY) return (uint8_t)-1;
//request	
if(mcml_request(PICC_REQSTD,buf) == 0) goto label_sel;
goto label_loop;
//select
label_sel:
if(mcml_select(gThisCardSnr,buf) != 0) goto label_loop;
//load key	
if(key_type==0){
    if(mcml_load_key(0,KEYA,block/4,gKeyA[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYA,block/4)) goto label_loop;	
    }
else{
    if(mcml_load_key(0,KEYB,block/4,gKeyB[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYB,block/4)) goto label_loop;	
	  }    

label_op:
if(mcml_restore(block) != 0) goto label_loop;
return 0;	
}


/*========================================================================================
函数：mcmh_transfer
功能：
==========================================================================================*/
uint8_t mcmh_transfer(uint8_t block, uint8_t op_type,uint8_t key_type)
{
uint8_t cnt;
uint8_t buf[2];

cnt = 0;
if(op_type == 2) goto label_op;

label_loop:
cnt++;
if(cnt > MCMH_MAX_TRY) return (uint8_t)-1;
//request	
if(mcml_request(PICC_REQSTD,buf) == 0) goto label_sel;
goto label_loop;
//select
label_sel:
if(mcml_select(gThisCardSnr,buf) != 0) goto label_loop;
//load key	
if(key_type==0){
    if(mcml_load_key(0,KEYA,block/4,gKeyA[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYA,block/4)) goto label_loop;	
    }
else{
    if(mcml_load_key(0,KEYB,block/4,gKeyB[block/4])) goto label_loop;
    if(mcml_authentication(0,KEYB,block/4)) goto label_loop;	
	  }    

label_op:
//if(mcml_transfer(block) != 0) return (uint8_t)-1;
if(mcml_transfer(block) != 0) goto label_loop;
return 0;	
}

/*========================================================================================
函数：mcmh_set_key
功能：
==========================================================================================*/
uint8_t mcmh_set_key(uint8_t sector,uint8_t key_type,uint8_t *inbuf)
{
if(sector >= MCMH_MAX_SECTOR) return (uint8_t)-1;
if(key_type == 0) memcpy(gKeyA[sector],inbuf,6);
else memcpy(gKeyB[sector],inbuf,6);
return 0;				
}	

/*========================================================================================
函数：mcmh_get_key
功能：
==========================================================================================*/
uint8_t mcmh_get_key(uint8_t sector,uint8_t key_type,uint8_t *outbuf)
{
if(sector >= MCMH_MAX_SECTOR) return (uint8_t)-1;
if(key_type == 0) memcpy(outbuf,gKeyA[sector],6);
else memcpy(outbuf,gKeyB[sector],6);
return 0;				
}




//end of file
#endif