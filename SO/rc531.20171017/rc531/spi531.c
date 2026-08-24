/*
 * RC531 testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <string.h>
#include "spi.h"
#include "rc_op.h"
#include "rc_base.h"




void set_beep_on(void)  
{     
	//3018
    //system("echo 1 > /sys/bus/platform/devices/leds-gpio/leds/beep/brightness");  
	//3030
    system("echo 1 > /sys/class/leds/beep/brightness");  
}  

void set_beep_off(void)  
{     
    //3018
    //system("echo 0 > /sys/bus/platform/devices/leds-gpio/leds/beep/brightness");  
    //3030
    system("echo 0 > /sys/class/leds/beep/brightness");  
}  


int main(int argc, char *argv[])
{
	int ret = 0;
	int spinum = 0;
	int speed = 200*1000; 
  char* dev   = NULL;
  char* baudrate   = NULL;
  uint16_t cardtype;

			//rf_select(1);//?????
    dev   = argv[1];
    baudrate   = argv[2];
    if (dev!=NULL)
    {
    	spinum = atoi(dev);
    	rf_select(spinum);
    }
    if (baudrate!=NULL)
    {
    	speed = atoi(baudrate);
    	spi_set_speed(speed);
    }


		//rc_init0();
		rc_init();
    rc_select_op_type(ISO14443A_M1_TYPE);
    rc_select_mifare_auth();
    rc_power_on();

    
		//spi_close();
		uint8_t buf[5];
		int i,t;
	
	  i=0;
	  while(1){
		   if(rc_request(PICC_REQSTD,buf) == 0){
		   	  printf("\n==========================have card %d --%02x%02x \n", i,buf[0],buf[1]);
		   	  //printf("=%02x%02x=", buf[0],buf[1]);
		   	  //---------------------------
		   	  //0x04= CARD_S50;           0x44=UL¿¨            0x08=CPU¿¨           0x03= SHUL¿¨
		   	  buf_to_uword(&buf[0], cardtype);
				 switch (cardtype) {
					case 0x4400:
						 set_beep_on();
						break;
					case 0x0800:
						 set_beep_on();
						break;
					case 0x0300:
						 set_beep_on();
						break;
					case 0x0400:
						 set_beep_on();
						break;
					default:
						break;
					}
		
		   	  //set_beep_on();
		   	  usleep(100000);
		   	  set_beep_off();
		   	  //usleep(100000);
		    }
		    rc_write_byte(REG_RC500_TX_CONTROL,0x58); //rc_power_off();
		    usleep(50000);
		     //puts("*****");
		     printf("try  %d \n", i++);	
	  }
}
