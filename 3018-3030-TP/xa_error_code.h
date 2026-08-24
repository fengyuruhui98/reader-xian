#ifndef   SZ_ERROR_CODE
#define  SZ_ERROR_CODE

#define	ERR_ISSUE_MAC			2
#define	ERR_SJT_REFUND			16
#define	ERR_TICKET_STATUS		20
#define	ERR_OK					21
#define	ERR_REUNLOCK			33

#define	ERR_SZ_PSAM				250

#define	ERR_CONTIUE_MULTIFRAME	200
#define	ERR_TOTAL_FRAME			201
#define	ERR_CRC					202
#define	ERR_FILE_LENGTH			203
#define	ERR_MD5					204
#define	ERR_MULTI_TICKET		207
#define ERR_NOPARAMETER			212

//for xian
#define CE_OK					0x00
#define	CE_BLACKLIST			0x01
#define	CE_EXPIREDDATE			0x02
#define	CE_CARDSTATUS			0x03
#define	CE_ENOUGH_BALANCE		0x04
#define	CE_READ					0x05
#define	CE_WRITE				0x06
#define	CE_TPUSTATUS			0x07
#define	CE_LOCKED_TICKET		0x08
#define	CE_SEARCH				0x10
#define	CE_NON_FEETYPE			0x11
#define CE_NONACTIVED			0x12
#define	CE_MACERR				0x13
#define CE_NOCARD				0x14
#define CE_INVADLIDCARD			0x15
#define	CE_MULTI_TICKET			0x16
#define	CE_M1AUTH				0x20
#define	CE_SAMERR				0x30

#define	CE_METROPSAM			0x33
#define	CE_METROISAM			0x34

#define	CE_COMMAND				0x41
#define CE_CHECKERROR			0x42
#define CE_BADPARAM				0x43

#define	CE_NO_ENTRY				0x51
#define	CE_FREE_UPDATE_ENTRY	0x52
#define	CE_FEE_UPDATE_ENTRY		0x53
#define CE_NO_UPDATE_ENTRY		0x54
#define	CE_OVERTIME				0x55
#define	CE_OVERRIDE				0x56
#define CE_OVERFARETIME			0x57
#define	CE_NOT_ISSUEDSTATION	0x58
#define	CE_TESTING_STATUS		0x59
#define	CE_ISSUED				0x5A
#define CE_CUR_EXIT				0x5B
#define	CE_ZONE					0x5C
#define	CE_PREISSUED			0x5D

#define CE_NOAUTH				0x61
#define CE_ADD_MOVED			0x62
#define	CE_ADD_MAC2				0x63
#define	CE_CONSUME_MOVED		0x64
#define CE_OLD_PEAK				0x65
#define CE_FINISHED				0x66
#define CE_WHITELIST			0x67

#define	CE_SETPARA				0x70
#define	CE_EOD_FILE				0x71
#define	CE_UDERR				0x80

#define	CE_HARDERROR			0x90

//
#define	CE_PHYICALID			0x04
#define CE_READERROR			0x11
#define CE_WRITEERROR			0x12
#define	CE_CARDREMOVED			0x13
#define	CE_LOCKED				0x15
#define	CE_NORECORD				0x16

#define	CE_NOMETROSAM			0x19
#define	CE_INITED				0x27

#define	CE_TESTSTATUS			0x32
#define	CE_FORBID_TICKET		0x33
#define	CE_NOTISSUED			0x34
#define	CE_INVALIDDATE			0x35
#define	CE_NOT_ISUEDSTATION		0x38
#define	CE_ENOUGH_RIDES			0x40
#define	CE_FREE_EXIT			0x43
#define	CE_FORBITSTATION		0x45
#define	CE_INVALIDBALANCE		0x46
#define	CE_NOENABLED			0x47
#define	CE_LOCKEDCARD			0x48
#define	CE_NOREFUND				0x49
#define	CE_NOUPDATE				0x50
#define	CE_STARTDATE			0x52
#define	CE_NOPARAM				0x53

#define	CE_CONTINUE_MULTIFRAME	0x92
#define	CE_TOTAL_FRAME			0x93
#define	CE_FILE_LENGTH			0x94
#define	CE_MD5					0x95
#define	CE_FORMAT				0x96
#define	CE_NO_FILE				0x97
#define	CE_CRC					0x98

#define CE_UNKNOWN				0xff


#endif
