#include <stdio.h>
#include "pulseOx.h"
#include "myi2clib.h"

int main(void)
{
	uint8_t rv;

	printf("Pulse Oximetry Reader\n\n");

	pulseOxSetup();

	// Read the device ID
	rv = pulseOxRead(0xF8);
	if(rv == 0x15) printf("Device with ID 0x15 connected.\n\n");
 	else printf("Error communicating with device.\n");

/*	i2cSetup();

	rv = i2cWrite(0xFF, 0x00);
	printf("rv = %d \n", rv);

	rv = i2cWrite(0xFF, 0xFF);
	printf("rv = %d \n", rv);

	rv = i2cWrite(0xFF, 0xAE);
	printf("rv = %d \n", rv);

	rv = i2cWrite(0xFF, 0xFF);
	printf("rv = %d \n", rv);
*/

	return 0;
}
