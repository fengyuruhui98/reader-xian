#ifndef BIN_EOD
#define BIN_EOD

//打开调试信息
//#define dbgout(format, ...) fprintf (stderr, format, ## __VA_ARGS__)
//关闭调试信息
//#define dbgout(...) 
//
#define	XA_PASSENGER_ADULT		0x01
#define	XA_PASSENGER_ELDER		0x03
#define XA_PASSENGER_STUDENT	0x04
#define	XA_PASSENGER_DISABLED	0x06


//xian Standard
struct ParaTitle
{//参数文件头（28 Byte）
	unsigned char format;				//1	包格式版本号	1	BIN	约定格式版本为0x01
	unsigned char source;				//2	数据来源方	1	BIN	0x01：ACC数据
	unsigned long length;				//3	数据包长度	4	BIN	数据包总长度，从包格式版本号开始（含）计算。传输时转换成INTEL序。
	unsigned char type[2];				//4	数据类型代码	2	BCD	1101
	unsigned long version;				//5	版本号	4	BIN	传输时转换成INTEL序。
	unsigned char create_time[7];		//6	生成时间	7	BCD	版本创建时间
	unsigned char start_time[4];		//7	生效日期	4	BCD	未到达该日期时，版本不能投入使用。
	unsigned short section_number;		//8	数据分段总数	2	BIN	本参数分段数：6。传输时转换成INTEL序。
	unsigned char rfu[3];				//9	预留字段	3	BIN	
}__attribute__( ( packed, aligned(1) ) );
typedef struct ParaTitle	ParaTitle;

struct section_offset
{
	unsigned long start_pos;			//10	分段起始偏移量	4	BIN	传输时转换成INTEL序。
	unsigned long section_rec;			//11	分段结构体记录数	4	BIN	本分段记录数，固定为1传输时转换成INTEL序。
};
typedef struct section_offset	section_offset;

struct system_parameters
{//分段1：系统参数（System Parameters）
	unsigned short	AuditRegisterSnapshotFrequency;		//12	2	BIN	ACC定义的审计数据发送时间间隔。Duration_t传输时转换成INTEL序。
	unsigned short	udKeyVersion;						//13	2	BIN	生成UD MAC 时的密钥版本编号。用于交易记录中产生MAC的密钥版本，与SAM卡中的密钥版本比对，不一致sam卡非法传输时转换成INTEL序。
	unsigned char 	TimeZone;							//14	1	BCD	时区，有效值范围1～24，依次表达东1区～东12区，西1区～西12区
}__attribute__( ( packed, aligned(1) ) );
typedef struct system_parameters	system_parameters;

struct service_provider_parameters
{//分段2：服务提供商参数（Service Provider Parameters）
	unsigned short	maxJourneyTime;						//2	BIN	乘客完成行程允许的最长时间。Duration_t传输时转换成INTEL序。
	unsigned short	MaxTransferTime;					//16	MaxTransferTime	2	BIN	最大换乘时间。Duration_t，指出站换乘时间传输时转换成INTEL序。
	unsigned short	maxExitTime;						//17	maxExitTime	2	BIN	补票后出站允许的最长时间。Duration_t传输时转换成INTEL序。
	unsigned char	exposePersonalDetails;              //18	exposePersonalDetails	1	BIN	查询票卡时设备是否向持卡人显示详细内容
	unsigned short	passbackTime;						//19	passbackTime	2	BIN	票卡再次在验票设备上使用间隔时间。Duration_t传输时转换成INTEL序。
	unsigned short	trainFaultValidityExtension;		//20	trainFaultValidityExtension	2	BIN	列车故障后票卡仍可使用的附加时间。Duration_t传输时转换成INTEL序。
	unsigned short	emergencyValidityExtension;			//21	emergencyValidityExtension	2	BIN	紧急出站时票卡仍可使用的附加时间。Duration_t传输时转换成INTEL序。
	unsigned short	modeCalendarValidityPeriod;			//22	modeCalendarValidityPeriod	2	BIN	模式履历中模式信息应保存的天数。Duration_t传输时转换成INTEL序。
	unsigned long	FinalRideMaxDiscountValue;			//23	FinalRideMaxDiscountValue	4	BIN	不够承担检出的旅程费用的钱包内的最大余额传输时转换成INTEL序。
	unsigned short	ImmediateRefundExpiryPeriod;		//24	ImmediateRefundExpiryPeriod	2	BIN	即时退款的期限。Duration_t传输时转换成INTEL序。
	unsigned long	ImmediateRefundMaximumThres;		//25	ImmediateRefundMaximumThreshold	4	BIN	最小允许退款金额传输时转换成INTEL序。
	unsigned short	ValidityToleranceDuration;			//26	ValidityToleranceDuration	2	BIN	过期产品的使用延长期。Duration_t传输时转换成INTEL序。
	unsigned short	BusinessDayDuration;				//27	BusinessDayDuration	2	BIN	正常营业日的持续时间，定义为跨日运营的扩展时间。Duration_t传输时转换成INTEL序。
	unsigned char	BusinessDayStart[3];				//28	BusinessDayStart	3	BCD	营业开始时间
	unsigned long	ServiceProviderId;					//29	ServiceProviderId	4	BIN	服务商ID传输时转换成INTEL序。
}__attribute__( ( packed, aligned(1) ) );
typedef struct service_provider_parameters service_provider_parameters;

struct participant_t
{//分段3：参与方参数（Participant）
//30	Participant 数量	1	BIN	
//参与方ID 1[1级循环]
//	unsigned long	*Participant_ID;				//31	Participant ID	4	BIN	传输时转换成INTEL序。约定固定值：0x41：一号线;0x42：二号线
//参与方ID…[1级循环]

//参与方详细信息1[1级循环]
	unsigned long 	Participant_ID;					//32	Participant ID	4	BIN	传输时转换成INTEL序。
	unsigned char	Participant_name_ch[20];		//33	中文名称	20	ASCII	
	unsigned char	Participant_name_en[20];		//34	英文名称	20	ASCII	
	unsigned short	Participant_number;				//35	可售车票产品类型数量	2	BIN	传输时转换成INTEL序。
//参与方详细信息1 — 可售产品类型记录1[2级循环]
	unsigned short	*ticket_type;					//36	车票产品类型	2	BIN	传输时转换成INTEL序。定义为出站票（0x01）、单程票（0x03）、预制单程票（0x05）、日票（0x06）、纪念票-定值（0x07）、纪念票-计次（0x09）、计次票-普通（0x11）、福利票（0x12）、储值票（0x10）、公务票等
//参与方详细信息1 — 可售产品类型记录…[2级循环]
//参与方详细信息…[1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef	struct participant_t participant_t;

struct CardPhyical_t
{
//票卡发行商信息1 — 卡物理类型记录1 [2级循环]
	unsigned char	Cardtype;						//40	Cardtype	1	BIN	卡物理类型，UL卡：0x03，CPU卡：0x02，交易记录中填写
	unsigned char	cardFormatVersion;				//41	cardFormatVersion	1	BIN	票卡发行商文件格式的版本
	unsigned short	encryptionKeyVersion;			//42	encryptionKeyVersion	2	BIN	密钥的版本。传输时转换成INTEL序。
	unsigned char	cardKeySetNumber;				//43	cardKeySetNumber	1	BIN	用于指示保护票卡发行商文件的安全，存取关键字的特殊设置
	unsigned short	maxLifeCycleCount;				//44	maxLifeCycleCount	2	BIN	票卡类型最多使用次数。传输时转成INTEL序。
	unsigned char	cardCanBeRecycled;				//45	cardCanBeRecycled	1	BIN	票卡是否为循环型
	unsigned long	cardDeposit;					//46	cardDeposit	4	BIN	购买卡时支付的可退押金值。传输时转换成INTEL序。
	unsigned long	cardFee;						//47	cardFee	4	BIN	购买卡时支付的不可退卡成本费。传输时转换成INTEL序。
}__attribute__( ( packed, aligned(1) ) );
typedef	struct CardPhyical_t CardPhyical_t;

struct CardSpecific
{//分段4：卡物理类型参数（CardSpecific）
//	unsigned long	CardParticipantNumber;			//37	票卡发行商数量	4	BIN	固定为1。传输时转换成INTEL序。
//票卡发行商信息1[1级循环]
	unsigned long	CardParticipantID;				//38	票卡发行商参与方ID	4	BIN	票卡发行商ID。传输时转换成INTEL序。
	unsigned char	CardPhyicalNumber;				//39	卡物理类型个数	1	BIN	
//票卡发行商信息1 — 卡物理类型记录… [2级循环]
	CardPhyical_t 	*CardPhyical;
	
//票卡发行商信息…[1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef	struct CardSpecific	CardSpecific;

struct ParticipantIdCodeMap
{//分段5：参与方编码映射参数（ParticipantIdCodeMap）
	//unsigned short	ParticipantId;					//48	ParticipantId数量	2	BIN	传输时转换成INTEL序。
//映射记录1 [1级循环]
	unsigned long	ParticipantId;					//49	ParticipantId 1	4	BIN	传输时转换成INTEL序。
	unsigned char	ParticipantCode;				//50	ParticipantCode	1	BIN	
//映射记录… [1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct	ParticipantIdCodeMap	ParticipantIdCodeMap;

struct CardTypeNameLanguage_t
{
//卡类型名称记录1 — 名称信息1 [2级循环]
	unsigned short	CardTypeNameLanguage;			//55	CardTypeNameLanguage	2	BIN	语言种类。传输时转换成INTEL序。
	unsigned char	CardTypeName[40];				//56	CardTypeName 	40	ASCII	对应语种的卡名称
}__attribute__( ( packed, aligned(1) ) );
typedef	struct CardTypeNameLanguage_t	CardTypeNameLanguage_t;

struct CardTypeNames_t
{//分段6：卡类型名称参数（CardTypeNames）
	//unsigned long	CardTypeNameID;					//51	票卡发行商参与方ID	4	BIN	票卡发行商ID。传输时转换成INTEL序。
	//unsigned char 	Cardphyicalnum;					//52	卡物理类型个数	1	BIN	
//卡类型名称记录1 [1级循环]
	unsigned char	cardType;						//53	cardType 1	1	BIN	卡物理类型
	unsigned short	cardLanguagenum;					//53	卡名称语种数量	2	BIN	传输时转换成INTEL序。
//卡类型名称记录1 — 名称信息… [2级循环]
	CardTypeNameLanguage_t *CardTypeNameLanguage_val;
//卡类型名称记录…[1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef	struct CardTypeNames_t	CardTypeNames_t;

struct PACC_1101
{
	ParaTitle 		paratitle;
	
	section_offset	*offset;
	
	system_parameters	system_parameters_val;
	
	service_provider_parameters	service_provider;

	unsigned char participant_len;
	unsigned long	*Participant_ID;
	participant_t	*participant_val;
	
	unsigned long	CardParticipantNumber;
	CardSpecific	*CardSpecific;
	
	unsigned short	ParticipantIDnum;
	ParticipantIdCodeMap	*ParticipantIdCodeMap_val;
	
	unsigned long	CardTypeNameID;
	unsigned char	Cardphyicalnum;
	CardTypeNames_t	*CardTypeName_val;
};
typedef	struct PACC_1101 PACC_1101;

//	分段数据偏移量1[1级循环]				
//分段起始偏移量	10	分段起始偏移量	4	BIN	传输时转换成INTEL序。
//分段结构体记录数	11	分段结构体记录数	4	BIN	本分段记录数，固定为1 传输时转换成INTEL序。
//	分段数据偏移量2~6[1级循环]				

struct SalesDiscount_t
{
	//优惠率记录1[1级循环]				
	unsigned short	Ticketnumber;				//14	Ticket number	2	BIN	车票数量，按Ticket number从小到大排列传输时转换成INTEL序。
	unsigned short	SalesVolumeDiscount;		//15	SalesVolumeDiscount	2	BIN	优惠率。传输时转换成INTEL序。
}__attribute__( ( packed, aligned(1) ) );
typedef	struct SalesDiscount_t SalesDiscount_t;

struct DeviceLocation_t
{
	unsigned long	FareLocationNumber;			//17	FareLocationNumber	4	BIN	车站位置，按FareLocationNumber从小到大排列
					//传输时转换成INTEL序。，虚拟的车站位置
					//车站位置组成定义：Byte0：0x09车站；0x11线路；
					//Byte1：
					//Byte2：线路
					//Byte3：车站
	unsigned long	DeviceLocationNumber;		//18	DeviceLocationNumber	4	BIN	设备位置，指实际车站位置，用于标记换乘站，进行快速对应。传输时转换成INTEL序。
};
typedef	struct DeviceLocation_t	DeviceLocation_t;

struct TransferStation_t
{
	//有效换乘站索引组合信息1[1级循环]				
	unsigned long	OdTransferStationOrigin;	//20	OdTransferStationOrigin	4	BIN	起点站。传输时转换成INTEL序。
	unsigned long	OdTransferStationDestination;		//21	OdTransferStationDestination	4	BIN	终点站。传输时转换成INTEL序。
	unsigned char	TransferStationIndex;		//22	TransferStationIndex	1	BIN	索引值
}__attribute__( ( packed, aligned(1) ) );
typedef	struct TransferStation_t TransferStation_t;

struct TransferMessage_t
{
	unsigned char	Exchangestationnumber;		//25	索引包含换乘站数量	1	BIN	
	unsigned char	Exchangestationindex;		//26	索引值	1	BIN	
	//有效换乘站索引详细记录1 — 换乘站信息1[2级循环]				
	unsigned long	*ExchangestationId;			//27	换乘站站码	4	BIN	传输时转换成INTEL序。
}__attribute__( ( packed, aligned(1) ) );
typedef	struct TransferMessage_t	TransferMessage_t;

struct BusinessRules_t
{//分段1：业务规则参数（Business Rules）				
	unsigned short	ExitTicketProductType;		//12	ExitTicketProductType	2	BIN	出站票产品类型，指的是出站票、计次、定期票等。传输时转换成INTEL序。
	unsigned short	SalesVolumeDiscountCount; 	//13	SalesVolumeDiscountCount 	2	BIN	传输时转换成INTEL序。
	//优惠率记录1[1级循环]				
	//unsigned short	Ticketnumber;				//14	Ticket number	2	BIN	车票数量，按Ticket number从小到大排列传输时转换成INTEL序。
	//unsigned short	SalesVolumeDiscount;		//15	SalesVolumeDiscount	2	BIN	优惠率。传输时转换成INTEL序。
	//优惠率记录…[1级循环]
	SalesDiscount_t	*SalesDiscount_val;
	unsigned short	DeviceLocationsAtFareLocationCount;		//16	DeviceLocationsAtFareLocationCount	2	BIN	车站设备位置传输时转换成INTEL序。
	//车站设备位置信息1[1级循环]				
	//unsigned long	FareLocationNumber;			//17	FareLocationNumber	4	BIN	车站位置，按FareLocationNumber从小到大排列
					//传输时转换成INTEL序。，虚拟的车站位置
					//车站位置组成定义：Byte0：0x09车站；0x11线路；
					//Byte1：
					//Byte2：线路
					//Byte3：车站
	//unsigned long	DeviceLocationNumber;		//18	DeviceLocationNumber	4	BIN	设备位置，指实际车站位置，用于标记换乘站，进行快速对应。传输时转换成INTEL序。
	//车站设备位置信息…[1级循环]
	DeviceLocation_t *DeviceLocation_val;
	
