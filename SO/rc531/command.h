#ifndef _COMMAND_H_
#define _COMMAND_H_
//start of file



/*---------------------------------------------------------------------------------------------
系统命令区
----------------------------------------------------------------------------------------------*/
#define CMD_REPORT_VER   0x00      //系统版本报告
//#define CMD_SET_STATUS   0x0e      //设置设备状态 
//#define CMD_RESET        0x0b      //系统热启动
//#define CMD_GET_NODE     0x01      //取设备的节点号
//#define CMD_SET_NODE     0x02

/*---------------------------------------------------------------------------------------------
EEPROM操作
-----------------------------------------------------------------------------------------------*/
#define CMD_RD_EE        0x03
#define CMD_WR_EE        0X04

/*---------------------------------------------------------------------------------------------
FLASH操作
----------------------------------------------------------------------------------------------*/
//#define CMD_RD_FLASH           0x05
//#define CMD_WR_FLASH           0x06
//#define CMD_ERASE_FLASH        0x07 
//#define CMD_WR_FLASH_BUF       0x1c
//#define CMD_FLASH_BUF_TO_MAIN  0x1d
//#define  CMD_FLASH2BUF 	       0x1E
//#define  CMD_READ_FLASHBUF     0x1F

/*---------------------------------------------------------------------------------------------
record操作
----------------------------------------------------------------------------------------------*/
//#define CMD_REC_GET_FORWARD    0x08
//#define CMD_REC_CLR_N          0x09
//#define CMD_REC_GET_BACKWARD   0x12
//#define CMD_REC_GET_NUM        0x16


/*-----------------------------------------------------------------------------------------------
实时时钟操作
------------------------------------------------------------------------------------------------*/
#define CMD_WR_TIME            0x14
#define CMD_RD_TIME            0x13 

/*---------------------------------------------------------------------------------------------
m1卡操作(0x20-0x2f)
-----------------------------------------------------------------------------------------------*/
#define CMD_MIF_START_CMD    0x20
#define CMD_MIF_END_CMD      0x2c
#define CMD_REQUEST          0x22
#define CMD_ANTICOLL         0x23
#define CMD_SELECT           0x24
#define CMD_LOAD_KEY         0x20
#define CMD_AUTHENTICATION   0x21
#define CMD_RD_BLOCK         0x25 
#define CMD_WR_BLOCK         0x26   
#define CMD_INCREASE         0x28 
#define CMD_DECREASE         0x29
#define CMD_RESTORE          0x2b 
#define CMD_TRANSFER         0x2c
#define CMD_HALT             0x27 
#define CMD_PWR_OFF          0x2a

/*-----------------------------------------------------------------------------------------------
CPU卡操作(ETU=372)
-------------------------------------------------------------------------------------------------*/
//#define CMD_CPU_ATR   0x30          //ATR 
//#define CMD_CPU_T0    0x31          //T0命令

/*-----------------------------------------------------------------------------------------------
按键操作
-------------------------------------------------------------------------------------------------*/
//#define CMD_KEY_CLASS   0x42
//#define CMD_KEY_CLR     0x01

//#define VOICE_CLASS     0x47

/*-----------------------------------------------------------------------------------------------
DES
-------------------------------------------------------------------------------------------------*/
//#define CMD_DES_ENCODE  0x70
//#define CMD_DES_DECODE  0x71



/*-----------------------------------------------------------------------------------------
Mifare pro类
-------------------------------------------------------------------------------------------*/
#define CMD_MIFPRO_CLASS          0x7c
#define CMD_MIFPRO_ATS            0x00 
#define CMD_MIFPRO_DESELECT       0x01
#define CMD_MIFPRO_ICMD_NOCHAIN   0x02
#define CMD_MIFPRO_ICMD_CHAIN     0x03
#define CMD_MIFPRO_WTX            0x04
#define CMD_MIFPRO_RBLOCK         0x05
#define CMD_CRC_A                 0x06
#define CMD_MIFPRO_ICMD           0x07
#define CMD_MIFPRO_NOACK          0x08
#define CMD_MIFPRO_PPS            0x09
#define CMD_MIFPRO_SET_SPEED      0x0a

/*-----------------------------------------------------------------------------------------
DL CLASS
-------------------------------------------------------------------------------------------*/
//#define CMD_PSD_DL_CLASS       0x8b
//#define CMD_PSD32_READ       0x00
//#define CMD_PSD32_WRITE      0x01
//#define CMD_PSD32_ERASE      0x02
//#define CMD_PSD256_READ        0x03
//#define CMD_PSD256_WRITE       0x04
//#define CMD_PSD256_ERASE       0x05
//#define CMD_32_TO_256          0x06
//#define CMD_256_TO_32          0x07
//#define CMD_DL_IS_IN_32K       0x08
//#define CMD_DL_WR_FLAG         0x09

/*------------------------------------------------------------------------------------
南京公交命令集
--------------------------------------------------------------------------------------*/
#define CMD_NJAFC_CLASS      0x91
//#define CMD_M400_INIT        0x01
//#define CMD_M400_REQ         0x02
//#define CMD_M400_CMD         0x03
//#define CMD_M400_POLL        0x04
#define CMD_SET_CRYPT        0x07
//#define CMD_DISP_STRING          0x1c        //显示字符串 
//#define CMD_DISP_STRING_ENLARGE  0x1d
//#define CMD_DISP_IMAGE           0x1e         
//#define CMD_LCD_CLR              0x1f        //清屏
//#define CMD_SET_HZ_MODE          0x20  



#define ISO14443A_M1_TYPE    0        
#define ISO14443A_SH_TYPE    1
#define ISO14443B_M4_TYPE    2
#define ISO15693_ICODE1_TYPE 3

/*------------------------------------------------------------------------------------
ISO15693 命令集
--------------------------------------------------------------------------------------*/
//#define CMD_ISO15693_SET    0x00
//#define CMD_ISO15693_CMD    0x01


/*---------------------------------------------------------------------------------------------
DEBUG操作: FF FF
----------------------------------------------------------------------------------------------*/
#define DEBUG_CLASS                 0xff
//#define DEBUG_GET_SYSTIME           0x00
//#define DEBUG_GET_LAST_EXEC_TIME    0x01
#define DEBUG_DELAY                 0x02
#define DEBUG_RC_RD_BYTE            0x03
#define DEBUG_RC_WR_BYTE            0x04
//#define DEBUG_RD_PORT               0x05
//#define DEBUG_WR_PORT               0x06
//#define DEBUG_CHG_BAUD              0x07
//#define DEBUG_SAM                   0x08
//#define DEBUG_LCD_CLASS             0x09
//#define DEBUG_ADC                   0x0a
//#define DEBUG_PRN_CLASS             0x0b
//#define DEBUG_GPRS_POWER            0x0c
//#define DEBUG_GPRS_PUT              0x0d
//#define DEBUG_FM_HS                   0x0e
//#define DEBUG_FM_BLOCK                0x0f
#define DEBUG_MIFPRO_ICMD_CALL_BACK 0x10
#define DEBUG_TEMP                  0xff

extern uint8_t bpgSamResetFlag;

//函数------------------------------------------------------------------------------------------
void dev0_cmd_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outuf,uint8_t *outbytes);
void cmd_debug_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outuf,uint8_t *outbytes);
void cmd_mifare_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outbuf,uint8_t *outbytes);
void cmd_njafc_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outbuf,uint8_t *outbytes);
void cmd_mifpro_process(uint8_t *inbuf,uint8_t inbytes,uint8_t *outbuf,uint8_t *outbytes);

//end of file
#endif