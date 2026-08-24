#ifndef HH_CPU_OPERATION_H
#define HH_CPU_OPERATION_H

//‘§∂®“Â
typedef void (*cpu_proc_callback) (unsigned char);

#ifdef	__ANDROID__
#include <android/log.h>
#define	PRINTK(...)	__android_log_print(ANDROID_LOG_DEBUG, "XiAn Android TP.so", __VA_ARGS__)
#else
#define	PRINTK printf
#endif

//#define DEBUG_TEST				1

#define XA_CPUT_15_LEN			30
#define SZ_CPU_19_LEN			50

#define XA_TRANSPORT_ENTRY			1
#define XA_TRANSPORT_EXIT			2

#define SZ_CPU_CAPP_1			1		//normal entry/exit/update debit non-confirm
#define SZ_CPU_CAPP_2			2		//reject entry/exit/update debit
#define SZ_CPU_CAPP_3			3		//non-confirm status for normal or reject

#define CPU_ED					2
#define CPU_EP					1

#define SZ_CPU_LOAD_1			1		//credit for load non-confirm
#define SZ_CPU_LOAD_2			2		//update file 14
#define SZ_CPU_LOAD_3			3		//update file 06
#define SZ_CPU_LOAD_4			4		//

struct cpu
{
	unsigned char curtime[4];
	char time_bcd[7];
	unsigned long hisecond;
	unsigned long lowsecond;
	unsigned short days;
	unsigned long midsecond;
	
	long balance;
	long tranamount;
	long trantimes;
	short deposit;
	
	char EDorEP;
	char sz_psam_index;
	char thread_mac1;
	char thread_mac2;
	char capp_type;
	char capp_len;
	
	unsigned short cardsn;
	unsigned long transn;
	
	unsigned char curstation[4];
	unsigned char laststation[4];
	
	unsigned short startdate;
	unsigned short enddate;
	//
	unsigned char tac[4];
	unsigned char sam_sn[4];
	
	unsigned short TicketType;
	long lngBonus;
};

struct cpu tpCPU;

unsigned char capp_init[19];			//init capp purcahse return
unsigned char capp_debit[8];			//debit capp purchase return

unsigned char mac1[8], mac2[4];

char CPU_init_for_capp(char key_index, int transvalue, char *device_id, unsigned char *user_sn, unsigned char *city, unsigned char city_len, char *out_buf);
char CPU_update_capp(char thread_id, unsigned char SFI_index, unsigned char rec_index, unsigned char len, unsigned char *rec_buf, unsigned char cycleflag);
char CPU_debit_for_capppurchase(unsigned char *rollback, cpu_proc_callback proc, unsigned char *out_buf);

char CPU_init_for_credit(int transvalue, unsigned char *deviceid, unsigned char *out_buf);

char CPU_gettransprove(unsigned char transtypeid, unsigned char *sfi, char sam_index, unsigned char *cpu_factor, unsigned char *out_buf);
char CPU_externauth(char extern_auth_type, char sam_index, unsigned char *cpu_factor,  unsigned char *out_buf);

void output_binary_trace(char *pStrTraceTitle, unsigned char* pBytesOutput, unsigned int dataLength);
char city_auth(unsigned char *out_buf);

char CPU_init_for_purchase(char key_index, int transvalue, char *device_id, unsigned char *user_sn, unsigned char *city, unsigned char *out_buf);
char CPU_debit_for_purchase(unsigned char *rollback, cpu_proc_callback proc, unsigned char *out_buf);

char CPU_VerifyPIN(unsigned char *pin, unsigned char len, unsigned char *out_buf);
unsigned char mifpro_apdu(unsigned char *inbuf, unsigned char inbytes, unsigned char *outbuf, unsigned short *outbytes);

#endif