#ifndef PULSE_I2C
#define PULSE_I2C

#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdint.h>

int pulseOxSetup();
uint8_t pulseOxRead(uint8_t);
//int pulseOxReadHeartRateData(uint32_t *arrayPtr);
int pulseOxReadHeartRateData();
void pulseOxWrite(uint8_t, uint8_t);
void pulseOxReadMulti(uint8_t, uint8_t *dataArray, int);
void pulseOxCleanSlate();
void pulseOxShutdown();
void pulseOxPrint();

#endif
