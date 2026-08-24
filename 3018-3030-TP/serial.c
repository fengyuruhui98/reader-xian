#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "serial.h"
#include "xa_ul_operation.h"
#include "time_tools.h"
#include "xa_error_code.h"
#include "xa_sam.h"
#include "linux2440lib.h"
#include "xa_cpu20_operation.h"
#include "xa_tong_operation.h"
#include "hh_cpu_operation.h"
#include "eeprom.h"
#include "bin_file_manage.h"

//#define	 DEBUG_SERIAL_PRINT

/*
open the serial port
*/
int open_port(char *SerialPort)
{
int fd;
	
	fd = open(SerialPort, O_RDWR | O_NOCTTY | O_NDELAY);
	//fd = open(SerialPort, O_RDWR | O_NOCTTY | 0x8000);
	if (fd == -1)
	{
		perror("open_port:Unable to open");
	}
	else
		fcntl(fd, F_SETFL, 0);
	
	return (fd);
}

/*
close the serial port
*/
int close_port(int fd)
{
int rtn;

	rtn = close(fd);
	return (rtn);
}

/*
set the serial port baud
*/
int speed_sel[] ={B115200, B57600, B38400, B19200, B9600};
int name_sel[] = {115200, 57600, 38400,  19200, 9600};
void speed_set(int fd, int speed)
{
int i, status, tempspeed;
struct termios opt;
//struct serial_struct p;
speed_t lngspeed;
	
	if(fd <= 0)
		return ;
		
	tcgetattr(fd, &opt);
	tempspeed = speed;
	if(speed == 28800)
		tempspeed = 38400;
	for(i = 0; i < sizeof(speed_sel) / sizeof(int); i++)
	{
		if(tempspeed == name_sel[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&opt, speed_sel[i]);
			cfsetospeed(&opt, speed_sel[i]);

			status = tcsetattr(fd, TCSANOW, &opt);
			if(status != 0)
			{
				PRINTK("set attr wrong!\n");
				return ;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
	lngspeed = cfgetispeed(&opt);
//	if(speed == 28800)
//	{
//		ioctl(fd, TIOCGSERIAL, &p);
//		p.flags = ASYNC_SPD_CUST;
//		p.custom_divisor = 4;
//		ioctl(fd, TIOCSSERIAL, &p);
//	}
	lngspeed = cfgetispeed(&opt);
}

/*
set parity
*/
int parity_set(int fd, int databits, int stopbits, int parity)
{
struct termios opt;
	
	if(fd <= 0)
		return 1;

	tcgetattr(fd, &opt);
	
	opt.c_cflag &= ~CSIZE;
	switch(databits)
	{
	case 7:
		opt.c_cflag |= CS7;
		break;	
	case 8:
		opt.c_cflag |= CS8;
		break;
	}
	switch(parity)
	{
	case 'n':
	case 'N':
		opt.c_cflag &= ~PARENB;
		opt.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		opt.c_cflag |= (PARODD | PARENB);
		opt.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		opt.c_cflag |= PARENB;
		opt.c_cflag &= ~PARODD;
		opt.c_iflag |= INPCK;
		break;
	case 'S':
	case 's': /*as no parity*/
		opt.c_cflag &= ~PARENB;
		opt.c_cflag &= ~CSTOPB;
		break;	
	}
	switch(stopbits)
	{
	case 1:
		opt.c_cflag &= ~CSTOPB;
		break;
	case 2:
		opt.c_cflag |= CSTOPB;
	}
	if(parity != 'n')
		opt.c_iflag |= INPCK;
	//如果不是开发终端，只是串口传输数据，而不需要串口来处理，那么使用原始模�?(Raw Mode)方式来通讯
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	opt.c_oflag &= ~OPOST;
	
	opt.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
	tcflush(fd, TCIFLUSH);
	opt.c_cc[VTIME] = 5;
	opt.c_cc[VMIN] = 0;
	tcsetattr(fd, TCSANOW, &opt);
	return(1);
}

/*
写数据到串口
*/
unsigned char writecom(int fd, unsigned char *pdata, long lnglen)
{
long lngwritelen;
FILE *intFile;
char chinfo[300];
long i;

	if(fd <= 0)
		return 0;
	//for bim can't use this function
	//tcflush(fd, TCIOFLUSH);
	
	lngwritelen = write(fd, pdata, lnglen);
/*	PRINTK("write com data len comman is %x real is %x\n", lnglen, lngwritelen);
	sprintf(chinfo, "send:");

	for(i = 0; i < lnglen; i++)
		sprintf(chinfo, "%s %02X", chinfo, pdata[i]);
	PRINTK("%s\n", chinfo);	
	
	intFile = fopen("./peilj.txt", "at+");
		fwrite(chinfo, strlen(chinfo), 1, intFile);
	chinfo[0] = 13;
	chinfo[1] = 10;
	fwrite(chinfo, 2, 1, intFile);
	fclose(intFile);
*/	
	if(lngwritelen == -1)
		return 0;
	if(lngwritelen != lnglen)
		return 0;
	return 0xff;
}

/*
读串口数�?
*/
unsigned char readcom(int fd, unsigned char *pdata, long lnglen)
{
long lngwritelen;

	if(fd <= 0)
		return 0;
		
	lngwritelen = read(fd, pdata, lnglen);
	if(lngwritelen == 0)
		return 0;
	if(lngwritelen == -1)
		return 0;
//	if(lngwritelen != lnglen)
//		return 0;
	return lngwritelen;
}

/*
function:insert the DLE into the command line
	the data including the data block length
parameter:	
return:the length including the data block length
*/
long InsertDLE(unsigned char *pbytData, int intLength)
{
unsigned char	tempData[600], bytCheck;
int	i, intInsertLen = 1;
unsigned short shCheck;	
	
	shCheck = cal_crc(pbytData, intLength);
	*(pbytData + intLength) = (unsigned char)(shCheck >> 8);
	*(pbytData + intLength + 1) = (unsigned char)shCheck;

	for(i = 0; i < intLength + 2; i++)
	{
		bytCheck = *(pbytData + i);
		if(bytCheck == 0x03 || bytCheck == 0x04 || bytCheck == 0x06 || bytCheck == 0x10)
			tempData[intInsertLen++] = 0x10;
		tempData[intInsertLen++] = bytCheck;
	}
	tempData[intInsertLen++] = 0x04;
	tempData[0] = 0x06;
	memcpy(pbytData, tempData, intInsertLen);

	return intInsertLen;
}
/*
function:delete the DLE 
parameter:
return:
	non zero:the data including the all data block from csc reader
	0: failure
	less than 0:response code
*/
long DeleteDLE(unsigned char *pbytData, int intLength)
{
unsigned char 	bytTempData[600];
char		inlen;
int 		intDeleteLen, i;
unsigned short	shCheck;
unsigned char 	chCheck[2];

	//check the message head
	if(pbytData[0] != 0x03) return 0;
	//delete the DLE
	memcpy(bytTempData, pbytData, intLength);
	intDeleteLen = intLength;
	for(i = 1; i < intLength; i++)
	{
		if(bytTempData[i] == 0x10)
		{
			memcpy(&bytTempData[i], &bytTempData[i + 1], intLength - 1 - i);
			intDeleteLen -= 1;
		}
	}
	//check the message length:
	inlen = intDeleteLen - 9;
	if((unsigned char)inlen != bytTempData[5])
	{//confirm the command is correct for response
		pbytData[6] = bytTempData[6];
		return -211;
	}
	//check the message CRC
	shCheck = cal_crc(&bytTempData[1], intDeleteLen - 4);

	if( ((unsigned char)(shCheck >> 8) == bytTempData[intDeleteLen - 3]) 
	   && ((unsigned char)shCheck == bytTempData[intDeleteLen - 2]))
		memcpy(pbytData, bytTempData, intDeleteLen);
	else
		return -202;
	return intDeleteLen;
}

/*
function:insert the DLE into the command line
	the data including the data block length
parameter:	
return:the length including the data block length
*/
long xaInsertDLE(unsigned char *pbytData, int intLength)
{
unsigned char	tempData[1000], bytCheck;
int	i, intInsertLen = 1;
unsigned short shCheck;	
	
	bytCheck = 0;
	for(i = 0; i < intLength; i++)
	{
		bytCheck ^= pbytData[i];
	}
	
	*(pbytData + intLength) = bytCheck;

	for(i = 0; i < intLength + 1; i++)
	{
		bytCheck = *(pbytData + i);
		if(bytCheck == 0x03 || bytCheck == 0x02 || bytCheck == 0x10)
			tempData[intInsertLen++] = 0x10;
		tempData[intInsertLen++] = bytCheck;
	}
	tempData[intInsertLen++] = 0x03;
	tempData[0] = 0x02;
	memcpy(pbytData, tempData, intInsertLen);

	return intInsertLen;
}

/*
function:delete the DLE 
parameter:
return:
	non zero:the data including the all data block from csc reader
	0: failure
	less than 0:response code
*/
long xaDeleteDLE(unsigned char *pbytData, int intLength)
{
unsigned char 	bytTempData[2048];
unsigned short	inlen;
int 		intDeleteLen, i;
unsigned short	shCheck, shLength;
unsigned char 	chCheck;

	//check the message head
	if(pbytData[0] != 0x02) return 0;
	//delete the DLE
	memcpy(bytTempData, pbytData, intLength);
	intDeleteLen = intLength;
	chCheck = 0;
	for(i = 1; i < intLength; i++)
	{
		if(bytTempData[i] == 0x10)
		{
			memcpy(&bytTempData[i], &bytTempData[i + 1], intLength - 1 - i);
			intDeleteLen -= 1;
		}
	}
	//check the message length--only data body length
	inlen = intDeleteLen - 5;
	memcpy(&shLength, &bytTempData[1], 2);
	if(inlen != shLength)
	{//confirm the command is correct for response
		memcpy(&pbytData[3], &bytTempData[3], 2);
		return -211;
	}
	//check the message XOR
	chCheck = 0;
	for(i = 1; i < intDeleteLen - 2; i++)
	{
		chCheck ^= bytTempData[i];
	}

	if(chCheck == bytTempData[intDeleteLen - 2])
		memcpy(pbytData, bytTempData, intDeleteLen);
	else
		return -202;
	return intDeleteLen;
}


/*
function:
*/
long com_serv(int fd, char *psend, long len, char *preceived)
{
static unsigned char buff[600], sendlen, byttemp[2048], chCmd, chResCode, reslen;
clock_t start, end;
long lreadlen, rtn;
long i, j, ret;
unsigned char fStat, blnTimeout;
static unsigned char recv_buf[2000];
struct timeval tv1,tv2;
struct timezone tz1,tz2;
long lngPrint;
char chinfo[4200], chtempinfo[10], lrc1, lrc2;
FILE *intFile;
//struct SYSTEMTIME	tpsystemtime;
struct timeval timeout;
fd_set readfd;
unsigned short xareslen;

	if(fd <= 0)
		return 0xff;
		
	memset(buff, 0x00, 600);
	memset(byttemp, 0x00, 600);
	memset(chtempinfo, 0x00, 10);
	lreadlen = 0;
	memset(retry_command, 0x00, 2);
	blnTimeout = 0xff;

	memset(chinfo, 0x00, sizeof(chinfo));
	//clear the data received but not rea
	tcflush(fd, TCIFLUSH);

	do
	{
		FD_ZERO(&readfd);
		FD_SET(fd, &readfd);
		timeout.tv_sec = 5;
		timeout.tv_usec = 100;
		if(blnCalMAC2)
		{
			sem_post(&g_samcalwait);
			blnCalMAC2 = 0;
		}
		ret = select(fd + 1, &readfd, NULL, NULL, &timeout);
		//PRINTK("comm Select return %d\n", ret);
		if(ret > 0)
		{
			rled(LED_ON);
			blnTimeout = 0xff;
			gettimeofday(&tv1,&tz1);
			memset(chinfo, 0x00, 2048);
			do
			{
				fStat = readcom(fd, buff, 1);
			//	PRINTK("read com return is %2x and buff is %2x\n", fStat, buff[0]);
				if(fStat)
				{
					memcpy(&byttemp[lreadlen], &buff[0], fStat);
					buff[0] = 0;
					lreadlen += fStat;
					if(lreadlen >= 2048)
					{
#ifdef DEBUG_SERIAL_PRINT
						PRINTK("read byte more than 2048 %s\n", chinfo);
#endif						
						lreadlen = 0;
						memset(chinfo, 0x00, sizeof(chinfo));
						continue;
					}
					if(byttemp[0] == 0x03)
					{//suzhou
						if(lreadlen > 3)
						{
							if(byttemp[lreadlen - 1] == 0x04)
							{
								j = 0;
								for(i = lreadlen - 2; i > 0; i--)
								{
									if(byttemp[i] != 0x10)
										break;
									else
										j++;
								}
								if(j % 2 == 0)
								{
#ifdef DEBUG_SERIAL_PRINT							
									PRINTK("recv: %s\n", chinfo);
#endif
									memset(chinfo, 0x00, 1000);
									rtn = DeleteDLE(byttemp, lreadlen);
#ifdef DEBUG_SERIAL_PRINT								
									PRINTK("recv frame len: %d cmd %02x rtn is %d\n", lreadlen, byttemp[6], rtn);
#endif
									if(rtn > 0)
									{
#ifdef DEBUG_SERIAL_PRINT
										sprintf(chinfo, "receive:");
										for(lngPrint = 0; lngPrint < rtn - 1; lngPrint++)
											sprintf(chinfo, "%s%02X", chinfo, byttemp[lngPrint]);
										PRINTK("%s\n", chinfo);
#endif
										chCmd = byttemp[6];
										chResCode = DealCommand(byttemp, rtn, recv_buf, &reslen);
										ReaderResponse(fd, chResCode, chCmd, recv_buf, reslen);
										rled(LED_OFF);
										lreadlen = 0;
										break;
									}else if(rtn == 0)
									{
										;
									}else
									{
										ReaderResponse(fd, (unsigned char)(abs(rtn)), byttemp[6], NULL, 0);
#ifdef DEBUG_SERIAL_PRINT									
										PRINTK("read message error :%d command %02x\n", rtn, byttemp[6]);
#endif
										lreadlen = 0;
										break;
									}
								}
							}
						}
					}else if(byttemp[0] == 0x7e)
					{//inner protocol
						if(lreadlen > 2)
						{
							if(lreadlen == byttemp[1] + 4)
							{//deal with all block
								if((lrc1 == byttemp[lreadlen - 2]) && (lrc2 == byttemp[lreadlen - 1]))
								{
#ifdef DEBUG_SERIAL_PRINT							
									PRINTK("recv: %s\n", chinfo);
#endif
									DealSmartCommand(fd, byttemp, lreadlen, recv_buf, &reslen);
									rled(LED_OFF);
									lreadlen = 0;
									break;
								}else
								{
									lreadlen = 0;
									break;
								}
							}else if(lreadlen == byttemp[1] + 3)
							{
							}else
							{
								lrc1 ^= byttemp[lreadlen - 1];
								lrc2 += byttemp[lreadlen - 1];
							}
						}else
						{
							lrc1 = 0x33;
							lrc2 = 0x33;
						}
					}else if(byttemp[0] == 0x02)
					{//xi`an metro protocol
						if(lreadlen > 3)
						{
							if(byttemp[lreadlen - 1] == 0x03)
							{
								j = 0;
								for(i = lreadlen - 2; i > 0; i--)
								{
									if(byttemp[i] != 0x10)
										break;
									else
										j++;
								}
								if(j % 2 == 0)
								{
#ifdef DEBUG_SERIAL_PRINT							
									PRINTK("recv: %s\n", chinfo);
#endif
									memset(chinfo, 0x00, sizeof(chinfo));
									rtn = xaDeleteDLE(byttemp, lreadlen);
#ifdef DEBUG_SERIAL_PRINT								
									PRINTK("recv frame len: %d cmd %02x rtn is %d\n", lreadlen, byttemp[6], rtn);
#endif
#ifdef DEBUG_SERIAL_PRINT
									sprintf(chinfo, "receive:");
									for(lngPrint = 0; lngPrint < rtn - 1; lngPrint++)
										sprintf(chinfo, "%s%02X", chinfo, byttemp[lngPrint]);
									PRINTK("%s\n", chinfo);
#endif
									if(rtn > 0)
									{
										xa_protocol_deal(fd, byttemp, rtn, recv_buf);
										rled(LED_OFF);
										lreadlen = 0;
										break;
									}else if(rtn == 0)
									{
										;
									}else
									{
										xaReaderResponse(fd, (unsigned char)(abs(rtn)), &byttemp[3], NULL, 0);
#ifdef DEBUG_SERIAL_PRINT									
										PRINTK("read message error :%d command %02x\n", rtn, byttemp[6]);
#endif
										lreadlen = 0;
										break;
									}
								}
							}
						}
					}else
					{
						lreadlen = 0;
#ifdef DEBUG_SERIAL_PRINT
						PRINTK("not start from 0x03 %s\n", chinfo);
#endif					
						memset(chinfo, 0x00, 1000);
						continue;
					}
				}
//				gettimeofday(&tv2,&tz2);
//				if ((((tv2.tv_sec-tv1.tv_sec)*1000000)+tv2.tv_usec-tv1.tv_usec)>=1000000)
//				{
//					blnTimeout = 0;
//					lreadlen = 0;
//#ifdef DEBUG_SERIAL_PRINT
//					PRINTK("read data time out\n");
//#endif
//				}
				FD_ZERO(&readfd);
				FD_SET(fd, &readfd);
				timeout.tv_sec = 0;
				timeout.tv_usec = 2000;
				ret = select(fd + 1, &readfd, NULL, NULL, &timeout);
				if(ret > 0)
				{
					continue;
				}else if(ret == 0)
				{
					blnTimeout = 0;
					lreadlen = 0;
				}else
				{
					blnTimeout = 0;
					lreadlen = 0;
				}
			}while(blnTimeout);
		}else if (ret == 0)
		{
			;//PRINTK("time out no data coming!\n");
		}
		else
		{
#ifdef	DEBUG_SERIAL_PRINT
			PRINTK("select failure \n" );
#endif
		}
	}while(1);
	if(lreadlen > 0)
		;
	return 0xff;
}

/*
function:
parameter:
	fd:com handle
	chCode:response code
	chCommnd:command
	psend:message body
	len:message length
*/
long ReaderResponse(int fd, unsigned char chCode, unsigned char chCommand, unsigned char *psend, unsigned char len)
{
unsigned char 	sendbuf[600];
long 		ret;
unsigned char chinfo[1000], chtemp[20];
long 	i;

	memset(sendbuf, 0x00, 600);
	//response data length + response code length + the command length)
	sendbuf[0] = len + 2;
	sendbuf[1] = chCommand;

	sendbuf[2] = chCode;
	if ((psend != NULL) && (len > 0))
		memcpy(&sendbuf[3], psend, len);

	ret = InsertDLE(sendbuf, len + 3);
#ifdef DEBUG_SERIAL_PRINT
	memset(chinfo, 0x00, 1000);
	for(i = 0; i < ret; i++)
	{
		sprintf((char *)chtemp, "%02X ", sendbuf[i]);
		strcat((char *)chinfo, (char *)chtemp);
	}
	PRINTK("response :%s\n", chinfo);
#endif
	//back up the last message
	if(!blnRetry)
	{
		retry_len = ret;
		memcpy(retry_buf, sendbuf, retry_len);
	}
	writecom(fd, sendbuf, ret);
}
/*
function:according to the command to deal
return:the response code
*/
unsigned char DealCommand(unsigned char *cmd_buf,int cmd_len, unsigned char *out_buf, unsigned char *out_len)
{
static unsigned char chResponseCode, chlen;
FILE *fl;
char 	temp[200], i;
unsigned char bln_pwr_off;

	blnRetry = 0;
	bln_pwr_off = 0xff;
	switch(cmd_buf[6])
	{
	case 0x01:	//reset reader
		*out_len = 0;
		chResponseCode = 21;
		ReaderResponse(csc_comm, chResponseCode, 0x01, NULL, 0);
		sleep(1);
		system("reboot");
		exit(0);
		break;
	case 0x02:	//select antenna
		if(cmd_buf[7] & 0x2)	//internal antenna for close to the USB
			rf_select(1);
		else	//external antenna for far away the USB
			rf_select(0);
		//close the antenna
		if(cmd_buf[7] & 0x1)
			mcml_pwr_off();
		chResponseCode = ERR_OK;
		*out_len = 0;
		bln_pwr_off = 0;
		break;
	case 0x03:	//get the reader hard version
		sprintf(out_buf, "HHJT");
		*out_len = 4;
		chResponseCode = ERR_OK;
		bln_pwr_off = 0;
		break;
	case 0x07:	//re-send the last message including from the message head to message tail
		blnRetry = 0xff;
		memcpy(out_buf,  retry_buf, retry_len);
		*out_len = (char)retry_len;
		chResponseCode = ERR_OK;
		bln_pwr_off = 0;
		break;
	case 0x08:	//get sam status
		memset(out_buf, 0x00, 7 * 8 + 6);
		for(i = 0; i < 8; i++)
		{
			if(i == xa_metro_psam_index)
			{
				out_buf[i * 7] = 1 << 2;
				memcpy(&out_buf[i * 7 + 1], ch_cpu20_psam_id, 6);
			}
#ifdef SUZHOU_TONG	
			if(i == xa_tong_psam_index)
			{
				out_buf[i * 7] = 1 << 2;
				out_buf[i * 7] |= 0x20;
				memcpy(&out_buf[i * 7 + 1], ch_cput_psam_id, 6);
			}
#endif
		}
		memcpy(&out_buf[58], "\x20\x11\x24\x40", 4);
		*out_len = 7 * 8 + 6;
		chResponseCode = ERR_OK;
		bln_pwr_off = 0;
		break;
	case 0xC0:
	case 0xC1:
	case 0xC2:
	case 0xC3:
	case 0xC4:
	case 0xC5:
	case 0xC6:
	case 0xC7:
	case 0xC8:
		bln_pwr_off = 0;
		chResponseCode = xdrFileManage(cmd_buf[6], cmd_buf, cmd_len, out_buf, out_len);
		break;
	case 0xF6:
		memcpy(out_buf, "HHJTV2.27-20120410", 18);
		//out_buf[0] = 0x84; out_buf[1] = 0x84; out_buf[2]= 0xa5; out_buf[3] = 0x45; out_buf[4] = 0x63; out_buf[5]= 0x22; out_buf[6] = 0xe3;
		//out_buf[7] = 0x23; out_buf[8] = 0x62; out_buf[9] = 0xd3; out_buf[10] = 0x23; out_buf[11] = 0x03; out_buf[12] = 0x13; out_buf[13] = 0x23;
		//out_buf[14] = 0x03; out_buf[15] = 0x43; out_buf[16] = 0x03; out_buf[17] = 0x54;
		left_move(out_buf, 18);
		*out_len = 18;
		chResponseCode = ERR_OK;
		bln_pwr_off = 0;
		break;
	case 0xF1:
		chResponseCode = reader_hard_test(cmd_buf, out_buf, out_len);
		break;
	case 0xF5:
		chResponseCode = reader_test_rtc(0, out_buf, out_len);
		break;
#ifdef DEBUG_TEST
	case 0xF2:
		chResponseCode = sz_test_get_file(cmd_buf, out_buf, out_len);
		break;
	case 0xF3:
		chResponseCode = sz_test_update_file(cmd_buf, out_buf, out_len);
		break;
	case 0xF4:
		chResponseCode = sz_test_ul(cmd_buf, out_buf, out_len);
		break;
#endif
	default:
		*out_len = 0;
		chResponseCode = 205;
		break;
	}
	if(bln_pwr_off)
		mcml_pwr_off();
	return chResponseCode;
}

/*
function:
parameter:
	fd:com handle
	chCode:response code
	chCommnd:command
	psend:message body
	len:message length
*/
long xaReaderResponse(int fd, unsigned char chCode, unsigned char *chCommand, unsigned char *psend, unsigned short len)
{
unsigned char 	sendbuf[2000];
long 		ret;
unsigned char chinfo[2100], chtemp[20];
long 	i;

	memset(sendbuf, 0x00, 2000);
	//parameter len only for data but protocal len include command(2) + retry(1) + responsecode(1) + data
	sendbuf[0] = (unsigned char)(len + 4);
	sendbuf[1] = (unsigned char)((len + 4) >> 8);
	//command + retry
	memcpy(&sendbuf[2], chCommand, 3);
	
	sendbuf[5] = chCode;
	if ((psend != NULL) && (len > 0))
		memcpy(&sendbuf[6], psend, len);

	ret = xaInsertDLE(sendbuf, len + 6);
#ifdef DEBUG_SERIAL_PRINT
	memset(chinfo, 0x00, 2100);
	for(i = 0; i < ret; i++)
	{
		sprintf((char *)chtemp, "%02X ", sendbuf[i]);
		strcat((char *)chinfo, (char *)chtemp);
	}
	PRINTK("response :%s\n", chinfo);
#endif
	//test retry 
	//if(!blnRetry)
	//{
	//	sendbuf[ret - 2] = 0xaa;
	//	blnRetry = 0xff;
	//}
	writecom(fd, sendbuf, ret);

	return 0;
}

/*
function:according to the command to deal
return:the response code
*/
unsigned char xaDealCommand(unsigned char *cmd_buf, int cmd_len, unsigned char *out_buf, unsigned short *out_len)
{
static unsigned char chResponseCode, chlen;
FILE *fl;
char 	temp[200];
unsigned char bln_pwr_off, blnPrintLog;
unsigned short shCommand, shCmdLen;
long i;

	blnPrintLog = 0xff;
	shCmdLen = cmd_len;
	//check IF retry
	if((cmd_buf[5] != 0x00) && (memcmp(retry_command, &cmd_buf[3], 2) == 0))
	{
		*out_len = retry_len;
		memcpy(out_buf, retry_buf, retry_len);
		return retry_ResponseCode;
	}
	blnRetry = 0;
	bln_pwr_off = 0x00;//0xff;
	memcpy(&shCommand, &cmd_buf[3], 2);
	switch(shCommand)
	{
	case 0x5301:	//initial TPU
		reader_status = XA_RW_IDLE;
		*out_len = 0;
		chResponseCode = xa_init(cmd_buf, out_buf, out_len);
		break;
	case 0x5302:	//reset TPU
		out_buf[0] = reader_status = XA_RW_INIT;
		rf_reset();
		chResponseCode = 0;
		*out_len = 1;
		break;
	case 0x5303:	//stop TPU
		*out_len = 1;
		chResponseCode = 0;
		out_buf[0] = reader_status = XA_RW_INIT;
		break;
	case 0x5304:	//abort TPU
		*out_len = 1;
		if((reader_status == XA_RW_STOP) || (reader_status == XA_RW_INIT) || (reader_status == XA_RW_RECORD))
		{
			chResponseCode = CE_TPUSTATUS;
			out_buf[0] = reader_status;
		}else
		{
			chResponseCode = CE_OK;
			out_buf[0] = reader_status = XA_RW_IDLE;
		}
		break;
	case 0x5305:	//set TPU time
		*out_len = 1;
		chResponseCode = rtc_wr_time(&cmd_buf[7]);
		out_buf[0] = reader_status;
		break;
	case 0x5322:	//read TPU time
		out_buf[0] = 0x20;
		chResponseCode = rtc_rd_time(&out_buf[1]);
		if(chResponseCode)
			*out_len = 0;
		else
			*out_len = 7;
		break;
	case 0x5321:		//get TPU status
		*out_len = 1;
		out_buf[0] = reader_status;
		chResponseCode = CE_OK;
		break;
	case 0x5324:	//get TPU information
		memset(out_buf, 0x00, 34);
		//reader_get_version(temp, &temp[199]);
#ifdef DEBUG_SERIAL_PRINT		
		for(i = 0; i < temp[199]; i++)
			PRINTK("%02X", temp[i]);
		PRINTK("\n");
#endif
		//reader id
		memcpy(&out_buf[0], temp, 4);
		//tp version 
		LongToByte(tp_ver, &out_buf[4]);
		//reader version
		memcpy(&out_buf[8], &temp[temp[199] - 2], 2);
		//YKT PSAM
		memcpy(&out_buf[10], ch_cput_psam_id, 6);
		//YKT ISAM
		memcpy(&out_buf[16], ch_cput_isam_id, 6);
		//METRO PSAM
		if(sam_type == 0x0e)
			memcpy(&out_buf[22], ch_cpu20_psam_id, 6);
		//METRO ISAM
		if(sam_type == 0x0f)
			memcpy(&out_buf[28], ch_cpu20_psam_id, 6);
		//auth status
		out_buf[34] = 0;
		if(tpauthLogin.LimiAmt == 0)
			out_buf[34] = CE_NOAUTH;
		*out_len = 35;
		chResponseCode = CE_OK;
		bln_pwr_off = 0;
		break;
	case 0x5331:	//search the ticket
		if((reader_status != XA_RW_IDLE) && (reader_status != XA_RW_SEARCH) && (reader_status != XA_RW_READ))
		{
			*out_len = 1;
			out_buf[0] = reader_status;
			//printf("xa_polling stop = %d\n",reader_status);// 20230103
			return CE_TPUSTATUS;
		}
		//chResponseCode = xa_polling_card(cmd_buf, out_buf, out_len);
		chResponseCode = xa_polling(cmd_buf, out_buf, out_len);
		bln_pwr_off = 0;
		break;
	case 0x5335:
		chResponseCode = xa_inquire(cmd_buf, out_buf, out_len);
		break;
	case 0x5336:
		chResponseCode = xa_sale(cmd_buf, out_buf, out_len);
		break;
	case 0x5332:
		chResponseCode = xa_entry(cmd_buf, out_buf, out_len);
		break;
	case 0x5333:
		chResponseCode = xa_exit(cmd_buf, out_buf, out_len);
		break;
	case 0x5334:
		chResponseCode = xa_getud(cmd_buf, out_buf, out_len);
		break;
	case 0x5337:
		chResponseCode = xa_update(cmd_buf, out_buf, out_len);
		break;
	case 0x5338:
		chResponseCode = xa_add(cmd_buf, out_buf, out_len);
		break;
	case 0x533c:
		chResponseCode = xa_active(cmd_buf, out_buf, out_len);
		break;
	case 0x533d:
		chResponseCode = xa_defer(cmd_buf, out_buf, out_len);
		break;
	case 0x533e:
		chResponseCode = xa_blacklist_request(cmd_buf, out_buf, out_len);
		break;
	case 0x533f:
		chResponseCode = xa_ul_reverse(cmd_buf, out_buf, out_len);
		break;
	case 0x5341:
		out_buf[0] = XA_RW_IDLE;
		chResponseCode = CE_OK;
		*out_len = 1;
		break;
	case 0x5343:	//get the price
		chResponseCode = xa_price(cmd_buf, out_buf, out_len);
		break;
	case 0x5401:
		if(reader_status == XA_RW_STOP)
		{
			*out_len = 1;
			out_buf[0] = reader_status;
			return CE_TPUSTATUS;
		}
		chResponseCode = xa_auth_login_init(cmd_buf, out_buf, out_len);
		//chResponseCode = xa_ext_auth_init(cmd_buf, out_buf, out_len);
		break;
	case 0x5402:
		chResponseCode = xa_auth_login(cmd_buf, out_buf, out_len);
		break;
	case 0x5403:
		chResponseCode = xa_auth_logout_init(cmd_buf, out_buf, out_len);
		break;
	case 0x5404:
		chResponseCode = xa_auth_logout(cmd_buf, out_buf, out_len);
		break;
	case 0x5306:		//download eod
	case 0x5307:		//download tp
		shCmdLen = 15;
	case 0x5342:		//set station mode
	case 0x5323:		//get eod version
	case 0x5308:		//active tp
	
	case 0x5454:
		bln_pwr_off = 0;
		chResponseCode = binFileManage(shCommand, cmd_buf, cmd_len, out_buf, out_len);
		break;
	case 0x5410:
		chResponseCode = xa_ext_auth_init(cmd_buf, out_buf, out_len);
		break;
	case 0x5411:
		chResponseCode = xa_ext_auth(cmd_buf, out_buf, out_len);
		break;
	default:
		*out_len = 0;
		chResponseCode = CE_COMMAND;
		break;
	}
#ifdef DEBUG_SERIAL_PRINT	
	if(blnPrintLog)
	{
		PRINTK("Receive:");
		for(i = 0; i < shCmdLen; i++)
		{
			PRINTK( "%02X ", cmd_buf[i]);
		}
		PRINTK("\n");
	}
#endif
	if(bln_pwr_off)
		mcml_pwr_off();
	//backup the message
	retry_ResponseCode = chResponseCode;
	retry_len = *out_len;
	memcpy(retry_buf, out_buf, retry_len);
	memcpy(retry_command, &cmd_buf[3], 2);
	//
	return chResponseCode;
}

/*
*/
long communicate(int fd, char *psend, long len, char *preceived)
{
unsigned char buff[400], sendlen, byttemp[400];
clock_t start, end;
long lreadlen, rtn;
long i, j;
unsigned char fStat, blnTimeout;
struct timeval tv1,tv2;
struct timezone tz1,tz2;
unsigned char bytPrint;
char chinfo[500], chtempinfo[10];
FILE *intFile;
//struct SYSTEMTIME	tpsystemtime;

	if(fd <= 0)
		return 0xff;
		
	memset(buff, 0x00, 100);
	memset(byttemp, 0x00, 200);
	memset(chinfo, 0x00, 500);
	memset(chtempinfo, 0x00, 10);
	lreadlen = 0;
	blnTimeout = 0xff;
	//PRINTK("the command is %x\n", psend[1]);
	if(len > 0)
	{
		memcpy(buff, psend, len);
		sendlen = InsertDLE(buff, len);
/*		GetLocalTime(&tpsystemtime);
		sprintf(&chinfo[0], "%u-%02u-%02u %02u:%02u:%02u.%03u SEND:", tpsystemtime.wYear, tpsystemtime.wMonth, tpsystemtime.wDay, tpsystemtime.wHour,
				tpsystemtime.wMinute, tpsystemtime.wSecond, tpsystemtime.wMilliseconds);
		for(j = 0; j < sendlen; j++)
		{
			sprintf(chtempinfo, "%02X ", buff[j]);
			strcat(chinfo, chtempinfo);
		}
		//WriteCSCLog(intCSCLogFile, chinfo, 1);
*/		if(!writecom(fd, buff, sendlen)) return 0xff;
	}
	//
	//PRINTK("the start time is %d\n", start);
	memset(chinfo, 0x00, 500);
	gettimeofday(&tv1,&tz1);
	
	//PRINTK("read com:");
	do
	{
		fStat = readcom(fd, buff, 1);
		//PRINTK("read com return is %2x and buff is %2x\n", fStat, buff[0]);
		if(fStat)
		{
			//PRINTK(" %2x", buff[0]);
			//PRINTK("\n");
/*			if(lreadlen == 0)
			{
				GetLocalTime(&tpsystemtime);
				sprintf(&chinfo[0], "%u-%02u-%02u %02u:%02u:%02u.%03u, Receive:", tpsystemtime.wYear, tpsystemtime.wMonth, tpsystemtime.wDay,
						tpsystemtime.wHour, tpsystemtime.wMinute, tpsystemtime.wSecond, tpsystemtime.wMilliseconds);
			}
*/			byttemp[lreadlen] = buff[0];
//			sprintf(chtempinfo, "%02X ", buff[0]);
//			strcat(chinfo, chtempinfo);
			buff[0] = 0;
			lreadlen += 1;
			if(lreadlen >= 400)
			{
				return 0xff;
			}
			if(byttemp[0] != 0x02)
			{
				lreadlen = 0;
				continue;
			}
				
			if(lreadlen > 3)
			{
				if(byttemp[lreadlen - 1] == 0x03)
				{
					j = 0;
					for(i = lreadlen - 2; i > 0; i--)
					{
						if(byttemp[i] != 0x10)
							break;
						else
							j++;
					}
					if(j % 2 == 0)
					{
						rtn = DeleteDLE(byttemp, lreadlen);
						if(rtn)
						{
							//if(rtn && (byttemp[4] != 0x04)) 	
							{
								memcpy(preceived, byttemp, rtn);
#ifdef DEBUG_SERIAL_PRINT
								sprintf(chinfo, "receive:");
								for(bytPrint = 0; bytPrint < rtn - 1; bytPrint++)
									sprintf(chinfo, "%s%02X", chinfo, byttemp[bytPrint]);
								PRINTK("%s\n", chinfo);
#endif
								/*intFile = fopen("./peilj.txt", "at+");
									fwrite(chinfo, strlen(chinfo), 1, intFile);
								chinfo[0] = 13;
								chinfo[1] = 10;
								fwrite(chinfo, 2, 1, intFile);
								fclose(intFile);*/
								return byttemp[4];
							}
						}//else
							//return 1;
					}
				}
			}
		}	
		//PRINTK("the end time is %d and sub is %d the max is %f\n", end, (end - start), 1.5 * CLOCKS_PER_SEC);
		gettimeofday(&tv2,&tz2);
		if ((((tv2.tv_sec-tv1.tv_sec)*1000000)+tv2.tv_usec-tv1.tv_usec)>=1800000)
		{
			blnTimeout = 0;
		}
	}while(blnTimeout);
#ifdef	DEBUG_SERIAL_PRINT
	PRINTK("communication return is timeout! \n");
#endif
	if(lreadlen > 0)
		;
	return 0xff;
}


void left_move(unsigned char *src_code, unsigned char len)
{
unsigned char buf[300], xor_code;
short i;
	
	memcpy(buf, src_code, len);
	xor_code = 0;
	for(i = 0; i < len - 1; i++)
	{
		xor_code ^= buf[i];
		src_code[i] = ((buf[i] & 0xf) << 4) + ((buf[i + 1] & 0xf0) >> 4);
	}
	src_code[i] = ((buf[0] & 0xf0) >> 4) + ((buf[i] & 0xf) << 4);
	
}

/*
Blocktype LEN  MYNODE  NODE  D0 ... D(N-1)  LRC1  LRC2
*/
unsigned char DealSmartCommand(int fd, unsigned char *cmd_buf, int cmd_len, unsigned char *out_buf, unsigned char *out_len)
{
unsigned char sendbuf[300], buf[100];
unsigned char ret, i;
unsigned char lrc0, lrc1;
unsigned short cpulen;

	//answer the 4e
	sendbuf[0] = 0x41;
	sendbuf[1] = 0x01;
	if(0 == (ret = writecom(fd, sendbuf, 2)))
		return 0;
	//deal command
	switch(cmd_buf[4])
	{
	case 0x15:
		sendbuf[0] = 0xff;
		if((cmd_buf[5] == 0) || (cmd_buf[5] == 1))
		{
			rf_select(cmd_buf[5]);
			sendbuf[0] = 0;
		}
		SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
		break;
	case 0x20:	//load key
		ret = mcml_load_key(cmd_buf[5], cmd_buf[6], cmd_buf[7], &cmd_buf[8]);
		SmartResponse(fd, cmd_buf[2], &ret, 0x01);
		break;
	case 0x21:	//AUTHENTICATION
		ret = mcml_authentication(cmd_buf[5], cmd_buf[6], cmd_buf[7]);
		SmartResponse(fd, cmd_buf[2], &ret, 0x01);
		break;
	case 0x2a:	//close antena
		mcml_pwr_off();
		sendbuf[0] = 0;
		ret = SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
		break;
	case 0x22:	//request 
		for(i = 0; i < cmd_buf[6]; i++)
		{
			if(mcml_request2(PICC_REQALL, sendbuf) == 0)
			{
				break;
			}
			set_timeout(3000);
		}
		if(i >= cmd_buf[6])
		{
			sendbuf[0] = 0x00;
			ret = SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
		}else
		{
			ret = SmartResponse(fd, cmd_buf[2], sendbuf, 0x02);
		}
		break;
	case 0x23:	//anticoll
		if((cmd_buf[1] - 2) == 1)
		{//first anticoll
			ret = mcml_anticoll(sendbuf);
			if(ret != 0)
				SmartResponse(fd, cmd_buf[2], &ret, 0x01);
			else
				SmartResponse(fd, cmd_buf[2], sendbuf, 0x05);
		}else if((cmd_buf[1] - 2) == 2)
		{//second anticoll
			//ret = mcml_anticoll2(sendbuf);
			if(ret != 0)
				SmartResponse(fd, cmd_buf[2], &ret, 0x01);
			else
				SmartResponse(fd, cmd_buf[2], sendbuf, 0x05);
		}
		break;
	case 0x24:	//select
		if((cmd_buf[1] - 2) == 6)
		{//first select
			ret = mcml_select(&cmd_buf[5], &i);
			if(ret != 0)
				SmartResponse(fd, cmd_buf[2], &ret, 0x01);
			else
			{
				sendbuf[0] = 0;
				sendbuf[1] = i;
				SmartResponse(fd, cmd_buf[2], sendbuf, 0x02);
			}
		}else if((cmd_buf[1] - 2) == 7)
		{//second select
			//ret = mcml_select2(&cmd_buf[5], &i);
			if(ret != 0)
				SmartResponse(fd, cmd_buf[2], &ret, 0x01);
			else
			{
				sendbuf[0] = 0;
				sendbuf[1] = i;
				SmartResponse(fd, cmd_buf[2], sendbuf, 0x02);
			}
		}
		break;
	case 0x25:	//read block
		if((cmd_buf[1] - 2) == 2)
		{//read m1...mcml
			sendbuf[0] = mcml_read(cmd_buf[5], &sendbuf[1]);
			if(sendbuf[0] != 0)
				SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
			else
				SmartResponse(fd, cmd_buf[2], &sendbuf[1], 16);
		}else if((cmd_buf[1] - 2) == 3)
		{//read token
			sendbuf[0] = UL_Page_Read(cmd_buf[5], &sendbuf[1]);
			if(sendbuf[0] != 0)
				SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
			else
				SmartResponse(fd, cmd_buf[2], &sendbuf[1], 16);
		}else if((cmd_buf[1] - 2) == 4)
		{//read m1
		}else
		{
			sendbuf[0] = 0xff;
			SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
		}
		break;
	case 0x26:	//write block
		if((cmd_buf[1] - 1) == 2)
		{
			sendbuf[0] = UL_Page_Write(cmd_buf[5], &cmd_buf[6]);
			SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
		}
		break;
	case 0x30:
		if((cmd_buf[1] - 2) == 1)
		{//atr
			sendbuf[0] = sam_atr(chSmartSAMIndex, &sendbuf[1], out_len);
			if(sendbuf[0] != 0)
			{
				SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
			}else
			{
				SmartResponse(fd, cmd_buf[2], &sendbuf[1], (*out_len));
			}
		}else if((cmd_buf[1] - 2) == 2)
		{//select
			ret = sam_select(cmd_buf[5]);
			SmartResponse(fd, cmd_buf[2], &ret, 0x01);
			chSmartSAMIndex = cmd_buf[5];
		}else if((cmd_buf[1] - 2) == 3)
		{//set speed
			if(cmd_buf[6] == 0)
				buf[0] = SAM_ETU_93;
			else
				buf[0] = SAM_ETU_372;
			sam_set(cmd_buf[5], buf[0], 4);
			sendbuf[0] = 0;
			SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
		}
		break;
	case 0x31:
		ret = sam_apdu(chSmartSAMIndex, &cmd_buf[5], cmd_buf[1] - 3, sendbuf, out_len, 0, 0);
		if(ret != 0)
			SmartResponse(fd, cmd_buf[2], &ret, 0x01);
		else
		{
			SmartResponse(fd, cmd_buf[2], sendbuf, *out_len);
		}
		break;
	case 0x7c:
		switch(cmd_buf[5])
		{
		case 0x00:		//ats
			ret = mifpro_ats(cmd_buf[6], sendbuf, out_len);
			if(ret != 0)
				SmartResponse(fd, cmd_buf[2], &ret, 0x01);
			else
				SmartResponse(fd, cmd_buf[2], sendbuf, *out_len);
			break;
		case 0x01:		//deselect
			//mifpro_deselect
			break;
		case 0x07:		//icmd
			ret = mifpro_apdu(&cmd_buf[6], cmd_buf[1] - 4, &sendbuf[1], &cpulen);
			*out_len = cpulen;
			if(ret != 0)
				SmartResponse(fd, cmd_buf[2], &ret, 0x01);
			else
			{
				sendbuf[0] = 0x00;
				SmartResponse(fd, cmd_buf[2], sendbuf, (*out_len) + 1);
			}
			break;
		}
		break;
	default:
		sendbuf[0] = 0;
		SmartResponse(fd, cmd_buf[2], sendbuf, 0x01);
		break;
	}
	//receive the 4e
	return ret;
}

/*
function:
parameter:
	fd:com handle
	chCode:response code
	chCommnd:command
	psend:message body
	len:message length
*/
unsigned char SmartResponse(int fd, unsigned char response_node, unsigned char *psend, unsigned char len)
{
unsigned char 	sendbuf[600], lrc0, lrc1;
long 		ret;
unsigned char chinfo[1000], chtemp[20], fStat;
long 	i;
struct timeval timeout_smart;
fd_set writefd;

	memset(sendbuf, 0x00, 600);
	//response data length + response code length + the command length)
	sendbuf[0] = 0x7e;
	sendbuf[1] = len + 2;
	sendbuf[2] = 0x01;
	sendbuf[3] = response_node;

	if ((psend != NULL) && (len > 0))
		memcpy(&sendbuf[4], psend, len);

	lrc0 = lrc1 = 0x33;
	for(i = 2; i < len + 4; i++)
	{
		lrc0 ^= sendbuf[i];
		lrc1 += sendbuf[i];
	}
	sendbuf[len + 4] = lrc0;
	sendbuf[len + 5] = lrc1;
#ifdef DEBUG_COMM	
	memset(chinfo, 0x00, 1000);
	for(i = 0; i < len + 6; i++)
	{
		sprintf((char *)chtemp, "%02X ", sendbuf[i]);
		strcat((char *)chinfo, (char *)chtemp);
	}
	PRINTK("response:%s\n", chinfo);
#endif
	writecom(fd, sendbuf, len + 4 + 2);
	//
	FD_ZERO(&writefd);
	FD_SET(fd, &writefd);
	timeout_smart.tv_sec = 0;
	timeout_smart.tv_usec = 100 * 1000;
	do
	{
		ret = select(fd + 2, &writefd, NULL, NULL, &timeout_smart);
		if(ret > 0)
		{
			do
			{
				fStat = readcom(fd, chtemp, 1);
				if(fStat)
				{
					if(chtemp[0] == 0x41)
						continue;
					else if(chtemp[0] == response_node)
						return 1;
				}
			}while(1);
		}else
		{
			return 0;
		}
	}while(1);
	
	return 1;
}


void xa_protocol_deal(int fd, unsigned char *bytCmd, long cmdLen, unsigned char *out_buf)
{
unsigned short out_len;
unsigned char chResCode;
unsigned short shCommand;

	if( g_blnHHJTorFounder )
	{//HHJT
		chResCode = xaDealCommand(bytCmd, cmdLen, out_buf, &out_len);
		//printf("xaDealCommand HHHJT chResCode = %d\n",chResCode); //20230103
		xaReaderResponse(fd, chResCode, &bytCmd[3], out_buf, out_len);
	}else 
	{//Founder
		memcpy(&shCommand, &bytCmd[3], 2);
		if( (shCommand == 0x5331) && (tpCmdInit.timeout == 0) )
		{
			memcpy(tpCmdPolling.command, bytCmd, cmdLen);
			reader_status = XA_RW_SEARCH;
			
			g_blnContinuePolling = 0xff;
			sem_post(&g_founderwait);
		}else if( shCommand == 0x5321)
		{
			chResCode = xaDealCommand(bytCmd, cmdLen, out_buf, &out_len);
			
			xaReaderResponse(fd, chResCode, &bytCmd[3], out_buf, out_len);
		}
		{
			g_blnContinuePolling = 0x00;
			chResCode = xaDealCommand(bytCmd, cmdLen, out_buf, &out_len);
			
			xaReaderResponse(fd, chResCode, &bytCmd[3], out_buf, out_len);
		}
	}
}


void *pthFounder()
{
unsigned char res_buf[300], chResCode;
unsigned short res_len;

#ifdef DEBUG_PRINT
	PRINTK("founder search thread start\n");
#endif
	for(;;)
	{
		sem_wait(&g_founderwait);
		if(xa_founder_polling(tpCmdPolling.command) == 0)
		{
			if( (chResCode = xa_polling_card(tpCmdPolling.command, &res_buf[0], &res_len)) == 0)
			{
				if(g_blnContinuePolling)
					xaReaderResponse(csc_comm, chResCode, &tpCmdPolling.command[3], res_buf, res_len);
				continue;
			}
		}
		if(g_blnContinuePolling)
			sem_post(&g_founderwait);
	}
}