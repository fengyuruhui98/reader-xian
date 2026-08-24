#include "xa_ul_operation.h"

//#include "xdrBaseType.h"
//#include "xdrEOD.h"
#include "xdr_file_manage.h"
#include "bin_file_manage.h"
#include "time_tools.h"
#include "xa_error_code.h"
#include "xa_sam.h"
#include "serial.h"
#include "hh_cpu_operation.h"
#include "xa_operation.h"
#include "eeprom.h"
#include "linux2440lib.h"

//;#define DEBUG_PRINT 1
static unsigned char ul_data[16][4], ul_data_bak[16][4];
extern unsigned char mac_ret;

unsigned char TicketOTP;
unsigned char bgUpdatableFlag;
/*
*/
void sz_ul_ee_write()
{
unsigned short addr;
	
	addr = EE_UL_BACKUP;
	ee_write(addr, 1, &ch_sz_sjt_rollback);
	addr += 1;
	ee_write(addr, 8, ch_ul_phyical_id_bak);
	addr += 8;
	ee_write(addr, 64, ul_data[0]);
	addr += 64;

	ee_write(addr, sizeof(tpSJT), tpSJT.curtime);
	addr += sizeof(tpSJT);
}
/*
*/
void sz_ul_ee_read()
{
unsigned short addr;
	
	addr = EE_UL_BACKUP;
	ee_read(addr, 1, &ch_sz_sjt_rollback);
	addr += 1;
	ee_read(addr, 8, ch_ul_phyical_id_bak);
	addr += 8;
	ee_read(addr, 64, ul_data_bak[0]);
	addr += 64;

	ee_read(addr, sizeof(tpSJT), tpSJT.curtime);
	addr += sizeof(tpSJT);

}

char standard_tocken_read(unsigned char block, unsigned char *out_buf)
{

	return 0;
}
char standard_tocken_write(unsigned char blockno, unsigned char *in_buf)
{
char i;

	for(i = 0; i < 2; i++)
	{
		if(0 != UL_Page_Write(blockno, in_buf))
			continue;
		break;
	}
#ifdef DEBUG_2_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, "\xf0\xf0", 2);
#endif	
	
	return 0;
}

char read_tocken_otp_info(unsigned char *ul_page)
{
unsigned char buf[20], i;

	if(UL_Page_Read(0, buf) != 0)
	{
		return CE_READ;
	}
	memcpy(ul_data[0], buf, 16);
#ifdef DEBUG_2_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, "\xf0\xf0", 2);
#endif	
#ifdef DEBUG_PRINT
	PRINTK("CSN %02x%02x%02x %02x%02x%02x%02x PhyicalID %02x%02x%02x%02x\n", ul_data[0][0], ul_data[0][1], ul_data[0][2], ul_data[1][0], ul_data[1][1], ul_data[1][2], ul_data[1][3],
					ul_data[3][0], ul_data[3][1], ul_data[3][2], ul_data[3][3]);
#endif
	//memcpy(&ch_ul_phyical_id[1], ul_data[0], 3);
	//memcpy(&ch_ul_phyical_id[4], ul_data[1], 4);
	//for xi'an not read page4~7 in here
	if(UL_Page_Read(4, buf) != 0)
	{
		return CE_READ;
	}
	memcpy(ul_data[4], buf, 16);
	
	memcpy(xaStaticZone.buff, ul_data[4], 12);
	tpSZ.Version = xaStaticZone._StaticZone.Version;
	tpSZ.lifecycleCount = (xaStaticZone._StaticZone.lifecycleCount_1 << 4) + xaStaticZone._StaticZone.lifecycleCount;
	tpSZ.keySetNumber = xaStaticZone._StaticZone.keySetNumber;
	tpSZ.cardBaseDataTime = (xaStaticZone._StaticZone.cardBaseDataTime_1 << 8) + xaStaticZone._StaticZone.cardBaseDataTime;
	tpSZ.cardBatchNumber = (xaStaticZone._StaticZone.cardBatchNumber_1 << 8) + (xaStaticZone._StaticZone.cardBatchNumber_2 << 2) + xaStaticZone._StaticZone.cardBatchNumber;
	tpSZ.testMode = xaStaticZone._StaticZone.testMode;
	tpSZ.productType = (xaStaticZone._StaticZone.productType_1 << 1) + xaStaticZone._StaticZone.productType;
	tpSZ.passengerType = xaStaticZone._StaticZone.passengerType;
	tpSZ.productID = xaStaticZone._StaticZone.productID;
	tpSZ.purchaseValue = (xaStaticZone._StaticZone.purchaseValue_1 << 15) + (xaStaticZone._StaticZone.purchaseValue_2 << 7) + xaStaticZone._StaticZone.purchaseValue;
	tpSZ.DurationType = xaStaticZone._StaticZone.DurationType;
	tpSZ.validityDuration = (xaStaticZone._StaticZone.validityDuration_1 << 3) + xaStaticZone._StaticZone.validityDuration;
	tpSZ.lavPaymentMethod = xaStaticZone._StaticZone.lavPaymentMethod;
	tpSZ.productIssuerId = xaStaticZone._StaticZone.productIssuerId;

	UL_TellSysCard(tpSZ.productType, NULL);
	
	return 0;
}


char read_tocken_info(void)
{
int i, j;
unsigned char buf[100], chByte, chProduct, chBasePage, chCode;
unsigned short shShort;
long lngLong;
unsigned char	activeStatusA, activeStatusB, activePage, initMAC[12];

	for(i = 2; i < 4; i++)
	{
		if(UL_Page_Read(i * 4, buf) != 0)
		{
			return CE_READ;
		}
		memcpy(ul_data[i * 4], buf, 16);
	}
#ifdef DEBUG_2_TIME
	ReaderResponse(csc_comm, ERR_OK, 0xF0, "\xf0\xf1", 2);
#endif
#ifdef	DEBUG_PRINT
	//source 
	PRINTK("Page0:%02x%02x%02x%02x Page1:%02x%02x%02x%02x Page2:%02x%02x%02x%02x Page3:%02x%02x%02x%02x\n", 
		ul_data[0][0], ul_data[0][1], ul_data[0][2], ul_data[0][3], ul_data[1][0], ul_data[1][1], ul_data[1][2], ul_data[1][3], ul_data[2][0], ul_data[2][1], ul_data[2][2], ul_data[2][3], ul_data[3][0], ul_data[3][1], ul_data[3][2], ul_data[3][3]);
	PRINTK("Page4:%02x%02x%02x%02x Page5:%02x%02x%02x%02x Page6:%02x%02x%02x%02x Page7:%02x%02x%02x%02x\n", 
		ul_data[4][0], ul_data[4][1], ul_data[4][2], ul_data[4][3], ul_data[5][0], ul_data[5][1], ul_data[5][2], ul_data[5][3], ul_data[6][0], ul_data[6][1], ul_data[6][2], ul_data[6][3], ul_data[7][0], ul_data[7][1], ul_data[7][2], ul_data[7][3]);
	PRINTK("Page8:%02x%02x%02x%02x Page9:%02x%02x%02x%02x Page10:%02x%02x%02x%02x Page11:%02x%02x%02x%02x\n", 
		ul_data[8][0], ul_data[8][1], ul_data[8][2], ul_data[8][3], ul_data[9][0], ul_data[9][1], ul_data[9][2], ul_data[9][3], ul_data[10][0], ul_data[10][1], ul_data[10][2], ul_data[10][3], ul_data[11][0], ul_data[11][1], ul_data[11][2], ul_data[11][3]);
	PRINTK("Page12:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", 
		ul_data[12][0], ul_data[12][1], ul_data[12][2], ul_data[12][3], ul_data[13][0], ul_data[13][1], ul_data[13][2], ul_data[13][3], ul_data[14][0], ul_data[14][1], ul_data[14][2], ul_data[14][3], ul_data[15][0], ul_data[15][1], ul_data[15][2], ul_data[15][3]);

	PRINTK("ver:%02x lifecycle %04x keySet %02x InitDate %04x batch %04x test %02x ticket %02x passenger %02x \nproduct %02x balance %04x rfu1 %04x pay %02x DurationType %02x validDuration %02x rfu3 %02x\n", 
			tpSZ.Version, tpSZ.lifecycleCount, tpSZ.keySetNumber, tpSZ.cardBaseDataTime, tpSZ.cardBatchNumber, tpSZ.testMode, tpSZ.productType,
			tpSZ.passengerType, tpSZ.productID, tpSZ.purchaseValue, tpSZ.rfu1, tpSZ.lavPaymentMethod, tpSZ.DurationType, tpSZ.validityDuration, tpSZ.productIssuerId);
#endif
	activeStatusA = (ul_data[8][0] & 0x80) >> 7;
	activeStatusB = (ul_data[12][0] & 0x80) >> 7;
#ifdef	DEBUG_PRINT
	PRINTK("activestatus A is %02x B is %02x\n", activeStatusA, activeStatusB);
#endif
	if((activeStatusA == 0) && (activeStatusB == 0))
	{//read B write A
		activePage = 0x0c;
		tpSJT.activeReadpage = 0x0c; tpSJT.activeWritepage = 0x08;
	}else if((activeStatusA == 1) && (activeStatusB == 0))
	{//read A write B
		activePage = 0x08;
		tpSJT.activeReadpage = 0x08; tpSJT.activeWritepage = 0x0c;
	}else if((activeStatusA == 1) && (activeStatusB == 1))
	{//read B write A
		activePage = 0x0c;
		tpSJT.activeReadpage = 0x0c; tpSJT.activeWritepage = 0x08;
	}else if((activeStatusA == 0) && (activeStatusB == 1))
	{//read A write B
		activePage = 0x08;
		tpSJT.activeReadpage = 0x08; tpSJT.activeWritepage = 0x0c;
	}
	//whether check the mac2 or not--not confirm
	tpSJT.mac2 = ((ul_data[tpSJT.activeReadpage + 3][2] & 0xF) << 8) + ul_data[tpSJT.activeReadpage + 3][3];
	chCode = UL_TellInitMAC(NULL);

	if(chCode == CE_MACERR)
	{
		ul_data[tpSJT.activeReadpage][0] ^= 0x80;
		if(tpSJT.activeReadpage == 0x08)
		{
			tpSJT.activeReadpage = 0x0c;
			tpSJT.activeWritepage = 0x08;
		}else if(tpSJT.activeReadpage == 0x0c)
		{
			tpSJT.activeReadpage = 0x08;
			tpSJT.activeWritepage = 0x0c;
		}
		tpSJT.mac2 = ((ul_data[tpSJT.activeReadpage + 3][2] & 0xF) << 8) + ul_data[tpSJT.activeReadpage + 3][3];
		if((chCode = UL_TellInitMAC(NULL)) != 0)
		{
			return chCode;
		}
	}
	else if(chCode != 0)
	{
		return chCode;
	}
	//another dynamic zone
#ifdef	DEBUG_PRINT
	//
	memcpy(xaDynamicZonePeriod.buff, ul_data[tpSJT.activeWritepage], 16);
	memcpy(xaDynamicZoneMultiride.buff, ul_data[tpSJT.activeWritepage], 16);
	memcpy(xaDynamicZonePurse.buff, ul_data[tpSJT.activeWritepage], 16);
	if(tpSZ.productID == XA_SJT_PERIOD)
	{
//		printf("tpSZ.productID == XA_SJT_PERIOD\n");
		tpDZperiod.activeStatus = xaDynamicZonePeriod._DynamicZone.activeStatus;
		tpDZperiod.cardStatus = xaDynamicZonePeriod._DynamicZone.cardStatus;
		tpDZperiod.transactionSequenceNumber = (xaDynamicZonePeriod._DynamicZone.transactionSequenceNumber_1 << 3) + xaDynamicZonePeriod._DynamicZone.transactionSequenceNumber;
		tpSJT.cardsn = tpDZperiod.transactionSequenceNumber;
		tpDZperiod.startDateTime = (xaDynamicZonePeriod._DynamicZone.startDateTime_1 << 18) + (xaDynamicZonePeriod._DynamicZone.startDateTime_2 << 10) + (xaDynamicZonePeriod._DynamicZone.startDateTime_3 << 2) + xaDynamicZonePeriod._DynamicZone.startDateTime;
		tpDZperiod.lastDateTime = (xaDynamicZonePeriod._DynamicZone.lastDateTime_1 << 2) + xaDynamicZonePeriod._DynamicZone.lastDateTime;
		tpDZperiod.lastLocation = (xaDynamicZonePeriod._DynamicZone.lastLocation_1 << 6) + xaDynamicZonePeriod._DynamicZone.lastLocation;;
		tpDZperiod.transfersTaken = xaDynamicZonePeriod._DynamicZone.transfersTaken;
		tpDZperiod.status = xaDynamicZonePeriod._DynamicZone.status;
		tpDZperiod.validityStartDateTime = (xaDynamicZonePeriod._DynamicZone.validityStartDateTime_1 << 17) + (xaDynamicZonePeriod._DynamicZone.validityStartDateTime_2 << 9) + (xaDynamicZonePeriod._DynamicZone.validityStartDateTime_3 << 1) + xaDynamicZonePeriod._DynamicZone.validityStartDateTime;
		tpDZperiod.validityOrigin =  (xaDynamicZonePeriod._DynamicZone.validityOrigin_1 << 5) + xaDynamicZonePeriod._DynamicZone.validityOrigin;
		tpDZperiod.validityDestination = (xaDynamicZonePeriod._DynamicZone.validityDestination_1 << 9) + (xaDynamicZonePeriod._DynamicZone.validityDestination_2 << 1) + xaDynamicZonePeriod._DynamicZone.validityDestination;
		tpDZperiod.activated = xaDynamicZonePeriod._DynamicZone.activated;
		tpDZperiod.mac2 = (xaDynamicZonePeriod._DynamicZone.mac2_1 << 8) + xaDynamicZonePeriod._DynamicZone.mac2;
		tpSJT.mac2 = tpDZperiod.mac2;
		PRINTK("active %02x card %02x tSN %02x starttime %06x lasttime %02x lastlocation %04x transfer %02x status %02x\n validity %06x origin %04x destination %04x activated %02x, mac2 %04x\n",
				tpDZperiod.activeStatus, tpDZperiod.cardStatus, tpDZperiod.transactionSequenceNumber, tpDZperiod.startDateTime, tpDZperiod.lastDateTime, tpDZperiod.lastLocation,
				tpDZperiod.transfersTaken, tpDZperiod.status, tpDZperiod.validityStartDateTime, tpDZperiod.validityOrigin, tpDZperiod.validityDestination, tpDZperiod.activated, tpDZperiod.mac2);
	}else if(tpSZ.productID == XA_SJT_RIDES)
	{
	//	printf("tpSZ.productID == XA_SJT_RIDES\n");
		tpDZmultiride.activeStatus = xaDynamicZoneMultiride._DynamicZone.activeStatus;
		tpDZmultiride.cardStatus = xaDynamicZoneMultiride._DynamicZone.cardStatus;
		tpDZmultiride.transactionSequenceNumber = (xaDynamicZoneMultiride._DynamicZone.transactionSequenceNumber_1 << 3) + xaDynamicZoneMultiride._DynamicZone.transactionSequenceNumber;
		tpSJT.cardsn = tpDZmultiride.transactionSequenceNumber;
		tpDZmultiride.startDateTime = (xaDynamicZoneMultiride._DynamicZone.startDateTime_1 << 18) + (xaDynamicZoneMultiride._DynamicZone.startDateTime_2 << 10) + (xaDynamicZoneMultiride._DynamicZone.startDateTime_3 << 2) + xaDynamicZoneMultiride._DynamicZone.startDateTime;
		tpDZmultiride.lastDateTime = (xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 << 2) + xaDynamicZoneMultiride._DynamicZone.lastDateTime;
		tpDZmultiride.lastLocation = (xaDynamicZoneMultiride._DynamicZone.lastLocation_1 << 6) + xaDynamicZoneMultiride._DynamicZone.lastLocation;;
		tpDZmultiride.transfersTaken = xaDynamicZoneMultiride._DynamicZone.transfersTaken;
		tpDZmultiride.status = xaDynamicZoneMultiride._DynamicZone.status;
		tpDZmultiride.validityStartDateTime = (xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_1 << 17) + (xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_2 << 9) + (xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_3 << 1) + xaDynamicZoneMultiride._DynamicZone.validityStartDateTime;
		tpDZmultiride.remainingRides = (xaDynamicZoneMultiride._DynamicZone.remainingRides_1 << 1) + xaDynamicZoneMultiride._DynamicZone.remainingRides;
		tpDZmultiride.validityOrigin = (xaDynamicZoneMultiride._DynamicZone.validityOrigin_1 << 5) + xaDynamicZoneMultiride._DynamicZone.validityOrigin;
		tpDZmultiride.validityDestination = (xaDynamicZoneMultiride._DynamicZone.validityDestination_1 << 9) + (xaDynamicZoneMultiride._DynamicZone.validityDestination_2 << 1) + xaDynamicZoneMultiride._DynamicZone.validityDestination;
		tpDZmultiride.activated = xaDynamicZoneMultiride._DynamicZone.activated;
		tpDZmultiride.totalValueUsed = xaDynamicZoneMultiride._DynamicZone.totalValueUsed;
		tpDZmultiride.mac2 = (xaDynamicZoneMultiride._DynamicZone.mac2_1 << 8) + xaDynamicZoneMultiride._DynamicZone.mac2;
		tpSJT.mac2 = tpDZmultiride.mac2;
		PRINTK("active %02x card %02x tSN %02x starttime %06x lasttime %02x lastlocation %04x transfer %02x status %02x\n validity %06x rides %02x origin %04x destination %04x activated %02x totalused %02x mac2 %04x\n",
				tpDZmultiride.activeStatus, tpDZmultiride.cardStatus, tpDZmultiride.transactionSequenceNumber, tpDZmultiride.startDateTime, tpDZmultiride.lastDateTime, tpDZmultiride.lastLocation,
				tpDZmultiride.transfersTaken, tpDZmultiride.status, tpDZmultiride.validityStartDateTime, tpDZmultiride.remainingRides, tpDZmultiride.validityOrigin, tpDZmultiride.validityDestination, tpDZmultiride.activated, tpDZmultiride.totalValueUsed, tpDZmultiride.mac2);
	}else
	//if(tpSZ.productID == XA_SJT_PURSE)
	{
	//	printf("tpSZ.productID == XA_SJT_PURSE\n");
		tpDZpurse.activeStatus = xaDynamicZonePurse._DynamicZone.activeStatus;
		tpDZpurse.cardStatus = xaDynamicZonePurse._DynamicZone.cardStatus;
		tpDZpurse.transactionSequenceNumber = (xaDynamicZonePurse._DynamicZone.transactionSequenceNumber_1 << 3) + xaDynamicZonePurse._DynamicZone.transactionSequenceNumber;
		tpSJT.cardsn = tpDZpurse.transactionSequenceNumber;
		tpDZpurse.startDateTime = (xaDynamicZonePurse._DynamicZone.startDateTime_1 << 18) + (xaDynamicZonePurse._DynamicZone.startDateTime_2 << 10) + (xaDynamicZonePurse._DynamicZone.startDateTime_3 << 2) + xaDynamicZonePurse._DynamicZone.startDateTime;
		tpDZpurse.lastDateTime = (xaDynamicZonePurse._DynamicZone.lastDateTime_1 << 2) + xaDynamicZonePurse._DynamicZone.lastDateTime;
		tpDZpurse.lastLocation = (xaDynamicZonePurse._DynamicZone.lastLocation_1 << 6) + xaDynamicZonePurse._DynamicZone.lastLocation;;
		tpDZpurse.transfersTaken = xaDynamicZonePurse._DynamicZone.transfersTaken;
		tpDZpurse.status = xaDynamicZonePurse._DynamicZone.status;
		tpDZpurse.validityStartDate = (xaDynamicZonePurse._DynamicZone.validityStartDate_1 << 7) + xaDynamicZonePurse._DynamicZone.validityStartDate;
		tpDZpurse.remainingValue = (xaDynamicZonePurse._DynamicZone.remainingValue_1 << 16) + (xaDynamicZonePurse._DynamicZone.remainingValue_2 << 8) + xaDynamicZonePurse._DynamicZone.remainingValue;
		tpDZpurse.activated = xaDynamicZonePurse._DynamicZone.activated;
		tpDZpurse.totalPurchaseValue = (xaDynamicZonePurse._DynamicZone.totalPurchaseValue_1 << 8) + xaDynamicZonePurse._DynamicZone.totalPurchaseValue;
		tpDZpurse.origin = (xaDynamicZonePurse._DynamicZone.origin_1 << 4) + xaDynamicZonePurse._DynamicZone.origin;
		tpDZpurse.mac2 = (xaDynamicZonePurse._DynamicZone.mac2_1 << 8) + xaDynamicZonePurse._DynamicZone.mac2;
		tpSJT.mac2 = tpDZpurse.mac2;
		PRINTK("active %02x card %02x tSN %02x starttime %06x lasttime %02x lastlocation %04x transfer %02x status %02x\n validity %04x balance %06x activated %02x totalvalue %06x, origin %04x mac2 %04x\n",
				tpDZpurse.activeStatus, tpDZpurse.cardStatus, tpDZpurse.transactionSequenceNumber, tpDZpurse.startDateTime, tpDZpurse.lastDateTime, tpDZpurse.lastLocation,
				tpDZpurse.transfersTaken, tpDZpurse.status, tpDZpurse.validityStartDate, tpDZpurse.remainingValue, tpDZpurse.activated, tpDZpurse.totalPurchaseValue, tpDZpurse.origin, tpDZpurse.mac2);
	}
#endif
	//
	memcpy(xaDynamicZonePeriod.buff, ul_data[tpSJT.activeReadpage], 16);
	memcpy(xaDynamicZoneMultiride.buff, ul_data[tpSJT.activeReadpage], 16);
	memcpy(xaDynamicZonePurse.buff, ul_data[tpSJT.activeReadpage], 16);
	if(tpSZ.productID == XA_SJT_PERIOD)
	{
	//	printf("tpSZ.productID == XA_SJT_PERIOD\n");
		tpDZperiod.activeStatus = xaDynamicZonePeriod._DynamicZone.activeStatus;
		tpDZperiod.cardStatus = xaDynamicZonePeriod._DynamicZone.cardStatus;
		tpDZperiod.transactionSequenceNumber = (xaDynamicZonePeriod._DynamicZone.transactionSequenceNumber_1 << 3) + xaDynamicZonePeriod._DynamicZone.transactionSequenceNumber;
		tpSJT.cardsn = tpDZperiod.transactionSequenceNumber;
		tpDZperiod.startDateTime = (xaDynamicZonePeriod._DynamicZone.startDateTime_1 << 18) + (xaDynamicZonePeriod._DynamicZone.startDateTime_2 << 10) + (xaDynamicZonePeriod._DynamicZone.startDateTime_3 << 2) + xaDynamicZonePeriod._DynamicZone.startDateTime;
		tpDZperiod.lastDateTime = (xaDynamicZonePeriod._DynamicZone.lastDateTime_1 << 2) + xaDynamicZonePeriod._DynamicZone.lastDateTime;
		tpDZperiod.lastLocation = (xaDynamicZonePeriod._DynamicZone.lastLocation_1 << 6) + xaDynamicZonePeriod._DynamicZone.lastLocation;;
		tpDZperiod.transfersTaken = xaDynamicZonePeriod._DynamicZone.transfersTaken;
		tpDZperiod.status = xaDynamicZonePeriod._DynamicZone.status;
		tpDZperiod.validityStartDateTime = (xaDynamicZonePeriod._DynamicZone.validityStartDateTime_1 << 17) + (xaDynamicZonePeriod._DynamicZone.validityStartDateTime_2 << 9) + (xaDynamicZonePeriod._DynamicZone.validityStartDateTime_3 << 1) + xaDynamicZonePeriod._DynamicZone.validityStartDateTime;
		tpDZperiod.validityOrigin =  (xaDynamicZonePeriod._DynamicZone.validityOrigin_1 << 5) + xaDynamicZonePeriod._DynamicZone.validityOrigin;
		tpDZperiod.validityDestination = (xaDynamicZonePeriod._DynamicZone.validityDestination_1 << 9) + (xaDynamicZonePeriod._DynamicZone.validityDestination_2 << 1) + xaDynamicZonePeriod._DynamicZone.validityDestination;
		tpDZperiod.activated = xaDynamicZonePeriod._DynamicZone.activated;
		tpDZperiod.mac2 = (xaDynamicZonePeriod._DynamicZone.mac2_1 << 8) + xaDynamicZonePeriod._DynamicZone.mac2;
		tpSJT.mac2 = tpDZperiod.mac2;
		//编码机不修改则强制修改为小时单位
		if(tpSZ.DurationType == 2)
		{
			tpSZ.DurationType = 1;
			tpSZ.validityDuration = tpSZ.validityDuration * 24;
		}
#ifdef	DEBUG_PRINT
		PRINTK("active %02x card %02x tSN %02x starttime %06x lasttime %02x lastlocation %04x transfer %02x status %02x\n validity %06x origin %04x destination %04x activated %02x, mac2 %04x\n",
				tpDZperiod.activeStatus, tpDZperiod.cardStatus, tpDZperiod.transactionSequenceNumber, tpDZperiod.startDateTime, tpDZperiod.lastDateTime, tpDZperiod.lastLocation,
				tpDZperiod.transfersTaken, tpDZperiod.status, tpDZperiod.validityStartDateTime, tpDZperiod.validityOrigin, tpDZperiod.validityDestination, tpDZperiod.activated, tpDZperiod.mac2);
#endif
	}else if(tpSZ.productID == XA_SJT_RIDES)
	{
	//	printf("tpSZ.productID == XA_SJT_RIDES\n");
		tpDZmultiride.activeStatus = xaDynamicZoneMultiride._DynamicZone.activeStatus;
		tpDZmultiride.cardStatus = xaDynamicZoneMultiride._DynamicZone.cardStatus;
		tpDZmultiride.transactionSequenceNumber = (xaDynamicZoneMultiride._DynamicZone.transactionSequenceNumber_1 << 3) + xaDynamicZoneMultiride._DynamicZone.transactionSequenceNumber;
		tpSJT.cardsn = tpDZmultiride.transactionSequenceNumber;
		tpDZmultiride.startDateTime = (xaDynamicZoneMultiride._DynamicZone.startDateTime_1 << 18) + (xaDynamicZoneMultiride._DynamicZone.startDateTime_2 << 10) + (xaDynamicZoneMultiride._DynamicZone.startDateTime_3 << 2) + xaDynamicZoneMultiride._DynamicZone.startDateTime;
		tpDZmultiride.lastDateTime = (xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 << 2) + xaDynamicZoneMultiride._DynamicZone.lastDateTime;
		tpDZmultiride.lastLocation = (xaDynamicZoneMultiride._DynamicZone.lastLocation_1 << 6) + xaDynamicZoneMultiride._DynamicZone.lastLocation;;
		tpDZmultiride.transfersTaken = xaDynamicZoneMultiride._DynamicZone.transfersTaken;
		tpDZmultiride.status = xaDynamicZoneMultiride._DynamicZone.status;
		tpDZmultiride.validityStartDateTime = (xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_1 << 17) + (xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_2 << 9) + (xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_3 << 1) + xaDynamicZoneMultiride._DynamicZone.validityStartDateTime;
		tpDZmultiride.remainingRides = (xaDynamicZoneMultiride._DynamicZone.remainingRides_1 << 1) + xaDynamicZoneMultiride._DynamicZone.remainingRides;
		tpDZmultiride.validityOrigin = (xaDynamicZoneMultiride._DynamicZone.validityOrigin_1 << 5) + xaDynamicZoneMultiride._DynamicZone.validityOrigin;
		tpDZmultiride.validityDestination = (xaDynamicZoneMultiride._DynamicZone.validityDestination_1 << 9) + (xaDynamicZoneMultiride._DynamicZone.validityDestination_2 << 1) + xaDynamicZoneMultiride._DynamicZone.validityDestination;
		tpDZmultiride.activated = xaDynamicZoneMultiride._DynamicZone.activated;
		tpDZmultiride.totalValueUsed = xaDynamicZoneMultiride._DynamicZone.totalValueUsed;
		tpDZmultiride.mac2 = (xaDynamicZoneMultiride._DynamicZone.mac2_1 << 8) + xaDynamicZoneMultiride._DynamicZone.mac2;
		tpSJT.mac2 = tpDZmultiride.mac2;
#ifdef	DEBUG_PRINT
		PRINTK("active %02x card %02x tSN %02x starttime %06x lasttime %02x lastlocation %04x transfer %02x status %02x\n validity %06x rides %02x origin %04x destination %04x activated %02x totalused %02x mac2 %04x\n",
				tpDZmultiride.activeStatus, tpDZmultiride.cardStatus, tpDZmultiride.transactionSequenceNumber, tpDZmultiride.startDateTime, tpDZmultiride.lastDateTime, tpDZmultiride.lastLocation,
				tpDZmultiride.transfersTaken, tpDZmultiride.status, tpDZmultiride.validityStartDateTime, tpDZmultiride.remainingRides, tpDZmultiride.validityOrigin, tpDZmultiride.validityDestination, tpDZmultiride.activated, tpDZmultiride.totalValueUsed, tpDZmultiride.mac2);
#endif
	}else
	//if(tpSZ.productID == XA_SJT_PURSE)
	{
	//	printf("tpSZ.productID == XA_SJT_PURSE\n");
		tpDZpurse.activeStatus = xaDynamicZonePurse._DynamicZone.activeStatus;
		tpDZpurse.cardStatus = xaDynamicZonePurse._DynamicZone.cardStatus;
		tpDZpurse.transactionSequenceNumber = (xaDynamicZonePurse._DynamicZone.transactionSequenceNumber_1 << 3) + xaDynamicZonePurse._DynamicZone.transactionSequenceNumber;
		tpSJT.cardsn = tpDZpurse.transactionSequenceNumber;
		tpDZpurse.startDateTime = (xaDynamicZonePurse._DynamicZone.startDateTime_1 << 18) + (xaDynamicZonePurse._DynamicZone.startDateTime_2 << 10) + (xaDynamicZonePurse._DynamicZone.startDateTime_3 << 2) + xaDynamicZonePurse._DynamicZone.startDateTime;
		tpDZpurse.lastDateTime = (xaDynamicZonePurse._DynamicZone.lastDateTime_1 << 2) + xaDynamicZonePurse._DynamicZone.lastDateTime;
		tpDZpurse.lastLocation = (xaDynamicZonePurse._DynamicZone.lastLocation_1 << 6) + xaDynamicZonePurse._DynamicZone.lastLocation;;
		tpDZpurse.transfersTaken = xaDynamicZonePurse._DynamicZone.transfersTaken;
		tpDZpurse.status = xaDynamicZonePurse._DynamicZone.status;
		tpDZpurse.validityStartDate = (xaDynamicZonePurse._DynamicZone.validityStartDate_1 << 7) + xaDynamicZonePurse._DynamicZone.validityStartDate;
		tpDZpurse.remainingValue = (xaDynamicZonePurse._DynamicZone.remainingValue_1 << 16) + (xaDynamicZonePurse._DynamicZone.remainingValue_2 << 8) + xaDynamicZonePurse._DynamicZone.remainingValue;
		tpDZpurse.activated = xaDynamicZonePurse._DynamicZone.activated;
		tpDZpurse.totalPurchaseValue = (xaDynamicZonePurse._DynamicZone.totalPurchaseValue_1 << 8) + xaDynamicZonePurse._DynamicZone.totalPurchaseValue;
		tpDZpurse.origin = (xaDynamicZonePurse._DynamicZone.origin_1 << 4) + xaDynamicZonePurse._DynamicZone.origin;
		tpDZpurse.mac2 = (xaDynamicZonePurse._DynamicZone.mac2_1 << 8) + xaDynamicZonePurse._DynamicZone.mac2;
		tpSJT.mac2 = tpDZpurse.mac2;
#ifdef	DEBUG_PRINT
		PRINTK("active %02x card %02x tSN %02x starttime %06x lasttime %02x lastlocation %04x transfer %02x status %02x\n validity %04x balance %06x activated %02x totalvalue %06x, origin %04x mac2 %04x\n",
				tpDZpurse.activeStatus, tpDZpurse.cardStatus, tpDZpurse.transactionSequenceNumber, tpDZpurse.startDateTime, tpDZpurse.lastDateTime, tpDZpurse.lastLocation,
				tpDZpurse.transfersTaken, tpDZpurse.status, tpDZpurse.validityStartDate, tpDZpurse.remainingValue, tpDZpurse.activated, tpDZpurse.totalPurchaseValue, tpDZpurse.origin, tpDZpurse.mac2);
#endif
	}
	return 0;
}

