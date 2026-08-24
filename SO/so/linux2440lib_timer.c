//timer.c

#ifndef _TIMER_C_
#define _TIMER_C_
//start of file


UDWORD dwgTimerCnt[MAX_TIMER_INDEX];
UDWORD dwgTimerLimit[MAX_TIMER_INDEX];

/*=============================================================================
函数：
功能：
===============================================================================*/
UDWORD  timer_get_ms(void) 
{
struct timeval tv;
UDWORD tlong;
	//
	gettimeofday(&tv,NULL);
	//
	tlong = tv.tv_usec/1000;
	tlong += tv.tv_sec*1000;
	//
	return tlong;	
}	

/*=============================================================================
函数：
功能：
===============================================================================*/
void timer_set(UBYTE index,UWORD limit)
{
	if((UBYTE)index >= MAX_TIMER_INDEX) return;
	dwgTimerCnt[index] = timer_get_ms()+limit;	
	dwgTimerLimit[index] = limit;
	return;	
}

/*=============================================================================
函数：
功能：
===============================================================================*/
UBYTE timer_check(UBYTE index)
{
	if((UBYTE)index >= MAX_TIMER_INDEX) return 0;	
	if(timer_get_ms() > dwgTimerCnt[index]) return 1;
	return 0;		
}	

/*=============================================================================
函数：
功能：
===============================================================================*/
void timer_clr(UBYTE index)
{
	if((UBYTE)index >= MAX_TIMER_INDEX) return;
	dwgTimerCnt[index] = timer_get_ms()+dwgTimerLimit[index];
	return;		
}	

/*=============================================================================
函数：
功能：
===============================================================================*/
UDWORD timer_get(UBYTE index)
{
	return (timer_get_ms()-(dwgTimerCnt[index]-dwgTimerLimit[index]));
}	


//end of file
#endif