	unsigned long	ExchangeStationnumber;		//有效换乘站索引组合数量	19	有效换乘站索引组合数量	4	BIN	传输时转换成INTEL序。
	//有效换乘站索引组合信息1[1级循环]				
	//unsigned long	OdTransferStationOrigin;	//20	OdTransferStationOrigin	4	BIN	起点站。传输时转换成INTEL序。
	//unsigned long	OdTransferStationDestination;		//21	OdTransferStationDestination	4	BIN	终点站。传输时转换成INTEL序。
	//unsigned char	TransferStationIndex;		//22	TransferStationIndex	1	BIN	索引值
	//有效换乘站索引组合信息…[1级循环]
	TransferStation_t *TransferStation_val;
	
	unsigned long	ValidExchangenumber;		//有效换乘站索引个数	23	有效换乘站索引个数	4	BIN	传输时转换成INTEL序。
	//有效换乘站索引1[1级循环]
	unsigned char	*index;						//24	索引值	1	BIN	
	//有效换乘站索引…[1级循环]				
	
	//有效换乘站索引详细记录1[1级循环]				
	TransferMessage_t	*TransferMessage_val;
	//unsigned char	Exchangestationnumber;		//25	索引包含换乘站数量	1	BIN	
	
	//unsigned char	Exchangestationindex;		//26	索引值	1	BIN	
	//有效换乘站索引详细记录1 — 换乘站信息1[2级循环]				
	//unsigned long	ExchagestationId;			//27	换乘站站码	4	BIN	传输时转换成INTEL序。
	//有效换乘站索引详细记录1 — 换乘站信息…[2级循环]				
	//有效换乘站索引详细记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef struct BusinessRules_t BusinessRules_t;

struct LocationTypeName_t
{
	//语种1 — 位置名称信息1 [2级循环]				
	unsigned short	LocationTypeID;				//31	LocationTypeID	2	BIN	传输时转换成INTEL序。
	unsigned char	LocationTypeName[14];			//32	LocationTypeName	14	ASCII	
}__attribute__( ( packed, aligned(1) ) );
typedef	struct LocationTypeName_t LocationTypeName_t;

struct LocationLanguage_t
{
	//语种1 [1级循环]				
	unsigned short	LanguageId;					//29	LanguageId	2	BIN	语言类别ID。传输时转换成INTEL序。
	unsigned short	LocationTypenumber;			//30	LocationType的数量	2	BIN	传输时转换成INTEL序。
	LocationTypeName_t	*LocationTypeName_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct LocationLanguage_t	LocationLanguage_t;

struct Names_t
{
	//分段2：名称参数（Names）				
	unsigned char	NumberofLanguageId;			//28	Number of LanguageId	1	BIN	语言种类数量
	//语种1 [1级循环]				
	LocationLanguage_t *LocationLanguage_val;
	//unsigned short	LanguageId;					//29	LanguageId	2	BIN	语言类别ID。传输时转换成INTEL序。
	//unsigned short	LocationTypenumber;			//30	LocationType的数量	2	BIN	传输时转换成INTEL序。
	//语种1 — 位置名称信息1 [2级循环]				
	//unsigned short	LocationTypeID;				//31	LocationTypeID	2	BIN	传输时转换成INTEL序。
	//unsigned char	LocationTypeName[14];			//32	LocationTypeName	14	ASCII	
	//语种1 — 位置名称信息…[2级循环]				
	//语种… [1级循环]				
};
typedef	struct Names_t Names_t;

struct	CardholderFeeValue_t
{
	unsigned char	CardholderFeeType;		//36	CardholderFeeType	1	BIN	费用类别：
					//2：换卡；
					//5：退卡、退资；
					//13：个性化
					//对应物理卡，退卡进行叠加
	unsigned long	CardholderFeeFixedValue;		//37	CardholderFeeFixedValue	4	BIN	持卡人费用定值部分。传输时转换成INTEL序。
	unsigned short	CardholderFeePercent;			//38	CardholderFeePercent	2	BIN	持卡人费用百分比部分。传输时转换成INTEL序。卡上余额的百分比
}__attribute__( ( packed, aligned(1) ) );
typedef	struct	CardholderFeeValue_t	CardholderFeeValue_t;

struct	CardholderFee_t
{
	unsigned char	Cardtype;				//34	Cardtype	1	BIN	卡物理类型 1：Cpu卡：0x02；Ul卡：0x03
	unsigned short	CardholderFeeCount;		//35	CardholderFeeCount	2	BIN	传输时转换成INTEL序。
	//卡类别记录1 — 手续费信息1 [2级循环]
	CardholderFeeValue_t	*CardholderFeeValue_val;
}__attribute__( ( packed, aligned(1) ) );
typedef	struct	CardholderFee_t	CardholderFee_t;

struct CardholderFeeTypes_t
{
	//分段3：手续费参数（CardholderFeeTypes）				
	unsigned char	Numberofcardtype;		//33	Number of cardtype	1	BIN	卡物理类型数量
	//卡类别记录1 [1级循环]
	CardholderFee_t	*CardholderFee_val;
	//卡类别记录1 — 手续费信息… [2级循环]				
	//卡类别记录… [1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct	CardholderFeeTypes_t	CardholderFeeTypes_t;

struct TextString_t
{
	unsigned char	TextStringId;					//44	TextStringId	1	BIN	按照TextStringId从小到大排列。
	unsigned short	LanguageId;						//45	LanguageId	2	BIN	按照LanguageId从小到大排列传输时转换成INTEL序。
	unsigned char	DeviceDisplayString[21];			//46	DeviceDisplayString	21	ASCII	
	//语种字符串记录… [1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct TextString_t	TextString_t;

struct DeviceParameters_t
{	
	//分段4：设备参数（Device Parameters）				
	unsigned long	UdUploadFrequency;				//39	UdUploadFrequency	4	BIN	交易数据上传时间间隔。Duration_t 传输时转换成INTEL序。
	unsigned short	UdUploadTimeOfDay;				//40	UdUploadTimeOfDay	2	BCD	一天中开始上传UD的时间：hhmm
	unsigned short	UdUploadTxnCount;				//41	UdUploadTxnCount	2	BIN	单个数据包包含交易记录最大数量。传输时转换成INTEL序。
	unsigned char	MaxCardsToProcess;				//42	MaxCardsToProcess	1	BIN	
	unsigned short	DeviceDisplaynumber;			//43	设备显示各语种字符串数量	2	BIN	保留。传输时转换成INTEL序。
	//语种字符串记录1[1级循环]	
	TextString_t	*TextString_val;			
}__attribute__( ( packed, aligned(1) ) );
typedef	struct	DeviceParameters_t	DeviceParameters_t;

struct	TimeCodeText_t
{
	unsigned char	TimeCodeID;						//48	TimeCodeID	1	BIN	按照TimeCodeID从小到大排列
	unsigned short	LanguageId;						//49	LanguageId	2	BIN	按照LanguageId从小到大排列传输时转换成INTEL序。
	unsigned char	TimeCodeName[20];					//50	TimeCodeName	20	ASCII	
	//时间代码记录… [1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct TimeCodeText_t	TimeCodeText_t;

struct TimeCodes_t
{
	//分段5：时间代码参数（Time Codes）				
	unsigned short	LanguageTimeCodenumber;			//47	各语种的时间代码名称数量	2	BIN	传输时转换成INTEL序。
	//时间代码记录1[1级循环]
	TimeCodeText_t	*TimeCodeText_val;
};
typedef	struct	TimeCodes_t	TimeCodes_t;

struct	PassengerText_t
{
	unsigned char	PassengerType;					//52	PassengerType	1	BIN	按照PassengerType从小到大排列，包括有：
					//成人：0x01
					//儿童：0x02
					//老年人：0x03
					//学生：0x04
					//军人：0x05
					//残疾人：0x06
	unsigned short	LanguageId;						//53	LanguageId	2	BIN	按照LanguageId从小到大排列传输时转换成INTEL序。
	unsigned char	PassengerTypeName[20];				//54	PassengerTypeName	20	ASCII	
	//乘客类型记录… [1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct PassengerText_t	PassengerText_t;

struct PassengerTypes_t
{	
	//分段6：乘客类型参数（Passenger Types）				
	unsigned short	LanguagePassengernumber;		//51	各语种乘客类型名称数量	2	BIN	传输时转换成INTEL序。
	//乘客类型记录1[1级循环]
	PassengerText_t	*PassengerText_val;
};
typedef	struct PassengerTypes_t	PassengerTypes_t;

struct PACC_1102
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	BusinessRules_t	BusinessRules_val;
	
	Names_t Names_val;
	
	CardholderFeeTypes_t CardholderFeeTypes_val;
	
	DeviceParameters_t	DeviceParameters_val;
	
	TimeCodes_t	TimeCodes_val;
	
