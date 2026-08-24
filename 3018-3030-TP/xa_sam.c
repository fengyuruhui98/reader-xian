#include "xa_sam.h"
#include "linux2440lib.h"
#include "xa_ul_operation.h"
#include "sz_xdr_api.h"
#include "xa_cpu20_operation.h"
#include "xa_tong_operation.h"
#include "hh_cpu_operation.h"
#include "serial.h"
#include "xa_error_code.h"
#include "sha1.h"
#include "xa_operation.h"
#include "eeprom.h"

extern unsigned char mac1[8], mac2[4], mac_ret;

/*======================================================================
函数：
功能：PSAM初始化
========================================================================*/
int ResetXAMetroSam(void)
{
unsigned char chret, retry, i, j;
unsigned char sambuf[257]; 
unsigned char sambytes;
unsigned char buf[257];
unsigned char inlen;
int  ret;
	
	memset(ch_cpu20_psam_id, 0x00, 6);
	memset(ch_cput_psam_id, 0x00, 6);
	memset(ch_cput_isam_id, 0x00, 6);
	memset(ch_cput_psam_sn, 0x00, 8);
	memset(ch_cput_isam_sn, 0x00, 8);
	//sam_get_ver(0, sambuf, &sambytes);
//	PRINTK("mcu0:");
//	for(j = 0; j < sambytes; j++)
//		PRINTK("%02x", sambuf[j]);
//	PRINTK("\n");
//	//sam_get_ver(1, sambuf, &sambytes);
//	PRINTK("mcu1:");
//	for(j = 0; j < sambytes; j++)
//		PRINTK("%02x", sambuf[j]);
//	PRINTK("\n");
	//metro psam/isam
	XAMetroSAM();
	XAYKTPSAM();
	XAYKTISAM();
	XATTBPSAM();
	//
#ifdef DEBUG_PBOC
	XAPbocPSAM();
#endif
	return 0;	
}

int XAMetroSAM(void)
{
unsigned char chret, retry, i, j;
unsigned char sambuf[257]; 
unsigned char sambytes;
unsigned char buf[257];
unsigned char inlen;
int  ret;

	i = 4;
	{
		if(sam_select(i) != 0)
		//	continue;
			PRINTK("select xian metro sam error \n");
		//
		sam_set(i, SAM_ETU_372, 4);
		for(retry = 0; retry < 3; retry++)
		{
		  	if((chret = sam_atr(i, sambuf, &sambytes)) != 0)
		  	{
		  		PRINTK("xa-metro sam atr return %02x\n", chret);
		    	continue;
		    }
		    PRINTK("xa-metro atr:");
		    for(j = 0; j < sambytes; j++)
		    	PRINTK("%02x", sambuf[j]);
		    PRINTK("\n");
		    ret = sam_pts(i, 0x13);		//38400
		    //ret = sam_pts(i, 0x38);			//57600
		    //PRINTK("sam pts return %d\n", ret);
		    //ret = sam_pps0(i, 0x13, sambuf, &sambytes);
		    //PRINTK("sam pts return %d bytes %d\n", ret, sambytes);
		    //for(j = 0; j < sambytes; j++)
		    //	PRINTK("%02x", sambuf[j]);
		    //PRINTK("\n");
		    sam_set(i, SAM_ETU_93, 4);	//38400
		    //sam_set(i, SAM_ETU_57600, 4);
		    set_timeout(5000);
		    //终端信息文件－终端机编号
		 	memcpy(buf,"\x00\xb0\x96\x00\x06",5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
		  	{
		  		PRINTK("sam apdu read file 16 return %02x\n", chret);
			    continue;
	    	}	
		  	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0x00))
		  	{
		  		PRINTK("file 16 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			    continue;
	  		}
		  	memcpy(ch_cpu20_psam_id, sambuf, 6);
		  	PRINTK("\nxa-metro sam id :%02x %02x %02x %02x %02x %02x\n", ch_cpu20_psam_id[0], ch_cpu20_psam_id[1], ch_cpu20_psam_id[2], ch_cpu20_psam_id[3], 
		  			ch_cpu20_psam_id[4], ch_cpu20_psam_id[5]);
		    //public info
		 	memcpy(buf, "\x00\xb0\x95\x00\x02", 5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
		  	{
		  		PRINTK("sam apdu read file 15 return %02x\n", chret);
			    continue;
	    	}	
		  	//if(sambytes != 4)2018/4/27 7:09:06
		  	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0x00))
		  	{
		  		PRINTK("file 15 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			    continue;
	  		}
		  	sam_type = sambuf[0];
		  	sam_version = sambuf[1];
		  	PRINTK("xa-metro sam type(0xe psam 0xf isam):%02x sam ver %02x\n", sam_type, sam_version);
			//generate the transfer key
			if(sam_type == 0x0F)
			{
				memset(sambuf, 0x00, 16);
				memcpy(&sambuf[2], ch_cpu20_psam_id, 6);
				//sambuf[6] = 0x06;
				//sambuf[7] = 0x97;
				for(j = 8; j < 16; j++)
					sambuf[j] = ~sambuf[j - 8];
				chret = cpu_cal_dcmk(i, "\x08\x01", NULL, 0, 0x00, sambuf, 16, Metro_Transfer_key, &sambytes);
				if(chret != 0)
					continue;
	  		}
	  		memcpy(buf,"\x00\xa4\x00\x00\x02\x2f\x01",7);
		  	if(sam_apdu(i, buf, 7, sambuf, &sambytes, 0, 0) != 0)
		  	{
		  		PRINTK("select 2f01 return error\n");
			    continue;
	    	}	
	    	PRINTK("select 2f01 :%02x%02x-%02x\n", sambuf[0], sambuf[1], sambytes);
		  	if((sambuf[0] == 0x61) && (sambytes == 2))
		  	{
				memcpy(buf, "\x00\xc0\x00\x00", 4);
				buf[4] = sambuf[1];
				if(sam_apdu(i, buf, 5, sambuf, &sambytes, 0, 0) != 0)
					continue;
	  		}
			PRINTK("select 2f01: ");
			for(j = 0; j < sambytes; j++) PRINTK("%02x", sambuf[i]);
			PRINTK("\n");
			if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
				continue;
			memcpy(xa_metro_psam_sfi, "\x2f\x01", 2);
			xa_metro_psam_index = i;
			break;
		}
	}
}

