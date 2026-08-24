#ifndef EEPROM_H
#define	EEPROM_H

unsigned char reader_status;

#define	XA_RW_STOP			0xFF
#define	XA_RW_INIT			0xFE
#define	XA_RW_IDLE			0x00
#define	XA_RW_SEARCH		0x01
#define	XA_RW_READ			0x02
#define	XA_RW_TRANSACTION	0x03
#define	XA_RW_RECORD		0x04

//eeprom 8192byte
#define	EE_RESTART			8190		//1byte
//ultralight back up information area
#define	EE_UL_BACKUP		6144		//64+9byte+100
//metro cpu back up information area
#define	EE_MCPU_BACKUP		6320		//?+9+100
//xian CPU back up informaiton area
#define	EE_CITY_BACKUP		6550		//+9+100
//xian transport back up information area
#define EE_TRANSPORT_BACKUP	6780

//0~1023 byte for UL transaction record
//0:upload the record flags:0,no; 1,ok
//1~2:record length
//3~1024:ul record
#define	EE_UL_TRANSACTION	0

//1024~2047 byte for metro-CPU transaction record
//0:upload the record flag :0,no; 1,ok
//1~2:record length
//3~1024 metro-cpu record
#define	EE_MCPU_TRANSACTION	1024

//2048~3071 byte for xian CPU transaction record
//0:upload the record flag :0,no; 1,ok
//1~2:record length
//3~1023 xian-cpu record
#define	EE_CITY_TRANSACTION	2048

//3072~4095 byte for xian CPU transaction record
//0:upload the record flag :0,no; 1,ok
//1~2:record length
//3~1023 xian-transport record
#define	EE_TRANSPORT_TRANSACTION	3072

struct YPT_txn
{
	unsigned short YPT_txnlen;
	unsigned char YPT_type;
	unsigned char YPT_flag;
	//由于共享内存，导致执行下一条命令时访问冲突，仅限于立即使用返回交易记录的情况
	unsigned char *pYPT_txn;	
	unsigned char YPT_txn[1024];
	unsigned char *pYPT_tac;
};
struct YPT_txn	tpYPT_txn_val;


#endif