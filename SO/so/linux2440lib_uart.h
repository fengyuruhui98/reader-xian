//linux2440lib_uart.h

#ifndef _LINUX2440LIB_UART_H_
#define _LINUX2440LIB_UART_H_
//start of file

#include     <stdio.h>      /*标准输入输出定义*/
#include     <string.h>
#include     <unistd.h>     /*Unix标准函数定义*/
#include     <sys/stat.h>   /**/
#include     <fcntl.h>
#include     <termios.h>    /*POSIX终端控制定义*/
#include     <errno.h>      /*错误号定义*/

         
//
#define MAX_UART_INDEX   5
#define UART0_INDEX  0
#define UART1_INDEX  1
#define UART2_INDEX  2
#define UART3_INDEX  3
#define UART4_INDEX  4


extern int gUartHandle[MAX_UART_INDEX];  

//函数
void uart_init(void);
int uart_open(int uart_index,int baudrate);
int uart_close(int uart_index);
//
UBYTE uart_rece_is_empty(UBYTE index);
UBYTE uart_send_is_full(UBYTE index);
void uart_sendbuf_clr(UBYTE index);
UBYTE uart_send_is_empty(UBYTE index);
void uart_recebuf_clr(UBYTE index);
void uart_int_enable(UBYTE index);
void uart_int_disable(UBYTE index);
UBYTE uart_put_byte(UBYTE index,UBYTE inbyte);
UBYTE uart_put_byte_safe(UBYTE index,UBYTE inbyte);
UBYTE uart_put_bytes(UBYTE index,UBYTE *inbuf,UWORD inbytes,UWORD time_out);
UBYTE uart_get_byte(UBYTE index);

//end of file
#endif


