/*****************************************************************************************

pulseOx.c

These funtions are used for communicating with the MAX30102 pulse oximeter
and heartrate sensor. They are built from the "myi2clib" library functions. To begin
using the MAX30102, call the pulseOxSetup() function to write to the registers some
basic values. Next, reset some values back to their initial states using
pulseOxCleanSlate(). Finally, you can begin reading using the pulseOxReadHeartRateData()
function. When you are finished using the device, shut it down with pulseOxShutdown().

*****************************************************************************************/

#include "pulseOx.h"

#define DEV_ID 57
#define SAMPLE 20
#define MAF 5

uint8_t sampleArray[SAMPLE*3];
uint32_t HRData[SAMPLE];		// Array for holding formatted HR data
//uint32_t HRDataExtended[2*SAMPLE];	// Array for holding
uint8_t heartRate[MAF];		// Array for holding averaged heart rate
uint32_t HRInput[MAF];		// Array for holding the averaged input samples

uint32_t highest = 0x80000;
uint32_t lowest  = 0x80000;
uint32_t prevSamp = 0;

/* int pulseOxSetup()
Run this function before attempting to read any data from the MAX30102.
Sets the correct configurations for communicating and reading. Returns
0 if device is found and confugured. Returns 1 otherwise. */
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
	pulseOxWrite(0x02, 0x80);

	// FIFO Configuration
	pulseOxWrite(0x08, 0x0C);

	// Mode Configuration
	pulseOxWrite(0x09, 0x02); //02h for red HR, 03h for IR PO

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

int pulseOxReadHeartRate()
{
	int rv, i, j;
	uint32_t avg;

	rv = pulseOxReadHeartRateData(HRData);

	// Returning -1 indicates that the data is not ready
	// otherwise return the value of the heart rate.
	if(rv == 0) return -1;

	for(i=0; i<SAMPLE; i++)
	{
		// First find the moving average of the input to smooth signal
		avg = 0;
		for(j=0; j<MAF; j++)
		{
			avg += HRData[i+SAMPLE-j];
		}

		// Determine if 
		if( avg > ((highest - lowest)/2) )
		{
			if( avg > highest )
			{
				highest = avg;
				smpCt = 0;
			}
		}
	}
}

/* int pulseOxReadHeartRateData()
This function is used to read data from the MAX30102. It first checks if enough
samples have been collected. Once enough have been stored in the device, a bulk
read is performed using pulseOxReadMulti(). The only parameter that needs to
be sent is an array of at least 32 uint8_t elements in length. The function will
only collect the amount of sample available. When the function reaches its end,
the calling source file will have the data stored in the array, properly
formatted. This uses a polling method where the register is read before every
attempt but an interrupt pin could also be used if available. Only when there
are at least 20 samples stored will it begin to read. The function returns 0
if no data was collected and a 1 otherwise. Use the data as soon as it it
returned or it will be deleted nect time the function is called. */
int pulseOxReadHeartRateData(uint32_t *formattedData)
{
	// Check if there are enough samples
	if(!(pulseOxRead(0x00) & 0x80)){ return 0;}

	// Begin reading data (is it 20 or 19??)
	pulseOxReadMulti(0x07, sampleArray, 60);

	// Organize Data
	int i = 0;
	int j = 0;

	if(!(sampleArray[i] & 0xFC)) i++;
	else if(!(sampleArray[i] & 0xFC)) i++;
	else if(!(sampleArray[i] & 0xFC)) i++;

	printf("i starts at %d \n", i);

	while(j<SAMPLE)
	{
		// Array is offset so algorithm can access past values
		formattedData[j+SAMPLE] = (sampleArray[i] << 16) | (sampleArray[i+1] << 8) | sampleArray[i+2];
		i+=3;
		j++;
	}

	// Indicate new data is ready
	return 1;
}



/* uint8_t pulseOxRead(uint8_t regAdd)
This is the general read function for the MAX31302. It informs the device
which register it wants to read from and then returns the 8-bit data.*/
uint8_t pulseOxRead(uint8_t regAdd)
{
	i2cWrite(0xAE, 1, 0);
	i2cWrite(regAdd, 0, 0);
	i2cWrite(0xAF, 1, 0);
	regAdd = i2cRead(0, 1);

	return regAdd;
}

/* void pulseOxReadMulti(uint8_t regAdd, uint8_t *dataArray, int size)
Simialr to pulseOxRead(), this function will read multiple bytes from a
single register. The amount of data to read is indicated by the 'size'
parameter. Data is stored in an array created by the calling source file. */
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

/* void pulseOxWrite(uint8_t regAdd, uint8_t value)
This function will write 8-bit data to the register specified. */
void pulseOxWrite(uint8_t regAdd, uint8_t value)
{
	i2cWrite(0xAE, 1, 0);
	i2cWrite(regAdd, 0, 0);
	i2cWrite(value, 0, 1);
}

/* void pulseOxCleanSlate()
Use this function to clear the values stored in the MAX31302's read, write,
and data registers. This should be called when switching between SpO2 and
HR modes. */
void pulseOxCleanSlate()
{
	uint8_t tempArray[32];
	pulseOxWrite(0x04, 0x00);
	pulseOxWrite(0x05, 0x00);
	pulseOxWrite(0x06, 0x00);
	pulseOxReadMulti(0x07, tempArray, 32);
}

void pulseOxShutdown()
{
	pulseOxWrite(0x09, 0x80);
}
