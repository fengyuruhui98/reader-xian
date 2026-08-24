#ifndef SZ_M1_OPERATION_H
#define SZ_M1_OPERATION_H

#define	SZ_E_EDU_SECTOR		16
#define	SZ_E_EDU_B0			64
#define	SZ_E_EDU_B1			65
#define	SZ_E_EDU_B2			66

unsigned char m1_key[32][6];
char ch_m1_phyical_id[9], ch_sh_psam_id[6], sh_psam_index;

char block_write(unsigned char auth, unsigned char *key, unsigned char keyab, unsigned char blockno, unsigned char *in_data);
char sector_read(unsigned char *key, unsigned char keyab, unsigned char type, unsigned char sectno, unsigned char block,  unsigned char *out_buf);


char sz_M1_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len);
char sz_M1_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len);
char sz_M1_update(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len);
char sz_M1_inquire(unsigned char *cmd_buf, unsigned char *out_buf, unsigned char *out_len);

#endif