	PassengerTypes_t	PassengerTypes_val;
};
typedef	struct PACC_1102 PACC_1102;

struct CardBlacklist
{
	unsigned long 	CardID;				//13	卡ID	4	BIN	按从小到大的排列顺序。传输时转换成INTEL序。车票的逻辑ID
	unsigned short	LifeCycleCounter;	//14	LifeCycleCounter 预留	2	BIN	传输时转换成INTEL序。
	unsigned char	cardActionCode;		//15	cardActionCode	1	BIN	固定为0。
	unsigned char 	cardStatusCode;		//16	cardStatusCode	1	BIN	锁卡时写入票卡的锁卡状态。
												//11，遗失/被盗；2，其他原因。
	//独立黑名单卡记录…[1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct CardBlacklist	CardBlacklist_t;

struct CardBlack
{
	//分段1：独立黑名单卡参数
	unsigned long blacknum;						//12	黑名单卡数量	4	BIN	传输时转换成INTEL序。
	//独立黑名单卡记录1[1级循环]
	CardBlacklist_t	*CardBlacklist_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct CardBlack	CardBlack_t;

struct SectionCardBlacklist
{
	unsigned long 	StartCardID;			//18	卡起始ID	4	BIN	按从小到大的顺序排列。传输时转换成INTEL序。
	unsigned long 	EndCardID;				//19	卡结束ID	4	BIN	传输时转换成INTEL序。
	unsigned char 	cardActionCode;			//20	cardActionCode	1	BIN	
	unsigned char 	cardStatusCode;			//21	cardStatusCode	1	BIN	
}__attribute__( ( packed, aligned(1) ) );
typedef struct SectionCardBlacklist		SectionCardBlacklist_t;

struct SectionCardBlack
{
	unsigned short 	sectionnum;					//17	黑名单区段数量	2	BIN	传输时转换成INTEL序。
	//区段黑名单卡记录1[1级循环]
	SectionCardBlacklist_t	*SectionCardBlacklist_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct SectionCardBlack			SectionCardBlack_t;

struct ProductBlacklist
{
	unsigned long 		CardID;					//23	卡ID 	4	BIN	按从小到大顺序排列。传输时转换成INTEL序。
	unsigned short 		LifeCycleCounter;		//24	LifeCycleCounter	2	BIN	传输时转换成INTEL序。
	unsigned short 		ProductSerialNumber;	//25	ProductSerialNumber	2	BIN	传输时转换成INTEL序。
	unsigned short 		ProductType;			//26	ProductType	2	BIN	传输时转换成INTEL序。
	unsigned char 		actionSequenceNumber;	//27	actionSequenceNumber	1	BIN	锁卡操作序列号。（卡内操作序号加1）
	unsigned char 		productActionCode;		//28	productActionCode	1	BIN	写到UD 的产品动作代码
	unsigned char 		productStatusCode;		//29	productStatusCode	1	BIN	写到卡上的产品状态代码
}__attribute__( ( packed, aligned(1) ) );
typedef struct ProductBlacklist			ProductBlacklist_t;

struct ProductBlack
{
	unsigned short 		productnum;				//22	产品黑名单卡数量	2	BIN	传输时转换成INTEL序。
	//产品黑名单卡记录1[1级循环]
	ProductBlacklist_t	*ProductBlacklist_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct ProductBlack				ProductBlack_t;

struct SAMBlacklist
{
	unsigned long 	SAMID;						//31	SAM ID	4	BIN	按从小到大顺序排列。传输时转换成INTEL序。
	unsigned char 	StolenStartTime[7];			//32	StolenStartTime	7	BCD	
	unsigned char 	StolenEndTime[7];			//33	StolenEndTime	7	BCD	
}__attribute__( ( packed, aligned(1) ) );
typedef struct SAMBlacklist				SAMBlacklist_t;

struct SAMBlack
{
	unsigned short 	samnum;						//30	黑名单SAM卡数量	2	BIN	传输时转换成INTEL序。
	//SAM黑名单记录1[1级循环]
	SAMBlacklist_t	*SAMBlacklist_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct SAMBlack		SAMBlack_t;

struct HighCardBlacklist
{
	unsigned long 	CardID;						//35	Card Id	4	BIN	按从小到大顺序排列。传输时转换成INTEL序。
	unsigned short 	LifeCycleCounter;			//36	LifeCycleCounter	2	BIN	传输时转换成INTEL序。
}__attribute__( ( packed, aligned(1) ) );
typedef struct HighCardBlacklist			HighCardBlacklist_t;

struct HighCardBlack
{
	unsigned short 		highnum;				//34	高安全黑名单记录数量	2	BIN	传输时转换成INTEL序。
	//高安全黑名单记录1[1级循环]
	HighCardBlacklist_t	*HighCardBlacklist_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct HighCardBlack				HighCardBlack_t;

struct CardBatchlist
{
	unsigned long 		CardBatchNumber;		//38	CardBatchNumber	4	BIN	票卡批次编号，按照从小到大的顺序排列。传输时转换成INTEL序。
	unsigned short 		NumberOfCardBaseDates;	//39	Number of CardBaseDates	2	BIN	批次回收票卡的发行日期记录数量。传输时转换成INTEL序。
	//卡批次回收记录1 — 发行日期记录1 [2级循环]
	unsigned long 		*CardBaseDates;			//40	CardBaseDates	4	BCD	批次回收卡的发行日期
}__attribute__( ( packed, aligned(1) ) );
typedef struct CardBatchlist				CardBatchlist_t;

struct CardBatch
{
	unsigned short 		batchnum;				//37	卡批次回收记录数量	2	BIN	传输时转换成INTEL序。
	//卡批次回收记录1[1级循环]
	//卡批次回收记录1 — 发行日期记录… [2级循环]
	CardBatchlist_t		*CardBatchlist_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct CardBatch					CardBatch_t;

struct PACC_1104
{
	ParaTitle 		paratitle;
	
	section_offset	*offset;
	
	CardBlack_t		CardBlack_val;
	
	SectionCardBlack_t	SectionCardBlack_val;
	
	ProductBlack_t		ProductBlack_val;
	
	SAMBlack_t			SAMBlack_val;
	
	HighCardBlack_t		HighCardBlack_val;
	
	CardBatch_t			CardBatch_val;
};
typedef struct PACC_1104	PACC_1104;

struct ProductOffset_t
{
	unsigned long	ProductIssuer;					//13	ProductIssuer	4	BIN	卡发行商。传输时转换成INTEL序。
					//99：一卡通
					//1：PACC
	unsigned short	ProductType;					//14	ProductType	2	BIN	票种。传输时转换成INTEL序。指的是出站票（0x01）、单程票（0x03）等
																			//如果是一卡通则主卡在前，子卡在后
	unsigned long	ProductParam_Offset;			//15	ProductParam Offset	4	BIN	从“产品数”后开始，到该产品的“产品详细参数信息记录体”的开始首字节的偏移量。传输时转换成INTEL序。
	//车票产品记录偏移量信息…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct ProductOffset_t	ProductOffset_t;

struct subProductName_t
{
	unsigned short	ProductTypeVariantNameLanguages;			//70	ProductTypeVariantNameLanguages	2	BIN	语种编码。传输时转换成INTEL序。
																		//2245：汉字 3751：英文
	unsigned char	ProductTypeVariantName[20];					//71	ProductTypeVariantName	20	ASCII	子类型的名称。
	//车票产品记录1 — 产品子类型记录1 — 子产品名称信息…[3级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct	subProductName_t	subProductName_t;

struct subProduct_t
{
	unsigned short	ProductTypeVariants;			//52	ProductTypeVariants	2	BIN	产品子类型编号。，如定期纪念票30次、定期纪念票50次等 传输时转换成INTEL序。
	unsigned char	IsDestinationRequired;			//53	IsDestinationRequired	1	BIN	是否需要指定目的站：0，不需要；1，需要
	unsigned char	IsDestinationUserInput;			//54	IsDestinationUserInput	1	BIN	销售时是否设置目的站：
															//0，不设置（validityDestination = currentLocation） 1，设置
	unsigned char	IsOriginRequired;				//55	IsOriginRequired	1	BIN	是否需要指定出发站：0，不需要；1，需要
	unsigned char	IsOriginUserInput;				//56	IsOriginUserInput	1	BIN	销售时是否设置出发站：
															//0，不设置（validityOrigin = currentLocation） 1，设置
	unsigned char	IsStartDateTimeUserInput;		//57	IsStartDateTimeUserInput	1	BIN	销售设置开始日期时间标志：
															//0，不设置（validityStartDateTime = 当前） 1，设置
	unsigned char	IsValidityDestinationCurrentStation;		//58	IsValidityDestinationCurrentStation	1	BIN	销售设置validityDestination为当前站标志：
																		//0，否；1，是
	unsigned char	IsValidityOriginCurrentStation;			//59	IsValidityOriginCurrentStation	1	BIN	销售设置validityOrigin为当前站标志：
																	//0，否；1，是
	unsigned char	NumberOfRides;					//60	NumberOfRides	1	BIN	有效乘次数（仅适用于计次产品）
	unsigned short	SalesCodeTableId;				//61	SalesCodeTableId	2	BIN	销售代码表索引ID。传输时转换成INTEL序。
	unsigned short	SalesPatternId;					//62	SalesPatternId	2	BIN	销售模式表索引ID。传输时转换成INTEL序。
	unsigned short	SalesTableId;					//63	SalesTableId	2	BIN	销售票价表ID。传输时转换成INTEL序。
	unsigned char	ShelfLife[7];					//64	ShelfLife	7	BCD	产品激活截止日期，无则全填0。
	unsigned long	ValidityDestination;			//65	ValidityDestination	4	BIN	传输时转换成INTEL序。
	unsigned short	Duration;						//66	Duration	2	BIN	产品有效期间。Duration_t传输时转换成INTEL序。
	unsigned long	ValidityOrigin;					//67	ValidityOrigin	4	BIN	传输时转换成INTEL序。
	unsigned char	ValidityStartDateTime[7];		//68	ValidityStartDateTime	7	BCD	True时为默认的开始日期/时间。无则填0
	unsigned short	ProductTypeVariantNameLanguagesCount;		//69	ProductTypeVariantNameLanguagesCount	2	BIN	返回此产品的可用语言列表。传输时转换成INTEL序。
	subProductName_t	*subProductName_val;
}__attribute__( ( packed, aligned(1) ) );
typedef	struct	subProduct_t	subProduct_t;

struct	ProductName_t
{
	unsigned short	ProductNameLanguages;			//81	ProductNameLanguages	2	BIN	语种编码。传输时转换成INTEL序。
	unsigned char	ProductName[20];				//82	ProductName	20	ASCII	产品名称。
	//车票产品记录1 — 产品名称信息…[2级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct ProductName_t	ProductName_t;

struct	Product_t
{
	unsigned long	ProductIssuer;					//16	ProductIssuer	4	BIN	卡发行商。传输时转换成INTEL序。
	unsigned short	ProductType;					//17	ProductType	2	BIN	产品类型。传输时转换成INTEL序。
	unsigned char	CanAllowFreeRide;				//18	CanAllowFreeRide	1	BIN	免费许可标记：0，不允许；1，允许，免费许可时可不需计算费用
	unsigned char	CanApplySalesVolumeDiscount;	//19	CanApplySalesVolumeDiscount	1	BIN	折扣许可标记：0，不允许；1，允许
															//具体折扣值参见3.5.1.3 SalesVolumeDiscount字段，具体为1102中15域
	unsigned short	CalendarId;						//20	CalendarId	2	BIN	日历标识符。传输时转换成INTEL序。，同1107的14域日历编号
	unsigned char	CanBePersonalised;				//21	CanBePersonalised	1	BIN	车票个性化标记：0，非个性化；1，个性化
	unsigned char	CanBeRecycled;					//22	CanBeRecycled	1	BIN	车票回收标记：0，不回收；1，回收
	unsigned char	CanBeRefunded;					//23	CanBeRefunded	1	BIN	车票退票许可标记：0，不允许；1，允许
	unsigned char	CanBeReportedLost;				//24	CanBeReportedLost	1	BIN	车票挂失许可标记：0，不允许；1，允许
	unsigned char	CanHaveValueAdded;				//25	CanHaveValueAdded	1	BIN	车票充值许可标记：0，不允许；1，允许
	unsigned char	ChargeCardDeposit;				//26	ChargeCardDeposit	1	BIN	可退款卡押金标记：0，无；1，有
															//卡押金值参见3.5.1.2 分段4 cardDeposit字段，是1102的35域
	unsigned char	ChargeCardFee;					//27	ChargeCardFee	1	BIN	不可退款卡成本费标记：0，无；1，有 卡成本费值参见3.5.1.2 分段4 cardFee字段，指的是1102的34域
	unsigned char	ChargeFareOnCheckout;			//28	ChargeFareOnCheckout	1	BIN	基于出站时间计算行程费用标记：0，不应用，表示以进站时间计算票价；1，应用，表示当前时间计算票价
	unsigned long	DamagedCardInvalidTicketFine;	//29	DamagedCardInvalidTicketFine	4	BIN	损坏/非法票卡罚款金额。传输时转换成INTEL序。
	unsigned short	FareCodeTableId;				//30	FareCodeTableId	2	BIN	消费代码表索引ID。传输时转换成INTEL序。
	unsigned short	FarePatternId;					//31	FarePatternId	2	BIN	消费模式表索引ID。传输时转换成INTEL序。
	unsigned short	FareTableId;					//32	FareTableId	2	BIN	消费票价表ID。传输时转换成INTEL序。
	unsigned char	FirstUseAtStationOfIssue;		//33	FirstUseAtStationOfIssue	1	BIN	售票站进站限定标记：
															//0，允许在任意车站进站；1，只允许在售票车站进站。
	unsigned char	FreeRideAtStationOfIssue;		//34	FreeRideAtStationOfIssue	1	BIN	产品发行站上提供了一个免费乘次。
	unsigned char	IgnoreEntryExitSequence;		//35	IgnoreEntryExitSequence	1	BIN	进出站次序免检标记：0，检查；1，免检
	unsigned char	IgnoreInsufficientFunds;		//36	IgnoreInsufficientFunds	1	BIN	尾程优惠标记：0，不优惠；1，优惠
	unsigned char	IgnoreMaxJourneyTime;			//37	IgnoreMaxJourneyTime	1	BIN	乘车超时免检标记：0，检查；1，免检
	unsigned char	IgnorePassback;					//38	IgnorePassback	1	BIN	回传超时（Passback）免检标记：
															//0，检查；1，免检，刷卡的最小间隔时间，进站、出站都需要判断，时间值为1101的19域passbacktime
	unsigned char	IsProductAutoloadable;			//39	IsProductAutoloadable	1	BIN	自动充值许可标记：0，不允许；1，允许
	unsigned char	IsIssuedActivated;				//40	IsIssuedActivated	1	BIN	发行时激活标志
	unsigned long	MaxPurseReload;					//41	MaxPurseReload	4	BIN	钱包最大充值额度。传输时转换成INTEL序。
	unsigned char	MaxTransfersAllowed;			//42	MaxTransfersAllowed	1	BIN	换乘次数上限
	unsigned long	MinPurseReload;					//43	MinPurseReload	4	BIN	钱包最小充值额度。传输时转换成INTEL序。
	unsigned long	MinRemainingValue;				//44	MinRemainingValue	4	BIN	产品余值下限，金额或乘车次数。传输时转换成INTEL序。
	unsigned long	MultipleMinimumFareFine;		//45	MultipleMinimumFareFine	4	BIN	逃票罚款额度：最小费率的倍数。传输时转换成INTEL序。
	unsigned char	OverrideFirstUseAtStationOfIssue;	//46	OverrideFirstUseAtStationOfIssue	1	BIN	ACC预赋值产品标志：0，非预赋值；1，预赋值 ACC预赋值产品可在任意车站使用
	unsigned char	ProductCategory;				//47	ProductCategory	1	BIN	产品类别：钱包、计次、定期
	unsigned long	RefundHandlingFee;				//48	RefundHandlingFee	4	BIN	退款的手续费。传输时转换成INTEL序。
	unsigned char	IsSingleUseOnly;				//49	IsSingleUseOnly	1	BIN	若产品只使用一次，它将被拒收如果需要生成上次旅程的延期出站交易，但不会被拒收如果旅程开始时间距现在时间在3 小时以上（说明新的旅程不是换乘）。(0:false;1:true)
	unsigned char	IsTicketCapturedIfTrainFault;	//50	IsTicketCapturedIfTrainFault	1	BIN	列车故障模式下车票回收标志：0，不回收；1，回收。
	
