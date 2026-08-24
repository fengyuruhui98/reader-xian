//timer.h

#ifndef _TIMER_H_
#define _TIMER_H_
//start of file

#include <sys/time.h>

#define TIMER_CMD_PROCESS_INDEX   0           //命令执行时钟      
#define TIMER_SAM_SEND_INDEX      1            //SAM模组协议发送定时器 
#define TIMER_SAM_RECE_INDEX      2            //SAM模组协议接收定时器


#define MAX_TIMER_INDEX           128

extern UDWORD dwgTimerCnt[MAX_TIMER_INDEX];

void timer_set(UBYTE index,UWORD limit);
UBYTE timer_check(UBYTE index);
void timer_clr(UBYTE index);
UDWORD timer_get(UBYTE index);

//end of file
#endif



