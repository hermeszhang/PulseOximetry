#include <stdint.h>

void i2cSetup();
uint8_t i2cWrite(uint8_t, uint8_t, uint8_t);
uint8_t i2cRead(uint8_t, uint8_t);
void i2cStart();
void i2cStop();