	unsigned short	ProductTypeVariantsCount;		//51	ProductTypeVariantsCount	2	BIN	产品子类型个数。传输时转换成INTEL序。
	subProduct_t	*subProduct_val;
	
	unsigned short	Passengernumber;				//72	乘客类型记录数	2	BIN	传输时转换成INTEL序。
	//车票产品记录1 — 适用乘客类型记录1[2级循环]				
	unsigned char	*Passengertype;					//73	乘客类型	1	BIN	如成人、儿童等
	//车票产品记录1 — 乘客类型记录…[2级循环]				

	unsigned short	LoadableFenValuesCount;			//74	LoadableFenValuesCount	2	BIN	传输时转换成INTEL序。
	//车票产品记录1 —LoadableFenValues记录1[2级循环]				
	unsigned long	*LoadableFenValues;				//75	LoadableFenValues	4	BIN	可载入产品的以分为单位的金额。传输时转换成INTEL序。
	//车票产品记录1 —LoadableFenValues记录…[2级循环]				

	unsigned short	SellableCardTypesCount;			//76	SellableCardTypesCount	2	BIN	传输时转换成INTEL序。
	//车票产品记录1 — 适用票卡介质记录1 [2级循环]				
	unsigned char	*SellableCardTypes;				//77	SellableCardTypes	1	BIN	票卡介质类型，物理类型
	//车票产品记录1 — 适用票卡介质记录…[2级循环]				

	unsigned short	SellableDeviceTypesCount;		//78	SellableDeviceTypesCount	2	BIN	传输时转换成INTEL序。
	//车票产品记录1 — 适用售票设备类型记录1 [2级循环]				
	unsigned short	*SellableDeviceTypes;			//79	SellableDeviceTypes	2	BIN	传输时转换成INTEL序。

	//车票产品记录1 — 适用售票设备类型记录…[2级循环]				
	unsigned short	ProductNameLanguagesCount;		//80	ProductNameLanguagesCount	2	BIN	传输时转换成INTEL序。
	//车票产品记录1 — 产品名称信息1 [2级循环]
	ProductName_t	*ProductName_val;	
}__attribute__( ( packed, aligned(1) ) );
typedef	struct Product_t	Product_t;

struct TicketParameter_t
{//分段1：车票产品参数				
	unsigned short	Ticketnumber;					//12	车票产品数量	2	BIN	传输时转换成INTEL序。
	//车票产品记录偏移量信息1[1级循环]				
	ProductOffset_t	*ProductOffset_val;
	
	//车票产品记录1[1级循环]				
	Product_t	*Product_val;
	//车票产品记录1 — 产品子类型记录1[2级循环]				
	
	//车票产品记录1 — 产品子类型记录1 — 子产品名称信息1[3级循环]				
	//车票产品记录1 — 产品子类型记录…[2级循环]				

	//车票产品记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct TicketParameter_t	TicketParameter_t;


struct	PACC_1105
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	TicketParameter_t	TicketParameter_val;
};
typedef	struct PACC_1105	PACC_1105;

struct Location_t
{
	unsigned long	Location_Number;				//13	Location Number	4	BIN	位置编号，按照顺序从大到小排列。传输时转换成INTEL序。
															//车站/线路/区段（0x09车站、0x11线路）+预留+线路+车站，0x09
	unsigned char	LocationNamech[20];				//14	Location中文名称	20	ASCII	
	unsigned char	LocationNameen[60];				//15	Location英文名称	60	ASCII	
	unsigned char	IsTransferStation;				//16	IsTransferStation	1	BIN	换乘车站标志：0，非换乘站；1，换乘站
	unsigned char	IsGatedTransfer;				//17	IsGatedTransfer	1	BIN	换乘方式：0，无障碍换乘；1，刷卡换乘，表示出站换乘
	unsigned long	fareLocationNumber;				//18	fareLocationNumber	4	BIN	消费票价表适用位置编号。适用于票价参数（3.7.1.8、3.7.1.9）?传输时转换成INTEL序。换乘站时采用此值计算票价
	unsigned char	OverrideFirstUseAtStationOfIssue;		//19	OverrideFirstUseAtStationOfIssue	1	BIN	非本站销售车票进站许可标志：0，不允许，1，允许留
	unsigned char	Numberofsection;				//20	Number of section	1	BIN	线路/区段内车站车站数量。仅适用于线路或区段位置信息记录。车站位置信息记录，该字段填0。
	//位置信息记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct Location_t	Location_t;

struct SectionLocation_t
{
	unsigned long	Location_Number_Group;			//22	Location Number_Group	4	BIN	线路/区段位置编号，按照顺序从大到小排列。传输时转换成INTEL序。
	unsigned char	NumberofLocation;				//23	Number of Location	1	BIN	包含车站数量
	//线路/区段记录1 — 组内车站记录1[2级循环]				
	unsigned long	*LocationNumber_Station;		//24	Location Number_Station	4	BIN	车站位置编号。传输时转换成INTEL序。
}__attribute__( ( packed, aligned(1) ) );
typedef	struct SectionLocation_t	SectionLocation_t;

struct Locations_t
{//分段1：线路车站区段信息参数（Locations）				
	unsigned long	Locationnumber;					//12	位置信息数量	4	BIN	传输时转换成INTEL序。
	//位置信息记录1[1级循环]
	Location_t	*Location_val;
	
	unsigned long	sectionnumber;					//21	线路/区段记录数量	4	BIN	传输时转换成INTEL序。
	//线路/区段记录1[1级循环]
	SectionLocation_t	*SectionLocation_val;
	//线路/区段记录1 — 组内车站记录…[2级循环]	
	//线路/区段记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct Locations_t	Locations_t;

struct LocationCard_t
{
	unsigned long	LocationNumber;					//26	LocationNumber	4	BIN	按从小到大排列。传输时转换成INTEL序。
	unsigned short	CardLocationCode;				//27	CardLocationCode	2	BIN	传输时转换成INTEL序。
															//用于车票内记录车站位置信息，截取后12位
	//位置编号映射记录…[1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef	struct LocationCard_t	LocationCard_t;

struct LocationNumberCodeMap_t
{	
	//分段2：位置编号映射参数（LocationNumberCodeMap）				
	unsigned short	LocationNumberCodeMapnumber;	//25	位置信息数量	2	BIN	传输时转换成INTEL序。
	//位置编号映射记录1[1级循环]		
	LocationCard_t	*LocationCard_val;
};
typedef	struct LocationNumberCodeMap_t	LocationNumberCodeMap_t;

struct CardLocation_t
{
	unsigned short	CardLocationCode;				//29	CardLocationCode	2	BIN	按从小到大排列。传输时转换成INTEL序。
	unsigned long	LocationNumber;					//30	LocationNumber	4	BIN	传输时转换成INTEL序。
	//位置编码映射记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct CardLocation_t	CardLocation_t;

struct LocationCodeMap_t
{	
	//分段3：位置编码映射参数（LocationCodeMap）				
	unsigned short	LocationCodeMapnumber;			//28	位置信息数量	2	BIN	传输时转换成INTEL序。
	//位置编码映射记录1[1级循环]
	CardLocation_t	*CardLocation_val;
};
typedef	struct LocationCodeMap_t	LocationCodeMap_t;

struct	PACC_1106
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	Locations_t	Locations_val;
	
	LocationNumberCodeMap_t LocationNumberCodeMap_val;
	
	LocationCodeMap_t	LocationCodeMap_val;
};
typedef	struct PACC_1106	PACC_1106;

struct DateTypeId_t
{
	unsigned char	calendardate[4];		//16	日期	4	BCD	按照日期来进行排序
	unsigned short	datetypeId;				//17	日期类型ID	2	BIN	传输时转换成INTEL序。无特殊定义，所有日期均含有
	//日历记录1 — 日期记录…[2级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct DateTypeId_t	DateTypeId_t;

struct Calendar_t
{
	unsigned short	calendarId;				//14	日历编号	2	BIN	传输时转换成INTEL序。
	unsigned short	calendardatenumber;		//15	包含日期数量	2	BIN	传输时转换成INTEL序。
	//日历记录1 — 日期记录1[2级循环]
	DateTypeId_t	*DateTypeId_val;
	//日历记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct Calendar_t	Calendar_t;

struct Calendars_t
{//分段1：日历参数				
	unsigned short	calendarnumber;			//13	日历数量	2	BIN	固定为1。传输时转换成INTEL序。
	//日历记录1[1级循环]
	Calendar_t	*Calendar_val;
};
typedef	struct Calendars_t	Calendars_t;

struct Time_t
{
	unsigned char	endtime[2];				//21	时段结束时间	2	BCD	HH24MI，判断条件为小于等于 必须有当天最大时间：2359
	unsigned char	timeCodeId;				//22	时段ID	1	BIN	
}__attribute__( ( packed, aligned(1) ) );
typedef	struct	Time_t	Time_t;

struct	DateTime_t
{
	unsigned short	datetypeId;				//19	日期类型ID	2	BIN	传输时转换成INTEL序。
	unsigned char	timecodenumber;			//20	包含时段数量	1	BIN	
	//日期类型记录1 — 时段记录1 [2级循环]
	Time_t	*Time_val;
	//日期类型记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct	DateTime_t	DateTime_t;

struct	DateTimes_t
{	
	//分段2：日期类型参数				
	unsigned char	calendartypenumber;		//18	日期类型数量	1	BIN	固定为1。传输时转换成INTEL序。
	//日期类型记录1[1级循环]				
	//日期类型记录1 — 时段记录… [2级循环]	
	DateTime_t	*DateTime_val;			
}__attribute__( ( packed, aligned(1) ) );
typedef	struct DateTimes_t	DateTimes_t;

struct	PACC_1107
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	Calendars_t	Calendars_val;
	
	DateTimes_t	DateTimes_val;
};
typedef	struct PACC_1107	PACC_1107;


struct FareTableID_t
{
	unsigned short	FaretableID;					//14	FaretableID	2	BIN	消费票价表ID。传输时转换成INTEL序。
	unsigned long	offset;							//15	偏移量 	4	BIN	从“消费票价表数量”字段（不含）到消费票价表首字节的偏移量。传输时转换成INTEL序。
	//消费票价表数据偏移量记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct FareTableID_t	FareTableID_t;

struct FareTable
{
	unsigned short	FaretableID;					//16	FaretableID	2	BIN	消费票价表ID。传输时转换成INTEL序。
	unsigned long	minFare; 						//17	minFare 	4	BIN	费率表的最低票价。传输时转换成INTEL序。
	unsigned long	maxFare;						//18	maxFare 	4	BIN	费率表的最高票价。传输时转换成INTEL序。
	
	unsigned char	NumberofFareSet;				//19	Number of Fare Set	1	BIN	票价表的费率模式数（票价表列数）
	unsigned char	*FareSetID;						//20	Fare Set ID	1	BIN	费率优惠模式ID

	unsigned short	NumberofFareCode;				//21	Number of Fare Code	2	BIN	票价表的费率代码数量（票价表行数）
	//消费票价表1 — 费率代码记录1[2级循环]				
	unsigned short	*FareCode;						//22	Fare Code	2	BIN	费率代码