/*
function:
	1. calculate the initial block MAC
	
return:
	0: check MAC ok and return correctly MAC
	1: MAC wrong .
*/
char UL_CalInitMAC(unsigned char activepage, short *cal_mac2, unsigned char *out_buf)
{
unsigned char factor[8], key[2], buf[100], mac2[2], keyvalue[12];
char ret, i, retry;
unsigned long	StaticCRC, DynamicCRC;

	//first check the mac1
	StaticCRC = DynamicCRC = 0xffffffff;
	StaticCRC = Crc32(ul_data[4], 16, StaticCRC);
	memcpy(buf, ul_data[activepage], 16);
	buf[14] &= 0xF0;
	buf[15] = 0;
	DynamicCRC = Crc32(buf, 16, DynamicCRC);
	//80 fe
	//buf[0] = ul_data[0][0]; 	buf[1] = ul_data[0][1] ^ ul_data[0][2];
	//buf[2] = ul_data[0][3] ^ ul_data[1][0];		buf[3] = ul_data[1][1] ^ ul_data[1][2];
	buf[0] = ul_data[0][0]; 	buf[1] = ul_data[0][1] ^ ul_data[0][2];
	buf[2] = ul_data[1][0] ^ ul_data[1][1];		buf[3] = ul_data[1][2] ^ ul_data[1][3];
	memcpy(&buf[4], ul_data[3], 4);
	//mac1
	memcpy(&buf[8], ul_data[7], 4);
	//csn same as the buf[0~3]
	memcpy(&buf[12], buf, 4);
	//logic-page3
	memcpy(&buf[16], ul_data[3], 4);
	LongToByte(StaticCRC, &buf[20]);
	LongToByte(DynamicCRC, &buf[24]);

	memcpy(key, "\x28\x01", 2);
	retry  = 0;
label_cal_mac:
	if(0 != (sjt_cal_mac(key, NULL, buf, 28, keyvalue)))
	{
//		if(retry >= 2)
			return CE_METROPSAM;
//		retry += 1;
//		goto label_cal_mac;
	}
	mac2[0] = mac2[1] = 0;
	for(i = 0; i < 12; i++)
	{
		if(i % 3 == 0)
		{
			mac2[1] = mac2[1] ^ (keyvalue[i] >> 4);
			mac2[0] = mac2[0] ^ (keyvalue[i] << 4);
		}else if(i % 3 == 1)
		{
			mac2[0] = mac2[0] ^ (keyvalue[i] >> 4);
			mac2[1] = mac2[1] ^ (keyvalue[i] & 0xf);
		}else
		{
			mac2[0] = mac2[0] ^ keyvalue[i];
		}
	}
	mac2[1] &= 0xF;
	memcpy(cal_mac2, mac2, 2);
#ifdef DEBUG_PRINT	
	PRINTK("cal mac2 is %02x%02x\n", mac2[0], mac2[1]);
#endif
	return 0;
}

/*
function:
	1. check the initial block MAC
	
return:
	0: check MAC ok and return correctly MAC
	1: MAC wrong .
*/
char UL_TellInitMAC(unsigned char *initMAC)
{
unsigned char mac[4], mac1[2];
char chCode;
short	mac2;

	//calculate initial mac
	if((chCode = UL_CalInitMAC(tpSJT.activeReadpage, &mac2, NULL)) != 0)
	{
		return chCode;
	}
#ifdef DEBUG_PRINT
	PRINTK("calinit mac %03x sjt %03x\n", mac2, tpSJT.mac2);
#endif
	
	if(mac2 == tpSJT.mac2)
		return 0;
	else
		return CE_MACERR;
}

/*
function:
	1. calculate the issue block MAC
parameter:
	
return:
	0: check MAC ok and return correctly MAC
	1: MAC wrong.
*/
char UL_CalIssueMAC(char *issueMAC)
{
unsigned char factor[8], key[2], chCode;
unsigned short mac2;

	//calculate mac2
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &mac2, NULL)) != 0)
		return chCode;
	//
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= (unsigned char)(mac2 >> 8);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)mac2;
	return 0;

}

/*
function:
	1. check the issue block MAC
return:
	0: check MAC ok and return correctly MAC
	1: MAC wrong .
*/
char UL_TellIssueMAC(char *issueMAC)
{
char mac[4];
	
	if(UL_CalIssueMAC(mac) != 0)
		return ERR_ISSUE_MAC;
	if(memcmp(issueMAC, mac, 4) == 0)
		return 0;
	else
		return ERR_ISSUE_MAC;
}

/*
function:
	1. calculate the entry block MAC
	2. check the entry block MAC
return:
	0: check MAC ok and return correctly MAC
	1: MAC wrong .
*/
char UL_CalEntryMAC(char *entryMAC)
{
unsigned char factor[8], key[2];

	memset(ch_mac_data, 0x00, 24);
	//phyical id - 7
	memcpy(ch_mac_data, &ch_ul_phyical_id[1], 7);
	//entry time - 4
	memcpy(&ch_mac_data[7], ul_data[0xb], 4);
	//entry station - 1.5
	ch_mac_data[11] = ul_data[0xc][0];
	ch_mac_data[12] = ul_data[0xc][1] & 0xf0;
	//issue value - 1.5
	ch_mac_data[12] |= ((ul_data[8][1] & 0xf0) >> 4);
	ch_mac_data[13] = (ul_data[8][1] & 0xf) << 4;
	ch_mac_data[13] |=(( ul_data[8][2] & 0xf0) >> 4);
	//mileclass - 1
	ch_mac_data[14] = ul_data[9][2];
	//rejectcode - 1
	ch_mac_data[15] = (ul_data[0xf][1] & 0xf) << 4;
	ch_mac_data[15] |= (ul_data[0xf][2] & 0xf0) >> 4;
	//80
	ch_mac_data[16] = 0x80;
	
	memcpy(factor, &ch_ul_phyical_id[1], 7);
	factor[7] = ul_data[4][0];

	memcpy(key, "\x28\x02", 2);
	if(sjt_cal_mac(key, factor, ch_mac_data, 24, entryMAC) != 0)
		return 1;
	return 0;
}

/*
function:
	1. check the entry block MAC
	2. whether check the mode or not. if FALSE then return the entry mac result.
return:
	0: check MAC ok and return correctly MAC
	18: entry MAC wrong .
*/
char UL_TellEntryMAC(char entryMAC, unsigned char mode_check)
{
char mac[4], chret;
unsigned long i, InitStationNum, InitSensitiveNum;
unsigned char chInitStationFare, chInitSensitiveFare, chCode;
unsigned char eod_station[4], sta_close_entry[4], sen_close_entry[4];
unsigned long lngsrcstation, lngdesstation, lngLosecond;
unsigned short shcardstation, cnt;
unsigned short shCalFare;

	chret = CE_NO_ENTRY;
	
	//0x01 for last is entry status and 0x04 for last is update
	if((entryMAC == 0x01) || (entryMAC == 0x04))
		chret = 0;

	if(!mode_check)
		return chret;
	if(chret == CE_NO_ENTRY)
	{
	//if current station is set to entry mode then the entry station/time is current station/time
		if(tpwaivermode.cur_sta_entry)
		{
			//entry time is set to the current time
			cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
			tpDZmultiride.lastDateTime = tpDZperiod.lastDateTime = tpDZpurse.lastDateTime = lngLosecond;
			//entry station is set the current station
			lngsrcstation= 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
			if(0 != (chCode = location_to_card(lngsrcstation, &tpDZpurse.lastLocation)))
				return chCode;
			tpDZmultiride.lastLocation = tpDZperiod.lastLocation = tpDZpurse.lastLocation;
			chret = 0;
		}else if(tpwaivermode.oth_sta_entry || tpwaivermode.sen_sta_entry)
		{
			//entry time is set to the current time
			cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
			tpDZmultiride.lastDateTime = tpDZperiod.lastDateTime = tpDZpurse.lastDateTime = lngLosecond;
			//select the most close entry station from station set
			chInitStationFare = 0xff;
			InitStationNum = 0;
			for(i = 0; i < tpStationWaiverMode.waivermode_len; i++)
			{
				if(tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 9] & 0x02)
					//&& (memcmp(tpSJT.curstation, &tpStationWaiverMode.waivermode_val[i * 3], 2) != 0))
				{
					lngsrcstation = (tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 4] << 24) + (tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 5] << 16)
									+ (tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 6] << 8) + tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 7];
					lngdesstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
					if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngdesstation, &shCalFare)))
						continue;
					InitStationNum += 1;
					if(shCalFare < chInitStationFare)
					{
						chInitStationFare = shCalFare;
						memcpy(sta_close_entry, &tpStationWaiverMode.waivermode_val[i * XA_WAIVER_LEN + 4], 4);
					}
				}
			}
			//if mode list waiver_entry station is zero
			if((InitStationNum == 0))// && (InitSensitiveNum == 0))
				return chret;
			/*else if(InitStationNum == 0)
				memcpy(eod_station, sen_close_entry, 2);
			else if(InitSensitiveNum == 0)
				memcpy(eod_station, sta_close_entry, 2);
			else if(chInitSensitiveFare < chInitStationFare)
				memcpy(eod_station, sen_close_entry, 2);
			else*/
				memcpy(eod_station, sta_close_entry, 4);
			//entry station is set the most close station
			lngsrcstation = (eod_station[0] << 24) + (eod_station[1] << 16) + (eod_station[2] << 8) + eod_station[3];
			if(0 != (chCode = location_to_card(lngsrcstation, &tpDZpurse.lastLocation)))
				return chCode;
			tpDZmultiride.lastLocation = tpDZperiod.lastLocation = tpDZpurse.lastLocation;
			chret = 0;
		}
	}
	
	return chret;
}

/*
function:
	1. calculate the exit block MAC
parameter:
	exitmac:return the calculate exit mac
return:
	?: the correctly MAC
*/
char UL_CalExitMAC(char *exitMAC)
{
char i;
unsigned char chreject, chExitMac;
char station_value[3];
	
	//phyical id - 8
	chExitMac = ch_ul_phyical_id[0];
	for(i = 1; i < 8; i++)
		chExitMac ^= ch_ul_phyical_id[i];
	//exit date time - 4
	for(i = 0; i < 4; i++)
		chExitMac ^= ul_data[0xe][i];
	//exit station - 1.5
	station_value[0] = ul_data[0xf][0];
	station_value[1] = (ul_data[0xf][1] & 0xf0) >> 4;
	//issue value - 1.5
	station_value[1] |= ((ul_data[8][1] & 0xf0) >> 4);
	station_value[2] = (ul_data[8][1] & 0xf) << 4;
	station_value[2] |= ((ul_data[8][2] & 0xf0) >> 4);
	chExitMac ^= station_value[0];
	chExitMac ^= station_value[1];
	chExitMac ^= station_value[2];
	//mile class - 1
	chExitMac ^= ul_data[9][2];
	//reject code - 1
	chreject = (ul_data[0xf][1] & 0xf) << 4;
	chreject |= ((ul_data[0xf][2] & 0xf0) >> 4);
	chExitMac ^= chreject;
	
	*exitMAC = chExitMac;
	return chExitMac;
}

/*
function:
	1. check the exit block MAC
parameter:
	1.
	
return:
	0: check MAC ok and return correctly MAC
	1: MAC wrong .
*/
char UL_TellExitMAC(char *exitMAC)
{
char chExitMAC, i;

	
	UL_CalExitMAC(&chExitMAC);
	
	if(chExitMAC == *exitMAC)
		return 0;
	else
		return 1;
}

/*
function:
	1. localdate-initialdate>phycialdate then return 4
	2. wrong MAC1(initial mac) return 1
	3. TicketDefinition_t.EnableFlag == 0 then return 22
	4. if souvenir ticket(?) then check the B7 of OT P== 1 then return 7
parameter:
	1.ticket type :the ticket type from command for sale, other from the ticket
	2.local date time
return:
	0:ok
	208: no eod 03 file
	other:above
*/
unsigned char UL_TellSysCard(unsigned short tickettype, unsigned char *in_localtime)
{
unsigned short day1, day2, tint;
unsigned char buf[5], chCode;
u_int	i;
long	lngHiSecond, lngLoSecond;
unsigned long lngMidnightSecond;
	
	//
	if((chCode = get_ticket_para(tickettype, &tpTicketDef)) != 0)
	{
		return chCode;
	}
	//check balance
	//if(tpSZ.purchaseValue < tpTicketDef.MinRemainingValue)
	//	return CE_BALANCE;
	return 0;
}

/*
function: check the sjt used times(only added one on saling mode) return ?
return:
	0:ok
	208: no eod 03 file
*/
char UL_TellCallback(void)
{
unsigned short cnt;
unsigned long maxchipionumbers, i;


		return 0;
}

/*
function:override
parameter:
	srcstation:source station or entry station
	desstation:destination station or current station[09 00 LL SS]
	laststation:last used station(check for the over fare station)
	journeyStatus:
	balance:
return:
	0:ok
*/
char UL_TellOverRide(unsigned char *cur_timebcd, unsigned short srcstation, unsigned char *desstation, unsigned short laststation, unsigned char journeyStatus, long balance)
{
char chCode;
unsigned short shFare;
unsigned long lngsrcstation, lngdesstation, lngPrice;
unsigned short shlastcard;

	if(0 != (chCode = card_to_location(srcstation, &lngsrcstation)))
		return chCode;
	//
	lngdesstation = 0x09000000 + (desstation[0] << 8) + desstation[1];
	if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrcstation, lngdesstation, &shFare)))
		return chCode;
	if(0 != (chCode = cal_fare_value(cur_timebcd, &tpTicketDef, shFare, XA_PASSENGER_ADULT, &tpSysPrice)))
		return chCode;
	//check the ticket status and last station is the current station
	if(0 != (chCode = location_to_card(lngdesstation, &shlastcard)))
		return chCode;
	//if((tpDZpurse.status == 4) && (tpDZpurse.lastLocation == shlastcard))
	//
	//if((journeyStatus == 4) && (laststation== shlastcard))
	if( (journeyStatus == 4) && (0 == check_same_station(lngdesstation, laststation)) )
		return 0;
	//current station is set to the failure mode
	if(tpwaivermode.cur_sta_failure)
		return 0;
	//current station is set to the fare mode
	if(tpwaivermode.cur_sta_fare)
		return 0;
	//
#ifdef	DEBUG_PRINT
	PRINTK("balance %04x price %04x", balance, tpSysPrice.price);
#endif
	if(balance >= tpSysPrice.price)
		return 0;
	else
		return CE_OVERRIDE;
}

/*
function:overtime
parameter:
	*entrytime:
	*curtime:
	*mileclass:
*/
/*
char UL_TellOverTime(unsigned char *entrytime, unsigned char *curtime, unsigned char mileclass)
{
unsigned long lngovertime;
long lngHisecond1, lngHisecond2, lngLosecond1, lngLosecond2;
char chCode;

	sz_localtimeToSecond(entrytime, &lngHisecond1, &lngLosecond1);
	sz_localtimeToSecond(curtime, &lngHisecond2, &lngLosecond2);

	if(0 !=(chCode = cal_fare_time(mileclass, &lngovertime)))
		return chCode;
#ifdef DEBUG_PRINT
	PRINTK("entry time %d %d exit time %d %d\n", lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2);
#endif
	//判断是否超时
	if((lngHisecond2 - lngHisecond1) > 0)
		return CE_OVERTIME;
	if((lngLosecond2 - lngLosecond1) > lngovertime)
		return CE_OVERTIME;
		
	return 0;
}*/

/*
function:
	1. if B6 of Flag1 in the issue block is 0 and device mode is normal return 11
	2. if B6 of Flag1 in the issue block is 1 and device mode is testing mode return 11
parameter:
	0:working mode
	1:test mode
return :
	11: not matching mode
	0: mode match ok
*/
//Flag1.6是否与测试模式匹配
//0-match,1-not match
char UL_TellTesting(unsigned char chTestMode)
{
char status_match;

	if(tpSZ.testMode) //test card
	{
		if(chTestMode)	status_match = 0;
		else 		status_match = CE_TESTING_STATUS;
	}
	else //working card
	{
		if(chTestMode)  status_match = CE_TESTING_STATUS;
		else 		status_match = 0;
	}	

	return status_match;
}


/******************************************
sjt sale
******************************************/
char xa_ul_sale(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned short 	shTicketType, cnt, cnt2, i;
unsigned char 	chCode, chFaretier, mac_buf[4][4], ul_sale_data[16];
long lngHisecond, lngLosecond;
unsigned short 	shDays, shFareValue, shTimes;
unsigned long 	lngMidnightsecond, lngstation, lngFareValue;
unsigned char 	tac[4], ret, end_timebcd[7];
unsigned long 	lngStartZone, lngEndZone;
short	shStartZone, shEndZone;

	*out_len = 2;

	memcpy(tpSJT.time_bcd, &cmd_buf[10], 7);
	tpSJT.lowsecond = timestr2long(&cmd_buf[11]) + TIME2000;
	//read the ul infor
	memcpy(out_buf, "\x20\x05", 2);
	if(read_tocken_info()!=0)
	{
		return CE_READ;
	}
	//check the SJT base date time 
	//xa_monthtodate(tpSZ.cardBaseDataTime, end_timebcd);
	//if(memcmp(tpSJT.time_bcd, end_timebcd, 4) < 0)
	//	return CE_EXPIREDDATE;
	xa_MinuteTolocaltime(&end_timebcd[0], tpSZ.cardBaseDataTime, 0x3FFFFF, &lngMidnightsecond);
	if( memcmp(tpSJT.time_bcd, end_timebcd, 4) >= 0 )
		return CE_INVADLIDCARD;
	//purse
	tpTxnProductPurseIssue.SysComHdr_val.formatVersion = tpTxnProductMultirideIssue.SysComHdr_val.formatVersion = tpTxnProductExitIssue.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
	tpTxnProductPurseIssue.SysComHdr_val.udType = toMoto(3);
	tpTxnProductPurseIssue.SysComHdr_val.udSubtype = toMoto(1);
	tpTxnProductPurseIssue.SysComHdr_val.txnDateTime = tpTxnProductMultirideIssue.SysComHdr_val.txnDateTime = tpTxnProductExitIssue.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond - ZONE8);
	//multiride
	tpTxnProductMultirideIssue.SysComHdr_val.udType = toMoto(3);
	tpTxnProductMultirideIssue.SysComHdr_val.udSubtype = toMoto(3);
	//exit ticket
	tpTxnProductExitIssue.SysComHdr_val.udType = toMoto(3);
	tpTxnProductExitIssue.SysComHdr_val.udSubtype = toMoto(121);
	//transaction serial number
	tpTxnProductPurseIssue.SysComHdr_val.udsn = tpSJT.transn = ByteToLong(NULL, &cmd_buf[6]);//toMoto(*(long *)(&cmd_buf[6]));
	tpTxnProductMultirideIssue.SysComHdr_val.udsn = tpTxnProductExitIssue.SysComHdr_val.udsn = tpSJT.transn;
	//PRINTK("%08x %08x %08x %08x %08x\n", &cmd_buf[6], &cmd_buf[4], (long *)&cmd_buf[6], *(long *)&cmd_buf[6],tpTxnProductPurseIssue.SysComHdr_val.udsn);
	tpTxnProductPurseIssue.SysAppCom_val.applicationProviderId = tpTxnProductMultirideIssue.SysAppCom_val.applicationProviderId = tpTxnProductExitIssue.SysAppCom_val.applicationProviderId = toMoto(1);
	tpTxnProductPurseIssue.SysAppCom_val.applicationSerialNumber = tpTxnProductMultirideIssue.SysAppCom_val.applicationSerialNumber = tpTxnProductExitIssue.SysAppCom_val.applicationSerialNumber = toMoto(1);
	tpTxnProductPurseIssue.SysAppCom_val.applicationPersonalliseCat = tpTxnProductMultirideIssue.SysAppCom_val.applicationPersonalliseCat = tpTxnProductExitIssue.SysAppCom_val.applicationPersonalliseCat = toMoto(1);
	tpTxnProductPurseIssue.SysAppCom_val.appActionSequenceNumber = tpTxnProductMultirideIssue.SysAppCom_val.appActionSequenceNumber = tpTxnProductExitIssue.SysAppCom_val.appActionSequenceNumber = 0;
	tpTxnProductPurseIssue.SysAppCom_val.applicationType = tpTxnProductMultirideIssue.SysAppCom_val.applicationType = tpTxnProductExitIssue.SysAppCom_val.applicationType = toMoto(1);
	tpTxnProductPurseIssue.SysAppCom_val.applicationPassengerType = tpTxnProductMultirideIssue.SysAppCom_val.applicationPassengerType = tpTxnProductExitIssue.SysAppCom_val.applicationPassengerType = toMoto(cmd_buf[24]);
	
	tpTxnProductMultirideIssue.DevUdProductValidity_val.vOrigin = tpTxnProductPurseIssue.DevUdProductValidity_val.vOrigin = tpTxnProductExitIssue.DevUdProductValidity_val.vOrigin = tpTxnProductPurseIssue.SysComHdr_val.deviceLocation;
	tpTxnProductMultirideIssue.DevUdProductValidity_val.vDestination = tpTxnProductPurseIssue.DevUdProductValidity_val.vDestination = tpTxnProductExitIssue.DevUdProductValidity_val.vDestination = tpTxnProductPurseIssue.SysComHdr_val.deviceLocation;

	memcpy(out_buf, "\x20\x06", 2);
	//
	tpTxnProductPurseIssue.SysCardCom_val.cardLifeCycleCount = tpTxnProductMultirideIssue.SysCardCom_val.cardLifeCycleCount = toMoto(tpSZ.lifecycleCount);
	tpTxnProductPurseIssue.SysCardCom_val.cardActionSequenceNumber = tpTxnProductMultirideIssue.SysCardCom_val.cardActionSequenceNumber = 0;
	
	tpTxnProductPurseIssue.SysProductCom_val.productSerialNumber = 0;
	tpTxnProductPurseIssue.SysProductCom_val.invoicePrinted = tpTxnProductMultirideIssue.SysProductCom_val.invoicePrinted = tpTxnProductExitIssue.SysProductCom_val.invoicePrinted = 0;
	memcpy(out_buf, "\x20\x07", 2);
	tpSJT.tickettype = cmd_buf[20];
	//ticket type-cmd_buf[19][20]
	memcpy(&shTicketType, &cmd_buf[19], 2);
	if((chCode = UL_TellSysCard(shTicketType, &cmd_buf[29])) != 0)
	{
		return chCode;
	}
	tpTxnProductPurseIssue.SysProductCom_val.productType = tpTxnProductMultirideIssue.SysProductCom_val.productType = tpTxnProductExitIssue.SysProductCom_val.productType = toMoto(shTicketType);
	tpTxnProductPurseIssue.SysProductCom_val.productIssuerId = tpTxnProductMultirideIssue.SysProductCom_val.productIssuerId = tpTxnProductPurseIssue.SysCardCom_val.cardissuerId = tpTxnProductMultirideIssue.SysCardCom_val.cardissuerId = tpTxnProductExitIssue.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
	memcpy(out_buf, "\x20\x08", 2);
	if((chCode = UL_TellCallback()) != 0)
	{
		return chCode;
	}
	memcpy(out_buf, "\x20\x09", 2);
	//static zone
	//produt--0x01(purse)/0x03(multiride)
	xaStaticZone._StaticZone.productID = cmd_buf[18];
	tpTxnProductPurseIssue.SysCardCom_val.cardType = tpTxnProductMultirideIssue.SysCardCom_val.cardType = tpTxnProductExitIssue.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);
	xaStaticZone._StaticZone.productType_1 = (char)shTicketType >> 1;
	xaStaticZone._StaticZone.productType = (char)shTicketType & 0x1;
	//passenger type-1
	xaStaticZone._StaticZone.passengerType = cmd_buf[24];
	//saled price
	memcpy(&lngFareValue, &cmd_buf[25], 4);
	memcpy(&tpSJT.tranamount, &cmd_buf[25], 4);
	tpSJT.balance = tpSZ.purchaseValue = tpSJT.tranamount;
	//saled price-2
	xaStaticZone._StaticZone.purchaseValue_1 = lngFareValue >> 15;
	xaStaticZone._StaticZone.purchaseValue_2 = (lngFareValue >> 7) & 0xFF;
	xaStaticZone._StaticZone.purchaseValue = lngFareValue & 0x7F;
	//paymentMethod
	xaStaticZone._StaticZone.lavPaymentMethod = 1;
	xaStaticZone._StaticZone.DurationType = 2;
	xaStaticZone._StaticZone.productIssuerId = 1;
	//validityDuration
	xaStaticZone._StaticZone.validityDuration_1 = 0;
	xaStaticZone._StaticZone.validityDuration = 1;

	tpTxnProductPurseIssue.DevUdProductValidity_val.vDuration = tpTxnProductMultirideIssue.DevUdProductValidity_val.vDuration = toMoto(0x4001);
	memcpy(ul_data[4], xaStaticZone.buff, 12);
	//first check whether exit-ticket or not--0x0001(exit ticket)
	if(cmd_buf[18] == XA_FEETYPE_TIMES)
	{//exit ticket is multiride but not only exit ticket is multiride
	//	printf("cmd_buf[18] == XA_FEETYPE_TIMES \n");
		tpTxnProductPurseIssue.SysProductCom_val.Ptsn = tpTxnProductMultirideIssue.SysProductCom_val.Ptsn = toMoto(tpDZmultiride.transactionSequenceNumber);
		//dynamic zone
		xaDynamicZoneMultiride._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;
		tpDZmultiride.transactionSequenceNumber += 1;
	
		tpSJT.cardsn = tpDZmultiride.transactionSequenceNumber;
		xaDynamicZoneMultiride._DynamicZone.cardStatus = XA_SJT_CARD_NOMAL;
		xaDynamicZoneMultiride._DynamicZone.transactionSequenceNumber_1 = tpDZpurse.transactionSequenceNumber >> 3;
		xaDynamicZoneMultiride._DynamicZone.transactionSequenceNumber = tpDZpurse.transactionSequenceNumber & 0x7;
		//
		cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
		xaDynamicZoneMultiride._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
		xaDynamicZoneMultiride._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
		xaDynamicZoneMultiride._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
		xaDynamicZoneMultiride._DynamicZone.startDateTime = lngLosecond & 0x3;
		xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 = 0;
		xaDynamicZoneMultiride._DynamicZone.lastDateTime = 0;
		//last station
		lngstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
		if(0 != (chCode = location_to_card(lngstation, &tpDZmultiride.lastLocation)))
			return chCode;
		xaDynamicZoneMultiride._DynamicZone.lastLocation_1 = (tpDZmultiride.lastLocation >> 6) & 0x3F;
		xaDynamicZoneMultiride._DynamicZone.lastLocation = tpDZmultiride.lastLocation & 0x3f;
		
		//
		xaDynamicZoneMultiride._DynamicZone.transfersTaken = 0;
		//
		if(shTicketType == 0x01)
		//exit ticket
			xaDynamicZoneMultiride._DynamicZone.status = 4;
		else
			xaDynamicZoneMultiride._DynamicZone.status = 0;
		//informaion for multiride
		xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_1 = (lngLosecond >> 17) & 0x1F;
		xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_2 = (lngLosecond >> 9) & 0xFF;
		xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_3 = (lngLosecond >> 1) & 0xFF;
		xaDynamicZoneMultiride._DynamicZone.validityStartDateTime = lngLosecond & 0x1;
		
		tpTxnProductMultirideIssue.DevUdProductValidity_val.vStartDateTime = toMoto(timestr2long(&tpSJT.time_bcd[1]) + TIME2000);
		memset(end_timebcd, 0x00, 7);
		memcpy(end_timebcd, tpSJT.time_bcd, 4);
		lngMidnightsecond = timestr2long(&end_timebcd[1]) + TIME2000;
		tpTxnProductMultirideIssue.DevUdProductValidity_val.vEndDateTime = toMoto(lngMidnightsecond + 24 * 3600);
		tpTxnProductMultirideIssue.DevUdProductValidity_val.vDuration = toMoto(0x4001);
		//
		memcpy(&shTimes, &cmd_buf[37], 2);
		xaDynamicZoneMultiride._DynamicZone.remainingRides_1 = (shTimes & 0xFF) >> 1;
		xaDynamicZoneMultiride._DynamicZone.remainingRides = shTimes & 0x01;

		tpTxnProductMultirideIssue.DevUdMultirideCommonHdr_val.remainingRides = toMoto(1);
		tpTxnProductMultirideIssue.DevUdMultirideCommonHdr_val.numRides = toMoto(1);
		
		xaDynamicZoneMultiride._DynamicZone.activated = 1;
		memcpy(&lngStartZone, &cmd_buf[29], 4);
		if(0 != location_to_card(lngStartZone, &shStartZone))
		{
			shStartZone = 0xfff;
		}
		xaDynamicZoneMultiride._DynamicZone.validityOrigin_1 = (shStartZone >> 5) & 0x7f;
		xaDynamicZoneMultiride._DynamicZone.validityOrigin = shStartZone & 0x1f;
		memcpy(&lngEndZone, &cmd_buf[33], 4);
		if(0 != location_to_card(lngEndZone, &shEndZone))
		{
			shEndZone = 0xfff;
		}
		xaDynamicZoneMultiride._DynamicZone.validityDestination_1 = (shEndZone >> 9) & 0x7;
		xaDynamicZoneMultiride._DynamicZone.validityDestination_2 = (shEndZone >> 1) & 0xFF;
		xaDynamicZoneMultiride._DynamicZone.validityDestination = shEndZone & 0x1;
		
		xaDynamicZoneMultiride._DynamicZone.totalValueUsed = 0;	
		//cal the mac2	
		memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZoneMultiride.buff, 16);
