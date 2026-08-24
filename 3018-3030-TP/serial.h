#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#ifndef	__ANDROID__
#include <linux/serial.h>
#include <sys/io.h>
#endif

#include <asm/ioctls.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
//#define DEBUG_TEST				1

unsigned char 	retry_buf[2100], blnRetry;
unsigned short 	retry_len;
unsigned char	retry_command[2];
unsigned char 	retry_ResponseCode;
unsigned char blnCalMAC2;
unsigned char chSmartSAMIndex;

unsigned char g_blnHHJTorFounder, g_blnContinuePolling;
sem_t g_founderwait;
pthread_t	g_pthfounderID;

struct CMD_POLLING
{
	unsigned char	command[200];
}__attribute__( ( packed, aligned(1) ) );
typedef struct CMD_POLLING	CMD_POLLING_t;
CMD_POLLING_t	tpCmdPolling;

int csc_comm, qr_comm, qr_comm_II;

int open_port(char *serial_port);
int close_port(int fd);
void speed_set(int fd, int speed);
int parity_set(int fd, int databits, int stopbits, int parity);
unsigned char writecom(int fd, unsigned char *pdata, long lnglen);
unsigned char readcom(int fd, unsigned char *pdata, long lnglen);

long DeleteDLE(unsigned char *pbytData, int intLength);
long InsertDLE(unsigned char *pbytdata, int intLength);

long ReaderResponse(int fd, unsigned char chCode, unsigned char chCommand, unsigned char *psend, unsigned char len);
unsigned char DealCommand(unsigned char *cmd_buf,int cmd_len, unsigned char *out_buf, unsigned char *out_len);

long communicate(int fd, char *psend, long len, char *preceived);
void left_move(unsigned char *src_code, unsigned char len);

unsigned char DealSmartCommand(int fd, unsigned char *cmd_buf, int cmd_len, unsigned char *out_buf, unsigned char *out_len);
unsigned char SmartResponse(int fd, unsigned char response_node, unsigned char *psend, unsigned char len);

long xaReaderResponse(int fd, unsigned char chCode, unsigned char *chCommand, unsigned char *psend, unsigned short len);
long xaInsertDLE(unsigned char *pbytData, int intLength);
long xaDeleteDLE(unsigned char *pbytData, int intLength);
unsigned char xaDealCommand(unsigned char *cmd_buf, int cmd_len, unsigned char *out_buf, unsigned short *out_len);


void xa_protocol_deal(int fd, unsigned char *bytCmd, long cmdLen, unsigned char *out_buf);
void *pthFounder();

#endif