int XAYKTPSAM(void)
{
unsigned char chret, retry, i, j;
unsigned char sambuf[257]; 
unsigned char sambytes;
unsigned char buf[257];
unsigned char inlen;
int  ret;

struct timeval tv1,tv2;
struct timezone tz1,tz2;

	i = 5;
	{
		if(sam_select(i) != 0)
		//	continue;
			PRINTK("select xian tong psam error \n");
		//
		sam_set(i, SAM_ETU_93, 4); 
	//	sam_set(i, SAM_ETU_93, 16); 
		for(retry = 0; retry < 3; retry++)
		{
		  	if((chret = sam_atr(i, sambuf, &sambytes)) != 0)
		  	{
		  		PRINTK("xa-tong sam atr return %02x\n", chret);
		    	continue;
		    }
		    PRINTK("xa-tong psam atr:");
		    for(j = 0; j < sambytes; j++)
		    	PRINTK("%02x", sambuf[j]);
		    PRINTK("\n");
			//delay_ms(500);//20230617 增加延时，看是否会读到16
		    //device id
			//sleep(1);
		    gettimeofday(&tv2,&tz2);
		    PRINTK("B %ds %dus\n", tv2.tv_sec, tv2.tv_usec);
		 	memcpy(buf,"\x00\xb0\x96\x00\x06",5);
			//memcpy(buf,"\x00\xb0\x95\x00\x0A",5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
		  	{
		  		PRINTK("sam apdu read file 16 return %02x\n", chret);
			    continue;
	    	}	
		    gettimeofday(&tv2,&tz2);
		    PRINTK("A %ds u%d\n", tv2.tv_sec, tv2.tv_usec);

		  	//if(sambytes != 8)
		  	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0x00))
		  	{
		  		PRINTK("file 16 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			
			    continue;
	  		}
		  	memcpy(ch_cput_psam_id, sambuf, 6);
		  	PRINTK("xa-tong psam id :%02x %02x %02x %02x %02x %02x\n", ch_cput_psam_id[0], ch_cput_psam_id[1], ch_cput_psam_id[2], ch_cput_psam_id[3], 
		  			ch_cput_psam_id[4], ch_cput_psam_id[5]);
			xa_tong_psam_index = i;
		    //sam serial number
		 	memcpy(buf,"\x00\xb0\x95\x02\x08",5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
		  	{
		  		PRINTK("sam apdu read file 15 return %02x\n", chret);
			    continue;
	    	}	
		  	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
		  	{
		  		PRINTK("file 15 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			    continue;
	  		}
		  	memcpy(ch_cput_psam_sn, sambuf, 8);
		  	PRINTK("xa-tong psam sn :%02x %02x %02x %02x %02x %02x %02x%02x\n", ch_cput_psam_sn[0], ch_cput_psam_sn[1], ch_cput_psam_sn[2], ch_cput_psam_sn[3], 
		  			ch_cput_psam_sn[4], ch_cput_psam_sn[5], ch_cput_psam_sn[6], ch_cput_psam_sn[7]);
			//select sz-tong psam file 1001
	  		memcpy(buf,"\x00\xa4\x00\x00\x02\x10\x01",7);
		  	if(sam_apdu(xa_tong_psam_index, buf, 7, sambuf, &sambytes, 0, 0) != 0)
		  	{
			    continue;
	    	}
	    	PRINTK("select xa-tong 1001 :%02x%02x\n", sambuf[0], sambuf[1]);
		  	if((sambuf[0] == 0x61) && (sambytes == 2))
			{
				memcpy(buf, "\x00\xc0\x00\x00", 4);
				buf[4] = sambuf[1];
				if(sam_apdu(xa_tong_psam_index, buf, 5, sambuf, &sambytes, 0, 0) != 0)
					continue;
			}
			PRINTK("select 1001: ");
			for(i = 0; i < sambytes; i++) PRINTK("%02x", sambuf[i]);
			PRINTK("\n");
			if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
				continue;
	  		
			break;
		}
	}
}

int XAYKTISAM(void)
{
unsigned char chret, retry, i, j;
unsigned char sambuf[257]; 
unsigned char sambytes;
unsigned char buf[257];
unsigned char inlen;
int  ret;

	i = 6;
	{
		if(sam_select(i) != 0)
			PRINTK("select xian isam error \n");
		//
		sam_set(i, SAM_ETU_372, 4);
		 
		for(retry = 0; retry < 3; retry++)
		{
		  	if((chret = sam_atr(i, sambuf, &sambytes)) != 0)
		  	{
		  		PRINTK("xian isam atr return %02x\n", chret);
		    	continue;
		    }
		    PRINTK("xa-tong isam-atr:");
		    for(j = 0; j < sambytes; j++)
		    	PRINTK("%02x", sambuf[j]);
		    PRINTK("\n");
		    //posid--6
		 	memcpy(buf,"\x00\xb0\x96\x00\x06", 5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
		  	{
		  		PRINTK("sam apdu read file 16 return %02x\n", chret);
			    continue;
	    	}
	    	if((sambytes == 2) && (sambuf[0] == 0x6c))
	    	{
	    		buf[4] = sambuf[1];
		  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
			  	{
			  		PRINTK("sam apdu read file 16 return %02x\n", chret);
				    continue;
		    	}
		  	}
		  	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0x00))
		  	{
		  		PRINTK("file 16 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			    continue;
	  		}
	  		for(j = 0; j < sambytes; j++)
	  			PRINTK("%02x ", sambuf[j]);
	  		PRINTK("\n");
		  	memcpy(ch_cput_isam_id, sambuf, 6);
		  	PRINTK("xa-tong isam id :%02x %02x %02x %02x %02x %02x\n", ch_cput_isam_id[0], ch_cput_isam_id[1], ch_cput_isam_id[2], ch_cput_isam_id[3], 
		  			ch_cput_isam_id[4], ch_cput_isam_id[5]);
			xa_tong_isam_index = i;
		    //samid--8
		 	memcpy(buf,"\x00\xb0\x95\x02\x08",5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
		  	{
		  		PRINTK("sam apdu read file 15 return %02x\n", chret);
			    continue;
	    	}
	    	if((sambytes == 2) && (sambuf[0] == 0x6c))
	    	{
	    		buf[4] = sambuf[1];
		  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
			  	{
			  		PRINTK("sam apdu read file 15 return %02x\n", chret);
				    continue;
		    	}
		  	}
	  		for(j = 0; j < sambytes; j++)
	  			PRINTK("%02x ", sambuf[j]);
	  		PRINTK("\n");
		  	if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
		  	{
		  		PRINTK("file 15 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			    continue;
	  		}
		  	memcpy(ch_cput_isam_sn, sambuf, 8);
		  	PRINTK("xa-tong posid :%02x %02x %02x %02x %02x %02x %02x%02x\n", ch_cput_isam_sn[0], ch_cput_isam_sn[1],  ch_cput_isam_sn[2], ch_cput_isam_sn[3], 
		  			ch_cput_isam_sn[4], ch_cput_isam_sn[5], ch_cput_isam_sn[6], ch_cput_isam_sn[7]);

			//select sz-tong psam file 1001
	  		memcpy(buf,"\x00\xa4\x00\x00\x02\x10\x01",7);
		  	if(sam_apdu(xa_tong_isam_index, buf, 7, sambuf, &sambytes, 0, 0) != 0)
		  	{
			    continue;
	    	}
	    	PRINTK("select xa-tong 1001 :%02x%02x\n", sambuf[0], sambuf[1]);
		  	if((sambuf[0] == 0x61) && (sambytes == 2))
			{
				memcpy(buf, "\x00\xc0\x00\x00", 4);
				buf[4] = sambuf[1];
				if(sam_apdu(xa_tong_isam_index, buf, 5, sambuf, &sambytes, 0, 0) != 0)
					continue;
			}
			PRINTK("select 1001: ");
			for(i = 0; i < sambytes; i++) PRINTK("%02x", sambuf[i]);
			PRINTK("\n");
			if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
				continue;
	  		
			break;
		}
	}
}

int XAPbocPSAM(void)
{
unsigned char chret, retry, i, j;
unsigned char sambuf[257]; 
unsigned char sambytes;
unsigned char buf[257];
unsigned char inlen;
int  ret;

	i = 7;
	{
		if(sam_select(i) != 0)
		//	continue;
			PRINTK("select pboc psam error \n");
		//
		sam_set(i, SAM_ETU_93, 4);
		for(retry = 0; retry < 3; retry++)
		{
		  	if((chret = sam_atr(i, sambuf, &sambytes)) != 0)
		  	{
		  		PRINTK("pboc sam atr return %02x\n", chret);
		    	continue;
		    }
		    PRINTK("pboc atr:");
		    for(j = 0; j < sambytes; j++)
		    	PRINTK("%02x", sambuf[j]);
		    PRINTK("\n");
		    //device id
		 	memcpy(buf, "\x00\xb0\x96\x00\x06", 5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
		  	{
		  		PRINTK("sam apdu read file 16 return %02x\n", chret);
			    continue;
	    	}	
		  	if(sambytes != 8)
		  	{
		  		PRINTK("file 16 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			    continue;
	  		}
		  	memcpy(ch_pboc_psam_id, sambuf, 6);
		  	PRINTK("pboc psam id :%02x %02x %02x %02x %02x %02x\n", ch_pboc_psam_id[0], ch_pboc_psam_id[1], ch_pboc_psam_id[2], ch_pboc_psam_id[3], 
		  			ch_pboc_psam_id[4], ch_pboc_psam_id[5]);
			xa_pboc_psam_index = i;
			//select pboc psam file 1002
	  		memcpy(buf,"\x00\xa4\x00\x00\x02\x10\x02",7);
		  	if(sam_apdu(xa_pboc_psam_index, buf, 7, sambuf, &sambytes, 0, 0) != 0)
		  	{
			    continue;
	    	}
	    	PRINTK("select pboc 1002 len %02x:%02x%02x\n", sambytes, sambuf[0], sambuf[1]);
		  	if((sambuf[0] == 0x61) && (sambytes == 2))
			{
				memcpy(buf, "\x00\xc0\x00\x00", 4);
				buf[4] = sambuf[1];
				if(sam_apdu(xa_pboc_psam_index, buf, 5, sambuf, &sambytes, 0, 0) != 0)
				{
					PRINTK("select 1002 lenth %02x, %02x%02x\n", sambytes, sambuf[0], sambuf[1]);
					continue;
				}
			}
			PRINTK("select 1002: ");
			for(i = 0; i < sambytes; i++) PRINTK("%02x", sambuf[i]);
			PRINTK("\n");
			if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
				continue;
	  		
			break;
		}
	}
}

int XATTBPSAM(void)
{
unsigned char chret, retry, i, j;
unsigned char sambuf[257]; 
unsigned char sambytes;
unsigned char buf[257];
unsigned char inlen;
int  ret;

	i = 7;
	{
		if(sam_select(i) != 0)
		//	continue;
			PRINTK("select ministry of transport psam error \n");
		//
		sam_set(i, SAM_ETU_93, 4);
		for(retry = 0; retry < 3; retry++)
		{
		  	if((chret = sam_atr(i, sambuf, &sambytes)) != 0)
		  	{
		  		PRINTK("xa-tranpsort sam atr return %02x\n", chret);
		    	continue;
		    }
		    PRINTK("ttb atr:");
		    for(j = 0; j < sambytes; j++)
		    	PRINTK("%02x", sambuf[j]);
		    PRINTK("\n");
		    sleep(1);
		    //device id
		 	memcpy(buf, "\x00\xb0\x96\x00\x06", 5);
		  	inlen = 5;
	  		if((chret = sam_apdu(i, buf, inlen, sambuf, &sambytes, 0, 0)) != 0)
		  	{
		  		PRINTK("sam apdu read file 16 return %02x\n", chret);
			    continue;
	    	}	
		  	if(sambytes != 8)
		  	{
		  		PRINTK("file 16 len %02x status %02x %02x\n", sambytes, sambuf[0], sambuf[1]);
			    continue;
	  		}
		  	memcpy(ch_transport_psam_id, sambuf, 6);
		  	PRINTK("transport psam id :%02x %02x %02x %02x %02x %02x\n", ch_transport_psam_id[0], ch_transport_psam_id[1], ch_transport_psam_id[2], ch_transport_psam_id[3], 
		  			ch_transport_psam_id[4], ch_transport_psam_id[5]);
			xa_transport_psam_index = i;
			//select pboc psam file 8011
	  		memcpy(buf,"\x00\xa4\x00\x00\x02\x80\x11", 7);
		  	if(sam_apdu(xa_transport_psam_index, buf, 7, sambuf, &sambytes, 0, 0) != 0)
		  	{
			    continue;
	    	}
	    	PRINTK("select tranport 8011 len %02x:%02x%02x\n", sambytes, sambuf[0], sambuf[1]);
		  	if((sambuf[0] == 0x61) && (sambytes == 2))
			{
				memcpy(buf, "\x00\xc0\x00\x00", 4);
				buf[4] = sambuf[1];
				if(sam_apdu(xa_transport_psam_index, buf, 5, sambuf, &sambytes, 0, 0) != 0)
				{
					PRINTK("select 8011 lenth %02x, %02x%02x\n", sambytes, sambuf[0], sambuf[1]);
					continue;
				}
			}
			PRINTK("select 8011: ");
			for(i = 0; i < sambytes; i++) PRINTK("%02x", sambuf[i]);
			PRINTK("\n");
			if((sambuf[sambytes - 2] != 0x90) || (sambuf[sambytes - 1] != 0))
				continue;
	  		
			break;
		}
	}
}

/*
function:calculate the sjt tac
parameter:
*/
char xasjt_cal_tac(char *in_buf, short in_len, unsigned long txnsn, unsigned char *tac)
{
unsigned char buf[50], sambuf[50], samlen;
char tmp[4];
int i;
//SHA1Context sha;
struct sha1_ctx sha;

#ifdef DEBUG_SAM
	memcpy(tac, "\x12\x34\x56\x78", 4);
	return 0;
#endif
	
	//init DES
	memset(buf, 0x00, 50);
	memcpy(buf, "\x80\x1a\x24\x01\x08", 5);
	//
	//txnsn = toMoto(txnsn);
	memcpy(&buf[5], &txnsn, 4);
	buf[9] = 0xFF;
#ifdef DEBUG_PRINT
	PRINTK("sjt tac init:");
	for(i = 0; i < 13; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(xa_metro_psam_index, buf, 5 + 8, sambuf, &samlen, 0, 0) != 0)
		return -3;
#ifdef DEBUG_PRINT
	PRINTK("init des return %02x %02x\n", sambuf[0], sambuf[1]);
#endif
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0x00))
		return -4;

	//cal tac
	memset(buf, 0x00, 50);
	memcpy(buf, "\x80\xfa\x05\x00\x20", 5);
/*	SHA1Reset(&sha);
	SHA1Input(&sha, in_buf, in_len);
	//if(!SHA1Result(&sha))
		return -2;
	else 
*/
#ifdef	DEBUG_PRINT
	PRINTK("Len %04x, TAC:", in_len);
	for(i = 0; i < in_len; i++) 
		PRINTK("%02x", in_buf[i]);
	PRINTK("\n");
#endif
	sha1_init(&sha);
	sha1_update(&sha, in_len, in_buf);
	sha1_final(&sha);
	{
#ifdef DEBUG_PRINT
		PRINTK("sha1:");
		for(i = 0; i < 5; i++)
			PRINTK("%08x ", sha.digest[i]);
		PRINTK("\n");
#endif
		LongToByte(sha.digest[0], &buf[13]);
		LongToByte(sha.digest[1], &buf[17]);
		LongToByte(sha.digest[2], &buf[21]);
		LongToByte(sha.digest[3], &buf[25]);
		LongToByte(sha.digest[4], &buf[29]);
	}

#ifdef DEBUG_PRINT
	PRINTK("sjt tac des:");
	for(i = 0; i < 37; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(xa_metro_psam_index, buf, 37, sambuf, &samlen, 0, 0) != 0)
		return -5;
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(xa_metro_psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
		if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
			return -7;
		memcpy(tac, sambuf, 4);
#ifdef DEBUG_PRINT		
		PRINTK("sjt tac %02x %02x %02x %02x\n", tac[0], tac[1], tac[2], tac[3]);
#endif
		return 0;
	}
	//if(samlen != 6)2018/4/27 7:11:14
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
		return -2;
	//tac
	memcpy(tac, sambuf, 4);

	return 0;
}
/*
function:calculate the sjt tac
parameter:
*/
char sjt_cal_mac(unsigned char *key, unsigned char *factor, unsigned char *in_data, unsigned char in_len, unsigned char *mac)
{
unsigned char buf[100], sambuf[50], samlen;
char tmp[4], i, chret;

#ifdef DEBUG_SAM
	memcpy(mac, "\x12\x34\x56\x78", 4);
	return 0;
#endif
	//select 1002 file
	if(memcmp(xa_metro_psam_sfi, "\x2f\x01", 2) != 0)
	{
		if(0 != sam_select_file(xa_metro_psam_index, "\x2f\x01", buf))
			return CE_METROPSAM;
		memcpy(xa_metro_psam_sfi, "\x2f\x01", 2);
	}
	
	memset(buf, 0x00, 50);
	memcpy(buf, "\x80\xfe\x01\x01\x1c", 5);
	memcpy(&buf[5], in_data, in_len);
#ifdef DEBUG_PRINT
	PRINTK("mac1 len %02x, data:", in_len);
	for(i = 0; i < in_len + 5; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if((chret = sam_apdu(xa_metro_psam_index, buf, 5 + in_len, sambuf, &samlen, 0, 0)) != 0)
	{
#ifdef DEBUG_PRINT
		PRINTK("	sam index %d apdu failure: %d\n", xa_metro_psam_index, chret);
#endif
		return -3;
	}
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < samlen; i++) PRINTK("%02x ", sambuf[i]);
	PRINTK("\n");
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(xa_metro_psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
	}

	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
	{
		return -4;
	}
#ifdef DEBUG_PRINT
	PRINTK("return:");
	for(i = 0; i < samlen; i++) PRINTK("%02x ", sambuf[i]);
	PRINTK("\n");
#endif
	memcpy(mac, sambuf, samlen - 2);
	return 0;
	//init DES
	memset(buf, 0x00, 50);
	memcpy(buf, "\x80\x1a\x28\x01\x08", 5);
	memcpy(&buf[2], key, 2);
	memcpy(&buf[5], factor, 8);
#ifdef DEBUG_2_PRINT
	PRINTK("sjt init des:");
	for(i = 0; i < 13; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(xa_metro_psam_index, buf, 13, sambuf, &samlen, 0, 0) != 0)
		return -3;
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0x00))
		return -4;
	//cal mac
	memset(buf, 0x00, 50);
	memcpy(buf, "\x80\xfa\x01\x00\x10", 5);
	//data len
	buf[4] = in_len;
	//data
	memcpy(&buf[5], in_data, in_len);
#ifdef DEBUG_PRINT
	PRINTK("sjt des data:");
	for(i = 0; i < 5 + in_len; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(xa_metro_psam_index, buf, 5 + in_len, sambuf, &samlen, 0, 0) != 0)
		return -5;
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(xa_metro_psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
		/*if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
			return -7;
		memcpy(mac, sambuf, 4);
		return 0;*/
	}
	if(samlen != 6)
		return -6;
	//tac
#ifdef DEBUG_2_PRINT
	PRINTK("sjt 3des:");
	for(i = 0; i < samlen; i++) PRINTK("%02x ", sambuf[i]);
	PRINTK("\n");
#endif
	memcpy(mac, sambuf, 4);
	return 0;
}

/*
function:calculate the external auth
parameter:
*/
char cpu_cal_dcmk(char psam_index, unsigned char *key, unsigned char *factor, unsigned char factor_len, unsigned char mac_type, unsigned char *in_data, unsigned char in_len, unsigned char *mac, unsigned char *maclen)
{
unsigned char buf[300], sambuf[300], samlen;
char tmp[4], i ;
unsigned char cpurandom[10], desdata[8];

	//init des
	memset(buf, 0x00, 50);
	memcpy(buf, "\x80\x1a\x28\x01\x08", 5);
	if(key != NULL)
		memcpy(&buf[2], key, 2);
	//factor
	buf[4] = factor_len;
	if(factor != NULL)
		memcpy(&buf[5], factor, factor_len);
#ifdef DEBUG_PRINT
	PRINTK("init des:");
	for(i = 0; i < 5 + factor_len; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(psam_index, buf, 5 + factor_len, sambuf, &samlen, 0, 0) != 0)
		return -5;
#ifdef DEBUG_PRINT
	PRINTK("init des %02x %02x\n", sambuf[0], sambuf[1]);
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
		if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
			return -7;
	}
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
		return -10;
	//des 
	memset(buf, 0x00, 50);
	memcpy(buf, "\x80\xfa\x00\x00", 4);
	buf[2] = mac_type;
	buf[4] = in_len;
	memcpy(&buf[5], in_data, in_len);
#ifdef DEBUG_PRINT
	PRINTK("des data:");
	for(i = 0; i < 5 + in_len; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(psam_index, buf, 5 + in_len, sambuf, &samlen, 0, 0) != 0)
		return -8;
#ifdef DEBUG_2_PRINT
	PRINTK("des return %02x %02x \n", sambuf[0], sambuf[1]);
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
		if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
			return -7;
	}
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
		return -11;
