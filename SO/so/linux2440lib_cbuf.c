//cbuf.c
//循环缓冲区管理程序
//编制：邓建华
//时间：20070801

#ifndef _CBUF_C_
#define _CBUF_C_
//start of file

/*=======================================================================
函数：cbuf_init
功能：
=========================================================================*/
void cbuf_init(CYCLE_BUF *cbuf,UWORD buf_size,UBYTE *buf)
{
cbuf->start_ptr = 0;
cbuf->end_ptr = 0;
cbuf->buf_size = buf_size;
cbuf->buf = buf;
return;	
}

/*=======================================================================
函数：cbuf_is_empty
功能：
=========================================================================*/
UBYTE cbuf_is_empty(CYCLE_BUF *cbuf)
{
if(cbuf->start_ptr == cbuf->end_ptr) return 1;
return 0;	
}	

/*=======================================================================
函数：cbuf_is_full
功能：
=========================================================================*/
UBYTE cbuf_is_full(CYCLE_BUF *cbuf)
{
UWORD temp_word;

temp_word = (cbuf->start_ptr+1)%cbuf->buf_size;
if(temp_word == cbuf->end_ptr) return 1;

return 0;	
}	

/*=======================================================================
函数：cbuf_put_byte
功能：
=========================================================================*/
UBYTE cbuf_put_byte(CYCLE_BUF *cbuf,UBYTE inbyte)
{
if(cbuf_is_full(cbuf)) return 0xff;
cbuf->buf[cbuf->start_ptr] = inbyte;
cbuf->start_ptr = (cbuf->start_ptr+1)%cbuf->buf_size;

return 0;	
}	

UBYTE cbuf_check_byte(CYCLE_BUF *cbuf)
{
return cbuf->buf[cbuf->end_ptr];	
}	

/*=======================================================================
函数：cbuf_is_empty
功能：
=========================================================================*/
UBYTE cbuf_get_byte(CYCLE_BUF *cbuf)
{
UBYTE temp_char;
	
if(cbuf_is_empty(cbuf)) return 0;
temp_char = cbuf->buf[cbuf->end_ptr];
cbuf->end_ptr = (cbuf->end_ptr+1)%cbuf->buf_size;

return temp_char;
}	


/*=======================================================================
函数：cbuf_clr
功能：
=========================================================================*/
void cbuf_clr(CYCLE_BUF *cbuf)
{
cbuf->start_ptr=0;
cbuf->end_ptr=0;
return;	
}	



//end of file
#endif