#ifdef	DEBUG_PRINT
		PRINTK("Page%d:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", tpSJT.activeWritepage,
			ul_data[tpSJT.activeWritepage][0], ul_data[tpSJT.activeWritepage][1], ul_data[tpSJT.activeWritepage][2], ul_data[tpSJT.activeWritepage][3],
			ul_data[tpSJT.activeWritepage+1][0], ul_data[tpSJT.activeWritepage+1][1], ul_data[tpSJT.activeWritepage+1][2], ul_data[tpSJT.activeWritepage+1][3], 
			ul_data[tpSJT.activeWritepage+2][0], ul_data[tpSJT.activeWritepage+2][1], ul_data[tpSJT.activeWritepage+2][2], ul_data[tpSJT.activeWritepage+2][3], 
			ul_data[tpSJT.activeWritepage+3][0], ul_data[tpSJT.activeWritepage+3][1], ul_data[tpSJT.activeWritepage+3][2], ul_data[tpSJT.activeWritepage+3][3]);
#endif
		if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZmultiride.mac2, NULL)) != 0)
		{
			return chCode;
		}
		ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
		ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZmultiride.mac2 >> 8) & 0xF);
		ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZmultiride.mac2;
#ifdef	DEBUG_PRINT
		PRINTK("Page%d:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", tpSJT.activeWritepage,
			ul_data[tpSJT.activeWritepage][0], ul_data[tpSJT.activeWritepage][1], ul_data[tpSJT.activeWritepage][2], ul_data[tpSJT.activeWritepage][3],
			ul_data[tpSJT.activeWritepage+1][0], ul_data[tpSJT.activeWritepage+1][1], ul_data[tpSJT.activeWritepage+1][2], ul_data[tpSJT.activeWritepage+1][3], 
			ul_data[tpSJT.activeWritepage+2][0], ul_data[tpSJT.activeWritepage+2][1], ul_data[tpSJT.activeWritepage+2][2], ul_data[tpSJT.activeWritepage+2][3], 
			ul_data[tpSJT.activeWritepage+3][0], ul_data[tpSJT.activeWritepage+3][1], ul_data[tpSJT.activeWritepage+3][2], ul_data[tpSJT.activeWritepage+3][3]);
#endif
		//
		//calculate tac from thread
		//logic-4
		out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
		out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
		tpTxnProductMultirideIssue.SysCardCom_val.cardSerialNumber = tpTxnProductExitIssue.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	
		//BefBalance-4
		memcpy(&out_buf[10], &tpSJT.balance, 4);
		//balance-4
		memcpy(&out_buf[14], &tpSJT.tranamount, 4);
		tpTxnProductMultirideIssue.SysFinDetails_val.transactionValue = tpTxnProductExitIssue.SysFinDetails_val.transactionValue = toMoto(tpSJT.tranamount);
		//
		tpTxnProductMultirideIssue.SysFinDetails_val.paymentMethod = tpTxnProductExitIssue.SysFinDetails_val.paymentMethod = toMoto(cmd_buf[43]);
		tpTxnProductMultirideIssue.SysFinDetails_val.partialTransactionValue = tpTxnProductExitIssue.SysFinDetails_val.partialTransactionValue = 0;
		tpTxnProductMultirideIssue.SysSecurityHdr_val.keyVersion = tpTxnProductExitIssue.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
		//
		if(shTicketType == 0x01)
		{
			cnt2 = cnt = sizeof(TxnProductMultirideExitTicketIssue_t);
			sh_mac_len = cnt - 12 - 10;
			memcpy(ch_mac_data, &tpTxnProductExitIssue.SysComHdr_val.formatVersion, 40);
			memcpy(&ch_mac_data[40], &tpTxnProductExitIssue.SysComHdr_val.reservedField, sh_mac_len - 40 - 4 - 24);
			memcpy(&ch_mac_data[sh_mac_len - 4 - 24], &tpTxnProductExitIssue.DevUdProductValidity_val.vStartDateTime, 20);
			sh_mac_len -= (4 + 4);
			g_sha1txnsn = tpTxnProductExitIssue.SysComHdr_val.udsn;
			tpYPT_txn_val.pYPT_tac = &tpTxnProductExitIssue.SysSecurityHdr_val.txnMac[0];
		}else
		{
			cnt2 = cnt = sizeof(TxnProductMultirideIssue_t);
			sh_mac_len = cnt - 12 - 10;
			memcpy(ch_mac_data, &tpTxnProductMultirideIssue.SysComHdr_val.formatVersion, 40);
			memcpy(&ch_mac_data[40], &tpTxnProductMultirideIssue.SysComHdr_val.reservedField, sh_mac_len - 40 - 4 - 24);
			memcpy(&ch_mac_data[sh_mac_len - 4 - 24], &tpTxnProductMultirideIssue.DevUdProductValidity_val.vStartDateTime, 20);
			sh_mac_len -= (4 + 4);
			g_sha1txnsn = tpTxnProductMultirideIssue.SysComHdr_val.udsn;
			tpYPT_txn_val.pYPT_tac = &tpTxnProductMultirideIssue.SysSecurityHdr_val.txnMac[0];
		}
		tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
		tpYPT_txn_val.pYPT_txn = &out_buf[33];
		tpYPT_txn_val.YPT_txnlen = cnt + 6;
		tpYPT_txn_val.YPT_flag = 0;
		
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
	}else if(cmd_buf[18] == XA_FEETYPE_VALUE)
	{//purse
	//	printf("cmd_buf[18] == XA_FEETYPE_VALUE \n");
		tpTxnProductPurseIssue.SysProductCom_val.Ptsn = tpTxnProductMultirideIssue.SysProductCom_val.Ptsn = toMoto(tpDZpurse.transactionSequenceNumber);
		//dynamic zone
		xaDynamicZonePurse._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;
		tpDZpurse.transactionSequenceNumber += 1;
		xaDynamicZonePurse._DynamicZone.cardStatus = XA_SJT_CARD_NOMAL;
		tpSJT.cardsn = tpDZpurse.transactionSequenceNumber;
		
		xaDynamicZonePurse._DynamicZone.transactionSequenceNumber_1 = tpDZpurse.transactionSequenceNumber >> 3;
		xaDynamicZonePurse._DynamicZone.transactionSequenceNumber = tpDZpurse.transactionSequenceNumber & 0x7;
		//
		cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
		xaDynamicZonePurse._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
		xaDynamicZonePurse._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
		xaDynamicZonePurse._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
		xaDynamicZonePurse._DynamicZone.startDateTime = lngLosecond & 0x3;
		xaDynamicZonePurse._DynamicZone.lastDateTime_1 = 0;
		xaDynamicZonePurse._DynamicZone.lastDateTime = 0;
		//
		lngstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
		if(0 != (chCode = location_to_card(lngstation, &tpDZpurse.lastLocation)))
			return chCode;
		xaDynamicZonePurse._DynamicZone.lastLocation_1 = (tpDZpurse.lastLocation >> 6) & 0x3F;
		xaDynamicZonePurse._DynamicZone.lastLocation = tpDZpurse.lastLocation & 0x3f;
		xaDynamicZonePurse._DynamicZone.transfersTaken = 0;
		xaDynamicZonePurse._DynamicZone.status = 0;
		//start date time
		xaDynamicZonePurse._DynamicZone.validityStartDate_1 = (cnt >> 7) & 0x1F;
		xaDynamicZonePurse._DynamicZone.validityStartDate = cnt & 0x7F;
		
		tpTxnProductPurseIssue.DevUdProductValidity_val.vStartDateTime = toMoto(timestr2long(&tpSJT.time_bcd[1]) + TIME2000);
		memset(end_timebcd, 0x00, 7);
		memcpy(end_timebcd, tpSJT.time_bcd, 4);
		lngMidnightsecond = timestr2long(&end_timebcd[1]) + TIME2000;
		tpTxnProductPurseIssue.DevUdProductValidity_val.vEndDateTime = toMoto(lngMidnightsecond + 24 * 3600);
		tpTxnProductPurseIssue.DevUdProductValidity_val.vDuration = toMoto((0x04 << 12) + 1);
		
		xaDynamicZonePurse._DynamicZone.remainingValue_1 = 0;
		xaDynamicZonePurse._DynamicZone.remainingValue_2 = lngFareValue >> 8;
		xaDynamicZonePurse._DynamicZone.remainingValue = lngFareValue & 0xFF;
		xaDynamicZonePurse._DynamicZone.activated = 1;
		xaDynamicZonePurse._DynamicZone.totalPurchaseValue_1 = 0;
		xaDynamicZonePurse._DynamicZone.totalPurchaseValue = 0;
		//Origin station
		memcpy(&lngStartZone, &cmd_buf[29], 4);
		if(0 != location_to_card(lngStartZone, &shStartZone))
		{
			shStartZone = 0xfff;
		}
		xaDynamicZonePurse._DynamicZone.origin_1 = (shStartZone >> 4) & 0xFF;
		xaDynamicZonePurse._DynamicZone.origin = shStartZone & 0xF;
		//cal the mac2	
		memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePurse.buff, 16);
#ifdef	DEBUG_PRINT
		PRINTK("Page%d:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", tpSJT.activeWritepage,
			ul_data[tpSJT.activeWritepage][0], ul_data[tpSJT.activeWritepage][1], ul_data[tpSJT.activeWritepage][2], ul_data[tpSJT.activeWritepage][3],
			ul_data[tpSJT.activeWritepage+1][0], ul_data[tpSJT.activeWritepage+1][1], ul_data[tpSJT.activeWritepage+1][2], ul_data[tpSJT.activeWritepage+1][3], 
			ul_data[tpSJT.activeWritepage+2][0], ul_data[tpSJT.activeWritepage+2][1], ul_data[tpSJT.activeWritepage+2][2], ul_data[tpSJT.activeWritepage+2][3], 
			ul_data[tpSJT.activeWritepage+3][0], ul_data[tpSJT.activeWritepage+3][1], ul_data[tpSJT.activeWritepage+3][2], ul_data[tpSJT.activeWritepage+3][3]);
#endif
		if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZpurse.mac2, NULL)) != 0)
		{
			return chCode;
		}
		ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
		ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZpurse.mac2 >> 8) & 0xF);
		ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZpurse.mac2;
#ifdef	DEBUG_PRINT
		PRINTK("Page%d:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", tpSJT.activeWritepage,
			ul_data[tpSJT.activeWritepage][0], ul_data[tpSJT.activeWritepage][1], ul_data[tpSJT.activeWritepage][2], ul_data[tpSJT.activeWritepage][3],
			ul_data[tpSJT.activeWritepage+1][0], ul_data[tpSJT.activeWritepage+1][1], ul_data[tpSJT.activeWritepage+1][2], ul_data[tpSJT.activeWritepage+1][3], 
			ul_data[tpSJT.activeWritepage+2][0], ul_data[tpSJT.activeWritepage+2][1], ul_data[tpSJT.activeWritepage+2][2], ul_data[tpSJT.activeWritepage+2][3], 
			ul_data[tpSJT.activeWritepage+3][0], ul_data[tpSJT.activeWritepage+3][1], ul_data[tpSJT.activeWritepage+3][2], ul_data[tpSJT.activeWritepage+3][3]);
#endif
		//
		//calculate tac from thread
		//logic-4
		out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
		out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
		tpTxnProductPurseIssue.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	
		//BefBalance-4
		memcpy(&out_buf[10], &tpSJT.balance, 4);
		//balance-4
		memcpy(&out_buf[14], &tpSJT.tranamount, 4);
		tpTxnProductPurseIssue.purseRemainingValue = toMoto(tpSJT.tranamount);
		tpTxnProductPurseIssue.SysFinDetails_val.transactionValue = toMoto(tpSJT.tranamount);
		//toMoto(1)
		tpTxnProductPurseIssue.SysFinDetails_val.paymentMethod = toMoto(cmd_buf[43]);
		tpTxnProductPurseIssue.SysFinDetails_val.partialTransactionValue = 0;
		tpTxnProductPurseIssue.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
		//
		cnt2 = cnt = sizeof(TxnProductPurseIssue_t);
		sh_mac_len = cnt - 12 - 10;
		memcpy(ch_mac_data, &tpTxnProductPurseIssue.SysComHdr_val.formatVersion, 40);
		sh_mac_len -= 4;
		memcpy(&ch_mac_data[40], &tpTxnProductPurseIssue.SysComHdr_val.reservedField, sh_mac_len - 40 - 24);
		memcpy(&ch_mac_data[sh_mac_len - 24], &tpTxnProductPurseIssue.DevUdProductValidity_val.vStartDateTime, 20);
		sh_mac_len -=  4;
		g_sha1txnsn = tpTxnProductPurseIssue.SysComHdr_val.udsn;
		tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
		tpYPT_txn_val.pYPT_txn = &out_buf[33];
		tpYPT_txn_val.pYPT_tac = &tpTxnProductPurseIssue.SysSecurityHdr_val.txnMac[0];
		tpYPT_txn_val.YPT_txnlen = cnt + 6;
		tpYPT_txn_val.YPT_flag = 0;
		
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
	}else 
	{
		return CE_BADPARAM;
	}

	memcpy(out_buf, "\x20\x0d", 2);
	//write static zone only page 5 and 6
	for(i = 5; i < 7; i++)
	{
		if(standard_tocken_write(i, ul_data[i]) != 0)
		{
			reader_status = XA_RW_IDLE;
			return CE_WRITE;
		}
	}
label_sz_rollback_1:
	memcpy(out_buf, "\x20\x0e", 2);
	for(i = tpSJT.activeWritepage; i < (tpSJT.activeWritepage + 4); i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			reader_status = XA_RW_IDLE;
			return CE_WRITE;
		}
	}
	//对成功完成写卡的数据进行回读并判断
	if(UL_Page_Read(tpSJT.activeWritepage, ul_sale_data) == 0)
	{
		if(memcmp(ul_sale_data, ul_data[tpSJT.activeWritepage], 16) != 0)
			return CE_WRITE;
	}
	
	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	memcpy(&out_buf[4], &cmd_buf[19], 2);
	//lock status -1
	out_buf[18] = 0x00;
	//rfu-14
	memset(&out_buf[19], 0x00, 14);
	
	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	sem_wait(&g_samreturn);
	if(cmd_buf[18] == XA_FEETYPE_TIMES)
	{
		if(shTicketType == 0x01)
			memcpy(&out_buf[39], tpTxnProductExitIssue.AFCHead_val.operatorid, cnt);
		else
			memcpy(&out_buf[39], tpTxnProductMultirideIssue.AFCHead_val.operatorid, cnt);
	}
	else
		memcpy(&out_buf[39], tpTxnProductPurseIssue.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	reader_status = XA_RW_IDLE;
	(*out_len) += cnt;
#ifdef	DEBUG_PRINT
	PRINTK("UL ISSUE:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif
	return CE_OK;
}

/******************************************
sjt entry
******************************************/
char xa_ul_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char IDbuf[6];
unsigned char buf[20], chCode;
unsigned char TimeTemp[7];
unsigned short i, shBalance, cnt;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
unsigned char chExitMac[4], tac[4];
unsigned short shDays, shFareValue, cur_station;
unsigned long lngMidnightSecond, lngstation, lngLosecond;
u_int 	transfer_station_id, transfer_station;

#ifdef DEBUG_PRINT
	PRINTK("\nentry command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("time %02x%02x-%02x-%02x %02x:%02x:%02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	PRINTK("SN:%02x%02x%02x%02x UD %02x\n", cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17]);
#endif
	*out_len = 2;
	memcpy(tpSJT.time_bcd, &cmd_buf[6], 7);
	tpSJT.lowsecond = timestr2long(&cmd_buf[7]);
	//check the parameter valid
	memcpy(out_buf, "\x21\x03", 2);
	get_degrade_mode(tpSJT.curstation);

	memcpy(out_buf, "\x21\x05", 2);
	if((chCode = read_tocken_info()) != 0)
	{
		return chCode;
	}

	memcpy(out_buf, "\x21\x06", 2);
	if((chCode = UL_TellSysCard(tpSZ.productType, NULL)) != 0)
	{
		return chCode;
	}
	memcpy(out_buf, "\x21\x07", 2);
	xa_MinuteTolocaltime(&TimeTemp[0], tpSZ.cardBaseDataTime, 0x3FFFFF, &lngHisecond1);
	if(memcmp(tpSJT.time_bcd, TimeTemp, 7) > 0)
		return CE_INVADLIDCARD;

	/*long issue_sec = lngHisecond1;
	long cmd_sec = tpSJT.lowsecond;
	long time_differ = cmd_sec - issue_sec;

	const long LIMIT_WALFARE = 10800;

	if(tpSZ.productType == 0x0C){
		if(time_differ > LIMIT_WALFARE) return CE_INVADLIDCARD;
	}*/
	
	memcpy(out_buf, "\x21\x17", 2);
	switch(tpSZ.productID)
	{
	case XA_FEETYPE_VALUE:			//value
		//
		tpTxnProductPurseEntry.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
		tpTxnProductPurseEntry.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]);//toMoto(*((long *)(&cmd_buf[13])));
		tpTxnProductPurseEntry.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
		tpTxnProductPurseEntry.SysComHdr_val.udType = toMoto(3);
		tpTxnProductPurseEntry.SysComHdr_val.udSubtype = toMoto(88);

		tpTxnProductPurseEntry.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);

		tpTxnProductPurseEntry.SysProductCom_val.productIssuerId = tpTxnProductPurseEntry.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
		tpTxnProductPurseEntry.SysProductCom_val.productType = toMoto(tpSZ.productType);
		tpTxnProductPurseEntry.SysProductCom_val.Ptsn = toMoto(tpDZpurse.transactionSequenceNumber);

		tpTxnProductPurseEntry.SysAppCom_val.applicationPassengerType = toMoto(1);
		
		tpTxnProductPurseEntry.DevUdJourneyHdr_val.currentLocation = tpTxnProductPurseEntry.SysComHdr_val.deviceLocation;
		tpTxnProductPurseEntry.DevUdJourneyHdr_val.tripOriginLocation = tpTxnProductPurseEntry.SysComHdr_val.deviceLocation;
		tpTxnProductPurseEntry.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductPurseEntry.SysComHdr_val.deviceLocation;
		
		memset(&tpTxnProductPurseEntry.DevUdPurseLavHdr_val.lavSamId, 0x00, sizeof(DevUdPurseLavHdr_t));
		//
		return xa_ul_entry_purse(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_TIMES:			//rides
		//
		tpTxnProductMultirideEntry.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
		tpTxnProductMultirideEntry.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
		tpTxnProductMultirideEntry.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]);//toMoto(*((long *)(&cmd_buf[13])));
		tpTxnProductMultirideEntry.SysComHdr_val.udType = toMoto(3);
		tpTxnProductMultirideEntry.SysComHdr_val.udSubtype = toMoto(90);
		//
		tpTxnProductMultirideEntry.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);
		tpTxnProductMultirideEntry.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
		//
		tpTxnProductMultirideEntry.SysProductCom_val.productType = toMoto(tpSZ.productType);
		tpTxnProductMultirideEntry.SysProductCom_val.Ptsn = toMoto(tpDZperiod.transactionSequenceNumber);

		tpTxnProductMultirideEntry.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideEntry.SysComHdr_val.deviceLocation;
		tpTxnProductMultirideEntry.DevUdJourneyHdr_val.tripOriginLocation = tpTxnProductMultirideEntry.SysComHdr_val.deviceLocation;
		tpTxnProductMultirideEntry.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductMultirideEntry.SysComHdr_val.deviceLocation;
		tpTxnProductMultirideEntry.DevUdJourneyHdr_val.passengerType = toMoto(tpSZ.passengerType);
		
		memset(&tpTxnProductMultirideEntry.DevUdMultirideLavHdr_val.lavSamId, 0x00, sizeof(DevUdMultirideLavHdr_t));
		return xa_ul_entry_multiride(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_PERIOD:		//period
		//
		tpTxnProductPassEntry.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
		tpTxnProductPassEntry.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
		tpTxnProductPassEntry.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]);//toMoto(*((long *)(&cmd_buf[13])));
		tpTxnProductPassEntry.SysComHdr_val.udType = toMoto(3);
		tpTxnProductPassEntry.SysComHdr_val.udSubtype = toMoto(89);
		//
		tpTxnProductPassEntry.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);
		//
		tpTxnProductPassEntry.SysProductCom_val.productIssuerId = tpTxnProductPassEntry.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
		tpTxnProductPassEntry.SysProductCom_val.productType = toMoto(tpSZ.productType);
		tpTxnProductPassEntry.SysProductCom_val.Ptsn = toMoto(tpDZperiod.transactionSequenceNumber);
		
		tpTxnProductPassEntry.SysAppCom_val.applicationPassengerType = toMoto(tpSZ.passengerType);

		tpTxnProductPassEntry.DevUdJourneyHdr_val.currentLocation = tpTxnProductPassEntry.SysComHdr_val.deviceLocation;
		tpTxnProductPassEntry.DevUdJourneyHdr_val.tripOriginLocation = tpTxnProductPassEntry.SysComHdr_val.deviceLocation;
		tpTxnProductPassEntry.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductPassEntry.SysComHdr_val.deviceLocation;
		tpTxnProductPassEntry.DevUdJourneyHdr_val.passengerType = toMoto(tpSZ.passengerType);
		
		memset(&tpTxnProductPassEntry.DevUdPassLavHdr_val.lavSamId, 0x00, sizeof(DevUdPassLavHdr_t));
		return xa_ul_entry_period(cmd_buf, out_buf, out_len);
	default:
		return CE_NON_FEETYPE;
	}

}

