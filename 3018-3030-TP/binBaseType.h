#ifndef BIN_BASETYPE_H
#define BIN_BASETYPE_H

//SJT & Metro CPU
#define	TX_SJTSale 						0x20
#define	TX_SJTAddValue					0x22
#define	TX_SJTSaleexit					0x37
#define	TX_SJTOverfare					0x33
#define	TX_SJTRefund					0x30
#define	TX_SJTEntry						0x2A
#define	TX_SJTExit						0x24
#define	TX_MetroImmediateRefund			0x40
//SJT & Metro CPU & City Card
#define	TX_SJTOvertime					0x32
#define	TX_SJTNoExit					0x36
#define	TX_SJTNoEntry					0x31
//Metro CPU & City Card
#define TX_Defer						0x34
#define	TX_Lock							0x35
	
#define	TX_CPUSale						0x21
#define	TX_CPUAddValue					0x23
#define	TX_CPUEntry						0x2B
#define	TX_CPUExit						0x25
#define TX_CPUDeduct					0x2F
//rfu
#define	TX_CPUOverstayingSurcharge		0x25
#define	TX_CPUUnderFareSurcharge		0x26
#define	TX_CPUNoExitSurcharge			0x27
#define	TX_CPUNoEntrySurcharge			0x28
#define	TX_CPUClearEntrySurcharge		0x29
#define	TX_CPUBlock						0x2A
#define	TX_CPUUnblock					0x2B
#define	TX_CPUDeduction					0x2C
#define	TX_CPUImmediateRefund			0x2E
#define	TX_CPUNonImmediateRefundRequest	0x2F
#define	TX_CPUMemberRegister			0x31
#define	TX_CPUClearWallet				0x32
#define	TX_CPUReject					0x33

#define	TX_YKTCardEntry					0x90
#define	TX_YKTCardExit					0x91
#define	TX_YKTCardOverstayingSurcharge	0x92
#define	TX_YKTCardUnderFareSurcharge	0x93
#define	TX_YKTCardNoExitSurcharge		0x94
#define	TX_YKTCardNoEntrySurcharge		0x95
#define	TX_YKTCardBlock					0x96


#endif 