	unsigned long	*FarePrice;						//23	消费票价	4	BIN	单位：分或次。传输时转换成INTEL序。
}__attribute__( ( packed, aligned(1) ) );
typedef	struct FareTable	FareTable_t;

struct FareTableMatrix_t
{//分段1：消费票价表参数（FareTableMatrix）				
	unsigned short	FareTableMatrixnumber;			//13	消费票价表数量	2	BIN	传输时转换成INTEL序。
	//消费票价表数据偏移量记录1[1级循环]
	FareTableID_t	*FareTableID_val;
	//消费票价表1[1级循环]
	FareTable_t		*FareTable_val;
	//消费票价表1 — 费率优惠模式记录1[2级循环]				
	//消费票价表1 — 费率模式记录…[2级循环]				
	//消费票价表1 — 费率代码记录…[2级循环]				
	//消费票价表1 — 票价表行1 [2级循环]（票价表[Fare Code]）				
	//消费票价表1 — 票价表行1 — 票价表列1 [3级循环] （票价表[Fare Code，Fare Set ID]）				
	//消费票价表1 — 票价表行1 — 票价表列… [3级循环] 				
	//消费票价表1 — 票价表行… [2级循环]（票价表[Fare Code]）				
	//消费票价表…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct FareTableMatrix_t	FareTableMatrix_t;

struct FarepatternOffset_t
{
	unsigned short	FarepatternID;					//25	Fare pattern ID	2	BIN	消费费率模式表编号。传输时转换成INTEL序。
	unsigned long	offset;							//26	偏移量 	4	BIN	从“消费费率优惠模式表数量”字段（不含）到消费票价表首字节的偏移量。传输时转换成INTEL序。
	//消费费率优惠模式表数据偏移量记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct FarepatternOffset_t	FarepatternOffset_t;

struct FarePassengerTimecode_t
{
	unsigned char	AdultFareSetID;					//30	成人Fare Set ID	1	BIN	指定时段成人费率优惠模式ID
	unsigned char	ChildrenFareSetID;				//31	儿童Fare Set ID	1	BIN	指定时段儿童费率优惠模式ID
	unsigned char	OldFareSetID;					//32	老人Fare Set ID	1	BIN	指定时段老人费率优惠模式ID
	unsigned char	StudentFareSetID;				//33	学生Fare Set ID	1	BIN	指定时段学生费率优惠模式ID
	unsigned char	SoldierFareSetID;				//34	军人Fare Set ID	1	BIN	指定时段军人费率优惠模式ID
	unsigned char	DisabledFareSetID;				//35	残疾人Fare Set ID	1	BIN	指定时段残疾人费率优惠模式ID
	unsigned char	EmployeeFareSetID;				//36	员工Fare Set ID	1	BIN	指定时段员工费率优惠模式ID
	//消费费率优惠模式表1 — 时段优惠模式信息… [2级循环] 				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct FarePassengerTimecode_t FarePassengerTimecode_t;

struct Farepattern_t
{
	unsigned short	FarepatternID;					//27	Fare pattern ID 1	2	BIN	消费费率模式表ID。传输时转换成INTEL序。
	unsigned char	Timecodenumbers;				//28	Time code numbers	1	BIN	本费率优惠模式表使用时段代码数量
	//消费费率优惠模式表1 — 时段代码1[2级循环]				
	unsigned char	*TimeCode;						//29	Time Code	1	BIN	时间代码（定义见日历参数 3.7.1.7）
	//消费费率优惠模式表1 — 时段代码…[2级循环]				
	//消费费率优惠模式表1 — 时段优惠模式信息1 [2级循环]（费率优惠模式表[time code，乘客类型]）
	FarePassengerTimecode_t	*FarePassengerTimecode_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct 	Farepattern_t	Farepattern_t;
	
struct FarePatternMatrix_t
{	
	//分段2：消费费率优惠模式表参数（FarePatternMatrix）				
	unsigned short	FarePatternMatrixnumber;		//24	消费费率优惠模式表数量	2	BIN	传输时转换成INTEL序。
	//消费费率优惠模式表数据偏移量记录1[1级循环]				
	FarepatternOffset_t	*FarepatternOffset_val;
	//消费费率优惠模式表1[1级循环]
	Farepattern_t	*Farepattern_val;
	//消费费率优惠模式表…[1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct FarePatternMatrix_t FarePatternMatrix_t;

struct FareCodeMatrixOffset_t
{
	unsigned short	FareCodeMatrixId;				//38	消费费率代码表ID	2	BIN	传输时转换成INTEL序。
	unsigned long	offset;							//39	偏移量	4	BIN	从“消费费率代码表数量” 字段（不含）到消费费率代码表首字节的偏移量。传输时转换成INTEL序。
	//消费费率代码表数据偏移量记录…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef	struct FareCodeMatrixOffset_t	FareCodeMatrixOffset_t;

struct FareCode_t
{
	unsigned short	FareCodeMatrixId;				//40	消费费率代码表ID	2	BIN	传输时转换成INTEL序。
	unsigned short	stationnumber;					//41	站点数量	2	BIN	传输时转换成INTEL序。
	//消费费率代码表1 — 站点记录1[2级循环]				
	unsigned long	*locationcode;					//42	站点代码	4	BIN	按从小到大顺序排列。（定义见线网车站信息参数 3.7.1.6）传输时转换成INTEL序。
	//消费费率代码表1 — 站点记录…[2级循环]
	unsigned short	*FareCode;						//43	Fare Code	2	BIN	费率代码。传输时转换成INTEL序。
	//消费费率代码表1 — 出发站记录1 — 目的地站记录… [3级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef struct 	FareCode_t	FareCode_t;

struct FareCodeMatrix_t
{	
	//分段3：消费费率代码表参数（FareCodeMatrix）				
	unsigned short	FareCodeMatrixnumber;			//37	消费费率代码表数量	2	BIN	传输时转换成INTEL序。
	//消费费率代码表数据偏移量记录1[1级循环]
	FareCodeMatrixOffset_t	*FareCodeMatrixOffset_val;
	//消费费率代码表1[1级循环]				
	//消费费率代码表1 — 出发站记录1[2级循环]
	//消费费率代码表1 — 出发站记录1 — 目的地站记录1 [3级循环]
	FareCode_t	*FareCode_val;
	//消费费率代码表1 — 出发站记录…[2级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef struct FareCodeMatrix_t	FareCodeMatrix_t;

struct	PACC_1108
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	FareTableMatrix_t	FareTableMatrix_val;
	
	FarePatternMatrix_t	FarePatternMatrix_val;
	
	FareCodeMatrix_t	FareCodeMatrix_val;
};
typedef	struct PACC_1108	PACC_1108;

struct SalesubProduct_t
{
	unsigned short	ProductType;						//28	ProductType	2	BIN	产品类型代码（定义见车票产品参数 3.7.1.5）传输时转换成INTEL序。
	unsigned short	ProductVariantID;					//29	ProductVariantID	2	BIN	产品子类型编号。传输时转换成INTEL序。
	//销售费率优惠模式表1 — 车票产品信息…[2级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef struct SalesubProduct_t	SalesubProduct_t;

struct SaleFarePattern_t
{
	unsigned short	FarepatternID;						//26	Fare pattern ID 1	2	BIN	销售费率模式表ID。传输时转换成INTEL序。
	unsigned char	Productnumbers;						//27	Product numbers	1	BIN	本费率优惠模式表使用车票产品数量
	//销售费率优惠模式表1 — 车票产品信息1[2级循环]				
	SalesubProduct_t	*SalesubProduct_val;
	//销售费率优惠模式表1 — 产品销售模式信息1 [2级循环]（费率优惠模式表[产品，乘客类型]）				
	FarePassengerTimecode_t	*FarePassengerTimecode_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct SaleFarePattern_t	SaleFarePattern_t;

struct FarePatternMatrix_Sales
{	
	//分段2：销售费率优惠模式表参数（FarePatternMatrix_Sales）				
	unsigned short	FarePatternMatrixSalesnumber;		//23	销售费率优惠模式表数量	2	BIN	传输时转换成INTEL序。
	//销售费率优惠模式表数据偏移量记录1[1级循环]	
	FarepatternOffset_t	*FarepatternOffset_val;
	//销售费率优惠模式表数据偏移量记录…[1级循环]				
	//销售费率优惠模式表1[1级循环]
	//销售费率优惠模式表1 — 产品销售信息… [2级循环]
	SaleFarePattern_t	*SaleFarePattern_val;
	//销售费率优惠模式表…[1级循环]				
}__attribute__( ( packed, aligned(1) ) );
typedef struct FarePatternMatrix_Sales	FarePatternMatrix_Sales_t;

struct	PACC_1109
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	FareTableMatrix_t	FareTableMatrix_val;
	
	FarePatternMatrix_Sales_t	FarePatternMatrix_Sales_val;
	
	FareCodeMatrix_t	FareCodeMatrix_val;
}__attribute__( ( packed, aligned(1) ) );
typedef	struct PACC_1109	PACC_1109;

struct YKTBlack
{
	unsigned char	CityCode[2];					//12	城市代码	2	BCD	固定为7100，西安
	unsigned char 	Business[2];					//13	行业代码	2	BCD	
	unsigned char 	cardid[8];						//14	黑名单卡号	8	BCD	按卡号从小到大排序。，指应用序列号
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKTBlack			YKTBlack_t;

struct YKT_1901
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	YKTBlack_t		*YKTBlack_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKT_1901	YKT_1901;

struct YKTParameter
{//分段1：一卡通消费可用卡类型参数
	//消费可用卡类型记录1 [1级循环]
	unsigned char	phyical;				//14	卡物理类型	1	BIN	固定为1，CPU卡
	unsigned char	subtype;				//15	子卡类型	1	BCD	对应长安通接口定义的子卡类型。
	unsigned char	maintype;				//16	主卡类型	1	BCD	对应长安通接口定义的主卡类型。
	unsigned char	name[16];				//17	卡类型名称	16	ASCII	
	unsigned char	property;				//18	卡片属性	1	BIN	固定为1，储值卡。
	unsigned char	ruf[13];				//19	预留	13	BIN	固定为0xFF…
	//消费可用卡类型记录… [1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKTParameter		YKTParameter_t;

struct YKTPassengeMap
{
	//分段2：一卡通持卡人类型与地铁乘客类型映射参数
	//映射记录1 [1级循环]
	unsigned char	YKTpassenage;			//21	一卡通持卡人类型	1	BCD	
	unsigned char	passenage;				//22	地铁乘客类型	1	BCD	
	//映射记录…[1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKTPassengeMap	YKTPassengeMap_t;

struct YKT_1912
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	unsigned char	Appmode;				//12	应用模式	1	BIN	固定0x03
	unsigned short	cardnumber;				//13	消费可用卡类型记录数	2	BIN	传输时转换成INTEL序。
	YKTParameter_t	*YKTParameter_val;
	
	unsigned short	mapnumber;				//20	映射记录数	2	BIN	传输时转换成INTEL序。
	YKTPassengeMap_t	*YKTPassengeMap_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKT_1912	YKT_1912;


struct YKTProperty_t
{//分段1：一卡通卡片属性参数
	//卡片属性记录1 [1级循环]
	unsigned char	phyical;				//12	卡物理类型	1	BIN	固定为1，CPU卡
	unsigned char	subtype;				//13	子卡类型	1	BCD	对应长安通接口定义的子卡类型。
	unsigned char	maintype;				//14	主卡类型	1	BCD	对应长安通接口定义的主卡类型。
	unsigned char	name[16];				//15	卡类型名称	16	ASCII	最多8个汉字。
	unsigned short	rfu;					//16	预留	2	BIN	固定为0
	unsigned char	cardproperty;			//16	卡片交易属性	1	BIN	高位第1bit，储值卡售卡开关；
													//高位第2bit，储值卡充值开关；
													//高位第3bit，储值卡退卡开关；
													//高位第4bit，储值卡退资开关。
													//bit开关：0，关；1，开。
	//卡片属性记录… [1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKTProperty_t	YKTProperty_t;

struct YKT_1913
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	YKTProperty_t	*YKTProperty_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKT_1913	YKT_1913;


struct YKTTerminal_t
{//分段1：一卡通消费终端限额参数
	//消费终端限额记录1 [1级循环]
	unsigned char	phyical;				//12	卡物理类型	1	BIN	固定为1，CPU卡
	unsigned char	subtype;				//13	子卡类型	1	BCD	对应长安通接口定义的子卡类型。
	unsigned char	maintype;				//14	主卡类型	1	BCD	对应长安通接口定义的主卡类型。
	unsigned short	rfu1;					//15	预留	2	BIN	固定0xFFFF
	unsigned short	minbalance;				//16	进闸最小限额	2	BIN	传输时转换成INTEL序。
	unsigned short	max;					//17	最大透支限额	2	BIN	传输时转换成INTEL序。
	unsigned char	rfu2[8];				//18	预留	8	BIN	固定0xFF…
	//消费终端限额记录…[1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKTTerminal_t	YKTTerminal_t;

struct YKT_1919
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	YKTTerminal_t	*YKTTerminal_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKT_1919		YKT_1919;


struct YKTContinue_t
{//分段1：一卡通行业间联乘优惠参数
	//联乘优惠记录1 [1级循环]
	unsigned char	operation[3];			//12	联乘来源运营商代码	3	BCD	
	unsigned short	continueminute;			//13	换乘时间	2	BIN	单位：分钟。值为0 时表示联乘优惠模式无效。传输时转换成INTEL序。
	unsigned short	bonuspercent;			//14	优惠率（A）	2	BIN	收费百分比，80代表8折。传输时转换成INTEL序。
	unsigned short	bonusvalue;				//15	优惠额度（B）	2	BIN	单位：分。传输时转换成INTEL序。
	unsigned char	rfu[11];				//16	预留	11	BIN	0xFF…
	//联乘优惠记录… [1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKTContinue_t	YKTContinue_t;

struct YKT_1920
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	YKTContinue_t 	*YKTContinue_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKT_1920		YKT_1920;

struct YKTLoad_t
{//分段1：一卡通储值卡充值业务参数
	//售卡充值业务记录1 [1级循环]
	unsigned char	phyical;				//12	物理卡类型	1	BIN	
	unsigned char	subtype;				//13	子卡类型	1	BCD	
	unsigned char	maintype;				//14	主卡类型	1	BCD	
	unsigned char	name[16];				//15	卡类型名称	16	ASCII	右补空格0x00
	unsigned char	enabledsale;			//16	售卡许可	1	BIN	0，不允许售卡；1，允许售卡。
	unsigned char	enableload;				//17	充值许可	1	BIN	0，不允许充值；1，允许充值。
	unsigned short	firstloadvalue;			//18	首次充值最小额度	2	BIN	传输时转换成INTEL序。
	unsigned short	perloadvalue;			//19	单笔交易基数额度	2	BIN	传输时转换成INTEL序。
	unsigned long	maxloadvalue;			//20	单笔交易最大额度	4	BIN	单笔充值的最大交易金额。传输时转换成INTEL序。
	unsigned long	maxbalance;				//21	最大卡内余额	4	BIN	传输时转换成INTEL序。
	unsigned long	extentiondays;			//22	卡有效期顺延时间	4	BIN	顺延天数，在充值交易时，自动将卡有效期从当前日期顺延一定的天数，如730 天。0x00 表示不顺延。传输时转换成INTEL序。
	//售卡充值业务记录… [1级循环]
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKTLoad_t	YKTLoad_t;

struct YKT_1914
{
	ParaTitle	paratitle;
	
	section_offset	*offset;
	
