// Sensor.h

#ifndef _SENSOR_h
#define _SENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#define BH1750_ADDRESS 	0x23

void sensor_init();

float readTemp1();
float readHumi1();
int readLight();
#endif

