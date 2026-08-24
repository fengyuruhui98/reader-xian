
#ifndef _SPI_H_
#define _SPI_H_
//start of file
#include <linux/types.h>

//‘§∂®“Â-----------------------------------------------------------------------------

void spi_set_speed(uint32_t sp);
void spi_init(void);
void spi_close(void);
int spidev_reg_read(unsigned char reg, unsigned char *value);
int spidev_reg_write(unsigned char reg, unsigned char value);
int spidev_fifo_read(unsigned char reg, unsigned char len, unsigned char *buf);
int spidev_fifo_write(unsigned char reg,  unsigned char len, const unsigned char *buf);

//end of file
#endif