	YKTLoad_t 	*YKTLoad_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKT_1914		YKT_1914;

struct Sensitive_Failure
{
	unsigned char 	execute_sle[4];
	unsigned short 	failure_code;
	unsigned char 	failure_start_time[7];
	unsigned char 	failure_end_time[7];
	unsigned char 	sensitive_time[4];
	
}__attribute__( ( packed, aligned(1) ) );
typedef struct Sensitive_Failure	Sensitive_Failure_t;

struct LCC_3021
{
	ParaTitle	paratitle;
	section_offset	offset;
	
	Sensitive_Failure_t	*Sensitive_Failure_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct LCC_3021		LCC_3021_t;

//transaction record
struct AFCHead_t
{
	unsigned char	operatorid[3];				//1	操作员ID	3	BCD	产生交易时设备上登录的操作员ID，如果没有操作员登录，填充为000000
	unsigned char	statisticday[4];			//2	发生日期	4	BCD	AFC系统的统计日期
	unsigned short	length;						//3	数据长度	2	BIN	传输时需要转换成INTEL序。
	unsigned char	rfu;						//4	预留	1	BIN	
}__attribute__( ( packed, aligned(1) ) );
typedef struct AFCHead_t	AFCHead_t;

struct SysComHdr_t
{
	unsigned long 	formatVersion;				//5	formatVersion	4	BIN	格式版本（下载），来源于车票内
	unsigned long	txnDateTime;				//6	txnDateTime	4	BIN	记录生成的时间UTC（本地）
	unsigned long	sourceParticipantId;		//7	sourceParticipantId	4	BIN	运营商的唯一ID 号（下载），一号线固定写0x41，其在1101中32域中有运营商的ID号列表
	unsigned long	deviceId;					//8	deviceId	4	BIN	设备的类型号码（本地），由设备类型+线路+车站+设备编号组成
	unsigned long	samId;						//9	samId	4	BIN	SAM 卡ID（本地），取SAM卡号16文件后四位
	unsigned long	udsn;						//10	udsn	4	BIN	UD 序列号，交易流水号，分别按一卡通（3.1.2）、一票通区分（3.1.1）维护
	unsigned long	serviceParticipantId;		//11	serviceParticipantId	4	BIN	运营商的唯一ID 号，同7
	unsigned long	deviceLocation;				//12	deviceLocation	4	BIN	设备的位置代码
	unsigned long	transactionStatus;			//13	transactionStatus	4	BIN	交易状态0:正常；1：测试
	unsigned long	cdVersion;					//14	cdVersion	4	BIN	配置数据的版本，默认填0
	unsigned long	reconciliationDate;			//15	reconciliationDate	4	BIN	数据生成日期(非MAC)（DateC20_t），上位机的数据转发日期
	unsigned long	reservedField;				//16	reservedField	4	BIN	预备
	unsigned long	udType;						//17	udType	4	BIN	UD组号=3
	unsigned long	udSubtype;					//18	udSubtype	4	BIN	UD子类别=90
}__attribute__( ( packed, aligned(1) ) );
typedef struct SysComHdr_t	SysComHdr_t;

struct SysCardCom_t
{
	unsigned long	cardissuerId;				//19	cardIssuerId	4	BIN	发行票卡的发行人的唯一ID
	unsigned long	cardSerialNumber;			//20	cardSerialNumber	4	BIN	票卡的序号
	unsigned long	cardType;					//21	cardType	4	BIN	票卡的类型
	unsigned long	cardLifeCycleCount;			//22	cardLifeCycleCount	4	BIN	票卡的当前使用周期计数
	unsigned long	cardActionSequenceNumber;	//23	cardActionSequenceNumber	4	BIN	操作表序列号（如果有）
}__attribute__( ( packed, aligned(1) ) );
typedef struct SysCardCom_t	SysCardCom_t;

struct SysCardholderCom_t
{
	unsigned long	crdholderSerianNum;			//24	cardholderSerialNum	4	BIN	在特定发行人范围内识别持卡人的专用号码
	unsigned long 	cardholderIssuerId;			//25	cardholderIssuerId	4	BIN	The issuer for the card holder 持卡人的发行人。（固定为ACC=0xffffffff）
	unsigned long 	companyId;					//26	companyId	4	BIN	ACC defined company ID number.ACC 定义公司标识符。For Staff ticket, this is the operator company ID.对员工票而言，这里指的是运营商编码。This is a proprietary participant identification number.这是参与方的标识码。
													//0..255 = Reserved for ACC and Operators participant IDs
													//0..255 = 为ACC 以及运营商预留。
													//256.. 65535 = available for other companies
													//256..65535 = 其他公司可用
	unsigned long 	classificationLevel;		//27	classificationLevel	4	BIN	Classification level of the passenger.
													//乘客级别1:VIP;255:未设
}__attribute__( ( packed, aligned(1) ) );
typedef struct SysCardholderCom_t	SysCardholderCom_t;

struct SysAppCom_t
{
	unsigned long	applicationProviderId;		//24	applicationProviderId	4	BIN	应用供应商（发布应用软件的供应商）的专用ID。
	unsigned long	applicationSerialNumber;	//25	applicationSerialNumber	4	BIN	它定义了支持该交易的应用
	unsigned long	applicationPersonalliseCat;	//26	applicationPersonaliseCat	4	BIN	适用于应用的个人化的种类 1.匿名的个性化卡的种类 2.记名应用的种类
	unsigned long	appActionSequenceNumber;	//27	appActionSequenceNumber	4	BIN	提供给与处理有关的票卡的操作表序列号（如果有）
	unsigned long 	applicationType;			//28	applicationType	4	BIN	1.transit 应用  255.未设
	unsigned long	applicationPassengerType;	//29	applicationPassengerType	4	BIN	乘客的类型.参看BEI-00110中的6 乘客编码规则
}__attribute__( ( packed, aligned(1) ) );
typedef struct SysAppCom_t	SysAppCom_t;

struct SysProductCom_t
{
	unsigned long	productIssuerId;			//30	productIssuerId	4	BIN	“0”表示无效，不能使用。“0xFFFFFFFF”表示未指.
	unsigned long	productSerialNumber;		//31	productSerialNumber	4	BIN	产品识别票卡的编号。
	unsigned long	productType;				//32	productType	4	BIN	详细说明用于完成处理的产品的类型
	unsigned long	productActionSequenceNumber;//33	productActionSequenceNumber	4	BIN	该值为一个专门用于排序的二进制值
	unsigned long	Ptsn;						//34	Ptsn	4	BIN	该txn 的产品序列号，当产品创建时设置
	unsigned long	invoicePrinted;				//35	invoicePrinted	4	BIN	指出是否某发票已打印。
}__attribute__( ( packed, aligned(1) ) );
typedef struct SysProductCom_t	SysProductCom_t;

struct DevUdJourneyHdr_t
{
	unsigned long	passengerType;				//36	passengerType	4	BIN	乘客的类型
	unsigned long	currentLocation;			//37	currentLocation	4	BIN	产生交易的站点， 1位位置类型+3位位置代码
	unsigned long	tripOriginLocation;			//38	tripOriginLocation	4	BIN	需由出站交易写入， 1位位置类型+3位位置代码
	unsigned long	tripPreviousLocation;		//39	tripPreviousLocation	4	BIN	刚刚经过的站点， 1位位置类型+3位位置代码
}__attribute__( ( packed, aligned(1) ) );
typedef struct DevUdJourneyHdr_t	DevUdJourneyHdr_t;

struct DevUdMultirideCommonHdr_t
{
	unsigned long	numRides;					//40	numRides	4	BIN	处理增加（增添）/扣除（使用）的乘次的数量
	unsigned long	remainingRides;				//41	remainingRides	4	BIN	处理完成后产品上剩余的乘次的数量
}__attribute__( ( packed, aligned(1) ) );
typedef struct DevUdMultirideCommonHdr_t	DevUdMultirideCommonHdr_t;

struct DevUdProductValidity_t
{
	unsigned long 	vStartDateTime;				//42	vStartDateTime	4	BIN	产品有效性开始的时间（也就是产品从此时开始有效）。如果产品有效性未定，那么，该字段为“0”。（UTC）
	unsigned long	vEndDateTime;				//43	vEndDateTime	4	BIN	产品有效性终止的时间（也就是产品在此时间之后无效）。如果产品有效性未定，那么，该字段为“0”。（UTC）
	unsigned long	vDuration;					//44	vDuration	4	BIN	The duration of the product. The units of measure are defined in CD.产品的有效期，单位在CD 里定义。
														//DurationUnit_t + DurationValue_t 高补0
	unsigned long	vOrigin;					//45	vOrigin	4	BIN	此产品可途经有效源站， 1位位置类型+3位位置代码，来自于卡内
	unsigned long	vDestination;				//46	vDestination	4	BIN	此产品可途经有效目的站点， 1位位置类型+3位位置代码，来自于卡内
}__attribute__( ( packed, aligned(1) ) );
typedef struct DevUdProductValidity_t	DevUdProductValidity_t;

struct DevUdPurseCommonHdr_t
{
	unsigned long	purseRemainingValue;		//40	purseRemainingValue	4	BIN	交易后的剩余金额
}__attribute__( ( packed, aligned(1) ) );
typedef struct DevUdPurseCommonHdr_t	DevUdPurseCommonHdr_t;

struct SysFinDetails_t
{
	unsigned long	transactionValue;			//41	transactionValue	4	BIN	交易涉及的实际财务量（以分为单位）
	unsigned long	paymentMethod;				//42	paymentMethod	4	BIN	支付方式 1现金 2 电子 3 优惠券 4 自动充值 255 未设
	unsigned long	partialTransactionValue;	//43	partialTransactionValue	4	BIN	应保留未取整的交易分数值，此字段在ACC 层上更新，而非由设备写入,固定为0，非MAC
}__attribute__( ( packed, aligned(1) ) );
typedef struct SysFinDetails_t	SysFinDetails_t;

struct DevUdMultirideLavHdr_t
{
	unsigned long	lavSamId;					//44	lavSamId	4	BIN	SAMID
	unsigned long	lavPariticipantId;			//45	lavParticipantId	4	BIN	负责为卡增值的代理商的ParticipantID。
	unsigned long	lavDate;					//46	lavDate	4	BIN	增值处理的日期和时间(DateC20_t)
	unsigned long	lavTxnValue;				//47	lavTxnValue	4	BIN	最近增值的货币值。
	unsigned long	lavRemainingValue;			//48	lavRemainingValue	4	BIN	完成增值后的余额
	unsigned long	lavPtsn;					//49	lavPtsn	4	BIN	加值交易时产品的交易顺序号
	unsigned long	lavMethodOfPayment;			//50	lavMethodOfPayment	4	BIN	增值使用的付款方法
	unsigned long	dataIsValid;				//51	dataIsValid	4	BIN	说明该标头中的数据是否有效（由于该数据并不总是通过设备提供）
	unsigned long	invoicePrinted;				//52	invoicePrinted	4	BIN	Specifies whether an invoice has been printed.发票是否打印
														//0 = Invoice not printed 发票未打印
														//1 = Invoice printed 发票已打印
}__attribute__( ( packed, aligned(1) ) );
typedef struct DevUdMultirideLavHdr_t	DevUdMultirideLavHdr_t;

struct DevUdPurseLavHdr_t
{
	unsigned long	lavSamId;					//44	lavSamId	4	BIN	SAMID
	unsigned long	lavPariticipantId;			//45	lavParticipantId	4	BIN	负责为卡增值的代理商的ParticipantID。
	unsigned long	lavDate;					//46	lavDate	4	BIN	增值处理的日期和时间(DateC20_t)
	unsigned long	lavTxnValue;				//47	lavTxnValue	4	BIN	最近增值的货币值。
	unsigned long	lavRemainingValue;			//48	lavRemainingValue	4	BIN	完成增值后的余额
	unsigned long	lavPtsn;					//49	lavPtsn	4	BIN	加值交易时产品的交易顺序号
	unsigned long	lavMethodOfPayment;			//50	lavMethodOfPayment	4	BIN	增值使用的付款方法
	unsigned long	dataIsValid;				//51	dataIsValid	4	BIN	说明该标头中的数据是否有效（由于该数据并不总是通过设备提供）
	unsigned long	invoicePrinted;				//52	invoicePrinted	4	BIN	Specifies whether an invoice has been printed.发票是否打印
														//0 = Invoice not printed 发票未打印
														//1 = Invoice printed 发票已打印
}__attribute__( ( packed, aligned(1) ) );
typedef struct DevUdPurseLavHdr_t	DevUdPurseLavHdr_t;

struct DevUdPassLavHdr_t
{
	unsigned long	lavSamId;					//44	lavSamId	4	BIN	SAMID
	unsigned long	lavPariticipantId;			//45	lavParticipantId	4	BIN	负责为卡增值的代理商的ParticipantID。
	unsigned long	lavDate;					//46	lavDate	4	BIN	增值处理的日期和时间(DateC20_t)
	unsigned long	lavTxnValue;				//47	lavTxnValue	4	BIN	最近增值的货币值。
	unsigned long	lavPassExpiryDateTime;		//48	lavPassExpiryDateTime	4	BIN	完成增值后的有效期。（UTC）
	unsigned long	lavPtsn;					//49	lavPtsn	4	BIN	加值交易时产品的交易顺序号
	unsigned long	lavMethodOfPayment;			//50	lavMethodOfPayment	4	BIN	增值使用的付款方法
	unsigned long	dataIsValid;				//51	dataIsValid	4	BIN	说明该标头中的数据是否有效（由于该数据并不总是通过设备提供）
	unsigned long	invoicePrinted;				//52	invoicePrinted	4	BIN	Specifies whether an invoice has been printed.发票是否打印
														//0 = Invoice not printed 发票未打印
														//1 = Invoice printed 发票已打印
}__attribute__( ( packed, aligned(1) ) );
typedef struct DevUdPassLavHdr_t	DevUdPassLavHdr_t;

struct SysSecurityHdr_t
{
	unsigned char	txnMac[8];					//56	txnMac	8	BIN	MAC
	unsigned long	keyVersion;					//57	keyVersion	4	BIN	用来产生交易MAC 的密钥版本
}__attribute__( ( packed, aligned(1) ) );
typedef struct SysSecurityHdr_t	SysSecurityHdr_t;
//************************************************************************
//entry transaction record for multiride
struct TxnProductMultirideUseOnEntry
{
	AFCHead_t		AFCHead_val;
	SysComHdr_t		SysComHdr_val;
	SysCardCom_t	SysCardCom_val;
	SysAppCom_t		SysAppCom_val;
	SysProductCom_t	SysProductCom_val;
	DevUdJourneyHdr_t	DevUdJourneyHdr_val;
	DevUdMultirideCommonHdr_t	DevUdMultirideCommonHdr_val;
	DevUdProductValidity_t		DevUdProductValidity_val;
	DevUdPurseLavHdr_t			DevUdMultirideLavHdr_val;
	unsigned long	startOfJourney;
	unsigned long	firstUseActivation;
	unsigned long	valuePerRide;
	SysSecurityHdr_t	SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductMultirideUseOnEntry	TxnProductMultirideUseOnEntry_t;
//entry transaction record for purse
struct TxnProductPurseUseOnEntry
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdJourneyHdr_t		DevUdJourneyHdr_val;
	DevUdPurseCommonHdr_t	DevUdPurseCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdPurseLavHdr_t		DevUdPurseLavHdr_val;
	unsigned long			startOfJourney;
	unsigned long			totalJourneyAmount;
	unsigned long 			firstUseActivation;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPurseUseOnEntry	TxnProductPurseUseOnEntry_t;
//entry transaction record for pass
struct TxnProductPassUseOnEntry	
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdJourneyHdr_t		DevUdJourneyHdr_val;
	unsigned long			passEndDateTime;
	DevUdProductValidity_t	DevUdProductValidity_val;
	DevUdPassLavHdr_t		DevUdPassLavHdr_val;
	unsigned long 			startOfJourney;
	unsigned long 			firstUseActivation;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPassUseOnEntry 	TxnProductPassUseOnEntry_t;
//exit transaction record for multiride
struct TxnProductMultirideUseOnExit
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdJourneyHdr_t		DevUdJourneyHdr_val;
	DevUdMultirideCommonHdr_t	DevUdMultirideCommonHdr_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	DevUdMultirideLavHdr_t	DevUdMultirideLavHdr_val;
	unsigned long 			entryTime;
	unsigned long 			cardCaptured;
	unsigned long 			endOfJourney;
	unsigned long 			valuePerRide;
	unsigned long 			firstUseActivation;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductMultirideUseOnExit		TxnProductMultirideUseOnExit_t;
//exit transaction record for pass
struct TxnProductPassUseOnExit
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdJourneyHdr_t		DevUdJourneyHdr_val;
	unsigned long 			passEndDateTime;
	DevUdProductValidity_t	DevUdProductValidity_val;
	DevUdPassLavHdr_t		DevUdPassLavHdr_val;
	unsigned long 			entryTime;
	unsigned long 			endOfJourney;
	unsigned long			firstUseActivation;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPassUseOnExit		TxnProductPassUseOnExit_t;
//exit transaction record for purse
struct TxnProductPurseUseOnExit
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdJourneyHdr_t		DevUdJourneyHdr_val;
	DevUdPurseCommonHdr_t	DevUdPurseCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdPurseLavHdr_t		DevUdPurseLavHdr_val;
	unsigned long 			entryTime;
	unsigned long 			cardCaptured;
	unsigned long 			totalJourneyAmount;
	unsigned long			endOfJourney;
	unsigned long			firstUseActivation;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPurseUseOnExit		TxnProductPurseUseOnExit_t;
//transaction record for multiride issue
struct TxnProductMultirideIssue_t
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdMultirideCommonHdr_t	DevUdMultirideCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductMultirideIssue_t	TxnProductMultirideIssue_t;
//transaction record for pass issue
struct TxnProductPassIssue_t
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	unsigned long			passEndDateTime;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPassIssue_t		TxnProductPassIssue_t;
//transaction record for purse issue 
struct TxnProductPurseIssue
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	unsigned long			purseRemainingValue;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPurseIssue		TxnProductPurseIssue_t;
//transaction record for exit-ticket issue
struct TxnProductMultirideExitTicketIssue
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdMultirideCommonHdr_t	DevUdMultirideCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	unsigned long 			invalidCardSN;
	unsigned long 			invalidProductIssuer;
	unsigned long 			invalidLifeCycleCount;
	unsigned long 			invalidProductType;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductMultirideExitTicketIssue		TxnProductMultirideExitTicketIssue_t;
//transaction record for mulriride compenstation
struct TxnProductMultirideCompensationFare_t
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdJourneyHdr_t		DevUdJourneyHdr_val;
	DevUdMultirideCommonHdr_t	DevUdMultirideCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	DevUdMultirideLavHdr_t	DevUdMultirideLavHdr_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductMultirideCompensationFare_t	TxnProductMultirideCompensationFare_t;
//transaction record for pass compensation
struct TxnProductPassCompensationFare_t
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdJourneyHdr_t		DevUdJourneyHdr_val;
	unsigned long 			passEndDateTime;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	DevUdPassLavHdr_t		DevUdPassLavHdr_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPassCompensationFare_t 		TxnProductPassCompensationFare_t;
//transaction record for purse compensation
struct TxnProductPurseCompensationFare_t
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdJourneyHdr_t		DevUdJourneyHdr_val;
	DevUdPurseCommonHdr_t	DevUdPurseCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdPurseLavHdr_t		DevUdPurseLavHdr_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPurseCompensationFare_t 		TxnProductPurseCompensationFare_t;
//transaction record for purse refund
struct TxnProductPurseRefund_t
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysCardholderCom_t		SysCardholderCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdPurseCommonHdr_t	DevUdPurseCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdPurseLavHdr_t		DevUdPurseLavHdr_val;
	unsigned	long		refundReason;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPurseRefund_t		TxnProductPurseRefund_t;
//transaction record for purse reverse
struct TxnProductPurseIssueReverse
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdPurseCommonHdr_t	DevUdPurseCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	unsigned	long		reversedUdsn;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPurseIssueReverse	TxnProductPurseIssueReverse_t;
//transaction record for blacklist card request
struct TxnEventBlacklistCardRequest
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	unsigned long 			reasonCode;
	unsigned char 			staffEntry[12];
	unsigned long 			startCardRange;
	unsigned long 			endCardRange;
	unsigned long 			highSecurity;
	unsigned long 			batchWithdraw;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnEventBlacklistCardRequest	TxnEventBlacklistCardRequest_t;
//transaction record for blacklist card request
struct TxnCardBlock
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	unsigned long 			reasonCode;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnCardBlock		TxnCardBlock_t;
//transaction record for multiride add
struct TxnProductMultirideAdd
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdMultirideCommonHdr_t	DevUdMultirideCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	DevUdMultirideLavHdr_t	DevUdMultirideLavHdr_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductMultirideAdd		TxnProductMultirideAdd_t;
//transaction record for Pass add
struct TxnProductPassAdd
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	unsigned long			passEndDateTime;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	DevUdPassLavHdr_t		DevUdPassLavHdr_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPassAdd		TxnProductPassAdd_t;
//transaction record for purse add
struct TxnProductPurseAdd
{
	AFCHead_t				AFCHead_val;
	SysComHdr_t				SysComHdr_val;
	SysCardCom_t			SysCardCom_val;
	SysAppCom_t				SysAppCom_val;
	SysProductCom_t			SysProductCom_val;
	DevUdPurseCommonHdr_t	DevUdPurseCommonHdr_val;
	SysFinDetails_t			SysFinDetails_val;
	DevUdProductValidity_t	DevUdProductValidity_val;
	DevUdPurseLavHdr_t		DevUdPurseLavHdr_val;
	SysSecurityHdr_t		SysSecurityHdr_val;
}__attribute__( ( packed, aligned(1) ) );
typedef struct TxnProductPurseAdd		TxnProductPurseAdd_t;

//**************************************************************
//transaction record for xian YKT purchase
struct YKTTxnPurchase
{
	AFCHead_t	AFCHead_val;
	unsigned char	udSubtype;					//1	交易类型 TxnType	udSubtype 	1	BIN	0x0C，进站；0x0D，出站；
														//0x0E，延迟进站；0x0F，延迟出站；
														//0x16，锁卡；0x11，补票（包括了各种更新处理）。
	unsigned char	udType;						//udType	1	BIN	0x21，YKT交易
	unsigned long 	LocalTxnSeq;				//2	本地流水号 LocalTxnSeq	4	BIN	取值范围为 1～4294967295 传输时需要转换成INTEL序。
	unsigned char 	PosId[6];					//3	PSAM机具号 PosId	6	BCD	
	unsigned char	SamId[8];					//4	PSAM物理卡号 SamId	8	BCD	
	unsigned long	SamSeq;						//5	PSAM流水号 SamSeq	4	BIN	取值范围为 1～4294967295 传输时需要转换成INTEL序。
	unsigned char	deviceid[4];				//6	产生交易终端编号	
														//设备类型	1	BIN
														//线路ID	1	BCD	
														//车站编号	1	BCD	
														//站内设备编号	1	BIN	
	unsigned char	PosOperId[3];				//7	操作员ID PosOperId	3	BCD	
	unsigned char	CityCode[2];				//8	城市代码 CityCode	2	BCD	固定7100，西安
	unsigned char	CardId[8];					//9	票卡ID CardId	8	BCD	卡片的卡内号
	unsigned long	CardCsn;					//10	卡物理序列号 CardCsn	4	BIN	传输时需要转换成INTEL序。
	unsigned short	CrdDebitCnt;				//11	卡计数器 CrdDebitCnt	2	BIN	取值范围为 1～65535传输时需要转换成INTEL序。
	unsigned char	CrdModel;					//12	卡物理类型 CardModel	1	BIN	1，CPU卡；3，M1卡。
	unsigned char	CrdMKnd;					//13	主卡类型 CrdMKnd	1	BCD	
	unsigned char	CrdSKnd;					//14	子卡类型 CrdSKnd	1	BCD	
	unsigned long	BefBalance;					//15	交易前余额 BefBalance	4	BIN	单位，分。取值范围为 0～4294967295传输时需要转换成INTEL序。
	unsigned long	TxnAmt;						//16	交易金额 TxnAmt	4	BIN	单位，分。取值范围为 0～4294967295传输时需要转换成INTEL序。
	unsigned char	UserType;					//17	优惠卡类型 UserType	1	BCD	00，无优惠；01，老年卡优惠；03，学生卡优惠；10，员工卡优惠
	unsigned long	OrigAmt;					//18	应收金额 OrigAmt	4	BIN	优惠前交易金额单位，分。取值范围为 0～4294967295传输时需要转换成INTEL序。
	unsigned char	TxnDate[4];					//19	交易发生日期 TxnDate	4	BCD	YYYYMMDD
	unsigned char	TxnTime[3];					//20	交易发生时间 TxnTime	3	BCD	hhmmss
	unsigned long	TAC;						//21	交易认证码 TAC	4	BIN	传输时需要转换成INTEL序。
	unsigned char	CrdVerNo;					//22	卡内版本号 CrdVerNo	1	BIN	取值范围为 1～255
	unsigned long	AftBalance;					//23	交易后余额 AftBalance	4	BIN	单位，分。取值范围为 0～4294967295传输时需要转换成INTEL序。
	unsigned long	participantid;				//24	产生交易的线路运营商ID	4	BIN	传输时需要转换成INTEL序。
	unsigned long	Txnlocation;				//25	产生交易车站编号	4	BIN	传输时需要转换成INTEL序。
	unsigned long	Enlocation;					//26	进站车站编号	4	BIN	仅适用于正常出站和延迟出站交易传输时需要转换成INTEL序。
	unsigned long	Entime;						//27	进站时间	4	BIN	仅适用于正常出站交易和延迟出站交易传输时需要转换成INTEL序。
	unsigned long	lastlocation;				//28	上一站点车站ID	4	BIN	传输时需要转换成INTEL序。
	unsigned char	Validday[4];				//29	票卡有效期	4	BCD	YYYYMMDD
	unsigned long	OrigEntime;					//30	延迟交易的原始发生时间	4	BIN	仅适用于延迟进站和延迟出站交易传输时需要转换成INTEL序。
	unsigned long	OrigEnlocation;				//31	延迟交易的原始发生车站ID	4	BIN	仅适用于延迟进站和延迟出站交易传输时需要转换成INTEL序。
	unsigned char	mode;						//32	造成延迟交易的运营模式	1	BIN	仅适用于延迟进站和延迟出站交易
}__attribute__( ( packed, aligned(1) ) );
typedef struct YKTTxnPurchase	YKTTxnPurchase_t;

struct YKTTxnLoad
{
	AFCHead_t	AFCHead_val;
	unsigned char	udSubtype;					//1	交易类型 TxnType	udSubtype 	1	BIN	0x0E，充值；0x0F，售卡。
	unsigned char	udType;						//		udType	1	BIN	0x08，YKT交易
	unsigned long	LocalTxnSeq;				//2	本地流水号 LocalTxnSeq	4	BIN	取值范围为 1～4294967295传输时需要转换成INTEL序。
	unsigned char	SamId[8];					//3	ISAM物理卡号 SamId	8	BCD	
	unsigned char	PosId[6];					//4	ISAM机具号 PosId	6	BCD	
	unsigned long	SamSeq;						//5	ISAM流水号 SamSeq	4	BIN	取值范围为 1～4294967295传输时需要转换成INTEL序。
	unsigned char	deviceid[4];				//6	产生交易终端编号	
														//设备类型	1	BIN	
														//线路ID	1	BCD	
														//车站编号	1	BCD	
														//站内设备编号	1	BIN	
	unsigned char	PosOperId[3];				//7	操作员ID PosOperId	3	BCD	
	unsigned char	CityCode[2];				//8	城市代码 CityCode	2	BCD	西安为7100，测试卡为0000
	unsigned char	CardId[8];					//9	票卡ID CardId	8	BCD	卡片的卡内号
	unsigned long	CardCsn;					//10	卡物理序列号 CardCsn	4	BIN	传输时需要转换成INTEL序。
												//		10：现金支付；11：储值卡内支付；12：支付宝电子支付；13：微信电子支付；14：银联电子支付；15：苹果电子支付；16：预留1；17：预留2；
	unsigned short	CrdDebitCnt;				//11	卡计数器 CrdDebitCnt	2	BIN	取值范围为 1～65535传输时需要转换成INTEL序。
	unsigned char	CardModel;					//12	卡物理类型 CardModel	1	BIN	1，长安通CPU卡
	unsigned char	CrdMKnd;					//13	主卡类型 CrdMKnd	1	BCD	
	unsigned char	CrdSKnd;					//14	子卡类型 CrdSKnd	1	BCD	
	unsigned char	TransType;					//15	业务类型 TransType	1	BCD	
	unsigned long	BefBalance;					//16	交易前余额 BefBalance	4	BIN	单位，分。允许负值。传输时需要转换成INTEL序。
	unsigned long	TxnAmt;						//17	交易金额 TxnAmt	4	BIN	单位，分。允许负值。传输时需要转换成INTEL序。
	unsigned long	Deposit;					//18	卡押金 Deposit	4	BIN	单位，分。允许负值。传输时需要转换成INTEL序。
	unsigned long	AftBalance;					//19	交易后余额 AftBalance	4	BIN	单位，分。允许负值。传输时需要转换成INTEL序。
	unsigned char	SaleDate[4];				//20	售卡日期 SaleDate	4	BCD	YYYYMMDD
	unsigned char	SaleMode;					//21	售卡方式 SaleMode	1	BIN	
	unsigned char	CardValDate[4];				//22	票卡有效期 CardValDate	4	BCD	YYYYMMDD
	unsigned char	TxnDate[4];					//23	交易发生日期 TxnDate	4	BCD	YYYYMMDD
	unsigned char	TxnTime[3];					//24	交易发生时间 TxnTime	3	BCD	hhmmss
	unsigned char	CrdVerNo;					//25	卡内版本号 CrdVerNo	1	BIN	取值范围为 1～255
	unsigned char	BatchNo[3];					//26	签到批次号 BatchNo	3	BCD	
	unsigned char	KeySeq[9];					//27	密钥授权流水号	9	BCD	
	//unsigned long	AuthSeq;					//28	额度授权流水号	4	BIN	传输时需要转换成INTEL序。
	unsigned char 	AuthSeq[4];
	unsigned short	Lasttype;					//29	上笔交易类型	2	BIN	传输时需要转换成INTEL序。
	unsigned char	LastPosid[6];				//30	上笔交易终端机具号	6	BCD	
	unsigned long	LastTxtAmt;					//31	上笔交易金额	4	BIN	传输时需要转换成INTEL序。
	unsigned short	LastCrdDebitCnt;			//32	上笔交易卡计数器	2	BIN	传输时需要转换成INTEL序。
	unsigned char	LastTxtTime[7];				//33	上笔交易时间	7	BCD	YYYYMMDDhhmmss
	unsigned long	LastBefBalance;				//34	上笔交易前金额	4	BIN	传输时需要转换成INTEL序。
	//unsigned long	LastTAC;					//35	上笔交易TAC	4	BIN	传输时需要转换成INTEL序。
	//unsigned long	TAC;						//36	交易认证码 TAC	4	BIN	传输时需要转换成INTEL序。
	//unsigned long	partipantid;				//37	产生交易的线路运营商ID	4	BIN	传输时需要转换成INTEL序。
	//unsigned long	TxnLocation;				//38	产生交易车站编号	4	BIN	传输时需要转换成INTEL序。
	unsigned char 	LastTAC[4];
	unsigned char 	TAC[4];
	unsigned char 	partipantid[4];
	unsigned char 	TxnLocation[4];
}__attribute__( ( packed) );//__attribute__( ( packed, aligned(1) ) );
typedef struct YKTTxnLoad	YKTTxnLoad_t;


//
struct PACC_1103
{
	ParaTitle	paratitle;
};
typedef	struct PACC_1103	PACC_1103;

struct PACC_1097
{
	ParaTitle	paratitle;
};
typedef	struct PACC_1097	PACC_1097;

struct PACC_1002
{
	ParaTitle	paratitle;
};
typedef	struct PACC_1002	PACC_1002;

//交通部
struct JTBBlack
{
//长安通黑名单记录1 [1级循环]
	unsigned char cardIssuer[11];					//发卡机构代码	11	ASCII	代码左对齐，不足11位右补空格。
	unsigned char PAN[19];							//黑名单卡号	19	ASCII	先按发卡机构代码排序，再按黑名单卡号排序
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBBlack			JTBBlack_t;

struct JTB_1931
{
	ParaTitle	paratitle;
	section_offset	*offset;
	
