#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "binEOD.h"
#include "xa_error_code.h"
#include "bin_file_manage.h"
#include "hh_cpu_operation.h"

int xa_get_ticketName(int ticketType, unsigned char *chName, unsigned char *enName)
{
unsigned long	i, j;
Product_t *td;
	
	//set the truct to zero
	if(tpProduct1105.TicketParameter_val.Product_val == NULL)
		return CE_EOD_FILE;
	
	for(i = 0; i < tpProduct1105.TicketParameter_val.Ticketnumber; i++)
	{
		if(ticketType == tpProduct1105.TicketParameter_val.Product_val[i].ProductType)
		{
			td = &tpProduct1105.TicketParameter_val.Product_val[i];
#ifdef	DEBUG_PRINT
		printf("productissuer %08x type %04x free %02x discount %02x CalendarId %04x personalised %02x recycled %02x refund %02x lost %02x added %02x deposit %02x charge %02x checkout %02x damaged %08x FareCode %04x FarePattern %04x FareTable %04x issuedstation %02x freeride %02x ignoreentry %02x ignorefund %02x ignoretime %02x ignorepassback %02x autolaodable %02x issuedactive %02x maxpurse %08x maxtransfer %02x minpurse %08x minremaining %08x penalty %08x override %02x productCategory %02x refundfee %02x singleuse %02x trainfault %02x\n", 
			tpProduct1105.TicketParameter_val.Product_val[i].ProductIssuer, tpProduct1105.TicketParameter_val.Product_val[i].ProductType, tpProduct1105.TicketParameter_val.Product_val[i].CanAllowFreeRide,
			tpProduct1105.TicketParameter_val.Product_val[i].CanApplySalesVolumeDiscount, tpProduct1105.TicketParameter_val.Product_val[i].CalendarId, tpProduct1105.TicketParameter_val.Product_val[i].CanBePersonalised,
			tpProduct1105.TicketParameter_val.Product_val[i].CanBeRecycled, tpProduct1105.TicketParameter_val.Product_val[i].CanBeRefunded, tpProduct1105.TicketParameter_val.Product_val[i].CanBeReportedLost, 
			tpProduct1105.TicketParameter_val.Product_val[i].CanHaveValueAdded, tpProduct1105.TicketParameter_val.Product_val[i].ChargeCardDeposit,
			tpProduct1105.TicketParameter_val.Product_val[i].ChargeCardFee, tpProduct1105.TicketParameter_val.Product_val[i].ChargeFareOnCheckout, 
			tpProduct1105.TicketParameter_val.Product_val[i].DamagedCardInvalidTicketFine, tpProduct1105.TicketParameter_val.Product_val[i].FareCodeTableId,
			tpProduct1105.TicketParameter_val.Product_val[i].FarePatternId, tpProduct1105.TicketParameter_val.Product_val[i].FareTableId,
			tpProduct1105.TicketParameter_val.Product_val[i].FirstUseAtStationOfIssue, tpProduct1105.TicketParameter_val.Product_val[i].FreeRideAtStationOfIssue, tpProduct1105.TicketParameter_val.Product_val[i].IgnoreEntryExitSequence,
			tpProduct1105.TicketParameter_val.Product_val[i].IgnoreInsufficientFunds, tpProduct1105.TicketParameter_val.Product_val[i].IgnoreMaxJourneyTime, tpProduct1105.TicketParameter_val.Product_val[i].IgnorePassback,
			tpProduct1105.TicketParameter_val.Product_val[i].IsProductAutoloadable, tpProduct1105.TicketParameter_val.Product_val[i].IsIssuedActivated, tpProduct1105.TicketParameter_val.Product_val[i].MaxPurseReload, 
			tpProduct1105.TicketParameter_val.Product_val[i].MaxTransfersAllowed, tpProduct1105.TicketParameter_val.Product_val[i].MinPurseReload, tpProduct1105.TicketParameter_val.Product_val[i].MinRemainingValue,
			tpProduct1105.TicketParameter_val.Product_val[i].MultipleMinimumFareFine, tpProduct1105.TicketParameter_val.Product_val[i].OverrideFirstUseAtStationOfIssue, tpProduct1105.TicketParameter_val.Product_val[i].ProductCategory,
			tpProduct1105.TicketParameter_val.Product_val[i].RefundHandlingFee, tpProduct1105.TicketParameter_val.Product_val[i].IsSingleUseOnly, tpProduct1105.TicketParameter_val.Product_val[i].IsTicketCapturedIfTrainFault);
			
		printf("subproduct number %04x\n", tpProduct1105.TicketParameter_val.Product_val[i].ProductTypeVariantsCount);
#endif
			//
			for(j = 0; j < td->ProductNameLanguagesCount; j++)
			{
				if( td->ProductName_val[j].ProductNameLanguages == 0x08c5)
				{
					memcpy(chName, td->ProductName_val[j].ProductName, 20);
				}
				if( td->ProductName_val[j].ProductNameLanguages == 0x0ea7)
				{
					memcpy(enName, td->ProductName_val[j].ProductName, 20);
				}
			}
			return 0;
		}
	}
	return CE_EOD_FILE;
}