char xa_ul_entry_purse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
char chCode;
unsigned short i, shBalance, cnt, shcardLocation;
unsigned long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
unsigned long lngMidnightSecond, lngstation, lngLosecond;
unsigned char start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned char active_page[16];

	//check whether the ticket is issued or not
	if(tpDZpurse.cardStatus != XA_SJT_CARD_NOMAL)
		return CE_ISSUED;
	//check the balance 
	tpSJT.balance = tpDZpurse.remainingValue;
	if(tpDZpurse.activated == 0)
	{
		xa_daytodate(tpSZ.cardBaseDataTime, tpDZpurse.validityStartDate, &lngHisecond1, &start_timebcd[0]);
		if(memcmp(tpSJT.time_bcd, start_timebcd, 4) > 0)
			return CE_EXPIREDDATE;
		else
		{
			tpDZpurse.validityStartDate = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngHisecond1);
			//start date time
			xaDynamicZonePurse._DynamicZone.validityStartDate_1 = (tpDZpurse.validityStartDate >> 7) & 0x1F;
			xaDynamicZonePurse._DynamicZone.validityStartDate = tpDZpurse.validityStartDate & 0x7F;
			//activated
			xaDynamicZonePurse._DynamicZone.activated = 1;
		}
	}
	//check the date
	memcpy(out_buf, "\x21\x0c", 2);
	xa_daytodate(tpSZ.cardBaseDataTime, tpDZpurse.validityStartDate, &lngHisecond1, &start_timebcd[0]);
	get_degrade_sensitive_mode(NULL, start_timebcd);
	xa_DurationTolocaltime(lngHisecond1, tpSZ.DurationType, tpSZ.validityDuration, end_timebcd);
	memset(&end_timebcd[4], 0x00, 3);
	lngMidnightSecond = timestr2long(&end_timebcd[1]) - 1;
	long2timestr(lngMidnightSecond, end_timebcd);
	xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZpurse.startDateTime, &lngHisecond1);
	lngHisecond1 += (tpDZpurse.lastDateTime * 60);
	long2timestr(lngHisecond1, last_timebcd);
	if((chCode = xa_TellDate(tpSJT.time_bcd, start_timebcd, end_timebcd, tpDZpurse.status, last_timebcd, tpSZ.DurationType)) != 0)
	{
		return chCode;
	}
	//sensitive mode change the ticket status
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpSJT.time_bcd, last_timebcd, 4) != 0))
	{
		tpDZpurse.status = 0;
		tpTicketDef.FirstUseAtStationOfIssue = 0;
	}
	//check the entry MAC 
	memcpy(out_buf, "\x21\x08", 2);
	if(UL_TellEntryMAC(tpDZpurse.status, 0) == 0)
	{
		if(lngHisecond1 > tpSJT.lowsecond)
			chCode = CE_FREE_UPDATE_ENTRY;
		else if((tpSJT.lowsecond - lngHisecond1) <= 20 * 60)
			chCode = CE_FREE_UPDATE_ENTRY;
		else
			chCode = CE_NO_UPDATE_ENTRY;
		return chCode;
	}
	//check the TEST mode
	memcpy(out_buf, "\x21\x0a", 2);
	if((chCode = UL_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//check the issue station
	if(tpTicketDef.FirstUseAtStationOfIssue)
	{
		lngstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
		if(0 != (chCode = location_to_card(lngstation, &shcardLocation)))
			return chCode;
		if(shcardLocation != tpDZpurse.lastLocation)
		{
			if(0 != check_same_station(lngstation, tpDZpurse.lastLocation))
				return CE_NOT_ISSUEDSTATION;
		}
	}
	//dynamic zone
	xaDynamicZonePurse._DynamicZone.activeStatus = ((((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1);
#ifdef	DEBUG_PRINT
	PRINTK("active status %02x, write page %02x\n", xaDynamicZonePurse.buff[0], tpSJT.activeWritepage);
#endif
	memcpy(out_buf, "\x21\x0d", 2);
	//entry time
	cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
	xaDynamicZonePurse._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
	xaDynamicZonePurse._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
	xaDynamicZonePurse._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
	xaDynamicZonePurse._DynamicZone.startDateTime = lngLosecond & 0x3;
	//
	xaDynamicZonePurse._DynamicZone.lastDateTime_1 = 0;
	xaDynamicZonePurse._DynamicZone.lastDateTime = 0;
	//entry station
	lngstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpDZpurse.lastLocation)))
		return chCode;
	xaDynamicZonePurse._DynamicZone.lastLocation_1 = (tpDZpurse.lastLocation >> 6) & 0x3F;
	xaDynamicZonePurse._DynamicZone.lastLocation = tpDZpurse.lastLocation & 0x3f;
	xaDynamicZonePurse._DynamicZone.origin_1 = (tpDZpurse.lastLocation >> 4) & 0xff;
	xaDynamicZonePurse._DynamicZone.origin = tpDZpurse.lastLocation & 0xf;
	
	xaDynamicZonePurse._DynamicZone.status = 1;
	//cal the mac2	
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePurse.buff, 16);
#ifdef DEBUG_PRINT
	PRINTK("Page8:%02x%02x%02x%02x Page9:%02x%02x%02x%02x Page10:%02x%02x%02x%02x Page11:%02x%02x%02x%02x\n", 
		ul_data[8][0], ul_data[8][1], ul_data[8][2], ul_data[8][3], ul_data[9][0], ul_data[9][1], ul_data[9][2], ul_data[9][3], ul_data[10][0], ul_data[10][1], ul_data[10][2], ul_data[10][3], ul_data[11][0], ul_data[11][1], ul_data[11][2], ul_data[11][3]);
	PRINTK("Page12:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", 
		ul_data[12][0], ul_data[12][1], ul_data[12][2], ul_data[12][3], ul_data[13][0], ul_data[13][1], ul_data[13][2], ul_data[13][3], ul_data[14][0], ul_data[14][1], ul_data[14][2], ul_data[14][3], ul_data[15][0], ul_data[15][1], ul_data[15][2], ul_data[15][3]);
#endif
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZpurse.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZpurse.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZpurse.mac2;

	//card sn-4
	ByteToShort(&tpSJT.cardsn, &ul_data[6][2]);
	//transaction amount
	tpSJT.tranamount = 0;
	//cal tac using thread
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	tpTxnProductPurseEntry.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &tpSJT.balance, 4);
	tpTxnProductPurseEntry.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpSJT.balance);
	tpTxnProductPurseEntry.SysFinDetails_val.transactionValue = 0;
	tpTxnProductPurseEntry.SysFinDetails_val.paymentMethod = toMoto(1);
	tpTxnProductPurseEntry.SysFinDetails_val.partialTransactionValue = 0;
	//
	tpTxnProductPurseEntry.startOfJourney = toMoto(1);
	tpTxnProductPurseEntry.totalJourneyAmount = 0;
	tpTxnProductPurseEntry.firstUseActivation = toMoto(tpDZpurse.activated);
	tpTxnProductPurseEntry.DevUdJourneyHdr_val.passengerType = toMoto(tpSZ.passengerType);
	tpTxnProductPurseEntry.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);

	cnt = sizeof(TxnProductPurseUseOnEntry_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPurseEntry.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPurseEntry.SysComHdr_val.reservedField, sh_mac_len - 12 - 36 - 4);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[sh_mac_len - 12 - 36], &tpTxnProductPurseEntry.DevUdPurseLavHdr_val.lavSamId, 12 + 36);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPurseEntry.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPurseEntry.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;
	//write dyanmic zone
	for(i = tpSJT.activeWritepage; i < tpSJT.activeWritepage + 4; i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			//sem_wait(&g_samreturn);
			//tpYPT_txn_val.YPT_flag = 0;
			//ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.pYPT_txn, tpYPT_txn_val.YPT_txnlen);
			return CE_WRITE;
		}
	}
	//check the write result 2016/3/28 21:12:57
	if( 0 != UL_Page_Read(tpSJT.activeWritepage, active_page) )
	{
		if(memcmp(active_page, ul_data[tpSJT.activeWritepage], 16) != 0)
			return CE_WRITE;
	}
	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &tpSJT.balance, 4);
	//lock status -1
	out_buf[18] = 0x00;
	//product category
	out_buf[19] = XA_FEETYPE_VALUE;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);
	
	//
	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductPurseEntry.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//calculate the TAC
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);

	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		//sem_wait(&g_samreturn);
		//ee_write_last_record(XA_SJT_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}
	return CE_OK;
}

char xa_ul_entry_multiride(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
char chCode;
unsigned short i, shBalance, cnt;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
unsigned long lngMidnightSecond, lngstation, lngLosecond;
unsigned char start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned char active_page[16];

	//check whether the ticket is issued or not
	if(tpDZmultiride.cardStatus != XA_SJT_CARD_NOMAL)
		return CE_ISSUED;
	//check the balance 
	tpSJT.balance = tpDZmultiride.remainingRides;
	if(tpDZmultiride.activated == 0)
	{
		xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZmultiride.validityStartDateTime, &lngHisecond1);
		if(memcmp(tpSJT.time_bcd, start_timebcd, 4) > 0)
			return CE_EXPIREDDATE;
		else
		{
			//
			xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &tpDZmultiride.validityStartDateTime);
			xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_1 = (tpDZmultiride.validityStartDateTime >> 17) & 0x1F;
			xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_2 = (tpDZmultiride.validityStartDateTime >> 9) & 0xFF;
			xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_3 = (tpDZmultiride.validityStartDateTime >> 1) & 0xFF;
			xaDynamicZoneMultiride._DynamicZone.validityStartDateTime = tpDZmultiride.validityStartDateTime & 0x1;
			//activated
			xaDynamicZoneMultiride._DynamicZone.activated = 1;
		}
	}
	//check the date
	memcpy(out_buf, "\x21\x0c", 2);
	xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZmultiride.validityStartDateTime, &lngHisecond2);
	xa_DurationTolocaltime(lngHisecond2, 2, tpSZ.validityDuration, end_timebcd);
	xa_MinuteTolocaltime(&last_timebcd[0], tpSZ.cardBaseDataTime, tpDZmultiride.startDateTime, &lngHisecond1);
	lngHisecond1 += (tpDZmultiride.lastDateTime * 60);
	long2timestr(lngHisecond1, last_timebcd);
	get_degrade_sensitive_mode(NULL, last_timebcd);
	if((chCode = xa_TellDate(tpSJT.time_bcd, start_timebcd, end_timebcd, tpDZmultiride.status, last_timebcd, tpSZ.DurationType)) != 0)
	{
		return chCode;
	}
	if(tpSZ.productType == 0x0C){
		if((chCode = CheckTicketOvertime(start_timebcd, tpSJT.time_bcd, 1)) != 0)
		{
			memcpy(out_buf, "\x22\x09", 2);
			return CE_INVADLIDCARD;
		}
	}
	//sensitive mode to change the ticket TRAVEL status
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpSJT.time_bcd, last_timebcd, 4) != 0))
	{
		tpDZmultiride.status = 0;
	}
	//check the entry MAC 
	memcpy(out_buf, "\x21\x08", 2);
	if(UL_TellEntryMAC(tpDZmultiride.status, 0) == 0)
	{
		if(lngHisecond1 > tpSJT.lowsecond)
			chCode = CE_FREE_UPDATE_ENTRY;
		else if((tpSJT.lowsecond - lngHisecond1) <= 20 * 60)
			chCode = CE_FREE_UPDATE_ENTRY;
		else
			chCode = CE_NO_UPDATE_ENTRY;
		return chCode;
	}
	//check the TEST mode
	memcpy(out_buf, "\x21\x0a", 2);
	if((chCode = UL_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//check the validityOrigin
	if(0 != (chCode = card_to_location(tpDZmultiride.validityOrigin, &lngstation)))
		return chCode;
	if(0 != (chCode = xa_ValidateArea(tpCmdInit.curstation, lngstation)))
		return chCode;
	//dynamic zone
	xaDynamicZoneMultiride._DynamicZone.activeStatus = ((((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1);
#ifdef	DEBUG_PRINT
	PRINTK("active status %02x, write page %02x\n", xaDynamicZoneMultiride.buff[0], tpSJT.activeWritepage);
#endif
	memcpy(out_buf, "\x21\x0d", 2);
	//entry time
	cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
	xaDynamicZoneMultiride._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
	xaDynamicZoneMultiride._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
	xaDynamicZoneMultiride._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
	xaDynamicZoneMultiride._DynamicZone.startDateTime = lngLosecond & 0x3;
	//
	xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 = 0;
	xaDynamicZoneMultiride._DynamicZone.lastDateTime = 0;
	//entry station
	lngstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpDZmultiride.lastLocation)))
		return chCode;
	xaDynamicZoneMultiride._DynamicZone.lastLocation_1 = (tpDZmultiride.lastLocation >> 6) & 0x3F;
	xaDynamicZoneMultiride._DynamicZone.lastLocation = tpDZmultiride.lastLocation & 0x3f;
	
	xaDynamicZoneMultiride._DynamicZone.status = 1;
	//cal the mac2	
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZoneMultiride.buff, 16);
#ifdef DEBUG_PRINT
	PRINTK("Page8:%02x%02x%02x%02x Page9:%02x%02x%02x%02x Page10:%02x%02x%02x%02x Page11:%02x%02x%02x%02x\n", 
		ul_data[8][0], ul_data[8][1], ul_data[8][2], ul_data[8][3], ul_data[9][0], ul_data[9][1], ul_data[9][2], ul_data[9][3], ul_data[10][0], ul_data[10][1], ul_data[10][2], ul_data[10][3], ul_data[11][0], ul_data[11][1], ul_data[11][2], ul_data[11][3]);
	PRINTK("Page12:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", 
		ul_data[12][0], ul_data[12][1], ul_data[12][2], ul_data[12][3], ul_data[13][0], ul_data[13][1], ul_data[13][2], ul_data[13][3], ul_data[14][0], ul_data[14][1], ul_data[14][2], ul_data[14][3], ul_data[15][0], ul_data[15][1], ul_data[15][2], ul_data[15][3]);
#endif
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZmultiride.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZmultiride.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZmultiride.mac2;

	//card sn-4
	ByteToShort(&tpSJT.cardsn, &ul_data[6][2]);
	//transaction amount
	tpSJT.tranamount = 0;
	//cal tac using thread
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	tpTxnProductMultirideEntry.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &tpSJT.balance, 4);
	//
	tpTxnProductMultirideEntry.DevUdMultirideCommonHdr_val.numRides = 0;
	tpTxnProductMultirideEntry.DevUdMultirideCommonHdr_val.remainingRides = toMoto(tpDZmultiride.remainingRides);
	//
	tpTxnProductMultirideEntry.startOfJourney = toMoto(1);
	tpTxnProductMultirideEntry.firstUseActivation = toMoto(tpDZpurse.activated);

	tpTxnProductMultirideEntry.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);

	cnt = sizeof(TxnProductMultirideUseOnEntry_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductMultirideEntry.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductMultirideEntry.SysComHdr_val.reservedField, sh_mac_len - 40);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductMultirideEntry.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductMultirideEntry.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;
	
	//calculate TAC
	if(cmd_buf[17] != 0x02)
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
	}
	//write dyanmic zone
	for(i = tpSJT.activeWritepage; i < tpSJT.activeWritepage + 4; i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			//sem_wait(&g_samreturn);
			//tpYPT_txn_val.YPT_flag = 0;
			//ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.pYPT_txn, tpYPT_txn_val.YPT_txnlen);
			return CE_WRITE;
		}
	}
	//check the write result 2016/3/28 21:12:57
	if( 0 != UL_Page_Read(tpSJT.activeWritepage, active_page) )
	{
		if(memcmp(active_page, ul_data[tpSJT.activeWritepage], 16) != 0)
			return CE_WRITE;
	}
	
	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &tpSJT.balance, 4);
	//lock status -1
	out_buf[18] = 0x00;
	//product category
	out_buf[19] = XA_FEETYPE_TIMES;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);
	
	//
	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductMultirideEntry.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);
	
	
	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		//sem_wait(&g_samreturn);
		//ee_write_last_record(XA_SJT_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}
	return CE_OK;
}

char xa_ul_entry_period(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
char chCode;
unsigned short i, shBalance, cnt, cnt2;
long lngHisecond1, lngLosecond1, lngHisecond2, lngLosecond2;
unsigned long lngMidnightSecond, lngstation, lngLosecond;
unsigned char start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned char active_page[16];

	//check whether the ticket is issued or not
	if(tpDZperiod.cardStatus != XA_SJT_CARD_NOMAL)
		return CE_ISSUED;
	//check the balance 
	if(tpDZperiod.activated == 0)
	{
		xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZperiod.validityStartDateTime, &lngHisecond1);
		if(memcmp(tpSJT.time_bcd, start_timebcd, 6) > 0)
			return CE_EXPIREDDATE;
		else
		{
			//
			xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &tpDZperiod.validityStartDateTime);
			xaDynamicZonePeriod._DynamicZone.validityStartDateTime_1 = (tpDZperiod.validityStartDateTime >> 17) & 0x1F;
			xaDynamicZonePeriod._DynamicZone.validityStartDateTime_2 = (tpDZperiod.validityStartDateTime >> 9) & 0xFF;
			xaDynamicZonePeriod._DynamicZone.validityStartDateTime_3 = (tpDZperiod.validityStartDateTime >> 1) & 0xFF;
			xaDynamicZonePeriod._DynamicZone.validityStartDateTime = tpDZperiod.validityStartDateTime & 0x1;
			//activated
			xaDynamicZonePeriod._DynamicZone.activated = 1;
		}
	}
	//check the date
	memcpy(out_buf, "\x21\x0c", 2);
	//calculate the initial day
	xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZperiod.validityStartDateTime, &lngHisecond2);
	//calculate the expired time
	xa_DurationTolocaltime(lngHisecond2, tpSZ.DurationType, tpSZ.validityDuration, end_timebcd);
	//calculate the last used time
	xa_MinuteTolocaltime(&last_timebcd[0], tpSZ.cardBaseDataTime, tpDZperiod.startDateTime, &lngHisecond1);
	lngHisecond1 += (tpDZperiod.lastDateTime * 60);
	long2timestr(lngHisecond1, last_timebcd);
	get_degrade_sensitive_mode(NULL, last_timebcd);
	if((chCode = xa_TellDate(tpSJT.time_bcd, start_timebcd, end_timebcd, tpDZperiod.status, last_timebcd, tpSZ.DurationType)) != 0)
	{
		return chCode;
	}
	//sensitive mode to change the ticket status
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpSJT.time_bcd, last_timebcd, 4) != 0))
	{
		tpDZperiod.status = 0;
	}
	//check the entry MAC 
	memcpy(out_buf, "\x21\x08", 2);
	if(UL_TellEntryMAC(tpDZperiod.status, 0) == 0)
	{
		if(lngHisecond1 > tpSJT.lowsecond)
			chCode = CE_FREE_UPDATE_ENTRY;
		else if((tpSJT.lowsecond - lngHisecond1) <= 20 * 60)
			chCode = CE_FREE_UPDATE_ENTRY;
		else
			chCode = CE_NO_UPDATE_ENTRY;
		return chCode;
	}
	//check the TEST mode
	memcpy(out_buf, "\x21\x0a", 2);
	if((chCode = UL_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	//
	tpTxnProductPassEntry.DevUdProductValidity_val.vStartDateTime = toMoto(lngHisecond2 + TIME2000);
	lngHisecond1 = timestr2long(&end_timebcd[1]);
	tpTxnProductPassEntry.DevUdProductValidity_val.vEndDateTime = tpTxnProductPassEntry.passEndDateTime = toMoto(lngHisecond1 + TIME2000);
	tpTxnProductPassEntry.DevUdProductValidity_val.vDuration = toMoto(((tpSZ.DurationType & 0xF) << 12) + tpSZ.validityDuration);
	card_to_location(tpDZperiod.validityOrigin, &lngstation);
	tpTxnProductPassEntry.DevUdProductValidity_val.vOrigin = toMoto(lngstation);
	card_to_location(tpDZperiod.validityDestination, &lngstation);
	tpTxnProductPassEntry.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	//dynamic zone
	xaDynamicZonePeriod._DynamicZone.activeStatus = ((((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1);
#ifdef	DEBUG_PRINT
	PRINTK("active status %02x, write page %02x\n", xaDynamicZonePeriod.buff[0], tpSJT.activeWritepage);
#endif
	memcpy(out_buf, "\x21\x0d", 2);
	//entry time
	cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
	xaDynamicZonePeriod._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
	xaDynamicZonePeriod._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
	xaDynamicZonePeriod._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
	xaDynamicZonePeriod._DynamicZone.startDateTime = lngLosecond & 0x3;
	//
	xaDynamicZonePeriod._DynamicZone.lastDateTime_1 = 0;
	xaDynamicZonePeriod._DynamicZone.lastDateTime = 0;
	//entry station
	lngstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
	if(0 != (chCode = location_to_card(lngstation, &tpDZperiod.lastLocation)))
		return chCode;
	xaDynamicZonePeriod._DynamicZone.lastLocation_1 = (tpDZperiod.lastLocation >> 6) & 0x3F;
	xaDynamicZonePeriod._DynamicZone.lastLocation = tpDZperiod.lastLocation & 0x3f;
	
	xaDynamicZonePeriod._DynamicZone.status = 1;
	//cal the mac2	
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePeriod.buff, 16);
#ifdef DEBUG_PRINT
	PRINTK("Page8:%02x%02x%02x%02x Page9:%02x%02x%02x%02x Page10:%02x%02x%02x%02x Page11:%02x%02x%02x%02x\n", 
		ul_data[8][0], ul_data[8][1], ul_data[8][2], ul_data[8][3], ul_data[9][0], ul_data[9][1], ul_data[9][2], ul_data[9][3], ul_data[10][0], ul_data[10][1], ul_data[10][2], ul_data[10][3], ul_data[11][0], ul_data[11][1], ul_data[11][2], ul_data[11][3]);
	PRINTK("Page12:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", 
		ul_data[12][0], ul_data[12][1], ul_data[12][2], ul_data[12][3], ul_data[13][0], ul_data[13][1], ul_data[13][2], ul_data[13][3], ul_data[14][0], ul_data[14][1], ul_data[14][2], ul_data[14][3], ul_data[15][0], ul_data[15][1], ul_data[15][2], ul_data[15][3]);
#endif
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZperiod.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZperiod.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZperiod.mac2;

	//card sn-4
	ByteToShort(&tpSJT.cardsn, &ul_data[6][2]);
	//transaction amount
	tpSJT.tranamount = 0;
	//cal tac using thread
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	tpTxnProductPassEntry.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	//BefBalance-4
	memset(&out_buf[10], 0x00, 4);
	//balance-4
	memset(&out_buf[14], 0x00, 4);
	//
	tpTxnProductPassEntry.startOfJourney = toMoto(1);
	tpTxnProductPassEntry.firstUseActivation = toMoto(tpDZperiod.activated);

	tpTxnProductPassEntry.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);

	cnt2 = cnt = sizeof(TxnProductPassUseOnEntry_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPassEntry.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPassEntry.SysComHdr_val.reservedField, sh_mac_len - 40);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPassEntry.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPassEntry.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;

	if(cmd_buf[17] != 0x02)
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
	}
	//write dyanmic zone
	for(i = tpSJT.activeWritepage; i < tpSJT.activeWritepage + 4; i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			//sem_wait(&g_samreturn);
			//tpYPT_txn_val.YPT_flag = 0;
			//ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.pYPT_txn, tpYPT_txn_val.YPT_txnlen);
			return CE_WRITE;
		}
	}
	//check the write result 2016/3/28 21:12:57
	if( 0 != UL_Page_Read(tpSJT.activeWritepage, active_page) )
	{
		if(memcmp(active_page, ul_data[tpSJT.activeWritepage], 16) != 0)
			return CE_WRITE;
	}
	
	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memset(&out_buf[10], 0x00, 4);
	//balance-4
	memset(&out_buf[14], 0x00, 4);
	//lock status -1
	out_buf[18] = 0x00;
	//product category
	out_buf[19] = XA_FEETYPE_PERIOD;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);
	
	//
	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductPassEntry.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	if(cmd_buf[17] == 0x02)
	{//ECU must read the transaction record
		memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);
		ch_mac_sel= 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		//sem_wait(&g_samreturn);
		//ee_write_last_record(XA_SJT_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
		tpYPT_txn_val.YPT_flag = 1;
	}else 
	{
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 0;
		reader_status = XA_RW_IDLE;
	}
	return CE_OK;
}
/******************************************
sjt exit
******************************************/
char xa_ul_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char TimeTemp[6];
unsigned char chCode;
unsigned char buf[20];
unsigned char chExitMac[4], tac[4];
long lngHisecond1, lngLosecond1;
unsigned short	shBalance, cnt, i;
unsigned long lngsrcstation, lngdesstation, lngLosecond;

#ifdef DEBUG_PRINT
	PRINTK("\nexit command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("time %02x%02x-%02x-%02x %02x:%02x:%02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12]);
	PRINTK("SN:%02x%02x%02x%02x UD %02x\n", cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17]);