#ifdef DEBUG_PRINT
	PRINTK("3des:");
	for(i = 0; i < samlen; i++) PRINTK("%02x ", sambuf[i]);
	PRINTK("\n");
#endif
	*maclen = samlen - 2;
	memcpy(mac, sambuf, samlen - 2);
	//external auth
	/*memset(buf, 0x00, 50);
	memcpy(buf, "\x00\x82\x00\x01\x08", 5);
	memcpy(&buf[5], desdata, 8);
	if(sam_apdu(xa_metro_psam_index, buf, 5 + 8, sambuf, &samlen, 0, 0) != 0)
		return -8;
	PRINTK("external auth %02x %02x \n", sambuf[0], sambuf[1]);
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
		return -9;
	*/
	return 0;
}

/*
function:calculate the line mac
parameter:
	1.facor:logic id(file 05) & suzhou city code(2150800000), 8 bytes
	2.in_data:command+data
	3.in_len:
	4.mac:return mac, 4 bytes;
*/
char cpu_cal_protect_mac(unsigned char sam_index, unsigned char *factor, unsigned char len, unsigned char *key, unsigned char *in_data, unsigned char in_len, unsigned char *mac)
{
unsigned char buf[100], sambuf[100], samlen;
char tmp[4], i ;
unsigned char cpurandom[10], desdata[8];

	//memcpy(mac, "\x12\x34\x56\x78", 4);
	//return 0;
	//select 1001 file
	/*memcpy(buf, "\x00\xa4\x00\x00\x02\x10\x01", 7);
	if(sam_apdu(xa_metro_psam_index, buf, 7, sambuf, &samlen, 0, 0) != 0)
		return -1;
	
	if(sambuf[0] != 0x61)
		return -2;
	*/
	/*get raddom
	memset(buf, 0x00, 50);
	memcpy(buf, "\x00\x84\x00\x00\x08", 5);
	if(sam_apdu(xa_metro_psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
		return -3;
	PRINTK("get random %02x %02x\n", sambuf[0], sambuf[1]);
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0x00))
		return -4;
	memcpy(cpurandom, sambuf, 8);
	*/
	//init des
	memset(buf, 0x00, 50);
	memcpy(buf, "\x80\x1a\x23\x01\x08", 5);
	memcpy(&buf[2], key, 2);
	buf[4] = len;
	//factor
	memcpy(&buf[5], factor, len);
	//memcpy(&buf[5], ch_cpu20_logic_id, 8);
	//memcpy(&buf[13], "\x21\x50\x80", 3);
#ifdef DEBUG_PRINT
	PRINTK("init des:");
	for(i = 0; i < 5 + len; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(sam_index, buf, 5 + len, sambuf, &samlen, 0, 0) != 0)
		return -5;
#ifdef DEBUG_PRINT
	PRINTK("init des %02x %02x\n", sambuf[0], sambuf[1]);
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(sam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
		if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
			return -7;
	}
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
		return -10;
	//des-calculate mac mode & have initial value first 8 bytes
	memset(buf, 0x00, 60);
	memcpy(buf, "\x80\xfa\x05\x00", 4);
	buf[4] = in_len;// - 8;
	memcpy(&buf[5], in_data, in_len);
#ifdef DEBUG_PRINT
	PRINTK("des data:");
	for(i = 0; i < 5 + in_len; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(sam_index, buf, 5 + in_len, sambuf, &samlen, 0, 0) != 0)
		return -8;
#ifdef DEBUG_PRINT
	PRINTK("des return %02x %02x \n", sambuf[0], sambuf[1]);
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(sam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
		if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
			return -7;
	}
#ifdef DEBUG_PRINT
	PRINTK("3des:");
	for(i = 0; i < samlen; i++) PRINTK("%02x ", sambuf[i]);
	PRINTK("\n");
#endif
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
		return -11;
	
	memcpy(mac, sambuf, samlen - 2);
	//external auth
	/*memset(buf, 0x00, 50);
	memcpy(buf, "\x00\x82\x00\x01\x08", 5);
	memcpy(&buf[5], desdata, 8);
	if(sam_apdu(xa_metro_psam_index, buf, 5 + 8, sambuf, &samlen, 0, 0) != 0)
		return -8;
	PRINTK("external auth %02x %02x \n", sambuf[0], sambuf[1]);
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
		return -9;
	*/
	return 0;
}

