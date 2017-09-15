CC=gcc
CFLAGS= -I. -lwiringPi
DEPS= pulseOx.h myi2clib.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

PulseOximetry: main.o pulseOx.o myi2clib.o
	$(CC) -o PulseOximetry main.o pulseOx.o myi2clib.o $(CFLAGS)