//line:0x110000+line(hex)
int xa_get_lineName(int line, unsigned char *chName, unsigned char *enName)
{
unsigned long 	srcindex, desindex, i;
unsigned long	srcstation, desstation;
unsigned char 	chCode;
unsigned int 	curstation_id;

	if(tpLocation1106.Locations_val.Location_val == NULL)
		return CE_EOD_FILE;
	
	curstation_id = line;
	for(i = 0; i < tpLocation1106.Locations_val.Locationnumber; i++)
	{
		if((curstation_id & 0xFF00FFFF) == (tpLocation1106.Locations_val.Location_val[i].Location_Number & 0xFF0000FF))
		{
			memcpy(chName, tpLocation1106.Locations_val.Location_val[i].LocationNamech, 20);
			memcpy(enName, tpLocation1106.Locations_val.Location_val[i].LocationNameen, 60);
			return 0;
		}
	}
	if(i >= tpLocation1106.Locations_val.Locationnumber)
		return CE_EOD_FILE;
		
	return CE_EOD_FILE;
}

//staion :0x0900+line_stationid(hex)
int xa_get_stationName(int station, unsigned char *chName, unsigned char *enName)
{
unsigned long 	srcindex, desindex, i;
unsigned long	srcstation, desstation;
unsigned char 	chCode;
unsigned int 	curstation_id;

	if(tpLocation1106.Locations_val.Location_val == NULL)
		return CE_EOD_FILE;
	
	curstation_id = station;
	for(i = 0; i < tpLocation1106.Locations_val.Locationnumber; i++)
	{
		if((curstation_id & 0xFF00FFFF) == (tpLocation1106.Locations_val.Location_val[i].Location_Number & 0xFF00FFFF))
		{
			memcpy(chName, tpLocation1106.Locations_val.Location_val[i].LocationNamech, 20);
			memcpy(enName, tpLocation1106.Locations_val.Location_val[i].LocationNameen, 60);
			return 0;
		}
	}
	if(i >= tpLocation1106.Locations_val.Locationnumber)
		return CE_EOD_FILE;
		
	return CE_EOD_FILE;}


