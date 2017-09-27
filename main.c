#include <stdio.h>
#include <wiringPi.h>
#include "pulseOx.h"
#include "myi2clib.h"

int main(void)
{
	uint8_t rv;

//	printf("Pulse Oximetry Reader\n\n");

//	pulseOxSetup();

	// LED TEST
	pinMode(0, OUTPUT);
	pinMode(2, OUTPUT);

	digitalWrite(0, 1);
	digitalWrite(2, 0);
	delay(1000);
	digitalWrite(0, 0);
	digitalWrite(2, 1);
	delay(1000);

	// Read the device ID
	rv = pulseOxRead();
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
