#ifndef  KM_ERROR_CODE
#define  KM_ERROR_CODE

#define CE_OK					0x00
#define CE_CHECKERROR			0x01
#define CE_BADCOMMAND			0x02
#define CE_BADPARAM				0x03
#define	CE_PHYICALID			0x04
#define	CE_STATUSERROR			0x05
#define CE_READERROR			0x11
#define CE_WRITEERROR			0x12
#define	CE_CARDREMOVED			0x13
#define	CE_LOCKED				0x15
#define	CE_NORECORD				0x16

#define	CE_NOMETROSAM			0x19
#define	CE_INITED				0x27
#define	CE_MULTI_TICKET			0x29

#define	CE_NOCARD				0x30
#define	CE_INVADLIDCARD			0x31
#define	CE_TESTSTATUS			0x32
#define	CE_FORBID_TICKET		0x33
#define	CE_NOTISSUED			0x34
#define	CE_INVALIDDATE			0x35
#define	CE_FREE_ENTRY			0x36
#define	CE_FEE_ENTRY			0x37
#define	CE_NOT_ISUEDSTATION		0x38
#define	CE_ENOUGH_BALANCE		0x39
#define	CE_ENOUGH_RIDES			0x40
#define	CE_OVERTIME				0x41
#define	CE_OVERFARE				0x42
#define	CE_FREE_EXIT			0x43
#define	CE_NO_ENTRY				0x44
#define	CE_FORBITSTATION		0x45
#define	CE_INVALIDBALANCE		0x46
#define	CE_NOENABLED			0x47
#define	CE_LOCKEDCARD			0x48
#define	CE_NOREFUND				0x49
#define	CE_NOUPDATE				0x50
#define	CE_ISSUED				0x51
#define	CE_STARTDATE			0x52
#define	CE_NOPARAM				0x53
#define	CE_MACERR				0x61
//#define	CE_NO_FREE_ENTRY		0x62

#define	CE_CONTINUE_MULTIFRAME	0x92
#define	CE_TOTAL_FRAME			0x93
#define	CE_FILE_LENGTH			0x94
#define	CE_MD5					0x95
#define	CE_FORMAT				0x96
#define	CE_NO_FILE				0x97
#define	CE_CRC					0x98

#define CE_UNKNOWN				0xff



#endif