void xa_get_rejectName(int reject, unsigned char *chName, unsigned char *enName)
{
char name1[100];	
char name2[100];

	memset(name1, 0x00, 100);
	memset(name2, 0x00, 100);
	switch(reject)
	{
	case CE_NO_ENTRY:			//0x51
		sprintf(name1, "未进站");
		sprintf(name2, "NO Entry");
		break;
	case CE_FREE_UPDATE_ENTRY:	//0x52
		sprintf(name1, "已进站――可免费更新");
		sprintf(name2, "Entry Ticket-Free Update");
		break;
	case CE_FEE_UPDATE_ENTRY:	//0x53
		sprintf(name1, "已进站――付费更新");
		sprintf(name2, "Entry Ticket-Fee Update");
		break;
	case CE_NO_UPDATE_ENTRY:	//0x54
		sprintf(name1, "已进站――车票回收，建议购买新车票");
		sprintf(name2, "Entry Ticket-Ticket Recycled, Issue another ticket");
		break;
	case CE_OVERTIME:			//0x55
		sprintf(name1, "超时");
		sprintf(name2, "Overtime");
		break;
	case CE_OVERRIDE:			//0x56
		sprintf(name1, "超程");
		sprintf(name2, "Overfare");
		break;
	case CE_OVERFARETIME:		//0x57
		sprintf(name1, "超时超程");
		sprintf(name2, "Overtime & Overfare");
		break;
	case CE_NOT_ISSUEDSTATION:	//0x58
		sprintf(name1, "非发售车站");
		sprintf(name2, "NOT Issued Station");
		break;
	case CE_TESTING_STATUS:		//0x59	测试位不匹配 
		sprintf(name1, "测试位不匹配");
		sprintf(name2, "Test Flag Matching");
		break;                       
	case CE_ISSUED:				//0x5A	未发售车票 
		sprintf(name1, "未发售车票");
		sprintf(name2, "No Issued Ticket");
		break;                  
	case CE_CUR_EXIT:			//0x5B	本站已出站――可发售免费出站票出站
		sprintf(name1, "本站已出站――可发售免费出站票出站");
		sprintf(name2, "Exit Ticket-Issue Free-exit Ticket");
		break;
	case CE_ZONE:				//0x5C	区域拒绝
		sprintf(name1, "区域拒绝");
		sprintf(name2, "");
		break;              
	case CE_PREISSUED:			//0x5D	非预售票不允许抵消 
		sprintf(name1, "非预售票不允许抵消");
		sprintf(name2, "Cancel Rejected");
		break;                      
	case CE_OK:					//0x00	正常
		sprintf(name1, "车票分析正常");
		sprintf(name2, "Normal");
		break;                      
	case CE_BLACKLIST:			//0x01	黑名单锁卡              
		sprintf(name1, "黑名单卡");
		sprintf(name2, "Blacklist");
		break;                      
	case CE_EXPIREDDATE:			//0x02	超过有效期              
		sprintf(name1, "过期");
		sprintf(name2, "Expired date");
		break;                      
	case CE_CARDSTATUS:			//0x03	卡片状态非法            
		sprintf(name1, "卡片状态错");
		sprintf(name2, "");
		break;                      
	case CE_ENOUGH_BALANCE:		//0x04	余额不足                
		sprintf(name1, "余额不足");
		sprintf(name2, "Not Enough Balance");
		break;                      
	case CE_READ:				//0x05	读卡失败                
		sprintf(name1, "读卡失败");
		sprintf(name2, "Read Failure");
		break;                      
	case CE_WRITE:				//0x06	写卡失败                
		sprintf(name1, "写卡失败");
		sprintf(name2, "Write Failure");
		break;                      
	case CE_TPUSTATUS:			//0x07	TPU状态非法             
		sprintf(name1, "状态错");
		sprintf(name2, "");
		break;                      
	case CE_LOCKED_TICKET:		//0x08	产品状态非法            
		sprintf(name1, "已锁卡");
		sprintf(name2, "Locked Card");
		break;                      
	case CE_SEARCH:				//0x10	寻卡类错误              
		sprintf(name1, "寻卡错误");
		sprintf(name2, "");
		break;                      
	case CE_NON_FEETYPE:			//0x11	无效产品类别            
		sprintf(name1, "无效产品类别");
		sprintf(name2, "Valid Family");
		break;                      
	case CE_NONACTIVED:			//0x12	产品未激活              
		sprintf(name1, "产品未激活");
		sprintf(name2, "Not Actived Product");
		break;                      
	case CE_MACERR:				//0x13	车票MAC错误             
		sprintf(name1, "车票MAC错误");
		sprintf(name2, "");
		break;                      
	case CE_NOCARD:				//0x14	无卡                    
		sprintf(name1, "未寻到卡");
		sprintf(name2, "");
		break;                      
	case CE_INVADLIDCARD:		//0x15	无效卡                  
		sprintf(name1, "无效卡");
		sprintf(name2, "Invalid Card");
		break;                      
	case CE_MULTI_TICKET:		//0x20	M1卡认证错误            
		sprintf(name1, "多张卡寻卡");
		sprintf(name2, "Multi-ticket");
		break;                      
	case CE_M1AUTH:				//0x30	SAM卡处理公共类错误     
		sprintf(name1, "M1卡认证失败");
		sprintf(name2, "");
		break;                      
	case CE_SAMERR:				
		sprintf(name1, "SAM故障");
		sprintf(name2, "");
		break;                      
	 
	case CE_METROPSAM:			//0x33	一票通PSAM操作类错误           
		sprintf(name1, "一票通PSAM卡错");
		sprintf(name2, "");
		break;                      
	case CE_METROISAM:			//0x34	一票通ISAM操作类错误           
		sprintf(name1, "一票通ISAM卡错");
		sprintf(name2, "");
		break;                      
	case CE_OLD_PEAK:			//0x65	高峰拒绝                       
		sprintf(name1, "高峰拒绝");
		sprintf(name2, "Peak Rejected");
		break;                      
	case CE_FINISHED:			//0x66	异地交易未完成                 
		sprintf(name1, "异地交易未完成");
		sprintf(name2, "Un-Finished Transaction on other union-city");
		break;                      
	case CE_WHITELIST:			//0x67	不在白名单                     
		sprintf(name1, "非互联互通城市");
		sprintf(name2, "Not Union-city");
		break;                      
	case CE_EOD_FILE:			//0x71	缺少参数                       
		sprintf(name1, "参数缺失");
		sprintf(name2, "No parameter");
		break;
	default:
		sprintf(name1, "未知错误");
		sprintf(name2, "Unknown");
		break;                      
	}

	//
	if(chName != NULL)
		strcpy(chName, name1);
	if(enName != NULL)
		strcpy(enName, name2);
	
	return ;
}

