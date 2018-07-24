// Sensor.h

#ifndef _SENSOR_h
#define _SENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SHT1x.h>

float readTemp1();
float readHumi1();
float readTemp2();
float readHumi2();
int readLight();
#endif