	JTBBlack_t	*JTBBlack_val;
};
typedef	struct JTB_1931	JTB_1931_t;

struct JTBWhite
{
//交通卡白名单记录1 [1级循环]
	unsigned char cardIssuer[11];					//发卡机构代码	11	ASCII	代码左对齐，不足11位右补空格。
	unsigned char IIN[10];							//卡Bin	10	ASCII	预留,不使用
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBWhite			JTBWhite_t;

struct JTB_1932
{
	ParaTitle	paratitle;
	section_offset	*offset;
	
	JTBWhite_t	*JTBWhite_val;
};
typedef	struct JTB_1932	JTB_1932_t;


struct JTBProperty
{
//卡片属性记录1 [1级循环]
	unsigned char cardIssuer[11];		//12	发卡机构代码	11	ASCII	代码左对齐，不足11位右补空格。（预留）
	unsigned char phyical;				//13	卡物理类型	1	BIN	固定为1，CPU卡
	unsigned char subtype;				//14	子卡类型	1	BCD	对应交通卡接口定义的子卡类型。
	unsigned char maintype;				//15	主卡类型	1	BCD	对应交通卡接口定义的主卡类型。
	unsigned char name[16];				//16	卡类型名称	16	ASCII	最多8个汉字。
	unsigned short ProductType;			//17	预留	2	BIN	固定为0
	unsigned char cardproperty;			//18	卡片交易属性	1	BIN	高位第1bit，储值卡售卡开关；
												//高位第2bit，储值卡充值开关；
												//高位第3bit，储值卡退卡开关；
												//高位第4bit，储值卡退资开关。
												//bit开关：0，关；1，开。
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBProperty			JTBProperty_t;

struct JTB_1933
{
	ParaTitle	paratitle;
	section_offset	*offset;
	