//line:0x110000+line(hex)
//public static native int get_all_line_name(int language, byte* pernumber, byte* pername );
int xa_get_all_lineName(int language, unsigned char *perNumber, unsigned char *lineName)
{
unsigned long 	srcindex, desindex, i, j, k;
unsigned long	srcstation, desstation;
unsigned char 	chCode;
unsigned int 	curstation_id;
unsigned char 	*lineIndex;

	if(tpLocation1106.Locations_val.Location_val == NULL)
		return 0;
	
	lineIndex = (unsigned char *)malloc(1);
	if( lineIndex == NULL )
		return 0;
	
	j = 0;	
	for(i = 0; i < tpLocation1106.Locations_val.Locationnumber; i++)
	{
		if( 0x11000000 == (tpLocation1106.Locations_val.Location_val[i].Location_Number & 0xFF000000))
		{
			lineIndex[j] = i;
			j++;
			lineIndex = (unsigned char *)realloc(lineIndex, j + 1);
			if( lineIndex == NULL )
				return 0;
		}
	}
	
	if( language == 0 )
	{//
		*perNumber = 20;
		lineName = (unsigned char *)malloc( j * 20);
		for(k = 0; k < j; k++)
		{
			memcpy(&lineName[k * 60], tpLocation1106.Locations_val.Location_val[lineIndex[k]].LocationNamech, 20);
		}
	}else
	{//
		*perNumber = 60;
		lineName = (unsigned char *)malloc( j * 60);
		for(k = 0; k < j; k++)
		{
			memcpy(&lineName[k * 60], tpLocation1106.Locations_val.Location_val[lineIndex[k]].LocationNameen, 60);
		}
	}
	
	return j;
}

