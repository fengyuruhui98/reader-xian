#ifndef _RC_H
#define _RC_H
#ifndef uchar
#define uchar unsigned char
#endif
#ifndef uint
#define uint unsigned int
#endif
 

//常量定义
#define FALSE 0
#define TRUE 1
 
//spi通信端口宏
#define RC531_IRQ  0
#define RC531_PD   1
#define RC531_CS  2
#define RC531_MOSI  3
#define RC531_MISO  4
#define RC531_CLK  5

#define RC531_CS_LOW()    {PORTB&=~(1<<RC531_CS);}
#define RC531_CS_HIGN()   {PORTB|=(1<<RC531_CS);}
#define RC531_CLK_LOW()  {PORTB&=~(1<<RC531_CLK);}
#define RC531_CLK_HIGN() {PORTB|=(1<<RC531_CLK);}
#define RC531_data()  {PINB&(1<<RC531_MISO);}
#define RC531_UP()  {PORTB|=(1<<RC531_PD);}
#define RC531_DOWN()  {PORTB&=~(1<<RC531_PD);}
 

//卡的命令
#define PICC_REQSTD      0x26
#define PICC_REQALL     0x52
#define PICC_HALT     0x50
 
#define PICC_ANTICOLL1  0x93
#define PICC_ANTICOLL2   0x95
#define PICC_ANTICOLL3 0x97
#define  PICC_AUTHENT1B   0x61
#define PICC_AUTHENT1A 0x60
#define PICC_READ    0x30
#define PICC_WRITE      0xA0

//RF531 set of commands
#define StartUp  0x3f
#define CmdIdle     0x00
#define Transmit  0x1a
#define Receive     0x16
#define CmdTransceive  0x1e
#define WritE2      0x01
#define ReadE2      0x03
#define CmdLoadKeyE2 0x0b
#define CmdLoadKey  0x19
#define CmdAuthent1 0x0c
#define CmdAuthent2 0x14
#define LoadConfig 0x07
#define CalcCRC  0x12

#define RF_TimeOut 0xfff  //发送命令延时时间
#define Req       0x01  
#define Sel       0x02

//RF531状态宏
#define FIFO_FULL ((PrimaryStatus&0x02)==1?,1：0）
#define FIFO_EMPTY ((PrimaryStatus&0x01)==1?,1:0)
#define PICC_DECREMENT  0xC0    //;decrement value
#define PICC_INCREMENT  0xC1    //;increment value
#define PICC_RESTORE    0xC2    //;restore command code
#define  PICC_TRANSFER   0xB0   // ;transfer command code
#define  PICC_HALT       0x50   // ;halt

//RF531  set of Registers addresses
#define RegPage  0x00
#define RegCommand   0x01
#define RegFIFOData   0x02
#define RegPrimaryStatus 0x03
#define SecondaryStatus 0x05
#define Int_Req      0x07
#define SPILevel  0x08
#define RegFIFOLevel  0x29
#define RegFIFOLength  0x04
 
#define RegControl      0x09
#define RegErrorFlag    0x0a
#define CollPos   0x0b
#define TimeValue  0x0c
#define RegCRCResuletLSB 0x0d
#define CRCResuletMSB 0x0e
#define RegBitFraming  0x0f
#define RegCRCPresetLSB 0x23 
#define RegCwConductance   0x12
#define RegModConductance 0x13
#define RegRxWait   0x21
#define MFOUTSelect     0x26
#define RegChannelRedundancy 0x22
#define RegTimerClock  0x2a
#define RegTimerControl 0x2b
#define RegTimerReload  0x2c
#define RegInterruptEn  0x06
#define RegTxControl  0x11
#define TypeSH   0x31
#define RegCoderControl   0x14
#define TypeBFraming 0x17
#define RegDecoderControl 0x1a
#define RegClockQControl 0x1f
#define RegBitPhase 0x1B
#define RegRxThreshhold 0x1c
#define RegRxControl1  0x19    
#define RegModWidth  0x15
#define RegBPSKDemControl  0x1d
#define RegIRqPinConfig 0x2D
#define RegCWConductance 0x12
#define RegRxControl2  0x1e
#define RegCRCPresetMSB     0x24
//卡片类型定义
#define TYPEA_MODE 0      //TYPEA模式
#define TYPEB_MODE 1      //TYPEB模式
#define SHANGHAI_MODE 2   //上海模式
//射频卡通信命令码定义
#define RF_CMD_REQUEST_STD 0x26
#define RF_CMD_REQUEST_ALL 0x52
#define RF_CMD_ANTICOL    0x93
#define RF_CMD_SELECT    0x93
#define RF_CMD_AUTH_LA    0x60
#define RF_CMD_AUTH_LB    0x61
#define RF_CMD_READ     0x30
#define RF_CMD_WRITE    0xa0
#define RF_CMD_INC     0xc1
#define RF_CMD_DEC     0xc0
#define RF_CMD_RESTORE    0xc2
#define RF_CMD_TRANSFER    0xb0
#define RF_CMD_HALT     0x50
//Status Value
#define  ALL   0x01
#define  KEYA  0x04
#define  KEYB  0x00
#define  _AB   0x40
#define  CRC_A 1
#define  CRC_B 2
#define  CEC_OK  0
#define  CRC_ERR 1
#define  BCC_OK   0
#define  BCC_ERR  1
 
