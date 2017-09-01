#include <stdio.h>
#include "pulseOx.h"

int main(void)
{
	printf("Pulse Oximetry Reader\n\n");

	pulseOxSetup();

	pulseOxRead();

	return 0;
}
