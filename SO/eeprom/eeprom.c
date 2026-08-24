#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h> 
#include <errno.h>
#include <fcntl.h> 

unsigned char EEPROM[8192];
static const char *eeprom = "/sys/bus/i2c/devices/2-0050/eeprom";

int main(int argc, char *argv[])
{
int		fdEEprom;
int i, addr = 0;
int bytes = 8192;
int ret, len;
const char *pEEFile;

	pEEFile = eeprom;
	if(argc == 2)
	{
		pEEFile = argv[2];
	}
	printf("open eeprom %s \n", pEEFile);
	fdEEprom = open(pEEFile, O_RDWR | O_SYNC);
	if( fdEEprom < 0)
	{
		printf("open failure\n");
		return 0xff;
	}
		
	len = ret = read(fdEEprom, EEPROM, sizeof(EEPROM));
	while(len != sizeof(EEPROM))
	{
		ret = read(fdEEprom, &EEPROM[len], sizeof(EEPROM) - len);
		len += ret;
		printf("read return %d len is %d\n", ret, len);
	}
	printf("Read EEPROM:");
	for(i = addr; i < addr + bytes; i++)
	{
		if((i % 16) == 0) printf("\n");
		printf("%02x", EEPROM[i]);
	}
	printf("\n");

	//if(argc > 2)
	{
		lseek(fdEEprom, 0, SEEK_SET);
		memset(EEPROM, 0x00, 8192);
		len = ret = write(fdEEprom, &EEPROM[0], 8192);
		while(len != sizeof(EEPROM))
		{
			ret = write(fdEEprom, &EEPROM[len], sizeof(EEPROM) - len);
			len += ret;
			printf("write return %d len is %d\n", ret, len);
		}
//		ret = read(fdEEprom, EEPROM, sizeof(EEPROM));
//		printf("WRITE EEPROM 0x00 Read Again\n");
//		for(i = addr; i < addr + bytes; i++)
//		{
//			printf("%02x", EEPROM[i]);
//			if((i % 16) == 0) printf("\n");
//		}
//		printf("\n");
		
		lseek(fdEEprom, 0, SEEK_SET);
		memset(EEPROM, 0x30, 8192);
		ret = write(fdEEprom, &EEPROM[0], 8192);
		
//		ret = read(fdEEprom, EEPROM, sizeof(EEPROM));
//		printf("WRITE EEPROM 0x30 Read Again\n");
//		for(i = addr; i < addr + bytes; i++)
//		{
//			printf("%02x", EEPROM[i]);
//			if((i % 16) == 0) printf("\n");
//		}
//		printf("\n");
	}
	close(fdEEprom);
	return 0;
}