/*
function:calculate the cpu tac
parameter:
*/
char cpu_cal_mac1(char psam_index, unsigned char *in_data, unsigned char in_len, unsigned char *mac1)
{
unsigned char buf[80], sambuf[50], samlen;
char tmp[4];
unsigned char cpurandom[50], desdata[8];
unsigned char i;

	//init sam for purchase
	memcpy(buf, "\x80\x70\x00\x00\x24", 5);
	buf[4] = in_len;
	//data
	memcpy(&buf[5], in_data, in_len);
	//buf[5 + in_len] = 0x08;
#ifdef DEBUG_PRINT
	PRINTK("cpu cal mac1 data:\n");
	for(i = 0; i < 5 + in_len; i++) PRINTK("%02x", buf[i]);
	PRINTK("\n");
#endif
	/*test:divide the sending to sam and receiving from sam to two function
	if(sam_apdu_send(xa_metro_psam_index, buf, 5 + in_len) != 0)
		return -5;
	return 0;
	*/
	//change expected data length from 10 to 2
	if(sam_apdu(psam_index, buf, 5 + in_len, sambuf, &samlen, 0, 0) != 0)
	{
		PRINTK("psam cal mac1 failure\n");
		return -5;
	}
#ifdef DEBUG_PRINT
	PRINTK("cal mac1 return %02x %02x\n", sambuf[0], sambuf[1]);
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
		{
			//PRINTK("get the mac1 return error----=====\n");
			return -6;
		}
		if(samlen == 2)
		{
			if(sam_apdu(psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
				return -6;
		}
		//PRINTK("reread mac1 data buf[4] %02x len %02x\n", buf[4], samlen);
	}
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
	{
		//PRINTK("reread cal mac1 %02x %02x -10\n", sambuf[samlen - 2], sambuf[samlen - 1]);
		return -10;
	}
	//
#ifdef DEBUG_PRINT
	PRINTK("mac1 return data:");
	for(i = 0; i < 10; i++) PRINTK("%02x ", sambuf[i]);
	PRINTK("\n");
#endif
	memcpy(mac1, sambuf, 8);
	return 0;
}

/*
function:calculate the cpu tac
parameter:
*/
char cpu_cal_mac2(char psam_index, unsigned char *in_data, unsigned char in_len, unsigned char *mac1)
{
unsigned char buf[80], sambuf[50], samlen;
char tmp[4];
unsigned char cpurandom[50], desdata[8];
unsigned char i;

	//select 1001 file
/*	memcpy(buf, "\x00\xa4\x00\x00\x02\x10\x01", 7);
	if(sam_apdu(xa_metro_psam_index, buf, 7, sambuf, &samlen, 0, 0) != 0)
		return -1;
	
	if(sambuf[0] != 0x61)
		return -2;
	*/
	memcpy(buf, "\x80\x72\x00\x00\x04", 5);
	//data
	memcpy(&buf[5], in_data, in_len);
#ifdef DEBUG_PRINT
	PRINTK("cal mac2 data:");
	for(i = 0; i < 5 + in_len; i++) PRINTK("%02x ", buf[i]);
	PRINTK("\n");
#endif
	if(sam_apdu(psam_index, buf, 5 + in_len, sambuf, &samlen, 0, 0) != 0)
	{
		PRINTK("psam cal mac2 failure \n");
		return -5;
	}
#ifdef DEBUG_PRINT
	PRINTK("cal mac2 return:");
	for(i = 0; i < samlen; i++) PRINTK("%02x ", sambuf[i]);
	PRINTK("\n");
#endif
	if((samlen == 2) && (sambuf[0] == 0x61))
	{
		memcpy(buf, "\x00\xc0\x00\x00", 4);
		buf[4] = sambuf[1];
		if(sam_apdu(psam_index, buf, 5, sambuf, &samlen, 0, 0) != 0)
			return -6;
		if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
			return -7;
	}
#ifdef DEBUG_2_PRINT
	PRINTK("cal mac2 return :");
	for(i = 0; i < 2; i++) PRINTK("%02x", sambuf[i]);
	PRINTK("\n");
#endif
	if((sambuf[samlen - 2] != 0x90) || (sambuf[samlen - 1] != 0))
		return -10;
	//
	
	return 0;
}
/*======================================================================
函数：ResetShangHaiMetroSam
功能：PSAM初始化
========================================================================*/ 
/*
int ResetShangHaiMetroSam(void)
{
	int ret,retry;
	UBYTE outbuf[257]; 
	UBYTE outbytes;
	UBYTE inbuf[257];
	UBYTE inbytes;
	gDebugStep=0x1000;
	bgSmpsamIndex = SAMLOCATION_4;
	sam_set(bgSmpsamIndex,SAM_ETU_93,4);
	
	
	#ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a entry shmetro reset");
	#endif	
	
	memset(bpgSmpsamNo,0,4);
	bgSmPsamValid=0;
	for(retry=0;retry<3;retry++)
	{
			#ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a shmetro atr");
	#endif	
		ret = sam_atr(bgSmpsamIndex,outbuf,&outbytes);
		gDebugStep=0x1001;
	  	if(ret != 0)
	  	{
	  		#ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a atr err");
	  		#endif	
	    	continue;
	    }	
	   #ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a get operator no");
	#endif	
		gDebugStep=0x1002;
	 	memcpy(inbuf,"\x00\xb0\x96\x00\x06",5);
	  	inbytes = 5;
	  	ret = sam_apdu(bgSmpsamIndex,inbuf,inbytes,outbuf,&outbytes);
	  	if(ret != 0)
	  	{
	  		#ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a send getoper err");
	  		#endif
		    continue;
	    }	
	    gDebugStep=0x1003;
	  	if(outbytes != 8)
	  	{
	  		#ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a getoper answer bytes err get=%02x",outbytes);
	  		#endif
		    continue;
	  	}
	  	gDebugStep=0x1004;
	  	memcpy(bpgSmpsamNo,&outbuf[2],4);
	  #ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a oper no =%02x %02x %02x %02x",bpgSmpsamNo[0],bpgSmpsamNo[1],bpgSmpsamNo[2],bpgSmpsamNo[3]);
	#endif	
	  	memcpy(inbuf,"\x00\xa4\x00\x00\x02\x10\x01",7);
	  	inbytes = 7;
	  	ret = sam_apdu(bgSmpsamIndex,inbuf,inbytes,outbuf,&outbytes);
	  	if(ret != 0)
	  	{
	  			#ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a selectfile send err");
	  		#endif
		    continue;
	    }	
	    gDebugStep=0x1005;
	  	if(((UBYTE)outbuf[0]!= (UBYTE)0x61) &&((UBYTE)outbuf[0]!= 0x90))
	  	{
	  			#ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a selectfile answer err,get %d %d",outbuf[0],outbuf[1]);
	  			#endif
		    continue;
	  	}
	  #ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a reset ok");
	#endif
	  	bgSmPsamValid = 1;
	  	return 0;	
	}
	#ifdef _DEBUG_SAM_
						debug_printf("\x0d\x0a reset err");
	#endif
	return -1;	
}
*/
/*======================================================================
函数：ResetShangHaiMobileSam
功能：手机支付PSAM初始化
========================================================================*/ 
/*int ResetShangHaiMobileSam(void)
{
	int ret,retry;
	UBYTE outbuf[257]; 
	UBYTE outbytes;
	UBYTE inbuf[257];
	UBYTE inbytes;
	bgSMobilepsamIndex = SAMLOCATION_7;
	sam_set(bgSMobilepsamIndex,SAM_ETU_372,8);
	memset(bpgSMobilepsamNo,0,6);
	bgSMobilePsamValid = 0;
	
	for(retry=0;retry<3;retry++)
	{
//		watchdog();
		sam_set(bgSMobilepsamIndex,SAM_ETU_372,8);
		ret = sam_atr(bgSMobilepsamIndex,outbuf,&outbytes);
		gDebugStep=0x1001;
	  if(ret != 0)
	  {
	   	continue;
	  }	
	  ret = sam_pps(bgSMobilepsamIndex,0x13,outbuf,&outbytes);
	  if(ret != 0)
	  {
	   	continue;
	  }
	  sam_set(bgSMobilepsamIndex,SAM_ETU_93,8);
		gDebugStep=0x1002;
	 	memcpy(inbuf,"\x00\xb0\x96\x00\x06",5);
	  inbytes = 5;
	  ret = sam_apdu(bgSMobilepsamIndex,inbuf,inbytes,outbuf,&outbytes);
	  if(ret != 0)
	  {
		   continue;
	  }	
	  gDebugStep=0x1003;
	  if(outbytes != 8)
	  {
		   continue;
	  }
	  gDebugStep=0x1004;
	  memcpy(bpgSMobilepsamNo,outbuf,6);
	  memcpy(inbuf,"\x00\xA4\x04\x00\x0E\x31\x50\x41\x59\x2E\x53\x59\x53\x2E\x41\x44\x46\x30\x31\x00",20);
	  inbytes = 20;
	  ret = sam_apdu(bgSMobilepsamIndex,inbuf,inbytes,outbuf,&outbytes);
	  if(ret != 0)
	  {
		   continue;
	  }	
	  gDebugStep=0x1005;
	  if(((UBYTE)outbuf[0]!= (UBYTE)0x61) &&((UBYTE)outbuf[0]!= 0x90))
	  {
		   continue;
	  }
	  bgSMobilePsamValid = 1;
	  return 0;	
	}
	return -1;		
	
}*/

void *pthsamcal()
{
char	sambuf[100], timebuf[10], buf[100], samlen;

#ifdef DEBUG_PRINT
	PRINTK("sam thread start\n");
#endif
	for(;;)
	{
		sem_wait(&g_samcalwait);
		//
		switch(ch_mac_sel)
		{
		case 1:
			if(memcmp(xa_metro_psam_sfi, "\x10\x01", 2) != 0)
			{
				if(0 != sam_select_file(xa_metro_psam_index, "\x10\x01", buf))
				{
					mac_ret = CE_METROPSAM;
					sem_post(&g_samreturn);
					break;
				}
			}
			mac_ret = cpu_cal_mac1(tpCPU.sz_psam_index, ch_cpu_mac_data, 0x24, sambuf);
			memcpy(mac1, sambuf, 8);
			//PRINTK("mac1 calculate finish\n");
			sem_post(&g_samreturn);
#ifdef DEBUG_2_TIME
			memcpy(timebuf, "\xf0\xf1", 2);
			ReaderResponse(csc_comm, 21, 0xF0, timebuf, 2);
#endif	
			break;
		case 2:
			mac_ret = cpu_cal_mac2(xa_metro_psam_index, ch_cpu_mac_data, 4, sambuf);
			//sem_post(&g_samreturn);
			break;
		case 3:
			if(0 == sam_select_file(xa_metro_psam_index, "\x10\x01", buf))
				memcpy(xa_metro_psam_sfi, "\x10\x01", 2);
			break;
		case 4:
			mac_ret = xasjt_cal_tac(ch_mac_data, sh_mac_len, g_sha1txnsn, tpYPT_txn_val.pYPT_tac);
			//
			memcpy(tpYPT_txn_val.pYPT_txn + tpYPT_txn_val.YPT_txnlen - 12, tpYPT_txn_val.pYPT_tac, 4);
			ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.pYPT_txn, tpYPT_txn_val.YPT_txnlen);
			sem_post(&g_samreturn);
			break;
		case 14:
			mac_ret = xasjt_cal_tac(ch_mac_data, sh_mac_len, g_sha1txnsn, tpYPT_txn_val.pYPT_tac);
			//
			memcpy(tpYPT_txn_val.YPT_txn + tpYPT_txn_val.YPT_txnlen - 12, tpYPT_txn_val.pYPT_tac, 4);
			ee_write_last_record(tpYPT_txn_val.YPT_type, tpYPT_txn_val.YPT_flag, tpYPT_txn_val.YPT_txn, tpYPT_txn_val.YPT_txnlen);
			sem_post(&g_samreturn);
			break;
		case 5:
			if(0 == sam_select_file(xa_metro_psam_index, "\x2f\x01", buf))
				memcpy(xa_metro_psam_sfi, "\x2f\x01", 2);
			break;
		case 6:
			mac_ret = cpu_cal_mac1(tpCPU.sz_psam_index, ch_cpu_mac_data, tpCPU.capp_len, sambuf);
			memcpy(mac1, sambuf, 8);
#ifdef DEBUG_PRINT
			PRINTK("metro/city card mac1 calculate finish\n");
#endif			
			sem_post(&g_samreturn);
#ifdef DEBUG_2_TIME
			memcpy(timebuf, "\xf0\xf1", 2);
			ReaderResponse(csc_comm, 21, 0xF0, timebuf, 2);
#endif	
			break;
		case 7:
			mac_ret = cpu_cal_mac2(tpCPU.sz_psam_index, ch_cpu_mac_data, 4, sambuf);
			//sem_post(&g_samreturn);
			break;
		}
		ch_mac_sel = 0;
	}
#ifdef DEBUG_PRINT
	PRINTK("sam thread return\n");
#endif
}

/*
function: select the file
*/
char sam_select_file(char psam_index, char *sfi, unsigned char *out_buf)
{
unsigned char buf[100], cpubuf[100], cpulen;
int ret, i;

#ifdef DEBUG_SAM
	return 0;
#endif
	//select file
	memcpy(out_buf, "\xfa\x80", 2);
	memcpy(buf, "\x00\xa4\x00\x00\x02", 5);
	memcpy(&buf[5], sfi, 2);
	ret = sam_apdu(psam_index, buf, 7, cpubuf, &cpulen, 0, 2);
	if(ret != 0)
	{
		return CE_METROPSAM;
	}
	memcpy(out_buf, "\xf0\x81", 2);
#ifdef DEBUG_PRINT
	PRINTK("select psam file %02x%02x:", sfi[0], sfi[1]);
	for(i = 0;i < cpulen; i++) PRINTK("%02x ", cpubuf[i]);
	PRINTK("\n");
#endif
	if((cpulen == 2) && (cpubuf[0] != 0x61))
	{
		return CE_METROPSAM;
	}

	return 0;
}
