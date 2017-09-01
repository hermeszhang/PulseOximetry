#include "pulseOx.h"

#define DEV_ID 57

int pulseFH;

void pulseOxSetup()
{
	int temp = -2;

	// Create filehandle
	pulseFH = wiringPiI2CSetup(DEV_ID);
	printf("Filehandle created with address %x \n\n", pulseFH);

	// Part ID
	temp = wiringPiI2CReadReg8(pulseFH, 0xFF);
	printf("Address should be 0x15. Returned address: %x", temp);

	// Disable Interrupts
	wiringPiI2CWriteReg8(pulseFH, 0x02, 0x00);

	// LED Pulse Amplitude
	wiringPiI2CWriteReg8(pulseFH, 0x0C, 0x3F);
	wiringPiI2CWriteReg8(pulseFH, 0x0D, 0x3F);

	// Mode Configuration
	temp = wiringPiI2CReadReg8(pulseFH, 0x09);
	printf("New Value for Mode Config: %x \n", temp);

}

int pulseOxRead()
{
	int i, value;

	for(i=0; i<20; i++)
	{
		value = wiringPiI2CReadReg8(pulseFH, 0x06);
		printf("%d \n", value);

		delay(1000);
	}

	return value;
}
