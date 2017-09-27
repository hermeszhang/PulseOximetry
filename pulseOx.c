#include "pulseOx.h"

#define DEV_ID 57

int pulseFH;

void pulseOxSetup()
{
	int temp = -2;

	// Setup I2C pins
	i2cSetup();

	// Part ID: Register = 0xFF
	temp = pulseOxRead(0xFF);
	if(temp == 0x15)
	{
		printf("Pulse Oximeter Connected...\n");
	}
	else
	{
		printf("Pulse Oximeter NOT Connected. - Skipping Setup...\n");
		return;
	}

	// Disable Interrupts
	pulseOxWrite(0x02,0x0F);

	// LED Pulse Amplitude
	pulseOxWrite(0x0C, 0x3F);
	pulseOxWrite(0x0D, 0x3F);
}


uint8_t pulseOxRead(uint8_t regAdd)
{
	i2cWrite(0xAE, 1, 0);
	i2cWrite(regAdd, 0, 0);
	i2cWrite(0xAF, 1, 0);
	regAdd = i2cRead(0, 1);

	return regAdd;
}


void pulseOxWrite(uint8_t regAdd, uint8_t value)
{
	i2cWrite(0xAE, 1, 0);
	i2cWrite(regAdd, 0, 0);
	i2cWrite(value, 0, 1);
}