#endif
	/*
	tpTxnProductPurseExit.SysComHdr_val.formatVersion = 0x00000200;
	tpTxnProductPurseExit.SysComHdr_val.txnDateTime = 0x745bc551;
	tpTxnProductPurseExit.SysComHdr_val.sourceParticipantId = 0x2a000000;
	tpTxnProductPurseExit.SysComHdr_val.deviceId = 0x0c23021f;
	tpTxnProductPurseExit.SysComHdr_val.samId = 0x80090000;
	tpTxnProductPurseExit.SysComHdr_val.udsn = 0x29130000;
	tpTxnProductPurseExit.SysComHdr_val.serviceParticipantId = 0x2a000000;
	tpTxnProductPurseExit.SysComHdr_val.deviceLocation = 0x23020009;
	tpTxnProductPurseExit.SysComHdr_val.transactionStatus = 0;
	tpTxnProductPurseExit.SysComHdr_val.cdVersion = 0x12000000;
	tpTxnProductPurseExit.SysComHdr_val.reconciliationDate = 0x0000a1e6;
	tpTxnProductPurseExit.SysComHdr_val.reservedField = 0;
	tpTxnProductPurseExit.SysComHdr_val.udType = 0x03000000;
	tpTxnProductPurseExit.SysComHdr_val.udSubtype = 0x5b000000;
	tpTxnProductPurseExit.SysCardCom_val.cardissuerId = 0x01000000;
	tpTxnProductPurseExit.SysCardCom_val.cardSerialNumber = 0x7f910f00;
	tpTxnProductPurseExit.SysCardCom_val.cardType = 0x03000000;
	tpTxnProductPurseExit.SysCardCom_val.cardLifeCycleCount = 0;
	tpTxnProductPurseExit.SysCardCom_val.cardActionSequenceNumber = 0;
	tpTxnProductPurseExit.SysAppCom_val.applicationProviderId = 0x01000000;
	tpTxnProductPurseExit.SysAppCom_val.applicationSerialNumber = 0x01000000;
	tpTxnProductPurseExit.SysAppCom_val.applicationPersonalliseCat = 0x01000000;
	tpTxnProductPurseExit.SysAppCom_val.appActionSequenceNumber = 0;
	tpTxnProductPurseExit.SysAppCom_val.applicationType = 0x01000000;
	tpTxnProductPurseExit.SysAppCom_val.applicationPassengerType = 0x01000000;
	tpTxnProductPurseExit.SysProductCom_val.productIssuerId = 0x01000000;
	tpTxnProductPurseExit.SysProductCom_val.productSerialNumber = 0x01000000;
	tpTxnProductPurseExit.SysProductCom_val.productType = 0x03000000;
	tpTxnProductPurseExit.SysProductCom_val.productActionSequenceNumber = 0;
	tpTxnProductPurseExit.SysProductCom_val.Ptsn = 0x04000000;
	tpTxnProductPurseExit.SysProductCom_val.invoicePrinted = 0;
	tpTxnProductPurseExit.DevUdJourneyHdr_val.passengerType = 0x01000000;
	tpTxnProductPurseExit.DevUdJourneyHdr_val.currentLocation = 0x23020009;
	tpTxnProductPurseExit.DevUdJourneyHdr_val.tripOriginLocation = 0x15010009;
	tpTxnProductPurseExit.DevUdJourneyHdr_val.tripPreviousLocation = 0x15010009;
	tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue = 0;
	tpTxnProductPurseExit.SysFinDetails_val.transactionValue = 0x90010000;
	tpTxnProductPurseExit.SysFinDetails_val.paymentMethod = 0xff000000;
	tpTxnProductPurseExit.SysFinDetails_val.partialTransactionValue = 0;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavSamId = 0;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavPariticipantId = 0xffffffff;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavDate = 0;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavTxnValue = 0x90010000;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavRemainingValue = 0x90010000;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavPtsn = 0;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavMethodOfPayment = 0x01000000;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.dataIsValid = 0x01000000;
	tpTxnProductPurseExit.DevUdPurseLavHdr_val.invoicePrinted = 0;
	tpTxnProductPurseExit.entryTime = 0x245bc551;
	tpTxnProductPurseExit.cardCaptured = 0x01000000;
	tpTxnProductPurseExit.totalJourneyAmount = 0x90010000;
	tpTxnProductPurseExit.endOfJourney = 0x01000000;
	tpTxnProductPurseExit.firstUseActivation = 0;
	cnt = sizeof(TxnProductPurseUseOnExit_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPurseExit.SysComHdr_val.formatVersion, 40);
	memcpy(&ch_mac_data[40], &tpTxnProductPurseExit.SysComHdr_val.reservedField, sh_mac_len - 8 - 20 - 36);
	memcpy(&ch_mac_data[sh_mac_len - 20 - 36 - 4 - 4], &tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavSamId, 20 + 36);
	sh_mac_len -= (4 + 4);
	xasjt_cal_tac(ch_mac_data, sh_mac_len, tpTxnProductPurseExit.SysComHdr_val.udsn, buf);
	*/
	//
	*out_len = 2;
	memcpy(tpSJT.time_bcd, &cmd_buf[6], 7);
	tpSJT.lowsecond = timestr2long(&cmd_buf[7]);
	memcpy(out_buf, "\x22\x01", 2);

	get_degrade_mode(tpSJT.curstation);

	if(read_tocken_info()!=0)
	{
		return CE_READ;
	}
	memcpy(out_buf, "\x22\x04", 2);
	if((chCode = UL_TellSysCard(tpSZ.productType, NULL)) != 0)
	{
		return chCode;
	}

	memcpy(out_buf, "\x22\x05", 2);
	/*
	xa_MinuteTolocaltime(&TimeTemp[0], tpSZ.cardBaseDataTime, 0x3FFFFF, &lngHisecond1);
	if(memcmp(tpSJT.time_bcd, TimeTemp, 7) > 0)
		return CE_INVADLIDCARD;

	long issue_sec = lngHisecond1;
	long cmd_sec = tpSJT.lowsecond;
	long time_differ = cmd_sec - issue_sec;

	const long LIMIT_EXIT = 1800;
	const long LIMIT_WALFARE = 10800;

	if(tpSZ.productID == XA_FEETYPE_TIMES){
		if(tpDZmultiride.status == 4){
			if(time_differ > LIMIT_EXIT) return CE_INVADLIDCARD;
		}
	}else if(tpSZ.productID == XA_FEETYPE_VALUE){
		if(tpSZ.passengerType == 0x0C){
			if(time_differ > LIMIT_WALFARE) return CE_INVADLIDCARD;
		}
	}
	*/
	
	switch(tpSZ.productID)
	{
	case XA_FEETYPE_VALUE:			//value
		tpTxnProductPurseExit.SysComHdr_val.udType = toMoto(3);
		tpTxnProductPurseExit.SysComHdr_val.udSubtype = toMoto(91);
		tpTxnProductPurseExit.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
		tpTxnProductPurseExit.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]); //toMoto(*(long *)&cmd_buf[13]);
		tpTxnProductPurseExit.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
		//
		tpTxnProductPurseExit.SysProductCom_val.productIssuerId = tpTxnProductPurseExit.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
		tpTxnProductPurseExit.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
		tpTxnProductPurseExit.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);
		
		tpTxnProductPurseExit.SysAppCom_val.applicationPassengerType = toMoto(1);
		
		tpTxnProductPurseExit.SysProductCom_val.productType = toMoto(tpSZ.productType);
		tpTxnProductPurseExit.SysProductCom_val.Ptsn = toMoto(tpDZpurse.transactionSequenceNumber);
	
		if(0 != (chCode = card_to_location(tpDZpurse.origin, &lngsrcstation)))
			return chCode;
		tpTxnProductPurseExit.DevUdJourneyHdr_val.currentLocation = tpTxnProductPurseExit.SysComHdr_val.deviceLocation;
		tpTxnProductPurseExit.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngsrcstation);
		tpTxnProductPurseExit.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductPurseExit.SysComHdr_val.deviceLocation;
		
		memset(&tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavSamId, 0x00, sizeof(DevUdPurseLavHdr_t));
		//default recycle
		tpTxnProductPurseExit.cardCaptured = toMoto(1);
		return xa_ul_exit_purse(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_TIMES:			//rides
		tpTxnProductMultirideExit.SysComHdr_val.udType = toMoto(3);
		tpTxnProductMultirideExit.SysComHdr_val.udSubtype = toMoto(93);
		tpTxnProductMultirideExit.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
		tpTxnProductMultirideExit.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]); //toMoto(*(long *)&cmd_buf[13]);
		tpTxnProductMultirideExit.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
		//
		tpTxnProductMultirideExit.SysProductCom_val.productIssuerId = tpTxnProductMultirideExit.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
		tpTxnProductMultirideExit.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);
		tpTxnProductMultirideExit.SysProductCom_val.productType = toMoto(tpSZ.productType);
		tpTxnProductMultirideExit.SysProductCom_val.Ptsn = toMoto(tpDZmultiride.transactionSequenceNumber);
	
		if(0 != (chCode = card_to_location(tpDZmultiride.lastLocation, &lngsrcstation)))
			return chCode;
		tpTxnProductMultirideExit.SysAppCom_val.applicationPassengerType = tpTxnProductMultirideExit.DevUdJourneyHdr_val.passengerType = toMoto(tpSZ.passengerType);
		tpTxnProductMultirideExit.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideExit.SysComHdr_val.deviceLocation;
		tpTxnProductMultirideExit.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngsrcstation);
		tpTxnProductMultirideExit.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductMultirideExit.SysComHdr_val.deviceLocation;
		
		memset(&tpTxnProductMultirideExit.DevUdMultirideLavHdr_val.lavSamId, 0x00, sizeof(DevUdMultirideLavHdr_t));
		//default recycle
		tpTxnProductMultirideExit.cardCaptured = toMoto(0);
		return xa_ul_exit_multiride(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_PERIOD:		//period
		tpTxnProductPassExit.SysComHdr_val.udType = toMoto(3);
		tpTxnProductPassExit.SysComHdr_val.udSubtype = toMoto(92);
		tpTxnProductPassExit.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
		tpTxnProductPassExit.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[13]); //toMoto(*(long *)&cmd_buf[13]);
		tpTxnProductPassExit.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);

		tpTxnProductPassExit.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);
		//
		tpTxnProductPassExit.SysProductCom_val.productIssuerId = tpTxnProductPassExit.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
		tpTxnProductPassExit.SysProductCom_val.productType = toMoto(tpSZ.productType);
		tpTxnProductPassExit.SysProductCom_val.Ptsn = toMoto(tpDZperiod.transactionSequenceNumber);
	
		if(0 != (chCode = card_to_location(tpDZperiod.lastLocation, &lngsrcstation)))
			return chCode;
		tpTxnProductPassExit.SysAppCom_val.applicationPassengerType = tpTxnProductPassExit.DevUdJourneyHdr_val.passengerType = toMoto(tpSZ.passengerType);
		
		tpTxnProductPassExit.DevUdJourneyHdr_val.currentLocation = tpTxnProductPassExit.SysComHdr_val.deviceLocation;
		tpTxnProductPassExit.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngsrcstation);
		tpTxnProductPassExit.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductPassExit.SysComHdr_val.deviceLocation;
		
		memset(&tpTxnProductPassExit.DevUdPassLavHdr_val.lavSamId, 0x00, sizeof(DevUdPassLavHdr_t));
		return xa_ul_exit_period(cmd_buf, out_buf, out_len);
	default:
		return CE_NON_FEETYPE;
	}

}

char xa_ul_exit_purse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode;
unsigned long lngsrcstation, lngdesstation, lngLosecond;
unsigned short	shBalance, cnt, i;
unsigned char buf[20], start_timebcd[7], end_timebcd[7], entry_timebcd[7];
unsigned char chExitMac[4], tac[4];

	//check whether the ticket is issued or not
	if(tpDZpurse.cardStatus != XA_SJT_CARD_NOMAL)
		return CE_ISSUED;

	if(tpDZpurse.activated == 0)
		return CE_NONACTIVED;
	xa_daytodate(tpSZ.cardBaseDataTime, tpDZpurse.validityStartDate, &lngLosecond, &start_timebcd[0]);
	lngLosecond += (tpSZ.validityDuration * 24 * 3600);
	long2timestr(lngLosecond, &end_timebcd[0]);

	tpSJT.balance = tpSZ.purchaseValue;
	tpSJT.tranamount = tpSJT.balance;
	{
		if((chCode = UL_TellEntryMAC(tpDZpurse.status, 0xff)) != 0)
		{
			memcpy(out_buf, "\x22\x07", 2);
			goto label_refuse_to_exit;
		}
		else
		{
			//超程是否修改DZ区
			if((chCode = UL_TellOverRide(tpSJT.time_bcd, tpDZpurse.origin, &tpSJT.curstation[0], tpDZpurse.lastLocation, tpDZpurse.status, tpDZpurse.remainingValue)) != 0)
			{
				/*unsigned long lngsrctemp,lngdestemp;
				unsigned short shFaretemp,shlastcardtemp;
				unsigned char * desstation = tpSJT.curstation;
				//2026.06.17
				if(0 != (chCode = card_to_location(tpDZpurse.origin, &lngsrctemp)))//1106
					memcpy(out_buf, "\x22\x18", 2);
				lngdestemp = 0x09000000 + (desstation[0] << 8) + desstation[1];
				if(0 != (chCode = cal_station_fare(tpTicketDef.FareCodeTableId, lngsrctemp, lngdestemp, &shFaretemp)))//1108
					memcpy(out_buf, "\x22\x28", 2);
				if(0 != (chCode = cal_fare_value(tpSJT.time_bcd, &tpTicketDef, shFaretemp, XA_PASSENGER_ADULT, &tpSysPrice)))//1108
					memcpy(out_buf, "\x22\x38", 2);
				if(0 != (chCode = location_to_card(lngdestemp, &shlastcardtemp)))//1106
					memcpy(out_buf, "\x22\x48", 2);*/
				memcpy(out_buf, "\x22\x08", 2);
				goto label_refuse_to_exit;
			}
			//renewflag = 0
			xa_MinuteTolocaltime(&entry_timebcd[0], tpSZ.cardBaseDataTime, tpDZpurse.startDateTime, &lngLosecond);
			lngLosecond += (tpDZpurse.lastDateTime * 60);
			tpTxnProductPurseExit.entryTime = toMoto(lngLosecond + TIME2000 - ZONE8);
			//2018/12/10 9:43:29
			long2timestr(lngLosecond, &entry_timebcd[0]);
			if((chCode = cal_overtime(entry_timebcd, tpSJT.time_bcd, 0, 0)) != 0)
			{
				memcpy(out_buf, "\x22\x09", 2);
				goto label_refuse_to_exit;
			}
			//
			memcpy(out_buf, "\x22\x0a", 2);
     		if((chCode = UL_TellTesting(tpCmdInit.test)) != 0)
    		{
				return chCode;
     		}
		}
	}
	ByteToShort(&tpSJT.cardsn, &ul_data[6][2]);
	//dynamic zone
	xaDynamicZonePurse._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;;
	memcpy(out_buf, "\x21\x0d", 2);
	//exit time
	cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
	xaDynamicZonePurse._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
	xaDynamicZonePurse._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
	xaDynamicZonePurse._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
	xaDynamicZonePurse._DynamicZone.startDateTime = lngLosecond & 0x3;
	
	xaDynamicZonePurse._DynamicZone.lastDateTime_1 = 0;
	xaDynamicZonePurse._DynamicZone.lastDateTime = 0;
	//exit station
	lngsrcstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
	if(0 != (chCode = location_to_card(lngsrcstation, &tpDZpurse.lastLocation)))
		return chCode;
	xaDynamicZonePurse._DynamicZone.lastLocation_1 = (tpDZpurse.lastLocation >> 6) & 0x3F;
	xaDynamicZonePurse._DynamicZone.lastLocation = tpDZpurse.lastLocation & 0x3f;
	//travel status
	xaDynamicZonePurse._DynamicZone.status = 2;
	
	if(tpwaivermode.cur_sta_failure)
	{//MUST reserve the remaining value for STATOIN FAILURE
		tpSJT.tranamount = 0;
		xaDynamicZonePurse._DynamicZone.status = 3;
	}else
	{//remaining value MUST be set NOT ZERO
		xaDynamicZonePurse._DynamicZone.cardStatus = XA_SJT_CARD_RECYCLE;
		tpDZpurse.remainingValue = 0;
		xaDynamicZonePurse._DynamicZone.remainingValue_1 = 0;
		xaDynamicZonePurse._DynamicZone.remainingValue_2 = tpDZpurse.remainingValue >> 8;
		xaDynamicZonePurse._DynamicZone.remainingValue = tpDZpurse.remainingValue & 0xFF;
	}
	
	tpDZpurse.totalPurchaseValue += tpDZpurse.remainingValue;
	xaDynamicZonePurse._DynamicZone.totalPurchaseValue_1 = (tpDZpurse.totalPurchaseValue >> 8) & 0x7F;
	xaDynamicZonePurse._DynamicZone.totalPurchaseValue = (unsigned char)tpDZpurse.totalPurchaseValue;
	//cal the mac2	
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePurse.buff, 16);
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZpurse.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZpurse.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZpurse.mac2;
	//return and transaction record
	tpTxnProductPurseEntry.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	tpTxnProductPurseExit.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpDZpurse.remainingValue);
	tpTxnProductPurseExit.SysFinDetails_val.transactionValue = toMoto(tpSJT.tranamount);
	tpTxnProductPurseExit.SysFinDetails_val.paymentMethod = toMoto(1);
	tpTxnProductPurseExit.SysFinDetails_val.partialTransactionValue = 0;
	//
	tpTxnProductPurseExit.cardCaptured = 0;
	if(tpTicketDef.CanBeRecycled)
	{
		tpTxnProductPurseExit.cardCaptured = toMoto(0x01);
		if(tpwaivermode.cur_sta_failure)
		{//STATION FAILURE
			if(tpTicketDef.IsTicketCapturedIfTrainFault)
				tpTxnProductPurseExit.cardCaptured = toMoto(0x01);
			else
				tpTxnProductPurseExit.cardCaptured = 0x00;
		}
	}
	tpTxnProductPurseExit.endOfJourney= toMoto(1);
	tpTxnProductPurseExit.totalJourneyAmount = 0;
	tpTxnProductPurseExit.firstUseActivation = toMoto(tpDZpurse.activated);
	tpTxnProductPurseExit.DevUdJourneyHdr_val.passengerType = toMoto(tpSZ.passengerType);

	//mac
	tpTxnProductPurseExit.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
	cnt = sizeof(TxnProductPurseUseOnExit_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPurseExit.SysComHdr_val.formatVersion, 40);
	memcpy(&ch_mac_data[40], &tpTxnProductPurseExit.SysComHdr_val.reservedField, sh_mac_len - 8 - 20 - 36);
	memcpy(&ch_mac_data[sh_mac_len - 20 - 36 - 4 - 4], &tpTxnProductPurseExit.DevUdPurseLavHdr_val.lavSamId, 20 + 36);
	sh_mac_len -= (4 + 4);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPurseExit.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPurseExit.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;
	//write dyanmic zone
	for(i = tpSJT.activeWritepage; i < tpSJT.activeWritepage + 4; i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			//sem_wait(&g_samreturn);
			//tpYPT_txn_val.YPT_flag = 0;
			//ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.pYPT_txn, tpYPT_txn_val.YPT_txnlen);
			//reader_status = XA_RW_IDLE;
			return CE_WRITE;
		}
	}

	//udsn-1
	out_buf[0] = 1;
	//recycle-1:00-NORECYCLE;01:RECYCLE
	out_buf[1] = toMoto(tpTxnProductPurseExit.cardCaptured);
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &tpDZpurse.remainingValue, 4);
	//lock status -1
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_VALUE;
	//rfu-14
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductPurseExit.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//calculate the TAC
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);
	if(cmd_buf[17] == 0x02)
	{
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		//ee_write_last_record(XA_SJT_FAMILY, 1, &out_buf[33], cnt);
		tpYPT_txn_val.YPT_flag = 1;
		reader_status = XA_RW_RECORD;
	}else 
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		tpYPT_txn_val.YPT_flag = 0;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		reader_status = XA_RW_IDLE;
	}
	return CE_OK;
	
label_refuse_to_exit:
#ifdef DEBUG_PRINT
	PRINTK("exit sta %02x reject code %02x %02x\n", ul_data[15][0], ul_data[15][1], ul_data[15][2]);
#endif
#ifdef DEBUG_PRINT
	PRINTK("exit mac is %02x\n", chExitMac[0]);
#endif
	return chCode;
}

char xa_ul_exit_multiride(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode;
unsigned long lngsrcstation, lngdesstation, lngLosecond;
unsigned short	shBalance, cnt, i;
unsigned char buf[20], start_timebcd[7], end_timebcd[7], entry_timebcd[7];
unsigned char chExitMac[4], tac[4];
unsigned long lngAftBalance, lngvaliditystation;

	if(tpDZmultiride.activated == 0)
		return CE_NONACTIVED;
	//check whether the ticket is issued or not
	if(tpDZmultiride.cardStatus != XA_SJT_CARD_NOMAL)
		return CE_ISSUED;
	xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZmultiride.validityStartDateTime, &lngLosecond);
	//
	memset(&start_timebcd[4], 0x00, 3);
	lngLosecond = timestr2long(&start_timebcd[1]);
	tpTxnProductMultirideExit.DevUdProductValidity_val.vStartDateTime = toMoto(lngLosecond);
	lngLosecond += (tpSZ.validityDuration * 24 * 3600);
	tpTxnProductMultirideExit.DevUdProductValidity_val.vEndDateTime = toMoto(lngLosecond);
	tpTxnProductMultirideExit.DevUdProductValidity_val.vDuration = ((tpSZ.DurationType & 0xF) << 12) + tpSZ.validityDuration;
	tpTxnProductMultirideExit.DevUdProductValidity_val.vDestination = tpTxnProductMultirideExit.DevUdProductValidity_val.vOrigin = tpTxnProductMultirideExit.SysComHdr_val.deviceLocation;
	
	long2timestr(lngLosecond, &end_timebcd[0]);
	tpSJT.tranamount = 1;
	//remaining rides must more than 1
	tpSJT.balance = tpDZmultiride.remainingRides;
	if(tpDZmultiride.remainingRides < tpSJT.tranamount)
		return CE_ENOUGH_BALANCE;
	tpDZmultiride.remainingRides = lngAftBalance = tpSJT.balance - tpSJT.tranamount;
	xa_MinuteTolocaltime(&entry_timebcd[0], tpSZ.cardBaseDataTime, tpDZmultiride.startDateTime, &lngLosecond);
	lngLosecond += (tpDZmultiride.lastDateTime * 60);
	if(tpSZ.productType != Type_Exit_Ticket)
	{
		if((chCode = UL_TellEntryMAC(tpDZmultiride.status, 0xff)) != 0)
		{
			memcpy(out_buf, "\x22\x07", 2);
			goto label_refuse_to_exit;
		}
		else
		{
			//check the validity origin
			if(0 != (chCode = card_to_location(tpDZmultiride.validityDestination, &lngvaliditystation)))
				return chCode;
			if(0 != (chCode = xa_ValidateArea(tpCmdInit.curstation, lngvaliditystation)))
				return chCode;
			//over time
			tpTxnProductMultirideExit.entryTime = toMoto(lngLosecond + TIME2000 - ZONE8);
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				//2018/12/10 9:43:29
				long2timestr(lngLosecond, &entry_timebcd[0]);
				if((chCode = cal_overtime(entry_timebcd, tpSJT.time_bcd, 0, 0)) != 0)
				{
					memcpy(out_buf, "\x22\x09", 2);
					goto label_refuse_to_exit;
				}
			}
			//
			memcpy(out_buf, "\x22\x0a", 2);
     		if((chCode = UL_TellTesting(tpCmdInit.test)) != 0)
    		{
				return chCode;
     		}
     		//failure MODE NO FEE
     		if(tpwaivermode.cur_sta_failure)
     		{
     			tpSJT.tranamount = 0;
     			xaDynamicZoneMultiride._DynamicZone.status = 3;
     		}
			//dynamic zone
			tpTxnProductMultirideExit.cardCaptured = 0;
			//if the current station is set to failure mode then issue & entry mac can't be set to zero and filure mode must be written
			if(tpTicketDef.CanBeRecycled)
			{
				tpTxnProductMultirideExit.cardCaptured = toMoto(1);
				if(tpwaivermode.cur_sta_failure)
				{
					if(tpTicketDef.IsTicketCapturedIfTrainFault)
						tpTxnProductMultirideExit.cardCaptured = toMoto(1);
					else
					{
						tpSJT.tranamount = 0;
						tpTxnProductMultirideExit.cardCaptured = 0;
					}
				}
			}
		}
	}else
	{//exit ticket
  		//比较当前站点号
		if(0 != (chCode = card_to_location(tpDZmultiride.lastLocation, &lngsrcstation)))
			return chCode;
		if((lngsrcstation & 0xFF00FFFF) != (tpCmdInit.curstation & 0xFF00FFFF))
		{
			if(0 != check_same_station(tpCmdInit.curstation, tpDZmultiride.lastLocation))
				return CE_NOT_ISSUEDSTATION;
		}
		//over time
		//tpTxnProductMultirideExit.entryTime = toMoto(lngLosecond + TIME2000 - ZONE8);
		//if(tpTicketDef.IgnoreMaxJourneyTime == 0)
		{
			//2026/6/15 9:43:29
			if (tpSZ.productType == 0x0C) {
				long2timestr(lngLosecond, &start_timebcd[0]);
				if((chCode = CheckTicketOvertime(start_timebcd, tpSJT.time_bcd, 1)) != 0)
				{
					memcpy(out_buf, "\x22\x09", 2);
					goto label_refuse_to_exit;
				}
		
			}else{
				long2timestr(lngLosecond, &start_timebcd[0]);
				if((chCode = CheckTicketOvertime(start_timebcd, tpSJT.time_bcd, 0)) != 0)
				{
					memcpy(out_buf, "\x22\x09", 2);
					goto label_refuse_to_exit;
				}
			}
		}
			//
		memcpy(out_buf, "\x22\x11", 2);
		if((chCode = xa_TellDate(tpSJT.time_bcd, start_timebcd, end_timebcd, tpDZmultiride.status, entry_timebcd, tpSZ.DurationType)) != 0)
		{
			return chCode;
		}
		tpTxnProductMultirideExit.cardCaptured = toMoto(1);
	}
	//
	xaDynamicZoneMultiride._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;;
	//no rides 
	if((tpSJT.balance - tpSJT.tranamount) == 0)
		xaDynamicZoneMultiride._DynamicZone.cardStatus = XA_SJT_CARD_RECYCLE;
	
	memcpy(out_buf, "\x21\x0d", 2);
	//exit time
	cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
	xaDynamicZoneMultiride._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
	xaDynamicZoneMultiride._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
	xaDynamicZoneMultiride._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
	xaDynamicZoneMultiride._DynamicZone.startDateTime = lngLosecond & 0x3;
	
	xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 = 0;
	xaDynamicZoneMultiride._DynamicZone.lastDateTime = 0;
	//exit station
	lngsrcstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
	if(0 != (chCode = location_to_card(lngsrcstation, &tpDZmultiride.lastLocation)))
		return chCode;
	xaDynamicZoneMultiride._DynamicZone.lastLocation_1 = (tpDZmultiride.lastLocation >> 6) & 0x3F;
	xaDynamicZoneMultiride._DynamicZone.lastLocation = tpDZmultiride.lastLocation & 0x3f;
	//
	xaDynamicZoneMultiride._DynamicZone.status = 2;
	xaDynamicZoneMultiride._DynamicZone.remainingRides_1 = tpDZmultiride.remainingRides >> 1;
	xaDynamicZoneMultiride._DynamicZone.remainingRides = tpDZmultiride.remainingRides & 0x1;
	
	tpDZmultiride.totalValueUsed += tpDZmultiride.remainingRides;
	xaDynamicZoneMultiride._DynamicZone.totalValueUsed = (tpDZmultiride.totalValueUsed & 0x1);
	//cal the mac2	
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZoneMultiride.buff, 16);
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZmultiride.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZmultiride.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZmultiride.mac2;
	//return and transaction record
	tpTxnProductMultirideExit.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.numRides = toMoto(tpSJT.tranamount);
	tpTxnProductMultirideExit.DevUdMultirideCommonHdr_val.remainingRides = toMoto(tpDZmultiride.remainingRides);
	
	//
	tpTxnProductMultirideExit.endOfJourney= toMoto(1);
	tpTxnProductMultirideExit.valuePerRide = 0;
	tpTxnProductMultirideExit.firstUseActivation = toMoto(tpDZmultiride.activated);

	//mac
	tpTxnProductMultirideExit.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
	cnt = sizeof(TxnProductMultirideUseOnExit_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductMultirideExit.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductMultirideExit.SysComHdr_val.reservedField, sh_mac_len - 40);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductMultirideExit.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductMultirideExit.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;
	if(cmd_buf[17] != 0x02)
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
	}
	//write dyanmic zone
	for(i = tpSJT.activeWritepage; i < tpSJT.activeWritepage + 4; i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			//sem_wait(&g_samreturn);
			//tpYPT_txn_val.YPT_flag = 0;
			//ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.pYPT_txn, tpYPT_txn_val.YPT_txnlen);
			reader_status = XA_RW_IDLE;
			return CE_WRITE;
		}
	}

	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = toMoto(tpTxnProductMultirideExit.cardCaptured);
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &lngAftBalance, 4);
	//lock status -1
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_TIMES;
	//rfu-14
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductMultirideExit.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	if(cmd_buf[17] == 0x02)
	{
		memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);
		tpYPT_txn_val.YPT_flag = 1;
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		//ee_write_last_record(XA_SJT_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
	}else 
	{
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		tpYPT_txn_val.YPT_flag = 0;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		reader_status = XA_RW_IDLE;
	}
	return CE_OK;
	
label_refuse_to_exit:
	return chCode;
}

char xa_ul_exit_period(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode;
unsigned long lngsrcstation, lngdesstation, lngLosecond;
unsigned short	shBalance, cnt, i;
unsigned char buf[20], start_timebcd[7], end_timebcd[7], entry_timebcd[7];
unsigned char chExitMac[4], tac[4];
unsigned long lngAftBalance, lngstation;

	if(tpDZperiod.activated == 0)
		return CE_NONACTIVED;
	//check whether the ticket is issued or not
	if(tpDZperiod.cardStatus != XA_SJT_CARD_NOMAL)
		return CE_ISSUED;
	//
	xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZperiod.validityStartDateTime, &lngLosecond);
	tpTxnProductPassExit.DevUdProductValidity_val.vStartDateTime = toMoto(lngLosecond + TIME2000);
	xa_DurationTolocaltime(lngLosecond, tpSZ.DurationType, tpSZ.validityDuration, end_timebcd);
	lngLosecond = timestr2long(&end_timebcd[1]);
	tpTxnProductPassExit.DevUdProductValidity_val.vEndDateTime = tpTxnProductPassExit.passEndDateTime = toMoto(lngLosecond + TIME2000);
	tpTxnProductPassExit.DevUdProductValidity_val.vDuration = toMoto(((tpSZ.DurationType & 0xF) << 12) + tpSZ.validityDuration);
	card_to_location(tpDZperiod.validityOrigin, &lngstation);
	tpTxnProductPassExit.DevUdProductValidity_val.vOrigin = toMoto(lngstation);
	card_to_location(tpDZperiod.validityDestination, &lngstation);
	tpTxnProductPassExit.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	
	
	if((chCode = UL_TellEntryMAC(tpDZperiod.status, 0xff)) != 0)
	{
		memcpy(out_buf, "\x22\x07", 2);
		goto label_refuse_to_exit;
	}
	else
	{
		//entry time-4
		memcpy(&out_buf[24], ul_data[0xb], 4);
		//entry station-2
		memcpy(tpSJT.laststation, &out_buf[28], 2);
		xa_MinuteTolocaltime(&entry_timebcd[0], tpSZ.cardBaseDataTime, tpDZperiod.startDateTime, &lngLosecond);
		lngLosecond += (tpDZperiod.lastDateTime * 60);
		tpTxnProductPassExit.entryTime = toMoto(lngLosecond + TIME2000 - ZONE8);
		if(tpTicketDef.IgnoreMaxJourneyTime == 0)
		{
			//2018/12/10 9:43:29
			long2timestr(lngLosecond, &entry_timebcd[0]);
			if((chCode = cal_overtime(entry_timebcd, tpSJT.time_bcd, 0, 0)) != 0)
			{
				memcpy(out_buf, "\x22\x09", 2);
				goto label_refuse_to_exit;
			}
		}
		//
		memcpy(out_buf, "\x22\x0a", 2);
   		if((chCode = UL_TellTesting(tpCmdInit.test)) != 0)
   		{
			return chCode;
   		}
	}
	
	//dynamic zone
	//if the current station is set to failure mode then issue & entry mac can't be set to zero and filure mode must be written
	if(tpwaivermode.cur_sta_failure)
	{
		xaDynamicZonePeriod._DynamicZone.status = 3;
		out_buf[34] = FLAG_NONRECYCLE;
	}else
	{
	}
	//
	xaDynamicZonePeriod._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;;
	
	memcpy(out_buf, "\x21\x0d", 2);
	//exit time
	cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
	xaDynamicZonePeriod._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
	xaDynamicZonePeriod._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
	xaDynamicZonePeriod._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
	xaDynamicZonePeriod._DynamicZone.startDateTime = lngLosecond & 0x3;
	
	xaDynamicZonePeriod._DynamicZone.lastDateTime_1 = 0;
	xaDynamicZonePeriod._DynamicZone.lastDateTime = 0;
	//exit station
	lngsrcstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
	if(0 != (chCode = location_to_card(lngsrcstation, &tpDZperiod.lastLocation)))
		return chCode;
	xaDynamicZonePeriod._DynamicZone.lastLocation_1 = (tpDZperiod.lastLocation >> 6) & 0x3F;
	xaDynamicZonePeriod._DynamicZone.lastLocation = tpDZperiod.lastLocation & 0x3f;
	//
	xaDynamicZonePeriod._DynamicZone.status = 2;
	
	//cal the mac2	
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePeriod.buff, 16);
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZperiod.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZperiod.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZperiod.mac2;
	//return and transaction record
	tpTxnProductPassExit.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	
	//
	tpTxnProductPassExit.endOfJourney= toMoto(1);
	tpTxnProductPassExit.firstUseActivation = toMoto(tpDZperiod.activated);

	//mac
	tpTxnProductPassExit.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
	cnt = sizeof(TxnProductPassUseOnExit_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPassExit.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPassExit.SysComHdr_val.reservedField, sh_mac_len - 40);
	//bakup the TXN
	g_sha1txnsn = tpTxnProductPassExit.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPassExit.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 1;
	if(cmd_buf[17] != 0x02)
	{
		ch_mac_sel = 4;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
	}
	//write dyanmic zone
	for(i = tpSJT.activeWritepage; i < tpSJT.activeWritepage + 4; i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			//sem_wait(&g_samreturn);
			//tpYPT_txn_val.YPT_flag = 0;
			//ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.pYPT_txn, tpYPT_txn_val.YPT_txnlen);
			reader_status = XA_RW_IDLE;
			return CE_WRITE;
		}
	}

	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	if(tpTicketDef.CanBeRecycled)
	{
		out_buf[1] = 0x01;
		if(tpwaivermode.cur_sta_failure)
		{//STATION FAILURE
			if(tpTicketDef.IsTicketCapturedIfTrainFault)
				out_buf[1] = 0x01;
			else
				out_buf[1] = 0x00;
		}
	}
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memset(&out_buf[10], 0x00, 4);
	//balance-4
	memset(&out_buf[14], 0x00, 4);
	//lock status -1
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_PERIOD;
	//rfu-14
	memcpy(&out_buf[20], end_timebcd, 6);
	memset(&out_buf[26], 0x00, 7);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnProductPassExit.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//
	memcpy(&tpYPT_txn_val.YPT_txn, &out_buf[33], tpYPT_txn_val.YPT_txnlen);
	
	if(cmd_buf[17] == 0x02)
	{
		tpYPT_txn_val.YPT_flag = 1;
		ch_mac_sel = 14;
		sem_init(&g_samreturn, 0, 0);
		sem_post(&g_samcalwait);
		//ee_write_last_record(XA_SJT_FAMILY, 1, &out_buf[33], cnt);
		reader_status = XA_RW_RECORD;
	}else 
	{
		sem_wait(&g_samreturn);
		(*out_len) += cnt;
		tpYPT_txn_val.YPT_flag = 0;
		//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
		reader_status = XA_RW_IDLE;
	}
	return CE_OK;
	
