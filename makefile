CC=gcc
CFLAGS= -I. -lwiringPi
DEPS= pulseOx.h 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

PulseOximetry: main.o pulseOx.o
	$(CC) -o PulseOximetry main.o pulseOx.o $(CFLAGS)
