#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
# include <stdio.h>

#include "linux2440lib.h"

pthread_t	g_pthtmrID;

/*
function:display the led and set watch dog
*/
void *pthtmr()
{
char	led_ctrl = 0;

#ifdef DEBUG_PRINT
	printf("timer thread start\n");
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
int 	i, j, ret;
char 	filename[40];
unsigned char chtemp[1000], out_buf[200], buf[7];
unsigned char chreturn;
pid_t	pid;
short firstdays, enddays;
struct tm tptime;
time_t lngtime;
unsigned short	respLen;
unsigned long 	lngsecond;

	
	//watchdog_init(WATCHDOG_STOP, 10);
	//return 0;
	//set signal
	signal(SIGCHLD,SIG_IGN);
	printf("argc %d argv[0] %s argv[1] %s\n", argc, argv[0], argv[1]);
	 do{
	 	if(atol(argv[1]) == 0)
	 		try_polling_1();
	 	else if (atol(argv[1]) == 1)
	 		try_polling_2();
	 	else
	 		func_polling();
	 	sleep(1);
	 }while(1);
	return 0;
}
