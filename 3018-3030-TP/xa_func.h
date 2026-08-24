#ifndef SZ_FUNC_H
#define SZ_FUNC_H

#define SZ_SJT_FAMILY		1
#define SZ_CPU_FAMILY		2
#define SZ_CITY_FAMILY		3

#define XA_SJT_FAMILY		0x12
#define XA_MCPU_FAMILY		0x11
#define XA_CITY_FAMILY		0x02
#define	XA_M1_FAMILY		0x01
#define XA_PBOC_FAMILY		0x20
#define XA_TRANSPORT_FAMILY	0x21
#define	XA_QR_FAMILY		0x22

unsigned char xa_ticket_family;

char xa_metro_psam_sfi[2];

char PPSE[300], PPSE_len;

unsigned char xa_polling_card(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
char sz_rf_on_off(unsigned char *in_buf, unsigned char *out_buf, unsigned char *out_len);
char xa_pboc_select(unsigned char *inbuf, unsigned char inlen, unsigned char *ticketFamily);

#endif
