/*****************************************************************************************

pulseOx.c

These funtions are used for communicating with the MAX30102 pulse oximeter
and heartrate sensor. They are built from the "myi2clib" library functions. To begin
using the MAX30102, call the pulseOxSetup() function to write to the registers some
basic values. Next, reset some values back to their initial states using
pulseOxCleanSlate(). Finally, you can begin reading using the pulseOxReadHeartRateData()
function. When you are finished using the device, shut it down with pulseOxShutdown().

*****************************************************************************************/

#include <stdio.h>
#include "pulseOx.h"

#define DEV_ID 57
#define SAMPLE 1
#define MAF 10   // moving average filter variables
#define Fs 100  // Sampling frequency
#define CMPR_AMT 10
#define TOTAL_SIZE 1000

// TO FIX: File to store data
FILE * pFile;
unsigned int colCount, rowCount;

uint8_t sampleArray[SAMPLE*3];  // Holds raw recorded data
uint32_t HRData[SAMPLE];		// Array for holding formatted HR data
uint32_t avgArray[SAMPLE*2];      // Array for averages
uint32_t HRInput[MAF];		    // Array for holding the averaged input samples
double heartRate;		// Array for holding averaged heart rate

uint32_t timeKeep[TOTAL_SIZE];
uint32_t formattedData[TOTAL_SIZE];
uint32_t records[300];
int globalCounter = 0;
unsigned int currentElement = 0;

uint8_t hrFlag = 0;
uint16_t heartRateFinal = 70;
uint16_t smpMax = 20;
uint16_t smpCt = 0;
uint16_t tmrCt = 0;
uint16_t tmrMax = 500;
uint32_t highest = 0x80000;

uint32_t lowest = 0x7FFFFF;

/* int pulseOxSetup()
Run this function before attempting to read any data from the MAX30102.
Sets the correct configurations for communicating and reading. Returns
0 if device is found and configured. Returns 1 otherwise. */
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
	pulseOxWrite(0x02, 0x40);

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

        heartRate = heartRateFinal;

	// TO FIX: Start array counters at 0
	colCount = 0;
	rowCount = 0;

	return 0;
}

int pulseOxReadHeartRate()
{
	int index, index2;
	int timeIndex = 0;
	uint32_t markTime[100];
	int tempTime = 0;


	// Calculate the averaged values
	int totalAvgValues = TOTAL_SIZE - MAF;
	double tempArray[totalAvgValues];

	for(index=0; index < totalAvgValues; index++)
	{
		tempArray[index] = 0;

		// Starts at 1 because sometimes the first sample
		// is extremely large and not representative of the
		// actual heart rate.
		for(index2 = 0; index2 < MAF; index2++)
		{
			tempArray[index] += (double)formattedData[1 + index + index2];
		}

		tempArray[index] /= MAF;
	}

//	printf("Values Averaged. Three random Values: \n %f, %f, %f \n", tempArray[10], tempArray[100], tempArray[500]);

	// Calculate the derivative of the signal
	int totalDerValues = totalAvgValues - 2;
	int timeIndexStart = (MAF + 3) / 2;
	double numerator, denominator;

	for(index = 0; index < totalDerValues; index++)
	{
		numerator = tempArray[index + 2] - tempArray[index];
		denominator = (double)timeKeep[timeIndexStart + index] - timeKeep[timeIndexStart + index + 2];

//		printf("N: %f, D: %f, t2: %d, t1: %d\n", numerator, denominator, timeKeep[timeIndexStart + index], timeKeep[timeIndexStart + index + 2]);

		tempArray[index] = numerator / denominator;
	}

//	printf("Derivative Calculated. Three Random Values:\n %f, %f, %f \n", tempArray[10], tempArray[100], tempArray[500]);

	// Smooth derivative signal
	int totalDerSmooth = totalDerValues - MAF;
	double der;

	for(index = 0; index < (totalDerSmooth - MAF); index++)
	{
		der = 0.0;

		for(index2 = 0; index2 < MAF; index2++)
		{
			der += tempArray[index + index2];
		}

		tempArray[index] = der / MAF;
	}

//	printf("Derivative Smoothed. Three Random Values: \n %f, %f, %f \n", tempArray[10], tempArray[100], tempArray[500]);

	// Determine largest negative value
	double max = -1.0;

	for(index = 0; index < totalDerSmooth; index++)
	{
		if(tempArray[index] < max) max = tempArray[index];
	}

//	printf("Largest Value Found. Value = %f \n", max);

	// Create threshold
	double thresh = max * 0.4;

//	printf("Threshold Calculated. Threshold = %f \n", thresh);

	// Find locations of zero crossing
	int flag = 0;
	int peaks = 0;
	timeIndexStart += ((MAF-1)/2);

	for(index = 0; index < totalDerSmooth; index++)
	{
		// Set flag if threshold is hit
		if((tempArray[index] <= thresh)&&(flag == 0))
		{
			flag = 1;

			// Keep checking previous samples until the
			// derivative is 0
			index2 = index;

			while(tempArray[index2] < 0.0)
			{
				index2--;
			}

//			printf("Value Above 0: %f \n", tempArray[index2]);

			// Store time stamp for later use
			markTime[peaks] = timeKeep[timeIndexStart + index2];
			peaks++;

			// Skip several samples to avoid glitches
			index += 6;
		}

		// Reset flag only when the signal passes zero again
		if(tempArray[index] >= 0.0) flag = 0;

	}

//	printf("Negative Peaks Found. \n");

	// Calculate heartrate from the difference in times
	heartRate = 0.0;
	int skips = 1;

	for(index = 1; index < peaks; index++)
	{
		if((markTime[index] - markTime[index - 1]) > 1500)
		{
			skips++;
		}
		else
		{
			heartRate += (double)(markTime[index] - markTime[index - 1]);
		}
	}

//	printf("Heart Rate Calculated! \n\n");

	// Average time in mili seconds
	heartRate /= (peaks-skips);

	// HR in seconds
	heartRate = 1000.0 / heartRate;

	// HR in bpm
	heartRate *= 60.0;

	printf("Heart Rate: %f \n", heartRate);

	return 0;
}

