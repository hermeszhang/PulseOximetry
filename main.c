#include <stdio.h>
#include <wiringPi.h>
#include "pulseOx.h"
#include "myi2clib.h"

#define DATA_ELEMENTS 32

int main(void)
{
	int rv = 0;
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

	while(rv != 42)
	{
		rv = pulseOxReadHeartRateData();

		if(rv == 42)
		printf("Completed.\n\n");
	}

	pulseOxReadHeartRate();

//	pulseOxPrint();

	pulseOxShutdown();

	return 0;
}