label_refuse_to_exit:
	return chCode;
}


/******************************************
sjt inquire
******************************************/
char xa_ul_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode, chRejectCode, chSJTRejectCode, chLastRejectCode, chDateRejectCode;
unsigned char buf[20], chByte, chProduct, chStartDate[7];
unsigned char chExitMac[4], blnOvertime, blnOverfare;
unsigned long lngHisecond1, lngLosecond1, lngLong, lngLocation, lngcurLocation;
unsigned short shShort, i;
unsigned long 	lngPenalty, lngOvertimePenalty, lngOverfarePenalty;

#ifdef DEBUG_PRINT
	PRINTK("\nul inquire command is %02x%02x and length is %02x%02x\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("check %02x function %02x Fee %02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8]);
	PRINTK("time %02x%02x-%02x-%02x %02x:%02x:%02x antelena %02x flag %02x\n", 
		cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12], cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16], cmd_buf[17]);
	
	PRINTK("station:%02x%02x%02x%02x history %02x\n", cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22]);
#endif
	*out_len = 2;
	//check the parameter valid
	memcpy(tpSJT.time_bcd, &cmd_buf[9], 7);
	tpSJT.lowsecond = timestr2long(&cmd_buf[10]);
	memcpy(out_buf, "\x24\x06", 2);
	if(read_tocken_info()!=0 )
	{
		return CE_READ;
	}
	chRejectCode = chSJTRejectCode = chLastRejectCode = 0;
#ifdef	DEBUG_PRINT
	//source 
	PRINTK("Page0:%02x%02x%02x%02x Page1:%02x%02x%02x%02x Page2:%02x%02x%02x%02x Page3:%02x%02x%02x%02x\n", 
		ul_data[0][0], ul_data[0][1], ul_data[0][2], ul_data[0][3], ul_data[1][0], ul_data[1][1], ul_data[1][2], ul_data[1][3], ul_data[2][0], ul_data[2][1], ul_data[2][2], ul_data[2][3], ul_data[3][0], ul_data[3][1], ul_data[3][2], ul_data[3][3]);
	PRINTK("Page4:%02x%02x%02x%02x Page5:%02x%02x%02x%02x Page6:%02x%02x%02x%02x Page7:%02x%02x%02x%02x\n", 
		ul_data[4][0], ul_data[4][1], ul_data[4][2], ul_data[4][3], ul_data[5][0], ul_data[5][1], ul_data[5][2], ul_data[5][3], ul_data[6][0], ul_data[6][1], ul_data[6][2], ul_data[6][3], ul_data[7][0], ul_data[7][1], ul_data[7][2], ul_data[7][3]);
	PRINTK("Page8:%02x%02x%02x%02x Page9:%02x%02x%02x%02x Page10:%02x%02x%02x%02x Page11:%02x%02x%02x%02x\n", 
		ul_data[8][0], ul_data[8][1], ul_data[8][2], ul_data[8][3], ul_data[9][0], ul_data[9][1], ul_data[9][2], ul_data[9][3], ul_data[10][0], ul_data[10][1], ul_data[10][2], ul_data[10][3], ul_data[11][0], ul_data[11][1], ul_data[11][2], ul_data[11][3]);
	PRINTK("Page12:%02x%02x%02x%02x Page13:%02x%02x%02x%02x Page14:%02x%02x%02x%02x Page15:%02x%02x%02x%02x\n", 
		ul_data[12][0], ul_data[12][1], ul_data[12][2], ul_data[12][3], ul_data[13][0], ul_data[13][1], ul_data[13][2], ul_data[13][3], ul_data[14][0], ul_data[14][1], ul_data[14][2], ul_data[14][3], ul_data[15][0], ul_data[15][1], ul_data[15][2], ul_data[15][3]);
	PRINTK("phyical ID %02x%02x%02x%02x\n-----static-----\n", ul_data[3][0], ul_data[3][1], ul_data[3][2], ul_data[3][3]);
	chByte = ((ul_data[4][0] & 0xc0) >> 6); PRINTK("ver:%02x ", chByte);
	shShort = (ul_data[4][0] & 0x3f) << 4;  shShort |= ((ul_data[4][1] & 0xf0) >> 4);  PRINTK("lifecycle %04x ", shShort);
	chByte = (ul_data[4][1] & 0xE) >> 1;  PRINTK("keySet %02x ", chByte);
	shShort = (ul_data[4][1] & 0x1) << 8; shShort |= (ul_data[4][2]);  PRINTK("InitDate%04x\n", shShort);
	shShort = ul_data[4][3] << 2; shShort |= ((ul_data[5][0] & 0xC0) >> 6); PRINTK("batch %04x ", shShort);
	chByte = (ul_data[5][0] & 0x20) >> 5; PRINTK("test %02x ", chByte);
	chByte = (ul_data[5][0] & 0x1F) << 1; chByte |= ((ul_data[5][1] & 0x80) >> 7); PRINTK("ticket %02x ", chByte);
	chByte = (ul_data[5][1] & 0x70) >> 4; PRINTK("passenger %02x ", chByte);
	chProduct = (ul_data[5][1] & 0xC) >> 2; PRINTK("product %02x ", chProduct);
	lngLong = (ul_data[5][1] & 0x3) << 15; lngLong |= (ul_data[5][2] << 7); lngLong |= ((ul_data[5][3] & 0xFE) >> 1); PRINTK("balance %06x ", lngLong);
	lngLong = (ul_data[5][3] & 0x1) << 16; lngLong |= (ul_data[6][0] << 8); lngLong |= (ul_data[6][1]); PRINTK("rfu %06x ", lngLong);
	chByte = (ul_data[6][2] & 0xC0) >> 6; PRINTK("pay method %02x\n", chByte);
	chByte = (ul_data[6][2] & 0x30) >> 4; PRINTK("rfu %02x ", chByte);
	shShort = (ul_data[6][2] & 0xf) << 4; shShort |= ((ul_data[6][3] & 0xe0) >> 5); PRINTK("validduration %04x ", shShort);
	chByte = ul_data[6][3] & 0x1f; PRINTK("rfu %02x ", chByte);
	PRINTK("mac1 %02x%02x%02x%02x\n------static end-----\n", ul_data[7][0], ul_data[7][1], ul_data[7][2], ul_data[7][3]);
	//bit2struct_StaticZone(ul_data[4], &tpSZ.Version);
	PRINTK("ver:%02x lifecycle %04x keySet %02x InitDate %04x batch %04x test %02x ticket %02x passenger %02x \nproduct %02x balance %04x rfu1 %04x pay %02x rfu2 %02x valid %02x rfu3 %02x\n", 
			tpSZ.Version, tpSZ.lifecycleCount, tpSZ.keySetNumber, tpSZ.cardBaseDataTime, tpSZ.cardBatchNumber, tpSZ.testMode, tpSZ.productType,
			tpSZ.passengerType, tpSZ.productID, tpSZ.purchaseValue, tpSZ.rfu1, tpSZ.lavPaymentMethod, tpSZ.DurationType, tpSZ.validityDuration, tpSZ.productIssuerId);
	
	chByte = (ul_data[8][0] & 0x80) >> 7; PRINTK("active flag %02x ", chByte);
	chByte = (ul_data[8][0] & 0x60) >> 5; PRINTK("card status %02x ", chByte);
	chByte = (ul_data[8][0] & 0x1F) << 3; chByte |= ((ul_data[8][1] & 0xE0) >> 5); PRINTK("TSN:%02x ", chByte);
	chByte = (ul_data[8][1] & 0x10) >> 4; PRINTK("rfu %02x ", chByte);
	lngLong = (ul_data[8][1] & 0xf) << 18; lngLong |= (ul_data[8][2] << 10); lngLong |= (ul_data[8][3] << 4); lngLong |= ((ul_data[9][0] & 0xC0) >> 6); PRINTK("starttime%06x ", lngLong);
	chByte = (ul_data[9][0] & 0x3F) << 2; chByte |= ((ul_data[9][1] & 0xC0) >> 6); PRINTK("lasttime %02x ", chByte);
	shShort = (ul_data[9][1] & 0x3F) << 6; shShort |= ((ul_data[9][2] & 0xFC) >> 2); PRINTK("laststation %04x ", shShort);
	chByte = (ul_data[9][2] & 0x3); PRINTK("transfer %02x ", chByte);
	chByte = (ul_data[9][3] & 0xE0) >> 5; PRINTK("status %02x \n", chByte);
	if(chProduct == 0x02)
	{
	//static ticket info
	lngLong = (ul_data[9][3] & 0x1F) << 17; lngLong |= (ul_data[10][0] << 9); lngLong |= (ul_data[10][1] << 1); lngLong |= ((ul_data[10][2] & 0x80) >> 7); PRINTK("start time %06x ", lngLong);
	shShort = (ul_data[10][2] & 0x7F) << 5; shShort |= ((ul_data[10][3] & 0xF8) >> 3); PRINTK("origin %04x ", shShort);
	shShort = (ul_data[10][3] & 0x7) << 9; shShort |= (ul_data[11][0] << 1); shShort |= ((ul_data[11][1] & 0x80) >> 7); PRINTK("destion %04x ", shShort);
	chByte = (ul_data[11][1] & 0x40) >> 6; PRINTK("activated %02x ", chByte);
	shShort = (ul_data[11][1] & 0x3f) << 4; shShort |= ((ul_data[11][2] & 0xF0) >> 4); PRINTK("rfu %04x \n", shShort);
	}else if(chProduct == 0x03)
	{
	//times ticket info
	lngLong = (ul_data[9][3] & 0x1F) << 17; lngLong |= (ul_data[10][0] << 9); lngLong |= (ul_data[10][1] << 1); lngLong |= ((ul_data[10][2] & 0x80) >> 7); PRINTK("start time %06x ", lngLong);
	shShort = (ul_data[10][2] & 0x7F) << 1; shShort |= ((ul_data[10][3] & 0x80) >> 7); PRINTK("times %02x ", shShort);
	shShort = (ul_data[10][3] & 0x7F) << 5; shShort |= ((ul_data[11][0] & 0xF8) >> 3);  PRINTK("origin %04x ", shShort);
	shShort = (ul_data[11][0] & 0x7) << 9; shShort |= (ul_data[11][1] << 1); shShort |= ((ul_data[11][2] & 0x80) >> 7); PRINTK("destion %04x", shShort);
	chByte = (ul_data[11][2] & 0x40) >> 6; PRINTK("activated %02x ", chByte);
	chByte = (ul_data[11][2] & 0x20) >> 5; PRINTK("totalused %02x ", chByte);
	chByte = (ul_data[11][2] & 0x10) >> 4; PRINTK("rfu %04x\n", shShort);
	}else if(chProduct == 0x01)
	{
	//purse ticket info
	lngLong = (ul_data[9][3] & 0x1F) << 7; lngLong |= ((ul_data[10][0] & 0xFE) >> 1); PRINTK("start time %04x ", lngLong);
	lngLong = (ul_data[10][0] & 0x1) << 16; lngLong |= (ul_data[10][1] << 8); lngLong |= ul_data[10][2]; PRINTK("balance %06x ", lngLong);
	chByte = (ul_data[10][3] & 0x80) >> 7; PRINTK("actived %04x ", chByte);
	shShort = (ul_data[10][3] & 0x7F) << 8; shShort |= (ul_data[11][0]); PRINTK("toltalvalue %04x ", shShort);
	shShort = (ul_data[11][1]) << 4; shShort |= ((ul_data[11][2] & 0xF0) >> 4); PRINTK("origin %04x \n", shShort);
	}else
		PRINTK("unknown ticket\n");
	//mac2
	shShort = (ul_data[11][2] & 0xF) << 8; shShort |= (ul_data[11][3]); PRINTK("mac2 %04x\n", shShort);

	chByte = (ul_data[12][0] & 0x80) >> 7; PRINTK("active flag %02x ", chByte);
	chByte = (ul_data[12][0] & 0x60) >> 5; PRINTK("card status %02x ", chByte);
	chByte = (ul_data[12][0] & 0x1F) << 3; chByte |= ((ul_data[12][1] & 0xE0) >> 5); PRINTK("TSN:%02x ", chByte);
	chByte = (ul_data[12][1] & 0x10) >> 4; PRINTK("rfu %02x ", chByte);
	lngLong = (ul_data[12][1] & 0xf) << 18; lngLong |= (ul_data[12][2] << 10); lngLong |= (ul_data[12][3] << 4); lngLong |= ((ul_data[13][0] & 0xC0) >> 6); PRINTK("starttime%06x ", lngLong);
	chByte = (ul_data[13][0] & 0x3F) << 2; chByte |= ((ul_data[13][1] & 0xC0) >> 6); PRINTK("lasttime %02x ", chByte);
	shShort = (ul_data[13][1] & 0x3F) << 6; shShort |= ((ul_data[13][2] & 0xFC) >> 2); PRINTK("laststation %04x ", shShort);
	chByte = (ul_data[13][2] & 0x3); PRINTK("transfer %02x ", chByte);
	chByte = (ul_data[13][3] & 0xE0) >> 5; PRINTK("status %02x \n", chByte);
	if(chProduct == 0x02)
	{
	//static ticket info
	lngLong = (ul_data[13][3] & 0x1F) << 17; lngLong |= (ul_data[14][0] << 9); lngLong |= (ul_data[14][1] << 1); lngLong |= ((ul_data[14][2] & 0x80) >> 7); PRINTK("start time %06x ", lngLong);
	shShort = (ul_data[14][2] & 0x7F) << 5; shShort |= ((ul_data[14][3] & 0xF8) >> 3); PRINTK("origin %04x ", shShort);
	shShort = (ul_data[14][3] & 0x7) << 9; shShort |= (ul_data[15][0] << 1); shShort |= ((ul_data[15][1] & 0x80) >> 7); PRINTK("destion %04x ", shShort);
	chByte = (ul_data[15][1] & 0x40) >> 6; PRINTK("activated %02x ", chByte);
	shShort = (ul_data[15][1] & 0x3f) << 4; shShort |= ((ul_data[15][2] & 0xF0) >> 4); PRINTK("rfu %04x \n", shShort);
	}else if(chProduct == 0x03)
	{
	//times ticket info
	lngLong = (ul_data[13][3] & 0x1F) << 17; lngLong |= (ul_data[14][0] << 9); lngLong |= (ul_data[14][1] << 1); lngLong |= ((ul_data[14][2] & 0x80) >> 7); PRINTK("start time %06x ", lngLong);
	shShort = (ul_data[14][2] & 0x7F) << 1; shShort |= ((ul_data[14][3] & 0x80) >> 7); PRINTK("times %02x ", shShort);
	shShort = (ul_data[14][3] & 0x7F) << 5; shShort |= ((ul_data[15][0] & 0xF8) >> 3);  PRINTK("origin %04x ", shShort);
	shShort = (ul_data[15][0] & 0x7) << 9; shShort |= (ul_data[15][1] << 1); shShort |= ((ul_data[15][2] & 0x80) >> 7); PRINTK("destion %04x", shShort);
	chByte = (ul_data[15][2] & 0x40) >> 6; PRINTK("activated %02x ", chByte);
	chByte = (ul_data[15][2] & 0x20) >> 5; PRINTK("totalused %02x ", chByte);
	chByte = (ul_data[15][2] & 0x10) >> 4; PRINTK("rfu %04x\n", shShort);
	}else if(chProduct == 0x01)
	{
	//purse ticket info
	lngLong = (ul_data[13][3] & 0x1F) << 7; lngLong |= ((ul_data[14][0] & 0xFE) >> 1); PRINTK("start time %04x ", lngLong);
	lngLong = (ul_data[14][0] & 0x1) << 16; lngLong |= (ul_data[14][1] << 8); lngLong |= ul_data[14][2]; PRINTK("balance %06x ", lngLong);
	chByte = (ul_data[14][3] & 0x80) >> 7; PRINTK("actived %04x ", chByte);
	shShort = (ul_data[14][3] & 0x7F) << 8; shShort |= (ul_data[15][0]); PRINTK("toltalvalue %04x ", shShort);
	shShort = (ul_data[15][1]) << 4; shShort |= ((ul_data[15][2] & 0xF0) >> 4); PRINTK("origin %04x \n", shShort);
	}else
		PRINTK("unknown ticket\n");
	//mac2
	shShort = (ul_data[15][2] & 0xF) << 8; shShort |= (ul_data[15][3]); PRINTK("mac2 %04x\n", shShort);
#endif	
	get_degrade_mode(tpSJT.curstation);

	//write the response message
	chCode = CE_OK;
	*out_len = 96 + 22 + 1;
	memset(&out_buf[0], 0x00, (*out_len));
	//phiycial type
	out_buf[0] = XA_SJT_FAMILY;
	//length
	out_buf[1] = 96;
	//non-fee area
	out_buf[2] = 0;
	//feearea 
	out_buf[3] = 0;
	//amount for overfare-4
	memset(&out_buf[4], 0x00, 4);
	//amount for refund
	memset(&out_buf[8], 0x00, 4);
	//transaction for refund
	memset(&out_buf[12], 0x00, 4);
	//amount for RefundHandlingFee
	memset(&out_buf[16], 0x00, 4);
	//max balance
	out_buf[20] = 0;
	//antelena
	out_buf[21] = 0x01;
	
	//information
	//product type
	out_buf[22] = tpSZ.productID;
	//ticket product type
	out_buf[23] = tpSZ.productType;
	out_buf[24] = 0;
	//city code
	out_buf[25] = 0;
	out_buf[26] = 0;
	//business code
	out_buf[27] = 0;
	out_buf[28] = 0;
	//phycial code
	memcpy(&out_buf[29], &ch_ul_phyical_id[1], 7);
	//logicial code
	out_buf[36] = ul_data[3][3]; out_buf[37] = ul_data[3][2]; 
	out_buf[38] = ul_data[3][1]; out_buf[39] = ul_data[3][0];
	//reinitial times
	memcpy(&out_buf[40], &tpSZ.lifecycleCount, 2);
	//test flag
	out_buf[42] = tpSZ.testMode;
	//card initial code 
	out_buf[43] = 0;
	//initial date-4
	xa_monthtodate(tpSZ.cardBaseDataTime, &out_buf[44]);
	//initial patch
	memcpy(&out_buf[48], &tpSZ.cardBatchNumber, 2);
	//passenger type
	out_buf[50] = tpSZ.passengerType;
	//deposit
	memset(&out_buf[51], 0x00, 4);
	//value/period/times number
	if(tpSZ.productID == XA_FEETYPE_VALUE)
	{//value
		//product type
		out_buf[55] = 2;
		//product status
		out_buf[58] = tpDZpurse.cardStatus;
		//actived flag
		out_buf[59] = tpDZpurse.activated;
		//valid start station-4
		memset(&out_buf[74], 0x00, 4);
		//valid end station-4
		memset(&out_buf[78], 0x00, 4);
		//balance 
		//memcpy(&out_buf[82], &tpSZ.purchaseValue, 4);
		memcpy(&out_buf[82], &tpDZpurse.remainingValue, 4);
		//card status 
		out_buf[88] = tpDZpurse.cardStatus;
		//last valid/period/times used 
		out_buf[89] = 1;
		//valid start date-7
		xa_daytodate(tpSZ.cardBaseDataTime, tpDZpurse.validityStartDate, &lngHisecond1, &out_buf[60]);
		if(tpDZpurse.activated == 0)
		{
			memcpy(&out_buf[67], &out_buf[60], 7);
			memcpy(&out_buf[60], tpSJT.time_bcd, 7);
			if(memcmp(tpSJT.time_bcd, &out_buf[60], 6) < 0)
				memcpy(&out_buf[60], tpSJT.time_bcd, 7);
		}else
		{
			//valid end date-7
			xa_DurationTolocaltime(lngHisecond1, tpSZ.DurationType, tpSZ.validityDuration, &out_buf[67]);
		}
		//起始时间
		memcpy(chStartDate, &out_buf[60], 7);
		memset(&out_buf[71], 0x00, 3);
		lngLosecond1 = timestr2long(&out_buf[68]) - 1;
		long2timestr(lngLosecond1, &out_buf[67]);
		//travel start time
		xa_MinuteTolocaltime(&out_buf[91], tpSZ.cardBaseDataTime, tpDZpurse.startDateTime, &lngHisecond1);
		//travel start station
		chRejectCode = card_to_location(tpDZpurse.lastLocation, &lngLocation);
		memcpy(&out_buf[98], &lngLocation, 4);
		//last used time
		lngHisecond1 += (tpDZpurse.lastDateTime * 60);
		long2timestr(lngHisecond1, &out_buf[106]);
		//last used Station
		memcpy(&out_buf[113], &lngLocation, 4);
		//last travel status 
		out_buf[117] = tpDZpurse.status;
//		printf("inquire XA_FEETYPE_VALUE :\n");
//		for(i = 0;i < 117;i++)
//		{
//			printf(" %02x",out_buf[i]);
//		}
//		printf("\n");
	}else if(tpSZ.productID == XA_FEETYPE_PERIOD)
	{//period
		out_buf[55] = 1;
		//product status
		out_buf[58] = tpDZperiod.cardStatus;
		//actived flag
		out_buf[59] = tpDZperiod.activated;
		//valid start station-4
		chRejectCode = card_to_location(tpDZperiod.validityOrigin, &lngLocation);
		memcpy(&out_buf[74], &lngLocation, 4);
		//valid end station-4
		chSJTRejectCode = card_to_location(tpDZperiod.validityDestination, &lngLocation);
		memcpy(&out_buf[78], &lngLocation, 4);
		//balance 
		memset(&out_buf[82], 0x00, 4);
		//card status 
		out_buf[88] = tpDZperiod.cardStatus;
		//last valid/period/times used 
		out_buf[89] = 2;
		//valid start date
		xa_MinuteTolocaltime(&out_buf[60], tpSZ.cardBaseDataTime, tpDZperiod.validityStartDateTime, &lngHisecond1);
		//valid end date
		//lngHisecond1 += (tpSZ.validityDuration * 24 * 3600);
		//long2timestr(lngHisecond1 - 1, &out_buf[67]);
		//
		xa_DurationTolocaltime(lngHisecond1, tpSZ.DurationType, tpSZ.validityDuration, &out_buf[67]);
		if(tpDZperiod.activated == 0)
		{//NOT actived
//			memcpy(&out_buf[67], &out_buf[60], 7);
//			memcpy(&out_buf[60], tpSJT.time_bcd, 7);
			//起始时间
			memcpy(chStartDate, &tpSJT.time_bcd[0], 7);
//			if(memcmp(tpSJT.time_bcd, &out_buf[60], 6) < 0)
//				memcpy(&out_buf[60], tpSJT.time_bcd, 7);
		}
		else
		{
			//起始时间
			memcpy(chStartDate, &out_buf[60], 7);
		}
		//travel start time
		xa_MinuteTolocaltime(&out_buf[91], tpSZ.cardBaseDataTime, tpDZperiod.startDateTime, &lngHisecond1);
		//travel start station
		chLastRejectCode = card_to_location(tpDZperiod.lastLocation, &lngLocation);
		memcpy(&out_buf[98], &lngLocation, 4);
		//last used time
		lngHisecond1 += (tpDZperiod.lastDateTime * 60);
		long2timestr(lngHisecond1, &out_buf[106]);
		//last used Station
		memcpy(&out_buf[113], &lngLocation, 4);
		//last travel status 
		out_buf[117] = tpDZperiod.status;
	}else if(tpSZ.productID == XA_FEETYPE_TIMES)
	{//times
		//product type
		out_buf[55] = 1;
		//product status
		out_buf[58] = tpDZmultiride.cardStatus;
		//actived flag
		out_buf[59] = tpDZmultiride.activated;
		//valid start station-4
		chRejectCode = card_to_location(tpDZmultiride.validityOrigin, &lngLocation);
		memcpy(&out_buf[74], &lngLocation, 4);
		//valid end station-4
		chSJTRejectCode = card_to_location(tpDZmultiride.validityDestination, &lngLocation);
		memcpy(&out_buf[78], &lngLocation, 4);
		//balance --4
		out_buf[82] = tpDZmultiride.remainingRides;
		//card status 
		out_buf[88] = tpDZmultiride.cardStatus;
		//last valid/period/times used 
		out_buf[89] = XA_FEETYPE_TIMES;
		//valid start date
		xa_MinuteTolocaltime(&out_buf[60], tpSZ.cardBaseDataTime, tpDZmultiride.validityStartDateTime, &lngHisecond1);
		//valid end date
		//lngHisecond1 += (tpSZ.validityDuration * 24 * 3600);
		if(tpDZmultiride.activated == 0)
		{//NOT actived
			memcpy(&out_buf[67], &out_buf[60], 7);
			memcpy(&out_buf[60], tpSJT.time_bcd, 7);
			if(memcmp(tpSJT.time_bcd, &out_buf[60], 6) < 0)
				memcpy(&out_buf[60], tpSJT.time_bcd, 7);
		}else
		{
			xa_DurationTolocaltime(lngHisecond1, 2, tpSZ.validityDuration, &out_buf[67]);
			//long2timestr(lngHisecond1, &out_buf[67]);
			memset(&out_buf[71], 0x00, 3);
			lngLosecond1 = timestr2long(&out_buf[68]) - 1;
			long2timestr(lngLosecond1, &out_buf[67]);
		}
		//起始时间
		memcpy(chStartDate, &out_buf[60], 7);
		//travel start time-7
		xa_MinuteTolocaltime(&out_buf[91], tpSZ.cardBaseDataTime, tpDZmultiride.startDateTime, &lngHisecond1);
		//travel start station
		chLastRejectCode = card_to_location(tpDZmultiride.lastLocation, &lngLocation);
		memcpy(&out_buf[98], &lngLocation, 4);
		//last used time
		lngHisecond1 += (tpDZmultiride.lastDateTime * 60);
		long2timestr(lngHisecond1, &out_buf[106]);
		//last used Station
		memcpy(&out_buf[113], &lngLocation, 4);
		//last travel status 
		out_buf[117] = tpDZmultiride.status;
	}else 
	{
		//product status
		out_buf[58] = 0;
		//product type
		out_buf[55] = 0;
		//actived flag
		out_buf[59] = 0;
		//valid start station-4
		memset(&out_buf[74], 0x00, 4);
		//valid end station-4
		memset(&out_buf[78], 0x00, 4);
		//card status 
		out_buf[88] = 0x01;
		//last valid/period/times used 
		out_buf[89] = 0;
		//
		out_buf[2] = out_buf[3] = CE_NON_FEETYPE;
		return chCode;
	}
	//last sam id
	memset(&out_buf[86], 0x00, 2);
	//transfer times
	out_buf[90] = 0;
	//tranamount had payed-4
	memset(&out_buf[102], 0x00, 4);
	//history record number
	out_buf[118] = 0;
	//location error
	if((chRejectCode != 0) || (chSJTRejectCode != 0))// || (chLastRejectCode != 0))
	{
		out_buf[2] = out_buf[3] = CE_EOD_FILE;
		return chCode;
	}
	if((chRejectCode = UL_TellSysCard(tpSZ.productType, NULL)) != 0)
	{
		out_buf[3] = out_buf[2] = (unsigned char)chRejectCode;
		return chCode;
	}
	//sub ticket type
	out_buf[56] = 0;
	if(tpSZ.productID == XA_FEETYPE_PERIOD)
	{//
		for(i = 0; i < tpTicketDef.ProductTypeVariantsCount; i++)
		{
			lngHisecond1 = tpTicketDef.subProduct_val[i].Duration & 0xFFF;
			if( ((tpTicketDef.subProduct_val[i].Duration & 0xF000) >> 12) == 4 )
				lngHisecond1 *= 24;
			if( lngHisecond1 == tpSZ.validityDuration)
			{
				out_buf[56] = tpTicketDef.subProduct_val[i].ProductTypeVariants;
				break;
			}
		}
	}
	//issued operation id 
	out_buf[57] = tpTicketDef.ProductIssuer;
	//check actived status 
	//if(out_buf[59] == 0)
	//{
	//	out_buf[2] = out_buf[3] = CE_NONACTIVED;
	//	return chCode;
	//}
	//check ticket status
	if(out_buf[58] != XA_SJT_CARD_NOMAL)
	{
		out_buf[2] = out_buf[3] = CE_ISSUED;
		return chCode;
	}
	//check ticket status
	if((tpSZ.productID == XA_FEETYPE_VALUE) && (tpDZpurse.cardStatus == XA_SJT_CARD_RECYCLE))
	{
		out_buf[2] = out_buf[3] = CE_ISSUED;
		return chCode;
	}
	//check the validate date
	get_degrade_sensitive_mode(NULL, &out_buf[106]);
	
	if(0 != (chDateRejectCode = xa_TellDate(tpSJT.time_bcd, chStartDate, &out_buf[67], out_buf[117], &out_buf[106], tpSZ.DurationType)))
	{
		out_buf[2] = out_buf[3] = chDateRejectCode;
		//
	}
	if((tpwaivermode.sen_sta_emergency || (tpwaivermode.sen_sta_failure && (out_buf[117] == 3))) 
		&& (memcmp(tpSJT.time_bcd, &out_buf[106], 4) != 0))
	{
		out_buf[117] = 0;
	}
	//according to the Fee-area or NONFEE-area to check the ticket status
	blnOverfare = blnOvertime = 0;
	chRejectCode = UL_TellEntryMAC(out_buf[117], 0xff);
	if(cmd_buf[8] == 1)
	{//Fee area
		if(chRejectCode == 0)
		{//entry status
			if(chLastRejectCode != 0)
			{
				out_buf[2] = CE_EOD_FILE;
				out_buf[3] = 0;
				return chCode;
			}
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				//错误使用旅程起始实际，应为最后使用时间
				//if(0 != (chRejectCode = cal_overtime(&out_buf[91], tpSJT.time_bcd, 0, 0)))
				if(0 != (chRejectCode = cal_overtime(&out_buf[106], tpSJT.time_bcd, 0, 0)))
				{
					out_buf[2] = chRejectCode;
					out_buf[3] = 0;
					//penalty
					if(tpStationPrice.SJTNum == 0)
						lngOvertimePenalty = 500;
					else
						lngOvertimePenalty = tpStationPrice.SJTPrice[tpStationPrice.SJTNum - 1];
					memcpy(&out_buf[4], &lngOvertimePenalty, 4);
					blnOvertime = 0xff;
				}
			}
			if(tpSZ.productID == XA_FEETYPE_VALUE)
			{
				if(0 != (chRejectCode = UL_TellOverRide(tpSJT.time_bcd, tpDZpurse.origin, &tpSJT.curstation[0], tpDZpurse.lastLocation, tpDZpurse.status, tpDZpurse.remainingValue)))
				{
					if(chRejectCode == CE_OVERRIDE)
					{
						out_buf[2] = CE_OVERRIDE;
						blnOverfare = 0xff;
						lngOverfarePenalty = tpSysPrice.price - tpDZpurse.remainingValue;
						memcpy(&out_buf[4], &lngOverfarePenalty, 4);
					}
					else
						out_buf[2] = chRejectCode;
				}
			}
			if(blnOverfare && blnOvertime)
			{
				out_buf[2] = CE_OVERFARETIME;
				lngPenalty = lngOverfarePenalty + lngOvertimePenalty;
				memcpy(&out_buf[4], &lngPenalty, 4);
			}
		}else 
		{
			if(chDateRejectCode != 0)
			{//20140415
				out_buf[2] = out_buf[3] = chDateRejectCode;
			}else
			{
				out_buf[2] = chRejectCode;
				out_buf[3] = 0;
			}
		}
	}else 
	{//NON-Fee area 
		if(chDateRejectCode != 0)
		{
			out_buf[2] = out_buf[3] = chDateRejectCode;
			return chCode;
		}
		if(chRejectCode == 0)
		{//entry status
			if(tpSZ.productID == XA_FEETYPE_TIMES)
			{
				out_buf[2] = 0;
				//out_buf[3] = CE_NO_UPDATE_ENTRY;
				lngLosecond1 = timestr2long(&out_buf[107]);
				if(lngLosecond1 > tpSJT.lowsecond)
				{
					out_buf[3] = CE_FREE_UPDATE_ENTRY;
				}else if((tpSJT.lowsecond - lngLosecond1) < 20 * 60)
				{
					out_buf[3] = CE_FREE_UPDATE_ENTRY;
				}else 
				{
					out_buf[3] = CE_NO_UPDATE_ENTRY;
				}
			}
			else if(tpSZ.productID == XA_FEETYPE_PERIOD)
			{
				out_buf[2] = 0;
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
			}
			else
			{
				out_buf[2] = 0;
				lngLosecond1 = timestr2long(&out_buf[107]);
				memcpy(&lngLocation, &out_buf[113], 4);
				lngcurLocation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
				if(lngcurLocation == lngLocation)
				{
					if(lngLosecond1 > tpSJT.lowsecond)
					{
						out_buf[3] = CE_FREE_UPDATE_ENTRY;
					}else if((tpSJT.lowsecond - lngLosecond1) < 20 * 60)
					{
						out_buf[3] = CE_FREE_UPDATE_ENTRY;
					}else 
					{
						out_buf[3] = CE_NO_UPDATE_ENTRY;
					}
				}else
				{
					out_buf[3] = CE_NO_UPDATE_ENTRY;
				}
			}
		}else
		{
			out_buf[2] = out_buf[3] = 0;
		}
	}
