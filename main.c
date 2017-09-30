#include <stdio.h>
#include <wiringPi.h>
#include "pulseOx.h"
#include "myi2clib.h"

#define DATA_ELEMENTS 32

int main(void)
{
	uint8_t rv;
	uint8_t pulseOxData[DATA_ELEMENTS];

	printf("Pulse Oximetry Reader\n\n");

	// Initiate GPIO pins
	wiringPiSetup();

	// Setup Pulse Oximeter
	pulseOxSetup();

	// Read the device ID
	rv = pulseOxRead(0xFF);
	if(rv == 0x15) printf("Device with ID 0x15 connected.\n\n");
 	else printf("Error communicating with device.\n");

	pulseOxCleanSlate();
	delay(1000);

	int i, j;
	for(j=0; j<30; j++)
	{
		pulseOxReadMulti(0x07, pulseOxData, DATA_ELEMENTS);

		for(i=0; i<DATA_ELEMENTS; i++)	printf("%d ", pulseOxData[i]);

		printf("\n");
	}

	return 0;
}