//
#define MIFARE_8K  0 //MIFARE系列8K卡片
#define MIFARE_TOKEN 1 //MIFARE系列1KTOKEN卡片
#define SHANGHAI_8K  2 //上海标准系列8K卡片
#define SHANGHAI_TOKEN 3 // 上海标准系列1KTOKEN卡片
 
//
#define RC_531_OK     0     //正确
#define RC_531_NOTAGERR   1     //无卡
#define RC_531_CRCERR   2     //卡片CRC校验错误
#define RC_531_EMPTY   3      //数值溢出错误
#define RC_531_AUTHERR   4     //验证不成功
#define RC_531_PARITYERR   5     //卡片奇偶校验错误
#define RC_531_CODEERR   6     //通信错误（BCC）校验错误
#define RC_531_SERNRERR   8     //卡片序列号错误（anti-collison错误）
#define RC_531_SELECTERR  9     //卡片数据长度字节错（SELECT错误）
#define RC_531_NOTAUTHERR  10     //卡片没有通过验证
#define RC_531_BITCOUNTERR   11    //从卡片接受到的位数错误
#define RC_531_BYTECOUNTERR    12    //从卡片接受到的字节错误（仅读函数有效）
#define RC_531_RESTERR   13   //调用restore函数错误
#define RC_531_TRANSERR   14   //调用transfer函数错误
#define RC_531_WRITEERR   15   //调用write函数错误
#define RC_531_INCRERR   16   //调用increment函数错误
#define RC_531_DECRERR   17   //调用decrement函数错误
#define RC_531_READERR   18   //调用resad函数错误
#define RC_531_LOADKEYERR  19   //调用LOADKEY函数错误
#define RC_531_FRAMINGERR  20     //RC_531桢错误
#define RC_531_REQERR   21   //调用req函数错误
#define RC_531_SELERR   22   //调用sel函数错误
#define RC_531_ANTICOLLERR  23   //调用anticoll函数错误
#define RC_531_INTIVALERR  24   //调用初始化函数错误
#define RC_531_READVALERR  25   //调用高级读块值函数错误
#define RC_531_DESELECTERR  26   
#define RC_531_CMD_ERR   42   //命令错误
 
//RF531 set of hanshu
extern void init_port(void);
extern uchar init_RF531(uchar);
extern void  SPI_send(uchar temp);
extern uchar SPI_receive();
extern void SPI_write_byte(uchar address,uchar content);
extern uchar SPI_read_byte(uchar address);
extern void rf_reset();
extern void rf_reg_init();
extern void typeA_init(void);
extern void uart_proc();
extern void RequestA();  //;TYPE A卡卡请求RequestA命令
extern void  Anticollision();  //;TYPE A卡防冲突Anticollision命令
extern void Select();     //;TYPE A卡选择卡Select命令
extern void CheckEEPROM();   //;TYPE A卡验证EEPROM密码命令
extern void  CheckUser();    //;TYPE A卡验证User密码命令
extern void  ReadTypeA();  //;TYPE A卡读卡命令
extern void  WriteTypeA();   //;TYPE A卡写卡命令
extern void  DecTypeA(); //;TYPE A卡钱包减命令
extern void  IncTypeA();   //;TYPE A卡钱包加命令
extern void  TranTypeA();   //;TYPE A卡Transfer命令
extern void  HaltTypeA(); //;TYPE A卡Halt命令
extern void  RestroeTypeA();  //;TYPE A卡Restore命令
extern void  ReadRC531();   // ;读读卡芯片寄存器
extern void  WriteRC531();   // ;写读卡芯片寄存器
extern void  ToTypeA();  // ;打开射频信号并设成TYPE A方式
  //case 0x0f: ToTypeB();break;   //;打开射频信号并设成TYPE B方式
extern void  Close();   //;关掉射频信号

#endif