#ifdef	DEBUG_PRINT
	PRINTK("UL:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif	
	return chCode;
}


/******************************************
单程票更新
******************************************/
char xa_ul_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
long lngUpdateValue;
unsigned char chCode, chRejectCode;
unsigned char buf[20], start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned char chUpdateMAC[4];
unsigned short i, shbalance, shFare, cnt, cnt2;
unsigned long lngHisecond1, lngLosecond, lngTravelsecond, lngLastsecond, lngLocation, lngstation;

#ifdef DEBUG_PRINT
unsigned char localtime[7];
	PRINTK("\nupdate command is %02x%02x and length is %02x%02x:\n", cmd_buf[3], cmd_buf[4], cmd_buf[1], cmd_buf[2]);
	PRINTK("SN %02x%02x%02x%02x time %02x%02x-%02x-%02x %02x:%02x:%02x\n", cmd_buf[6], cmd_buf[7], cmd_buf[8], cmd_buf[9], cmd_buf[10], cmd_buf[11], cmd_buf[12], cmd_buf[13], cmd_buf[14], cmd_buf[15], cmd_buf[16]);
	PRINTK("chiptype %02x type %02x%02x paytype %02x payamount %02x%02x%02x%02x\n", 
		cmd_buf[17], cmd_buf[18], cmd_buf[19], cmd_buf[20], cmd_buf[21], cmd_buf[22], cmd_buf[23], cmd_buf[24]);
	PRINTK("bom/efo %02x updatetype %02x entrystation %02x%02x%02x%02x exit station %02x%02x%02x%02x ", 
		cmd_buf[25], cmd_buf[26], cmd_buf[27], cmd_buf[28], cmd_buf[29], cmd_buf[30], cmd_buf[31], cmd_buf[32], cmd_buf[33], cmd_buf[34]);
#endif
	*out_len = 2;
	memcpy(tpSJT.time_bcd, &cmd_buf[10], 7);
	tpSJT.lowsecond = timestr2long(&cmd_buf[11]);
	
	memcpy(out_buf, "\x25\x05", 2);
	if(read_tocken_info()!=0)
	{
		return CE_READ;
	}
	
	memcpy(out_buf, "\x25\x06", 2);
	if((chCode = UL_TellSysCard(tpSZ.productType, &cmd_buf[32])) != 0)
	{
		return chCode;
	}

	memcpy(out_buf, "\x25\x08", 2);
	if((chCode = UL_TellTesting(tpCmdInit.test)) != 0)
	{
		return chCode;
	}
	switch(tpSZ.productID)
	{
	case XA_FEETYPE_VALUE:
		//pay method
		if( (cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
			return CE_BADPARAM;
		return xa_ul_update_purse(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_TIMES:
		return xa_ul_update_multiride(cmd_buf, out_buf, out_len);
	case XA_FEETYPE_PERIOD:
		//pay method
		if( (cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
			return CE_BADPARAM;
		return xa_ul_update_period(cmd_buf, out_buf, out_len);
	default:
		return CE_NON_FEETYPE;
	}
}

char xa_ul_update_purse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
long lngUpdateValue;
unsigned char chCode, chRejectCode;
unsigned char buf[20], start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned char chUpdateMAC[4];
unsigned short i, shbalance, shFare, cnt, cnt2;
unsigned long lngHisecond1, lngLosecond, lngTravelsecond, lngLastsecond, lngLocation, lngstation;

	//
	tpSJT.balance = tpDZpurse.remainingValue;
	
	tpTxnProductPurseCompensate.SysComHdr_val.udType = toMoto(3);
	tpTxnProductPurseCompensate.SysComHdr_val.udSubtype = toMoto(118);
	tpTxnProductPurseCompensate.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
	tpTxnProductPurseCompensate.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[6]);
	tpTxnProductPurseCompensate.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
	
	tpTxnProductPurseCompensate.SysProductCom_val.productIssuerId = tpTxnProductPurseCompensate.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
	tpTxnProductPurseCompensate.SysCardCom_val.cardSerialNumber = *(long *)ul_data[3];
	tpTxnProductPurseCompensate.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);

	tpTxnProductPurseCompensate.SysAppCom_val.applicationPassengerType = toMoto(tpSZ.passengerType);
	
	tpTxnProductPurseCompensate.SysProductCom_val.productType = toMoto(tpSZ.productType);
	tpTxnProductPurseCompensate.SysProductCom_val.Ptsn = toMoto(tpDZpurse.transactionSequenceNumber);
	
	tpTxnProductPurseCompensate.DevUdJourneyHdr_val.passengerType = toMoto(0x01);
	tpTxnProductPurseCompensate.DevUdJourneyHdr_val.currentLocation = tpTxnProductPurseCompensate.SysComHdr_val.deviceLocation;
	tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripOriginLocation = 0;
	tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripPreviousLocation = 0;
	memcpy(out_buf, "\x25\x09", 2);
	//valid start date-7
	xa_daytodate(tpSZ.cardBaseDataTime, tpDZpurse.validityStartDate, &lngLastsecond, start_timebcd);
	if(tpDZpurse.activated == 0)
	{//NOT ACTIVED
		memcpy(start_timebcd, tpSJT.time_bcd, 7);
	}
	xa_DurationTolocaltime(lngLastsecond, 2, tpSZ.validityDuration, end_timebcd);
	xa_MinuteTolocaltime(&last_timebcd[0], tpSZ.cardBaseDataTime, tpDZpurse.startDateTime, &lngTravelsecond);
	//last used time
	lngLastsecond = lngTravelsecond + (tpDZpurse.lastDateTime * 60);
	long2timestr(lngLastsecond, &last_timebcd[0]);
	get_degrade_sensitive_mode(NULL, last_timebcd);
	if((chRejectCode = xa_TellDate(tpSJT.time_bcd, start_timebcd, end_timebcd, tpDZpurse.status, last_timebcd, tpSZ.DurationType)) != 0)
	{
		return chRejectCode;
	}
	if((tpwaivermode.sen_sta_emergency || (tpwaivermode.sen_sta_failure && (tpDZpurse.status == 3)) )
		&& (memcmp(tpSJT.time_bcd, last_timebcd, 4) != 0))
	{
		tpDZpurse.status = 0;
	}
	//travel start date-time
	memcpy(out_buf, "\x25\x0a", 2);
	//travel start station
	if(0 != (chCode = card_to_location(tpDZpurse.origin, &lngLocation)))
		return chCode;
	tpTxnProductPurseCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
	tpTxnProductPurseCompensate.DevUdPurseCommonHdr_val.purseRemainingValue = toMoto(tpDZpurse.remainingValue);
	//
	memcpy(&lngLosecond, &cmd_buf[21], 4);
	tpTxnProductPurseCompensate.SysFinDetails_val.transactionValue = toMoto(lngLosecond);
	//
	tpTxnProductPurseCompensate.SysFinDetails_val.paymentMethod = toMoto(cmd_buf[20]);
	tpTxnProductPurseCompensate.SysFinDetails_val.partialTransactionValue = 0;
	memset(&tpTxnProductPurseCompensate.DevUdPurseLavHdr_val.lavSamId, 0x00, sizeof(DevUdPurseLavHdr_t));
	memcpy(out_buf, "\x25\x0b", 2);
	chRejectCode = UL_TellEntryMAC(tpDZpurse.status, 0);
	if(cmd_buf[25] == 1)
	{//Fee area
		if(chRejectCode == 0)
		{//entry status
			if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpSJT.time_bcd, 0, 0)))
			{
				if(cmd_buf[26] != 0x02)
					return CE_BADPARAM;
				if(lngTravelsecond == lngLastsecond)
				{//travel start time is the ENTRY time
					if((tpSJT.lowsecond - lngLastsecond) > 255 * 60)
					{//need change the travel start time
						cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
						xaDynamicZonePurse._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
						xaDynamicZonePurse._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
						xaDynamicZonePurse._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
						xaDynamicZonePurse._DynamicZone.startDateTime = lngLosecond & 0x3;
						xaDynamicZonePurse._DynamicZone.lastDateTime_1 = 0;
						xaDynamicZonePurse._DynamicZone.lastDateTime = 0;
					}else 
					{//only change the last date/time
						xaDynamicZonePurse._DynamicZone.lastDateTime_1 = (((tpSJT.lowsecond - lngLastsecond) / 60) >> 2) & 0x3F;
						xaDynamicZonePurse._DynamicZone.lastDateTime = ((tpSJT.lowsecond - lngLastsecond) / 60) & 0x3;
					}
				}else 
				{
					cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
					xaDynamicZonePurse._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
					xaDynamicZonePurse._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
					xaDynamicZonePurse._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
					xaDynamicZonePurse._DynamicZone.startDateTime = lngLosecond & 0x3;
					xaDynamicZonePurse._DynamicZone.lastDateTime_1 = 0;
					xaDynamicZonePurse._DynamicZone.lastDateTime = 0;
				}
			}
			if(tpSZ.productID == XA_FEETYPE_VALUE)
			{
				if(0 != (chRejectCode = UL_TellOverRide(tpSJT.time_bcd, tpDZpurse.origin, &tpSJT.curstation[0], tpDZpurse.lastLocation, tpDZpurse.status, tpDZpurse.remainingValue)))
				{
					if(chRejectCode != CE_OVERRIDE)
					{
						return CE_BADPARAM;
					}
					lngstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
					if(0 != (chCode = location_to_card(lngstation, &tpDZpurse.lastLocation)))
						return chCode;
					xaDynamicZonePurse._DynamicZone.lastLocation_1 = (tpDZpurse.lastLocation >> 6) & 0x3f;
					xaDynamicZonePurse._DynamicZone.lastLocation = tpDZpurse.lastLocation & 0x3f;
					xaDynamicZonePurse._DynamicZone.status = 4;
				}
			}
		}else 
		{
			//update mode error
			if(cmd_buf[26] != 0x01)
				return CE_BADPARAM;
			//entry datetime
			cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
			xaDynamicZonePurse._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
			xaDynamicZonePurse._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
			xaDynamicZonePurse._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
			xaDynamicZonePurse._DynamicZone.startDateTime = lngLosecond & 0x3;
			xaDynamicZonePurse._DynamicZone.lastDateTime_1 = 0;
			xaDynamicZonePurse._DynamicZone.lastDateTime = 0;
			//entry station
			memcpy(&lngstation, &cmd_buf[27], 4);
			if(0 != (chCode = location_to_card(lngstation, &tpDZpurse.lastLocation)))
				return chCode;
			xaDynamicZonePurse._DynamicZone.lastLocation_1 = (tpDZpurse.lastLocation >> 6) & 0x3F;
			xaDynamicZonePurse._DynamicZone.lastLocation = tpDZpurse.lastLocation & 0x3f;
			xaDynamicZonePurse._DynamicZone.origin_1 = (tpDZpurse.lastLocation >> 4) & 0xff;
			xaDynamicZonePurse._DynamicZone.origin = tpDZpurse.lastLocation & 0xf;
			
			xaDynamicZonePurse._DynamicZone.status = 1;
			//activated
			if(tpDZpurse.activated == 0)
			{
				tpDZpurse.validityStartDate = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngHisecond1);
				xaDynamicZonePurse._DynamicZone.validityStartDate_1 = (tpDZpurse.validityStartDate >> 7) & 0x1F;
				xaDynamicZonePurse._DynamicZone.validityStartDate = tpDZpurse.validityStartDate & 0x7F;
				xaDynamicZonePurse._DynamicZone.activated = 1;
			}
		}
	}else 
	{//NON-Fee area
		if(chRejectCode == 0)
		{//entry status
			if(cmd_buf[26] != 0x03)
				return CE_BADPARAM;
			out_buf[2] = 0;
			if(lngLastsecond > tpSJT.lowsecond)
			{
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
				xaDynamicZonePurse._DynamicZone.status = 2;
			}else if((tpSJT.lowsecond - lngLastsecond) < 20 * 60)
			{
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
				xaDynamicZonePurse._DynamicZone.status = 2;
			}else 
			{
				return CE_NO_UPDATE_ENTRY;
			}
		}else
		{
			return CE_BADPARAM;
		}
	}
	//
	xaDynamicZonePurse._DynamicZone.activeStatus = ((((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1);
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePurse.buff, 16);
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZpurse.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZpurse.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZpurse.mac2;
	//mac
	tpTxnProductPurseCompensate.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductPurseCompensationFare_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPurseCompensate.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPurseCompensate.SysComHdr_val.reservedField, sh_mac_len - 4 - 36);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[sh_mac_len - 36], &tpTxnProductPurseCompensate.DevUdPurseLavHdr_val.lavSamId, 36);
	//
	g_sha1txnsn = tpTxnProductPurseCompensate.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPurseCompensate.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 0;
	ch_mac_sel = 4;
	sem_init(&g_samreturn, 0, 0);
	sem_post(&g_samcalwait);
	//write the page
	for(i = tpSJT.activeWritepage; i < (tpSJT.activeWritepage + 4); i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			reader_status = XA_RW_IDLE;
			sem_wait(&g_samcalwait);
			tpYPT_txn_val.YPT_flag = 0;
			ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.pYPT_txn, tpYPT_txn_val.YPT_txnlen);
			return CE_WRITE;
		}
	}

	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &tpSJT.balance, 4);
	//lock status -1
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_VALUE;
	//rfu-14
	memset(&out_buf[20], 0x00, 13);
	
	*out_len = 33;
	//UD length-2
	//UD record number-1
	out_buf[35] = 1;
	//UD record type-1
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD 
	sem_wait(&g_samreturn);
	memcpy(&out_buf[39], &tpTxnProductPurseCompensate.AFCHead_val.operatorid[0], cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	reader_status = XA_RW_IDLE;
	(*out_len) += cnt;
#ifdef	DEBUG_PRINT
	PRINTK("UL ISSUE:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif

	return CE_OK;
}

char xa_ul_update_multiride(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
long lngUpdateValue;
unsigned char chCode, chRejectCode;
unsigned char buf[20], start_timebcd[7], end_timebcd[7], last_timebcd[7];
unsigned char chUpdateMAC[4];
unsigned short i, shbalance, shFare, cnt, cnt2;
unsigned long lngHisecond1, lngLosecond, lngTravelsecond, lngLastsecond, lngLocation, lngstation;

	//
	tpSJT.balance = tpDZmultiride.remainingRides;
	
	tpTxnProductMultirideCompensate.SysComHdr_val.udType = toMoto(3);
	tpTxnProductMultirideCompensate.SysComHdr_val.udSubtype = toMoto(120);
	tpTxnProductMultirideCompensate.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
	tpTxnProductMultirideCompensate.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[6]);
	tpTxnProductMultirideCompensate.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
	
	tpTxnProductMultirideCompensate.SysProductCom_val.productIssuerId = tpTxnProductMultirideCompensate.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
	tpTxnProductMultirideCompensate.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	tpTxnProductMultirideCompensate.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);

	tpTxnProductMultirideCompensate.SysAppCom_val.applicationPassengerType = toMoto(tpSZ.passengerType);
	
	tpTxnProductMultirideCompensate.SysProductCom_val.productType = toMoto(tpSZ.productType);
	tpTxnProductMultirideCompensate.SysProductCom_val.Ptsn = toMoto(tpDZmultiride.transactionSequenceNumber);
	
	tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.passengerType = toMoto(tpSZ.passengerType);
	tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.currentLocation = tpTxnProductMultirideCompensate.SysComHdr_val.deviceLocation;
	tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripOriginLocation = 0;
	tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripPreviousLocation = 0;
	memcpy(out_buf, "\x25\x09", 2);
	//valid start date-7
	xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZmultiride.validityStartDateTime, &lngLastsecond);
	xa_DurationTolocaltime(lngLastsecond, tpSZ.DurationType, tpSZ.validityDuration, end_timebcd);
	//travel start date-time
	memcpy(out_buf, "\x25\x0a", 2);
	xa_MinuteTolocaltime(&last_timebcd[0], tpSZ.cardBaseDataTime, tpDZmultiride.startDateTime, &lngTravelsecond);
	if((chCode = xa_TellDate(tpSJT.time_bcd, start_timebcd, end_timebcd, tpDZmultiride.status, last_timebcd, tpSZ.DurationType)) != 0)
	{
		return chCode;
	}	
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpSJT.time_bcd, last_timebcd, 4) != 0))
	{
		tpDZmultiride.status = 0;
	}
	//travel start station
	if(0 != (chCode = card_to_location(tpDZmultiride.lastLocation, &lngLocation)))
		return chCode;
	tpTxnProductMultirideCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
	
	tpTxnProductMultirideCompensate.DevUdMultirideCommonHdr_val.numRides = 0;
	tpTxnProductMultirideCompensate.DevUdMultirideCommonHdr_val.remainingRides = toMoto(tpDZmultiride.remainingRides);
	//last used time
	lngLastsecond = lngTravelsecond + (tpDZmultiride.lastDateTime * 60);
	long2timestr(lngLastsecond, &last_timebcd[0]);
	//
	memcpy(&lngLosecond, &cmd_buf[21], 4);
	tpTxnProductMultirideCompensate.SysFinDetails_val.transactionValue = toMoto(lngLosecond);
	tpTxnProductMultirideCompensate.SysFinDetails_val.paymentMethod = toMoto(cmd_buf[20]);
	tpTxnProductMultirideCompensate.SysFinDetails_val.partialTransactionValue = 0;
	memset(&tpTxnProductMultirideCompensate.DevUdMultirideLavHdr_val.lavSamId, 0x00, sizeof(DevUdMultirideLavHdr_t));
	memcpy(out_buf, "\x25\x0b", 2);
	chRejectCode = UL_TellEntryMAC(tpDZmultiride.status, 0);
	if(cmd_buf[25] == 1)
	{//Fee area
		if(chRejectCode == 0)
		{//entry status
			if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpSJT.time_bcd, 0, 0)))
			{
				if(cmd_buf[26] != 0x02)
					return CE_BADPARAM;
				if(lngTravelsecond == lngLastsecond)
				{//travel start time is the ENTRY time
					if((tpSJT.lowsecond - lngLastsecond) > 255 * 60)
					{//need change the travel start time
						cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
						xaDynamicZoneMultiride._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
						xaDynamicZoneMultiride._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
						xaDynamicZoneMultiride._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
						xaDynamicZoneMultiride._DynamicZone.startDateTime = lngLosecond & 0x3;
						xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 = 0;
						xaDynamicZoneMultiride._DynamicZone.lastDateTime = 0;
					}else 
					{//only change the last date/time
						xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 = (((tpSJT.lowsecond - lngLastsecond) / 60) >> 2) & 0x3F;
						xaDynamicZoneMultiride._DynamicZone.lastDateTime = ((tpSJT.lowsecond - lngLastsecond) / 60) & 0x3;
					}
				}else 
				{
					cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
					xaDynamicZoneMultiride._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
					xaDynamicZoneMultiride._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
					xaDynamicZoneMultiride._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
					xaDynamicZoneMultiride._DynamicZone.startDateTime = lngLosecond & 0x3;
					xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 = 0;
					xaDynamicZoneMultiride._DynamicZone.lastDateTime = 0;
				}
			}
		}else 
		{
			//update mode error
			if(cmd_buf[26] != 0x01)
				return CE_BADPARAM;
			//entry datetime
			cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
			xaDynamicZoneMultiride._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
			xaDynamicZoneMultiride._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
			xaDynamicZoneMultiride._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
			xaDynamicZoneMultiride._DynamicZone.startDateTime = lngLosecond & 0x3;
			xaDynamicZoneMultiride._DynamicZone.lastDateTime_1 = 0;
			xaDynamicZoneMultiride._DynamicZone.lastDateTime = 0;
			//entry station
			memcpy(&lngstation, &cmd_buf[27], 4);
			if(0 != (chCode = location_to_card(lngstation, &tpDZmultiride.lastLocation)))
				return chCode;
			xaDynamicZoneMultiride._DynamicZone.lastLocation_1 = (tpDZmultiride.lastLocation >> 6) & 0x3F;
			xaDynamicZoneMultiride._DynamicZone.lastLocation = tpDZmultiride.lastLocation & 0x3f;
			
			xaDynamicZoneMultiride._DynamicZone.status = 1;
			if(tpDZmultiride.activated == 0)
			{
				xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &tpDZmultiride.validityStartDateTime);
				xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_1 = (tpDZmultiride.validityStartDateTime >> 17) & 0x1F;
				xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_2 = (tpDZmultiride.validityStartDateTime >> 9) & 0xFF;
				xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_3 = (tpDZmultiride.validityStartDateTime >> 1) & 0xFF;
				xaDynamicZoneMultiride._DynamicZone.validityStartDateTime = tpDZmultiride.validityStartDateTime & 0x1;
				//activated
				xaDynamicZoneMultiride._DynamicZone.activated = 1;
				
			}
		}
	}else 
	{//NON-Fee area
		if(chRejectCode == 0)
		{//entry status
			if(cmd_buf[26] != 0x03)
				return CE_BADPARAM;
			out_buf[2] = 0;
			if(lngLastsecond > tpSJT.lowsecond)
			{
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
				xaDynamicZoneMultiride._DynamicZone.status = 2;
			}else if((tpSJT.lowsecond - lngLastsecond) < 20 * 60)
			{
				out_buf[3] = CE_FREE_UPDATE_ENTRY;
				xaDynamicZoneMultiride._DynamicZone.status = 2;
			}else 
			{
				return CE_NO_UPDATE_ENTRY;
			}
		}else
		{
			return CE_BADPARAM;
		}
	}
	//
	xaDynamicZoneMultiride._DynamicZone.activeStatus = ((((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1);
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZoneMultiride.buff, 16);
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZmultiride.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZmultiride.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZmultiride.mac2;
	//mac
	tpTxnProductMultirideCompensate.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductMultirideCompensationFare_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductMultirideCompensate.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductMultirideCompensate.SysComHdr_val.reservedField, sh_mac_len - 4 - 56);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[sh_mac_len - 56], &tpTxnProductMultirideCompensate.DevUdProductValidity_val.vStartDateTime, 56);
	//
	g_sha1txnsn = tpTxnProductMultirideCompensate.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductMultirideCompensate.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 0;
	ch_mac_sel = 4;
	sem_init(&g_samreturn, 0, 0);
	sem_post(&g_samcalwait);
	//write the page
	for(i = tpSJT.activeWritepage; i < (tpSJT.activeWritepage + 4); i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			reader_status = XA_RW_IDLE;
			return CE_WRITE;
		}
	}

	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &tpSJT.balance, 4);
	//lock status -1
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_TIMES;
	//rfu-14
	memset(&out_buf[20], 0x00, 13);
	
	*out_len = 33;
	//UD length-2
	//UD record number-1
	out_buf[35] = 1;
	//UD record type-1
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD 
	sem_wait(&g_samreturn);
	memcpy(&out_buf[39], &tpTxnProductMultirideCompensate.AFCHead_val.operatorid[0], cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	reader_status = XA_RW_IDLE;
	(*out_len) += cnt;
#ifdef	DEBUG_PRINT
	PRINTK("UL UPDATE:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif

	return CE_OK;
}


char xa_ul_update_period(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
long lngUpdateValue;
unsigned char chCode, chRejectCode;
unsigned char buf[20], start_timebcd[7], end_timebcd[7], last_timebcd[7], active_timebcd[7];
unsigned char chUpdateMAC[4];
unsigned short i, shbalance, shFare, cnt, cnt2;
unsigned long lngHisecond1, lngLosecond, lngTravelsecond, lngLastsecond, lngLocation, lngstation;

	tpSJT.balance = 0;
	//
	tpTxnProductPassCompensate.SysComHdr_val.udType = toMoto(3);
	tpTxnProductPassCompensate.SysComHdr_val.udSubtype = toMoto(119);
	tpTxnProductPassCompensate.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
	tpTxnProductPassCompensate.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[6]);
	tpTxnProductPassCompensate.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
	
	tpTxnProductPassCompensate.SysProductCom_val.productIssuerId = tpTxnProductPassCompensate.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
	tpTxnProductPassCompensate.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	tpTxnProductPassCompensate.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);

	tpTxnProductPassCompensate.SysAppCom_val.applicationPassengerType = toMoto(tpSZ.passengerType);
	
	tpTxnProductPassCompensate.SysProductCom_val.productType = toMoto(tpSZ.productType);
	tpTxnProductPassCompensate.SysProductCom_val.Ptsn = toMoto(tpDZperiod.transactionSequenceNumber);
	
	tpTxnProductPassCompensate.DevUdJourneyHdr_val.passengerType = toMoto(tpSZ.passengerType);
	tpTxnProductPassCompensate.DevUdJourneyHdr_val.currentLocation = tpTxnProductPassCompensate.SysComHdr_val.deviceLocation;
	tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripOriginLocation = 0;
	tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripPreviousLocation = tpTxnProductPassCompensate.SysComHdr_val.deviceLocation;
	memcpy(out_buf, "\x25\x09", 2);
	//valid start date-7
	xa_MinuteTolocaltime(&start_timebcd[0], tpSZ.cardBaseDataTime, tpDZperiod.validityStartDateTime, &lngLastsecond);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vStartDateTime = toMoto(lngLastsecond + TIME2000);
	if(tpDZperiod.activated == 0)
	{
		memcpy(end_timebcd, start_timebcd, 7);
		//
		memcpy(start_timebcd, tpSJT.time_bcd, 7);
	}else
	{
		xa_DurationTolocaltime(lngLastsecond, tpSZ.DurationType, tpSZ.validityDuration, end_timebcd);
	}
	//最晚激活时间
	memcpy(active_timebcd, end_timebcd, 7);
	lngLosecond = timestr2long(&end_timebcd[1]);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vEndDateTime = tpTxnProductPassCompensate.passEndDateTime = toMoto(lngLosecond + TIME2000);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vDuration = toMoto(((tpSZ.DurationType & 0xF) << 12) + tpSZ.validityDuration);
	card_to_location(tpDZperiod.validityOrigin, &lngstation);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vOrigin = toMoto(lngstation);
	card_to_location(tpDZperiod.validityDestination, &lngstation);
	tpTxnProductPassCompensate.DevUdProductValidity_val.vDestination = toMoto(lngstation);
	
	//travel start date-time
	memcpy(out_buf, "\x25\x0a", 2);
	xa_MinuteTolocaltime(&last_timebcd[0], tpSZ.cardBaseDataTime, tpDZperiod.startDateTime, &lngTravelsecond);
	
	if((chCode = xa_TellDate(tpSJT.time_bcd, start_timebcd, end_timebcd, tpDZperiod.status, last_timebcd, tpSZ.DurationType)) != 0)
	{
		return chCode;
	}
	if((tpwaivermode.sen_sta_emergency || tpwaivermode.sen_sta_failure || tpwaivermode.sen_sta_exit)
		&& (memcmp(tpSJT.time_bcd, last_timebcd, 4) != 0))
	{
		tpDZperiod.status = 0;
	}
	
	//last used time
	lngLastsecond = lngTravelsecond + (tpDZperiod.lastDateTime * 60);
	long2timestr(lngLastsecond, &last_timebcd[0]);
	//
	memcpy(&lngLosecond, &cmd_buf[21], 4);
	tpTxnProductPassCompensate.SysFinDetails_val.transactionValue = toMoto(lngLosecond);
	tpTxnProductPassCompensate.SysFinDetails_val.paymentMethod = toMoto(cmd_buf[20]);
	tpTxnProductPassCompensate.SysFinDetails_val.partialTransactionValue = 0;
	memset(&tpTxnProductPassCompensate.DevUdPassLavHdr_val.lavSamId, 0x00, sizeof(DevUdPassLavHdr_t));
	memcpy(out_buf, "\x25\x0b", 2);
	chRejectCode = UL_TellEntryMAC(tpDZperiod.status, 0);
	if(cmd_buf[25] == 1)
	{//Fee area
		if(chRejectCode == 0)
		{//entry status
			//travel start station
			if(0 != (chCode = card_to_location(tpDZperiod.lastLocation, &lngLocation)))
				return chCode;
			tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			if(tpTicketDef.IgnoreMaxJourneyTime == 0)
			{
				if(0 != (chRejectCode = cal_overtime(&last_timebcd[0], tpSJT.time_bcd, 0, 0)))
				{
					//pay method
					if((cmd_buf[20] == 0x02) || (cmd_buf[20] == XA_PAYTYPE_CARD) )
						return CE_BADPARAM;
					//according to the MONEY
					if(cmd_buf[26] != 0x02)
						return CE_BADPARAM;
					if(lngTravelsecond == lngLastsecond)
					{//travel start time is the ENTRY time
						if((tpSJT.lowsecond - lngLastsecond) > 255 * 60)
						{//need change the travel start time
							cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
							xaDynamicZonePeriod._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
							xaDynamicZonePeriod._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
							xaDynamicZonePeriod._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
							xaDynamicZonePeriod._DynamicZone.startDateTime = lngLosecond & 0x3;
							xaDynamicZonePeriod._DynamicZone.lastDateTime_1 = 0;
							xaDynamicZonePeriod._DynamicZone.lastDateTime = 0;
						}else 
						{//only change the last date/time
							xaDynamicZonePeriod._DynamicZone.lastDateTime_1 = (((tpSJT.lowsecond - lngLastsecond) / 60) >> 2) & 0x3F;
							xaDynamicZonePeriod._DynamicZone.lastDateTime = ((tpSJT.lowsecond - lngLastsecond) / 60) & 0x3;
						}
					}else 
					{
						cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
						xaDynamicZonePeriod._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
						xaDynamicZonePeriod._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
						xaDynamicZonePeriod._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
						xaDynamicZonePeriod._DynamicZone.startDateTime = lngLosecond & 0x3;
						xaDynamicZonePeriod._DynamicZone.lastDateTime_1 = 0;
						xaDynamicZonePeriod._DynamicZone.lastDateTime = 0;
					}
				}
			}
		}else 
		{
			//update mode error
			if(cmd_buf[26] != 0x01)
				return CE_BADPARAM;
			//entry datetime
			cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
			xaDynamicZonePeriod._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
			xaDynamicZonePeriod._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
			xaDynamicZonePeriod._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
			xaDynamicZonePeriod._DynamicZone.startDateTime = lngLosecond & 0x3;
			xaDynamicZonePeriod._DynamicZone.lastDateTime_1 = 0;
			xaDynamicZonePeriod._DynamicZone.lastDateTime = 0;
			//entry station
			memcpy(&lngstation, &cmd_buf[27], 4);
			if(0 != (chCode = location_to_card(lngstation, &tpDZperiod.lastLocation)))
				return chCode;
			xaDynamicZonePeriod._DynamicZone.lastLocation_1 = (tpDZperiod.lastLocation >> 6) & 0x3F;
			xaDynamicZonePeriod._DynamicZone.lastLocation = tpDZperiod.lastLocation & 0x3f;
			
			xaDynamicZonePeriod._DynamicZone.status = 1;
			if(tpDZperiod.activated == 0)
			{
				if( memcmp(tpSJT.time_bcd, active_timebcd, 6) > 0)
					return CE_EXPIREDDATE;
				xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &tpDZperiod.validityStartDateTime);
				
				xaDynamicZonePeriod._DynamicZone.validityStartDateTime_1 = (tpDZperiod.validityStartDateTime >> 17) & 0x1F;
				xaDynamicZonePeriod._DynamicZone.validityStartDateTime_2 = (tpDZperiod.validityStartDateTime >> 9) & 0xFF;
				xaDynamicZonePeriod._DynamicZone.validityStartDateTime_3 = (tpDZperiod.validityStartDateTime >> 1) & 0xFF;
				xaDynamicZonePeriod._DynamicZone.validityStartDateTime = tpDZperiod.validityStartDateTime & 0x1;
				//activated
				xaDynamicZonePeriod._DynamicZone.activated = 1;
				
			}
		}
	}else 
	{//NON-Fee area
		if(chRejectCode == 0)
		{//entry status
			if(cmd_buf[26] != 0x03)
				return CE_BADPARAM;
			//travel start station
			if(0 != (chCode = card_to_location(tpDZperiod.lastLocation, &lngLocation)))
				return chCode;
			tpTxnProductPassCompensate.DevUdJourneyHdr_val.tripOriginLocation = toMoto(lngLocation);
			xaDynamicZonePeriod._DynamicZone.status = 2;
		}else
		{
			return CE_BADPARAM;
		}
	}
	//
	xaDynamicZonePeriod._DynamicZone.activeStatus = ((((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1);
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePeriod.buff, 16);
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZperiod.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZperiod.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZperiod.mac2;
	//mac
	tpTxnProductPassCompensate.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
	cnt2 = cnt = sizeof(TxnProductPassCompensationFare_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnProductPassCompensate.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnProductPassCompensate.SysComHdr_val.reservedField, sh_mac_len - 4 - 56);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[sh_mac_len - 56], &tpTxnProductPassCompensate.DevUdProductValidity_val.vStartDateTime, 56);
	//
	g_sha1txnsn = tpTxnProductPassCompensate.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnProductPassCompensate.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 0;
	ch_mac_sel = 4;
	sem_init(&g_samreturn, 0, 0);
	sem_post(&g_samcalwait);
	//write the page
	for(i = tpSJT.activeWritepage; i < (tpSJT.activeWritepage + 4); i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			reader_status = XA_RW_IDLE;
			return CE_WRITE;
		}
	}

	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memset(&out_buf[10], 0x00, 4);
	//balance-4
	memset(&out_buf[14], 0x00, 4);
	//lock status -1
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_PERIOD;
	//rfu-14
	memset(&out_buf[20], 0x00, 13);
	
	*out_len = 33;
	//UD length-2
	//UD record number-1
	out_buf[35] = 1;
	//UD record type-1
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD 
	sem_wait(&g_samreturn);
	memcpy(&out_buf[39], &tpTxnProductPassCompensate.AFCHead_val.operatorid[0], cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	//AR
	out_buf[39 + cnt2] = 0x00;
	out_buf[39 + cnt2 + 1] = 0x00;
	cnt += 2;
	reader_status = XA_RW_IDLE;
	(*out_len) += cnt;
#ifdef	DEBUG_PRINT
	PRINTK("UL ISSUE:");
	for(i = 0; i < (*out_len); i++)
		PRINTK("%02x", out_buf[i]);
	PRINTK("\n");
#endif

	return CE_OK;
}
char xa_ul_active(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned long lngLosecond;
unsigned short cnt;
char 	chCode;

	*out_len = 2;
	memcpy(tpSJT.time_bcd, &cmd_buf[6], 7);
	tpSJT.lowsecond = timestr2long(&cmd_buf[7]);
	
	memcpy(out_buf, "\x21\x05", 2);
	if((chCode = read_tocken_info()) != 0)
	{		
		return chCode;
	}
	memcpy(out_buf, "\x21\x06", 2);
	if((chCode = UL_TellSysCard(tpSZ.productType, NULL)) != 0)
	{
		return chCode;
	}
	memcpy(out_buf, "\x21\x07", 2);
	switch(tpSZ.productID)
	{
	case XA_FEETYPE_VALUE:			//value
		if(tpDZpurse.activated == 1)
			return CE_LOCKED_TICKET;
		xaDynamicZonePurse._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;
		cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
		xaDynamicZonePurse._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
		xaDynamicZonePurse._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
		xaDynamicZonePurse._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
		xaDynamicZonePurse._DynamicZone.startDateTime = lngLosecond & 0x3;
		xaDynamicZonePurse._DynamicZone.activated = 1;
		//start date time
		xaDynamicZonePurse._DynamicZone.validityStartDate_1 = (cnt >> 7) & 0x1F;
		xaDynamicZonePurse._DynamicZone.validityStartDate = cnt & 0x7F;
		memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePurse.buff, 16);
		break;
	case XA_FEETYPE_TIMES:
		if(tpDZmultiride.activated == 1)
			return CE_LOCKED_TICKET;
		xaDynamicZoneMultiride._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;
		cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
		xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_1 = (lngLosecond >> 17) & 0x1F;
		xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_2 = (lngLosecond >> 9) & 0xFF;
		xaDynamicZoneMultiride._DynamicZone.validityStartDateTime_3 = (lngLosecond >> 1) & 0xFF;
		xaDynamicZoneMultiride._DynamicZone.validityStartDateTime = lngLosecond & 0x1;
		xaDynamicZoneMultiride._DynamicZone.activated = 1;
		memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZoneMultiride.buff, 16);
		break;
	case XA_FEETYPE_PERIOD:
		if(tpDZperiod.activated == 1)
			return CE_LOCKED_TICKET;
		xaDynamicZonePeriod._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;
		cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
		xaDynamicZonePeriod._DynamicZone.validityStartDateTime_1 = (lngLosecond >> 17) & 0x1F;
		xaDynamicZonePeriod._DynamicZone.validityStartDateTime_2 = (lngLosecond >> 9) & 0xFF;
		xaDynamicZonePeriod._DynamicZone.validityStartDateTime_3 = (lngLosecond >> 1) & 0xFF;
		xaDynamicZonePeriod._DynamicZone.validityStartDateTime = lngLosecond & 0x1;
		xaDynamicZonePeriod._DynamicZone.activated = 1;
		memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePeriod.buff, 16);
		break;
	default:
		return CE_NON_FEETYPE;
	}

	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &cnt, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((cnt >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)cnt;

	//udsn-1
	out_buf[0] = 0;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memcpy(&out_buf[14], &tpSJT.balance, 4);
	//lock status -1
	out_buf[18] = 0x00;
	//product category
	out_buf[19] = tpSZ.productID;
	//rfu-14-----using the first byte as the PRODUCT CATEGORY
	memset(&out_buf[20], 0x00, 13);
	
	*out_len = 33;
	return CE_OK;
}



