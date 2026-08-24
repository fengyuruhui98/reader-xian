#ifndef XA_TRANSPORT_OPERATION_H
#define XA_TRANSPORT_OPERATION_H

#include "tlv.h"
#include "binEOD.h"
#include "hh_cpu_operation.h"

#define SZ_CPUT_15_LEN			30
#define	SZ_TRANSPORT_16_LEN			55
#define	TRANSPORT_1E_LEN			48
#define	SZ_TRANSPORT_18_LEN			23
#define	TRANSPORT_1A_LEN			128
#define XA_TRANSPROT_19_LEN			32


#define	XA_CODE_CITY				"\x79\x10"
#define	XA_CODE_ORGANIZATION		"\x02\x01\x79\x10\xff\xff\xff\xff"
#define	SZ_FEETYPE_TRANSPORT		8

#define	SZ_TRANSPORT_ENABLE			0

TLVEntity_t tlv_ppse, tlv_aid;

struct TLVEntity *tag_bf0c;				//
struct TLVEntity *tag_61;				//应用模块，包含应用入口
struct TLVEntity *AIDtag_4f;			//选中的AID标签
struct TLVEntity *fci_9f0c;				//

char ch_transport_phyical_id[8], ch_transport_logic_id[10];
char ch_transport_phyical_id_bak[8], ch_transport_code_bak;
char ch_sz_transport_rollback;


char Transport_GetFiles15(unsigned char *out_buf);

char Transport_TellEntry(char entryMAC, unsigned char mode_check);
char get_transport_purchase_ticket(unsigned short type, JTBTerminal_t *td);
char Transport_Map(unsigned char *organitionCode, unsigned char transport_subType, unsigned char transport_mainType, unsigned short *metro_type);
char get_transport_bonus(unsigned char *issuerCode, unsigned short mainType, long *transAmount, long *bonus);

char xa_transport_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_transport_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_transport_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_transport_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);


#endif
