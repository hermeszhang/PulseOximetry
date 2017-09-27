#ifndef PULSE_I2C
#define PULSE_I2C

#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdint.h>

void pulseOxSetup();
uint8_t pulseOxRead();

#endif
