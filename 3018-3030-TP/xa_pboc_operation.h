#ifndef SZ_PBOC_OPERATION_H
#define SZ_PBOC_OPERATION_H

#include "tlv.h"

unsigned char ch_sz_pboc_rollback;

struct TLVEntity tlv_ppse, tlv_aid, tlv_gpo;

char pboc_read_fci(unsigned char *in_buf, unsigned char in_len, unsigned char *out_buf);


char xa_pboc_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_pboc_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char xa_pboc_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);


#endif