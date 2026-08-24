#ifndef SZ_SAM_H
#define SZ_SAM_H

#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include "xa_func.h"

sem_t g_samcalwait, g_samreturn;
pthread_t	g_pthsamID;

char	ch_cpu20_psam_id[6], ch_cput_psam_id[6], ch_cput_isam_id[6];
char	ch_pboc_psam_id[6], ch_cput_psam_sn[8], ch_cput_isam_sn[8];
char 	ch_transport_psam_id[6];

char	sam_type, sam_version;
char 	ch_cpu20_keyversion;
char	ch_cpu_mac_data[36], ch_mac_sel;

unsigned char xa_metro_psam_index, mac_ret, xa_tong_psam_index, xa_tong_isam_index;
unsigned char xa_transport_psam_index, xa_pboc_psam_index;

int ResetXAMetroSam(void);
int XAMetroSAM(void);
int XAYKTISAM(void);
int XAYKTPSAM(void);
int XAPbocPSAM(void);
int XATTBPSAM(void);

char sjt_cal_mac(unsigned char *key, unsigned char *factor, unsigned char *in_data, unsigned char in_len, unsigned char *mac);
char cpu_cal_dcmk(char psam_index, unsigned char *key, unsigned char *factor, unsigned char factor_len, unsigned char mac_type, unsigned char *in_data, unsigned char in_len, unsigned char *mac, unsigned char *maclen);
char cpu_cal_mac1(char psam_index, unsigned char *in_data, unsigned char in_len, unsigned char *mac1);
char cpu_cal_mac2(char psam_index, unsigned char *in_data, unsigned char in_len, unsigned char *mac1);
char sam_select_file(char psam_index, char *sfi, unsigned char *out_buf);
char cpu_cal_protect_mac(unsigned char sam_index, unsigned char *factor, unsigned char len, unsigned char *key, unsigned char *in_data, unsigned char in_len, unsigned char *mac);

char xasjt_cal_tac(char *in_buf, short in_len, unsigned long txnsn, unsigned char *tac);
void *pthsamcal();

#endif
