#ifndef PULSE_I2C
#define PULSE_I2C

#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

void pulseOxSetup();
int pulseOxRead(int);

#endif
