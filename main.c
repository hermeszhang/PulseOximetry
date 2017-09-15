#include <stdio.h>
#include "pulseOx.h"
#include "myi2clib.h"

int main(void)
{
	printf("Pulse Oximetry Reader\n\n");

//	pulseOxSetup();

//	pulseOxRead();

	i2cSetup();
	uint8_t rv;
	rv = i2cWrite(0xFF, 0x00);
	printf("rv = %d \n", rv);

	rv = i2cWrite(0xFF, 0xFF);
	printf("rv = %d \n", rv);

	rv = i2cWrite(0xFF, 0xAE);
	printf("rv = %d \n", rv);

	rv = i2cWrite(0xFF, 0xFF);
	printf("rv = %d \n", rv);


	return 0;
}