/*
int pulseOxReadHeartRate()
{
	int rv, i, j;
	int largestAvgSmpFl = 1;
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

        // Write new averaged value to array
		avgArray[i+SAMPLE] = avg;

		// Determine if avg is greater than the past
		for(j=0; j<CMPR_AMT; j++)
        {
            if(avg < avgArray[i+SAMPLE-CMPR_AMT+j])
            {
                largestAvgSmpFl = 0;
                break;
            }
        }

        // Flag set if calculated avg was more than the past CMPR_AMT values
        if(largestAvgSmpFl == 1)
        {
            // Avg is at least 75% of the highest value
            if(avg > (0.75*highest))
            {
                hrFlag = 1;

                // Determine if this is the new highest value
                if(avg > highest)
                {
                    highest = (avg + highest) / 2;
                }

                // Reset smpCt to 0 because new high value was found
                smpCt = 0;
            }
            else
            {
                hrFlag = 0;
            }
        }

        // Increment counters
        smpCt++;
        tmrCt++;

        // Check if tmrCt has been going on too long
        if(tmrCt > tmrMax)
        {
            highest = 0x80000;
            tmrCt = 0;
            smpCt = 0;
        }

        // Check if smpCt has reached max
        if((smpCt > smpMax)&&(hrFlag == 1))
        {
            // Shift values up
            for(j=0; j<MAF-1; j++)
            {
                heartRate[j+1] = heartRate[j];
            }

            // Calculate heart rate
            heartRate[0] = (60.0 * Fs) / (tmrCt - smpCt + 1);

            // Reset smpCt
            smpCt = 0;

            // Calculate heart rate
            heartRateFinal = heartRate[0];

            for(j=1; j<MAF; j++)
            {
                heartRateFinal+=heartRate[j];
            }

            heartRateFinal/=MAF;
        }

	}

	// Shift avg values down
	for(j=0; j<SAMPLE; j++)
	{
		avgArray[j] = avgArray[SAMPLE+j];
	}

	return heartRateFinal;
}
*/


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
int pulseOxReadHeartRateData()
{
	// Check if there are enough samples
//	if(!(pulseOxRead(0x00) & 0x80)){ return 0; }

	// Check if new data is available
	if((pulseOxRead(0x00) & 0x40) == 0){ return 0; }

	// Begin reading data; 3 Bytes per sample
	pulseOxReadMulti(0x07, sampleArray, 3);

	// Record time
	timeKeep[colCount] = millis();

	// Clear data in register
	pulseOxWrite(0x04, 0x00);
	pulseOxWrite(0x06, 0x00);

	// Format data
	formattedData[colCount] = (sampleArray[0] << 16) | (sampleArray[1] << 8)| sampleArray[2];

	colCount++;
	if(colCount >= TOTAL_SIZE)
	{
		pulseOxPrint();

		return 42;
	}
/*
	// Organize Data
	int i = 0;
	int j = 0;

	if(!(sampleArray[i] & 0xFC)) i++;
	else if(!(sampleArray[i] & 0xFC)) i++;
	else if(!(sampleArray[i] & 0xFC)) i++;

	while(j<SAMPLE)
	{
		// Array is offset so algorithm can access past values
		formattedData[currentElement] = (sampleArray[0] << 16) | (sampleArray[1] << 8) | sampleArray[2];
		i+=3;
		j++;

		if(formattedData[j-1+SAMPLE] < lowest) lowest = formattedData[j-1+SAMPLE];
	}

	if(globalCounter >= 300) return 1;

	for(j=0;j<SAMPLE;j++)
	{
		records[globalCounter + j] = formattedData[j+SAMPLE];
	}
	globalCounter += SAMPLE;
*/
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

/*Save data to a CSV file to graph the points later*/
void pulseOxPrint()
{
	int col, row;

	//const char *fileName = "Heart_Rate_Data.csv";
	pFile = fopen("Heart_Rate_Data.csv", "w");

	for(col=0; col<TOTAL_SIZE; col++)
	{
		fprintf(pFile, "%u,", formattedData[col]); 
		//fwrite(&formattedData[col], sizeof(uint32_t), 1, pFile);
		//fputs(", ", pFile);
	}

	fprintf(pFile, "\n");

	for(col=0; col<TOTAL_SIZE; col++)
	{
		fprintf(pFile, "%u,", timeKeep[col]); 
		//fwrite(&timeKeep[col], sizeof(uint8_t), 1, pFile);
		//fputs(", ", pFile);
	}

	fprintf(pFile, "\n");

	fclose(pFile);
}