	JTBProperty_t	*JTBProperty_val;
};
typedef	struct JTB_1933	JTB_1933_t;

struct JTBTerminal
{
//消费终端限额记录1 [1级循环]
	unsigned char phyical;					//12	卡物理类型	1	BIN	固定为1，CPU卡
	unsigned short type;					//13 	ACC系统产品类型	2	BIN	INTEL字节序。
	unsigned short rfu1;					//14	预留	2	BIN	固定0xFFFF
	unsigned short minbalance;				//15	进闸最小限额	2	BIN	INTEL字节序。
	unsigned short max;						//16	最大透支限额	2	BIN	INTEL字节序。
	unsigned char rfu2[8];					//17	预留	8	BIN	固定0xFF…
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBTerminal			JTBTerminal_t;

struct JTB_1934
{
	ParaTitle	paratitle;
	section_offset	*offset;
	
	JTBTerminal_t	*JTBTerminal_val;
};
typedef	struct JTB_1934	JTB_1934_t;

struct JTBPreferential
{
//城市优惠记录1 [1级循环]
	unsigned char cardIssuer[11];			//12	发卡机构代码	11	ASCII	代码左对齐，不足11位右补空格。
	unsigned char phyical;					//13	卡物理类型	1	BIN	固定为1，CPU卡
	unsigned short type;					//14	ACC系统产品类型	2	BIN	INTEL字节序。
	unsigned short bonusPercent;			// 16	卡优惠率（A）	2	BIN	卡种优惠率
	unsigned short bonusValue;				//17	卡种优惠额度（B）	2	BIN	卡种优惠额度
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBPreferential			JTBPreferential_t;

struct JTB_1935
{
	ParaTitle	paratitle;
	section_offset	*offset;
	
	JTBPreferential_t	*JTBPreferential_val;
};
typedef	struct JTB_1935	JTB_1935_t;

struct JTBLoad
{
//售卡充值业务记录1 [1级循环]
	unsigned char phyical;					//12	物理卡类型	1	BIN	
	unsigned char subtype;					//13	子卡类型	1	BCD	
	unsigned char maintype;					//14	主卡类型	1	BCD	
	unsigned char name[16];					//15	卡类型名称	16	ASCII	右补空格0x00
	unsigned char enabledSale;				//16	售卡许可	1	BIN	0，不允许售卡；1，允许售卡。
	unsigned char enabledLoad;				//17	充值许可	1	BIN	0，不允许充值；1，允许充值。
	unsigned short firstloadvalue;			//18	首次充值最小额度	2	BIN	INTEL字节序。
	unsigned short perloadvalue;			//19	单笔交易基数额度	2	BIN	INTEL字节序。
	unsigned long maxloadvalue;				//20	单笔交易最大额度	4	BIN	单笔充值的最大交易金额。INTEL字节序。
	unsigned long maxbalance;				//21	最大卡内余额	4	BIN	INTEL字节序。
	unsigned long extentiondays;			//22	卡有效期顺延时间	4	BIN	顺延天数，在充值交易时，自动将卡有效期从当前日期顺延一定的天数，如730 天。0x00 表示不顺延。INTEL字节序。
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBLoad			JTBLoad_t;

struct JTB_1938
{
	ParaTitle	paratitle;
	section_offset	*offset;
	
	JTBLoad_t	*JTBLoad_val;
};
typedef	struct JTB_1938	JTB_1938_t;

struct JTBServer
{
//通讯记录1 [1级循环]
	unsigned char serverIP[40];				//12	服务器1 IP	40	ASCII	交通卡ISAM验证服务器IP
	unsigned char port[3];					//13	服务器1端口	3	BCD	服务器1 通讯端口，前补0
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBServer			JTBServer_t;

struct JTB_1939
{
	ParaTitle	paratitle;
	section_offset	*offset;
	
	JTBServer_t	*JTBServer_val;
};
typedef	struct JTB_1939	JTB_1939_t;

//non-local card
struct JTBTxnPurchaseEx
{
	unsigned char cardIssuer[11];				//35	发卡机构代码	11	ASCII	代码左对齐，不足11位右补空格。
	unsigned char businessCode[4];				//36	商户标识符	4	BCD	固定为71000301
	unsigned char businessType[2];				//37	商户类型	2	BCD	固定为4412
	unsigned char unionCity[2];					//38	互通城市代码	2	BCD	卡内城市代码
	unsigned char version;						//39	消费密钥版本	1	BIN	
	unsigned char index;						//40	消费密钥索引	1	BIN	
	unsigned char algorithm;					//41	算法标识	1	BIN	
	unsigned char updateType;					//42	补票类型	1	BIN	0x00，非补票交易；0x01 超程补票；0x02 超时补票；0x03 超时并且超程补票；0x10补进站；0x11 补出站；
	unsigned char payType;						//43	支付方式	1	BIN	支付方式 0x01现金 0x02 电子 0x03 优惠券 0x04 自动充值 0xFF 未设
	unsigned char rfu[10];						//44	预留	10		预留
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBTxnPurchaseEx	JTBTxnPurchaseEx_t;

//local card
struct JTBTxnPurchaseExII
{
	unsigned char cardIssuer[11];				//35	发卡机构代码	11	ASCII	代码左对齐，不足11位右补空格。
	unsigned char businessCode[4];				//36	商户标识符	4	BCD	固定为71000301
	unsigned char businessType[2];				//37	商户类型	2	BCD	固定为4412
	unsigned char unionCity[2];					//38	互通城市代码	2	BCD	卡内城市代码
	unsigned char updateType;					//42	补票类型	1	BIN	0x00，非补票交易；0x01 超程补票；0x02 超时补票；0x03 超时并且超程补票；0x10补进站；0x11 补出站；
	unsigned char payType;						//43	支付方式	1	BIN	支付方式 0x01现金 0x02 电子 0x03 优惠券 0x04 自动充值 0xFF 未设
	unsigned char rfu[10];						//44	预留	10		预留
}__attribute__( ( packed, aligned(1) ) );
typedef struct JTBTxnPurchaseExII	JTBTxnPurchaseExII_t;

#endif