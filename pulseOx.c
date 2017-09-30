#include "pulseOx.h"

#define DEV_ID 57

int pulseFH;

int pulseOxSetup()
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
		return 1;
	}

	// Reset Device
	pulseOxWrite(0x09, 0x40);

	// Disable Interrupts
	pulseOxWrite(0x02, 0x00);

	// FIFO Configuration
	pulseOxWrite(0x08, 0x00);

	// Mode Configuration
	pulseOxWrite(0x09, 0x03); //02h for red HR, 03h for IR PO

	// SpO2 Configuration
	pulseOxWrite(0x0A, 0x27);

	// LED Pulse Amplitude
	pulseOxWrite(0x0C, 0x23);
	pulseOxWrite(0x0D, 0x23);
	pulseOxWrite(0x10, 0xFF);

	// Multi-LED Mode Control Registers
//	pulseOxWrite(0x11, 0x00);

	return 0;
}


uint8_t pulseOxRead(uint8_t regAdd)
{
	i2cWrite(0xAE, 1, 0);
	i2cWrite(regAdd, 0, 0);
	i2cWrite(0xAF, 1, 0);
	regAdd = i2cRead(0, 1);

	return regAdd;
}

void pulseOxReadMulti(uint8_t regAdd, uint8_t *dataArray, int size)
{
	i2cWrite(0xAE, 1, 0);
	i2cWrite(regAdd, 0, 0);
	i2cWrite(0xAF, 1, 0);

	int element;
	for(element=0; element<(size-1); element++)
	{
		dataArray[element] = i2cRead(0, 0);
	}

	dataArray[size-1] = i2cRead(0, 1);
}

void pulseOxWrite(uint8_t regAdd, uint8_t value)
{
	i2cWrite(0xAE, 1, 0);
	i2cWrite(regAdd, 0, 0);
	i2cWrite(value, 0, 1);
}

void pulseOxCleanSlate()
{
	pulseOxWrite(0x04, 0x00);
	pulseOxWrite(0x05, 0x00);
	pulseOxWrite(0x06, 0x00);
}