char xa_ul_repair(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode;

	*out_len = 2;
	//
	memcpy(out_buf, "\x21\x01", 2);
	chCode = read_tocken_info();
	if((chCode != CE_MACERR) && (chCode != CE_OK))
	{
		return chCode;
	}
	//
	return CE_COMMAND;
}

char xa_ul_reverse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode;

	*out_len = 2;
	memcpy(tpSJT.time_bcd, &cmd_buf[10], 7);
	tpSJT.lowsecond = timestr2long(&cmd_buf[11]);
	//
	memcpy(out_buf, "\x21\x01", 2);
	if((chCode = read_tocken_info()) != 0)
	{		
		return chCode;
	}
	memcpy(out_buf, "\x21\x06", 2);
	if((chCode = UL_TellSysCard(tpSZ.productType, NULL)) != 0)
	{
		return chCode;
	}
	//
	switch(tpSZ.productID)
	{
	case XA_FEETYPE_VALUE:			//value
		//
		tpTxnproductPurseReverse.SysComHdr_val.formatVersion = toMoto(tpSZ.Version);
		tpTxnproductPurseReverse.SysComHdr_val.udsn = ByteToLong(NULL, &cmd_buf[6]);//toMoto(*((long *)(&cmd_buf[13])));
		tpTxnproductPurseReverse.SysComHdr_val.txnDateTime = toMoto(tpSJT.lowsecond + TIME2000 - ZONE8);
		tpTxnproductPurseReverse.SysComHdr_val.udType = toMoto(3);
		tpTxnproductPurseReverse.SysComHdr_val.udSubtype = toMoto(81);

		tpTxnproductPurseReverse.SysAppCom_val.applicationPassengerType = toMoto(1);

		tpTxnproductPurseReverse.SysProductCom_val.productIssuerId = tpTxnProductPurseEntry.SysCardCom_val.cardissuerId = toMoto(tpTicketDef.ProductIssuer);
		tpTxnproductPurseReverse.SysCardCom_val.cardType = toMoto(XA_CARD_PHYSICAL_UL);
		tpTxnproductPurseReverse.SysProductCom_val.productType = toMoto(tpSZ.productType);
		tpTxnproductPurseReverse.SysProductCom_val.Ptsn = toMoto(tpDZpurse.transactionSequenceNumber);

		tpTxnproductPurseReverse.SysCardCom_val.cardSerialNumber = (*(long *)ul_data[3]);
	
		memset(&tpTxnproductPurseReverse.DevUdProductValidity_val.vStartDateTime, 0x00, sizeof(DevUdProductValidity_t));
		//
		return xa_ul_reverse_purse(cmd_buf, out_buf, out_len);
	default:
		return CE_NON_FEETYPE;
	}
}

char xa_ul_reverse_purse(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len)
{
unsigned char chCode;
unsigned long lngsrcstation, lngdesstation, lngLosecond;
unsigned short	shBalance, cnt, i;
unsigned char buf[20], start_timebcd[7], end_timebcd[7], entry_timebcd[7];
unsigned char chExitMac[4], tac[4];
unsigned short	shCardstation;

	if(tpDZpurse.activated == 0)
		return CE_NONACTIVED;
	if(tpDZpurse.cardStatus != XA_SJT_CARD_NOMAL)
		return CE_ISSUED;
	xa_daytodate(tpSZ.cardBaseDataTime, tpDZpurse.validityStartDate, &lngLosecond, &start_timebcd[0]);
	lngLosecond += (tpSZ.validityDuration * 24 * 3600);
	long2timestr(lngLosecond, &end_timebcd[0]);
	//
	memcpy(out_buf, "\x22\x0a", 2);
    if((chCode = UL_TellTesting(tpCmdInit.test)) != 0)
    {
		return chCode;
    }
	if(tpDZpurse.status != 0)
	{
		memcpy(out_buf, "\x22\x07", 2);
		return CE_NO_UPDATE_ENTRY;
	}
	
	//the balance must be zero for tac
	tpSJT.balance = tpSZ.purchaseValue;
	tpSJT.tranamount = tpSJT.balance;
	//dynamic zone
	xaDynamicZonePurse._DynamicZone.activeStatus = (((ul_data[tpSJT.activeWritepage][0] & 0x80) >> 7) + 1) & 0x1;;
	xaDynamicZonePurse._DynamicZone.cardStatus = XA_SJT_CARD_RECYCLE;
	memcpy(out_buf, "\x21\x0d", 2);
	//exit time
	cnt = xa_localtimeToMinute(tpSJT.time_bcd, tpSZ.cardBaseDataTime, &lngLosecond);
	xaDynamicZonePurse._DynamicZone.startDateTime_1 = (lngLosecond >> 18) & 0xF;
	xaDynamicZonePurse._DynamicZone.startDateTime_2 = (lngLosecond >> 10) & 0xFF;
	xaDynamicZonePurse._DynamicZone.startDateTime_3 = (lngLosecond >> 2) & 0xFF;
	xaDynamicZonePurse._DynamicZone.startDateTime = lngLosecond & 0x3;
	
	xaDynamicZonePurse._DynamicZone.lastDateTime_1 = 0;
	xaDynamicZonePurse._DynamicZone.lastDateTime = 0;
	//exit station
	lngsrcstation = 0x09000000 + (tpSJT.curstation[0] << 8) + tpSJT.curstation[1];
	if(0 != (chCode = location_to_card(lngsrcstation, &shCardstation)))
		return chCode;
	if(shCardstation != tpDZpurse.lastLocation)
		return CE_NOT_ISSUEDSTATION;
	//NOT PRE-ISSUED card
	if(tpDZpurse.origin != 0xfff)
		return CE_PREISSUED;
	xaDynamicZonePurse._DynamicZone.lastLocation_1 = (shCardstation >> 6) & 0x3F;
	xaDynamicZonePurse._DynamicZone.lastLocation = shCardstation & 0x3f;
	//
	xaDynamicZonePurse._DynamicZone.status = 0;
	xaDynamicZonePurse._DynamicZone.remainingValue_1 = 0;
	xaDynamicZonePurse._DynamicZone.remainingValue_2 = 0;
	xaDynamicZonePurse._DynamicZone.remainingValue = 0;
	
	tpDZpurse.totalPurchaseValue += tpDZpurse.remainingValue;
	xaDynamicZonePurse._DynamicZone.totalPurchaseValue_1 = (tpDZpurse.totalPurchaseValue >> 8) & 0x7F;
	xaDynamicZonePurse._DynamicZone.totalPurchaseValue = (unsigned char)tpDZpurse.totalPurchaseValue;
	//cal the mac2	
	memcpy(ul_data[tpSJT.activeWritepage], xaDynamicZonePurse.buff, 16);
	if((chCode = UL_CalInitMAC(tpSJT.activeWritepage, &tpDZpurse.mac2, NULL)) != 0)
	{
		return chCode;
	}
	ul_data[tpSJT.activeWritepage + 3][2] &= 0xF0;
	ul_data[tpSJT.activeWritepage + 3][2] |= ((tpDZpurse.mac2 >> 8) & 0xF);
	ul_data[tpSJT.activeWritepage + 3][3] = (unsigned char)tpDZpurse.mac2;
	//return and transaction record
	tpTxnproductPurseReverse.DevUdPurseCommonHdr_val.purseRemainingValue = 0;
	
	tpTxnproductPurseReverse.SysFinDetails_val.transactionValue = toMoto(tpSJT.tranamount);
	tpTxnproductPurseReverse.SysFinDetails_val.paymentMethod = toMoto(1);
	tpTxnproductPurseReverse.SysFinDetails_val.partialTransactionValue = 0;
	//

	//mac
	tpTxnproductPurseReverse.SysSecurityHdr_val.keyVersion = toMoto(tpSZ.keySetNumber);
	cnt = sizeof(TxnProductPurseIssueReverse_t);
	sh_mac_len = cnt - 12 - 10;
	memcpy(ch_mac_data, &tpTxnproductPurseReverse.SysComHdr_val.formatVersion, 40);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[40], &tpTxnproductPurseReverse.SysComHdr_val.reservedField, sh_mac_len - 4 - 40 - 24);
	sh_mac_len -= 4;
	memcpy(&ch_mac_data[sh_mac_len - 24], &tpTxnproductPurseReverse.DevUdProductValidity_val.vStartDateTime, 24);
	//bakup the TXN
	g_sha1txnsn = tpTxnproductPurseReverse.SysComHdr_val.udsn;
	tpYPT_txn_val.YPT_type = XA_SJT_FAMILY;
	tpYPT_txn_val.pYPT_txn = &out_buf[33];
	tpYPT_txn_val.pYPT_tac = &tpTxnproductPurseReverse.SysSecurityHdr_val.txnMac[0];
	tpYPT_txn_val.YPT_txnlen = cnt + 6;
	tpYPT_txn_val.YPT_flag = 0;
	ch_mac_sel = 4;
	sem_init(&g_samreturn, 0, 0);
	sem_post(&g_samcalwait);
	//write dyanmic zone
	for(i = tpSJT.activeWritepage; i < tpSJT.activeWritepage + 4; i++)
	{
		if(0 != standard_tocken_write(i, ul_data[i]))
		{
			reader_status = XA_RW_IDLE;
			return CE_WRITE;
		}
	}

	//udsn-1
	out_buf[0] = 1;
	//recycle-1
	out_buf[1] = 0x00;
	//blacklist-1
	out_buf[2] = 0x00;
	//family-1
	out_buf[3] = XA_SJT_FAMILY;
	//ticket type-2
	out_buf[4] = tpSZ.productType;
	out_buf[5] = 0;
	//logic-4
	out_buf[6] = ul_data[3][3]; out_buf[7] = ul_data[3][2];	
	out_buf[8] = ul_data[3][1]; out_buf[9] = ul_data[3][0];
	//BefBalance-4
	memcpy(&out_buf[10], &tpSJT.balance, 4);
	//balance-4
	memset(&out_buf[14], 0x00, 4);
	//lock status -1
	out_buf[18] = 0;
	//product category
	out_buf[19] = XA_FEETYPE_VALUE;
	//rfu-14
	memset(&out_buf[20], 0x00, 13);

	*out_len = 33;
	//UD record number 
	out_buf[35] = 1;
	//UD record type
	out_buf[36] = 0x02;
	//UD record length
	memcpy(&out_buf[37], &cnt, 2);
	//UD
	memcpy(&out_buf[39], tpTxnproductPurseReverse.AFCHead_val.operatorid, cnt);
	//UD length including the UD record length(sizeof) + record number(1) + record type(1) + ud length(2)
	cnt += 4;
	memcpy(&out_buf[33], &cnt, 2);
	cnt += 2;
	sem_wait(&g_samreturn);
	(*out_len) += cnt;
	//ee_write_last_record(XA_SJT_FAMILY, 0, &out_buf[33], cnt);
	reader_status = XA_RW_IDLE;
	return CE_OK;
	
label_refuse_to_exit:
	return chCode;
}

#ifdef DEBUG_TEST
char sz_test_ul(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len)
{
int ret;
unsigned char buf[40];
unsigned char cpubuf[80], cpulen, Le;
char chret;
unsigned char mac[4], mac1[2];

	*out_len = 1;
	chret = 0xff;
	switch(cmd_buf[7])
	{
	case 0x01:
		if(read_tocken_info() != 0)
		{
			return chret;
		}
		memcpy(out_buf, ul_data, 64);
		*out_len = 64;
		break;
	case 0x02:
		if(read_tocken_info() != 0)
		{
			return chret;
		}
		if(UL_CalInitMAC(mac) != 0)
			return chret;
		mac[0] ^= mac[1];
		mac[1] = mac[2] ^ mac[3];
		memcpy(&ul_data[5][2], mac, 2);
		standard_tocken_write(5, ul_data[5]);
		chret = CE_OK;
		break;
	}
	return chret;
}
#endif