unsigned long *lineRow, Numberoflines;
unsigned long *Numberofstations;
unsigned long **LinesStations;
//首先获取线路数，同时记录线路指针
//返回0表示无线路数，其它数表示线路数目
int xa_get_line_number()
{
unsigned long 	srcindex, desindex;
unsigned long  	i, lineNumber, j, k;
unsigned long 	lineLocation;

	if(tpLocation1106.Locations_val.Location_val == NULL)
		return 0;
	
	if( Numberoflines == 0 )
	{
		free(lineRow);
		lineRow = NULL;
	}
	//
	Numberoflines = 0;
	lineRow = (unsigned long *)malloc((Numberoflines + 1) * sizeof(long));
	for(i = 0; i < tpLocation1106.Locations_val.Locationnumber; i++)
	{
		if( 0x11000000 == (tpLocation1106.Locations_val.Location_val[i].Location_Number & 0xFF000000))
		{
			lineRow[Numberoflines] = i;
			Numberoflines ++;
			lineRow = (unsigned long *)realloc(lineRow, (Numberoflines + 1) * sizeof(long));
		}
	}
	//
	LinesStations = (unsigned long **)malloc(sizeof(unsigned long *) * Numberoflines);
	Numberofstations = (unsigned long *)malloc(sizeof(unsigned long) * Numberoflines);
	for(j = 0; j < Numberoflines; j++)
	{
		k = 0;
		LinesStations[j] = NULL;
		for(i = 0; i < tpLocation1106.Locations_val.Locationnumber; i++)
		{
			lineLocation = tpLocation1106.Locations_val.Location_val[lineRow[j]].Location_Number;
			if( (((lineLocation & 0x000000FF) << 8) + 0x09000000) == (tpLocation1106.Locations_val.Location_val[i].Location_Number & 0xFF00FF00) )
			{
				k++;
				LinesStations[j] = (unsigned long *)realloc(LinesStations[j], sizeof(unsigned long) * k);
				LinesStations[j][k - 1] = i;
			}
		}
		Numberofstations[j] = k;
	}
#ifdef	DEBUG_PRINT
	PRINTK("xian metro number of lines %d\n", Numberoflines);
	for(i = 0; i < Numberoflines; i++)
	{
		PRINTK("%dth line locationid %08x index = %d and number of staitons %d\n", 
			i, tpLocation1106.Locations_val.Location_val[lineRow[i]].Location_Number, 
			lineRow[i], Numberofstations[i]);
		for(j = 0; j < Numberofstations[i]; j++)
		{
			PRINTK(" %dth station locationid %08x index=%d\n",
				j, tpLocation1106.Locations_val.Location_val[LinesStations[i][j]].Location_Number, LinesStations[i][j]);
		}
	}
#endif
	return Numberoflines;
}
//根据线路代码逐次获取线路名称，同时记录所属车站数
//返回0表示成功
//其它值失败
int xa_get_line_index_name(int lineIndex, char *chName, char *enName, int *LocationID)
{
unsigned long 	i;

	if(lineIndex >= Numberoflines )
		return CE_BADPARAM;
	if(lineRow == NULL)
		return CE_BADPARAM;
		
	i = lineRow[lineIndex];
	memcpy(chName, tpLocation1106.Locations_val.Location_val[i].LocationNamech, 20);
	memcpy(enName, tpLocation1106.Locations_val.Location_val[i].LocationNameen, 60);
	*LocationID = tpLocation1106.Locations_val.Location_val[i].Location_Number;
	
	return 0;
}
//根据线路索引获取指定线路的所属车站数
//返回0表示无线路下属车站数，其它数表示所属线路的车站数目
int xa_get_line_index_station_number(int lineIndex)
{
	if( lineIndex >= Numberoflines)
		return 0;
	
	if( Numberofstations == NULL)
		return 0;
	
	return Numberofstations[lineIndex];
}

//根据线路代码逐次获取线路车站名称，同时记录所属车站数
//返回0表示成功
//其它值失败
int xa_get_line_index_station_index_name(int lineIndex, int stationIndex, char *chName, char *enName, int *LocationID)
{
unsigned long LocationIndex;

	if( lineIndex >= Numberoflines )
		return CE_BADPARAM;
	
	if( stationIndex >= Numberofstations[lineIndex] )
		return CE_BADPARAM;
		
	LocationIndex = LinesStations[lineIndex][stationIndex];
	
	memcpy(chName, tpLocation1106.Locations_val.Location_val[LocationIndex].LocationNamech, 20);
	memcpy(enName, tpLocation1106.Locations_val.Location_val[LocationIndex].LocationNameen, 60);
	*LocationID = tpLocation1106.Locations_val.Location_val[LocationIndex].Location_Number;
	
	return 0;
}