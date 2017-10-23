#include <stdio.h>
#include <wiringPi.h>
#include "pulseOx.h"
#include "myi2clib.h"

#define DATA_ELEMENTS 32

int main(void)
{
	uint8_t rv;
	uint32_t pulseOxData[DATA_ELEMENTS];

	printf("Pulse Oximetry Reader\n\n");

	// Initiate GPIO pins
	wiringPiSetup();

	// Setup Pulse Oximeter
	pulseOxSetup();

	// Read the device ID (Returns 0x15)
	rv = pulseOxRead(0xFF);
	if(rv == 0x15) printf("Device with ID 0x15 connected.\n\n");
 	else printf("Error communicating with device.\n");

	pulseOxCleanSlate();
	delay(1000);

	int i, j;
	for(j=0; j<1000; j++)
	{
		rv = pulseOxReadHeartRateData(pulseOxData);

		if(rv == 1)
		{
			for(i=0; i<20; i++)	printf("%x \n", pulseOxData[i]);
		}
	}

	pulseOxShutdown();

	return 0;
}
