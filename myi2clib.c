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

/* I2CWRITE *
----------------------------------------------------------------------------------------------
i2cWrite takes two parameters, the data to be sent and a flag that determines if the transfer
ends with a stop bit. Set the 'stopTrue' flag to end the transfer with a stop, or clear it to
allow further operations.
--------------------------------------------------------------------------------------------*/
uint8_t i2cWrite(uint8_t toSend, uint8_t startTrue, uint8_t stopTrue)
{
	uint8_t bit, temp;
	uint8_t mask = 0x80;

	// SDA should be an output
	// Both pins should start off high

	// Start sequence if startTrue set
	if(startTrue)
	{
		// START: SCL is High, SDA goes high-to-low
		digitalWrite(SDA, HIGH);
		digitalWrite(SCL, HIGH);
		delay(WAIT);
		digitalWrite(SDA, LOW);
		delay(WAIT);
		digitalWrite(SCL, LOW);
	}

	// Send data
	for(bit=0; bit<8; bit++)
	{
		temp = toSend & (mask >> bit);

		if(temp != 0) digitalWrite(SDA, HIGH);
		else digitalWrite(SDA, LOW);

		delay(WAIT);
		digitalWrite(SCL, HIGH);
		delay(WAIT);
		digitalWrite(SCL, LOW);
	}

	//Check Acknowledge bit
	pinMode(SDA, INPUT);
	delay(WAIT);
	digitalWrite(SCL, HIGH);
	temp = digitalRead(SDA);
	digitalWrite(SCL, LOW);

	// Revert SDA back to output
	pinMode(SDA, OUTPUT);
	delay(WAIT);

	if(stopTrue)
	{
		// STOP
		digitalWrite(SDA, LOW);
		delay(WAIT);
		digitalWrite(SCL, HIGH);
		delay(WAIT);
		digitalWrite(SDA, HIGH);
		delay(WAIT);

		// Reset clock
		digitalWrite(SCL, LOW);
	}

	return temp;

}

uint8_t i2cRead(uint8_t startTrue, uint8_t stopTrue)
{
	uint8_t bit, temp;
	uint8_t mask = 0x80;
	uint8_t recData = 0;

	if(startTrue)
	{
		// START: SCL is High, SDA goes high-to-low
		digitalWrite(SDA, HIGH);
		digitalWrite(SCL, HIGH);
		delay(WAIT);
		digitalWrite(SDA, LOW);
		delay(WAIT);
		digitalWrite(SCL, LOW);
		delay(WAIT);
	}

	// Set SDA to input to read
	pinMode(SDA, INPUT);
	delay(WAIT);

	for(bit=0; bit<8; bit++)
	{
		digitalWrite(SCL, HIGH);

		temp = digitalRead(SDA);

		if(temp != 0) recData |= (mask >> bit);

		digitalWrite(SCL, LOW);
		delay(WAIT);
	}

	// Set SDA to output for ACK or NACK
	pinMode(SDA, OUTPUT);

	// Either send ACK or NACK and stop based on stopTrue
	if(stopTrue)
	{
		// Send NACK bit
		digitalWrite(SDA, HIGH);
		delay(WAIT);
		digitalWrite(SCL, HIGH);
		delay(WAIT);
		digitalWrite(SCL, LOW);
		delay(WAIT);

		// Stop transfer
		digitalWrite(SCL, HIGH);
		delay(WAIT);
		digitalWrite(SDA, HIGH);
	}
	else
	{
		// Send ACK bit
		digitalWrite(SDA, LOW);
		delay(WAIT);
		digitalWrite(SCL, HIGH);
		delay(WAIT);
		digitalWrite(SCL, LOW);
		delay(WAIT);

		// Prep pins for future actions
		digitalWrite(SDA, HIGH);
		delay(WAIT);
		digitalWrite(SCL, HIGH);
	}

	return recData;
}
