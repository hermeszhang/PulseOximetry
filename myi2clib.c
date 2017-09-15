#include <wiringPi.h>
//#include <stdint.h>
#include <stdio.h>
#include "myi2clib.h"

#define SDA 2
#define SCL 0
#define WAIT 1		// 1 millisecond

void i2cSetup()
{
	// Set the clock pin as an output
	pinMode(SCL, OUTPUT);
	digitalWrite(SCL , HIGH);
	pinMode(SDA, OUTPUT);
	digitalWrite(SDA, HIGH);

	// START
	digitalWrite(SDA, LOW);
	digitalWrite(SCL, LOW);

	delay(WAIT);

	digitalWrite(SCL, HIGH);
	digitalWrite(SDA, HIGH);

	delay(WAIT);
}

uint8_t i2cWrite(uint8_t toSend, uint8_t devAdd)
{
	uint8_t bit, temp;
	uint8_t mask = 0x80;

	// Device ID + Write
	devAdd &= 0xFE;

	// Set SDA as output
	pinMode(SDA, OUTPUT);

	// Start both pins off high
	digitalWrite(SDA, HIGH);
	digitalWrite(SCL, HIGH);

	// Wait appropriate time
	delay(WAIT);

	// START: SCL is High, SDA goes high-to-low
	digitalWrite(SDA, LOW);
	delay(WAIT);
	digitalWrite(SCL, LOW);

	for(bit=0; bit<8; bit++)
	{
		temp = devAdd & (mask >> bit);

		if(temp != 0) digitalWrite(SDA, HIGH);
		else digitalWrite(SDA, LOW);

		delay(WAIT);
		digitalWrite(SCL, HIGH);
		delay(WAIT);
		digitalWrite(SCL, LOW);
	}

	//Check Acknowledge bit
	//pinMode(SDA, INPUT);
	delay(WAIT);
	digitalWrite(SCL, HIGH);
	temp = digitalRead(SDA);
	digitalWrite(SCL, LOW);

	// REPEATED START

	// Send Data

	// Check Acknoledge bit

	// STOP
	digitalWrite(SCL, HIGH);
	pinMode(SDA, OUTPUT);
	delay(WAIT);
	digitalWrite(SDA, HIGH);

	return temp;

}

uint8_t i2cRead()
{

}
