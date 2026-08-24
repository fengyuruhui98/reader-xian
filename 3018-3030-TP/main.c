#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#include "sz_xdr_api.h"
#include "bin_file_manage.h"
#include "serial.h"
#include "xa_sam.h"
#include "xa_cpu20_operation.h"
#include "xa_tong_operation.h"
#include "linux2440lib.h"
#include "xa_error_code.h"
#include "xa_func.h"
#include "eeprom.h"
#include "xa_pboc_operation.h"
#include "tlv.h"
#include "xa_qr_operation.h"
#include "hh_cpu_operation.h"




pthread_t	g_pthtmrID;

/*
function:display the led and set watch dog
*/
void *pthtmr()
{
char	led_ctrl = 0;

#ifdef DEBUG_PRINT
	PRINTK("timer thread start\n");
#endif
	for(;;)
	{
		sleep(1);
		if(led_ctrl == 0)
		{
			//gled(LED_OFF);
			rled(LED_ON);
			led_ctrl += 1;
		}else
		{
			rled(LED_OFF);
			//gled(LED_ON);
			led_ctrl = 0;
		}
		watchdog(); 
	}
}

int main(int argc, char *argv[])
{
time_t	lnglocaltime;
FILE 	*fl, *f2;
int 	i, j, ret;
char 	filename[40];
unsigned char chtemp[1000], out_buf[200];
unsigned char chreturn;
pid_t	pid;
short firstdays, enddays;
struct tm tptime;
time_t lngtime;
long 	mon;

	mkdir("./paranew", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("./prognew", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("./progbak", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("./para", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("./cache", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	//watchdog_init(WATCHDOG_STOP, 10);
	//return 0;
	//set signal
	signal(SIGCHLD,SIG_IGN);
	
	pid = fork();
	if(pid < 0)
	{
		perror("The fork failed! may reboot the reader");
		system("reboot");
		exit(1);
	}else if(pid == 0)
	{
		PRINTK("my parent pid is %d\n", getppid());
	}else
	{
		//
		if(0 == FileisExist("./progbak/", "xian", chtemp))
			system("rm ./progbak/*");
		
		waitpid(pid, NULL, 0);
		//if(execl("/mnt/yaffs/sz_reader/", "sz.sh", NULL) < 0)
		system("./sz.sh &");
			//perror("execl sz.sh failure");
		exit(0);
	}
		//
	watchdog_init(WATCHDOG_START, 20); 
	g_blnHHJTorFounder = 0xff;
	//arm-linux-gcc 2.95
	tp_ver = 0x1400207;
	//arm-linux-gcc 4.93
	tp_ver = 0x2150628;//添加虎年纪念票日志
	tp_ver = 0x2250628;//西安交通部本地卡异地卡卡扣异常
	//tp_ver = 0x1600519;//死机 2.15
	tp_ver = 0x2160707;//智能客服读卡时有时读不到的问题
	tp_ver = 0x2170714;//计次票员工票监控日志优化
	tp_ver = 0x2180714;//删除日志 补票时新增17.1A文件更新交易
	tp_ver = 0x2190714;//1106文件新增字段0x13xxxxxx 车站09找不到时找区段0x13
	tp_ver = 0x2200714;//员工卡员工卡有效期改变日志 20250312
	tp_ver = 0x2210714;//老年卡取消进出站超时 20250507
	tp_ver = 0x2220714;//员工票有效期过期判断，更新相关字段 20250803
	
	/* tp_ver = 0x2230714; //SPI降频测试  1M
	tp_ver = 0x2240714; //SPI降频测试  2M */
	tp_ver = 0x2260714;//住建部卡交易优惠类型0xFF，延迟交易缺少进出站ID　
	//memset(&tpSystem1101.paratitle.format, 0x00, sizeof(tpSystem1101));
	tp_ver = 0x2270714;//修改免费出站票、福利票进站时间限制
	tp_ver = 0x2280714;//修改交通部本地卡异地卡历史交易完整性检查,修改区域拒绝判断失效问题,修改员工卡有效期检查,进出站检查有效期
	PACC_1101_SystemParameter();
	PACC_1102_Business();
	PACC_1104_Blacklist();
	PACC_1105_Product();
	PACC_1106_Location();
	PACC_1107_Calendar();
	PACC_1108_FareTable();
	PACC_1109_SaleTable();
	
	YKT_1901_Blacklist();
	YKT_1912_Card();
	YKT_1913_Property();
	YKT_1914_Load();
	YKT_1919_Terminal();
	YKT_1920_Continue();
	
	JTB_1931_Blacklist();
	JTB_1932_White();
	JTB_1933_Property();
	JTB_1934_Terminal();
	JTB_1935_Preferential();
	JTB_1938_Load();
	JTB_1939_Server();

	LCC_3021_Sensitive();
	//
	LCC_1002();
	LCC_1097();
	LCC_1103();
	//
	memset(&eod_download.filetype, 0x00, sizeof(struct EOD_DOWNLOAD));
	tpMCPUProtectIndex = 0;
	memset(tpMCPUProtect, 0x00, 10 * sizeof(MCPU_PROTECT_t));
	tpXACPUProtectIndex = 0;
	memset(tpXACPUProtect, 0x00, 10 * sizeof(XACPU_PROTECT_t));

	memset(&tlv_ppse, 0x00, sizeof(struct TLVEntity));
	memset(&tlv_aid, 0x00, sizeof(struct TLVEntity));
	memset(&tlv_gpo, 0x00, sizeof(struct TLVEntity));

	xa_get_line_number();
/*
	memcpy(chtemp, "\x03\x00\x01\x00\x01\x1e\x20\x01\x40\x01\x40\x06\x05\x01\x88\x04\x8e\x31\x79\x01\x80\x00\x00\x00\x01\x01\x01\x00\x00\x2a\x66\xcd\xd7\x00\x01\x4a\x14\xab\x04", 39);
	chreturn = xa_ul_sale(chtemp, out_buf, &out_buf[200]);
	PRINTK("return value is %d: \n", chreturn);
	if(out_buf[200] == 2)
		PRINTK("error code %02x %02x\n", out_buf[0], out_buf[1]);
*/
	reader_status = XA_RW_STOP;
	blnCalMAC2 = 0;
	memset(xa_metro_psam_sfi, 0x00, 2);
	
	sem_init(&g_samcalwait, 0, 0);
	sem_init(&g_samreturn, 0, 0);
	//sem_init(&g_sha1wait, 0, 0);
	sem_init(&g_founderwait, 0, 0);
	
	ret = pthread_create(&g_pthsamID, NULL, &pthsamcal, NULL);
	gled(LED_ON);
	ret = pthread_create(&g_pthtmrID, NULL, &pthtmr, NULL);
	ret = pthread_create(&g_pthfounderID, NULL, &pthFounder, NULL);
	//open comm
	csc_comm = open_port(argv[1]);
	
	if(csc_comm < 0)
	{
	 
      return -1;
	}
	speed_set(csc_comm, 115200);
	parity_set(csc_comm, 8, 1, 'n');
	tcflush(csc_comm, TCIOFLUSH);
	//
	if(0 != (ret = ee_read(EE_RESTART, 1, &chreturn)))
		PRINTK("ee read failure %d ", ret);
	if(chreturn)
	{
		//ReaderResponse(csc_comm, 0x21, 0xc8, out_buf, 2);
		out_buf[0] = reader_status;
		out_buf[1] = reader_status;
		xaReaderResponse(csc_comm, CE_OK, "\x08\x53", out_buf, 2);
	}
	chreturn = 0;
	ee_write(EE_RESTART, 1, &chreturn);
	//init sam
	ResetXAMetroSam();
  
	memset(tpauthLogout.MsgCode, 0x00, (sizeof(struct auth_out) - sizeof(struct auth_head)));

	fl = fopen("./para/auth", "rb");
	if(fl > 0)
	{
		fread(&tpauthLogin.head.startFlag, 1, sizeof(struct auth_in), fl);
		memcpy(tpauthLogout.SettDate, tpauthLogin.SettDate, 4);
		memcpy(tpauthLogout.BatchNo, tpauthLogin.BatchNo, 3);
		fclose(fl);
	}
	//clear the authoriation message
	tpauthLogin.LimiAmt = 0;
	memset(tpauthLogin.LimiTime, 0x00, 7);
	//open QR comm
	qr_timeout  = 100;
	if(argc > 2)
	{
		qr_comm = open_port(argv[2]);
		if(qr_comm > 0) 
		{
			speed_set(qr_comm, 115200);
			parity_set(qr_comm, 8, 1, 'n');
			tcflush(qr_comm, TCIFLUSH);
		}else
		{
			PRINTK("open_port %s is not exist!\n", argv[2]);
		}
	}
	if(argc > 3)
	{
		qr_comm_II = open_port(argv[3]);
		if(qr_comm_II > 0)
		{
			speed_set(qr_comm_II, 115200);
			parity_set(qr_comm_II, 8, 1, 'n');
			tcflush(qr_comm_II, TCIFLUSH);
		}else
		{
			PRINTK("open_port %s is not exist!\n", argv[3]);
		}
	}
	if(argc > 4)
	{
		qr_timeout = atol(argv[4]);
	}
	//
	com_serv(csc_comm, NULL, 0, NULL);
	//
	do{
	 }while(1);
	return 0;
}
