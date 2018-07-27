// 
// 
// 
#include "Sensor.h"
#include <DHT.h>
#include <SHT1x.h>
#include "hardware.h"

static SHT1x sht1(SHT1_DAT, SHT1_DAT);
static SHT1x sht2(SHT2_DAT, SHT2_DAT);


float readTemp1() {
	ulong t = millis();
	float temp = sht1.readTemperatureC(); //change waitForResultSHT loop from 100 times to 10 times
	temp = temp < 0.01f ? -1.0f : temp;
	DEBUG.print(("Read Temp 1: "));
	DEBUG.print(String(temp, 2));
	DEBUG.print(" in ");
	DEBUG.print(String(millis() - t));
	DEBUG.println(("ms"));
	return (temp < 0.01f ? -1.0f : temp);

}
float readHumi1() {
	float h = sht1.readHumidity();
	DEBUG.print(("Read Humi 1: "));
	DEBUG.print(String(h, 2));
	DEBUG.print(" in ");
	DEBUG.print(String(millis() - h));
	DEBUG.println(("ms"));
	return (h < 0.01f ? -1.0f : h);
}

float readTemp2() {
	ulong t = millis();
	float temp = sht2.readTemperatureC(); //change waitForResultSHT loop from 100 times to 10 times
	temp = temp < 0.01f ? -1.0f : temp;
	DEBUG.print(("Read Temp 2: "));
	DEBUG.print(String(temp, 2));
	DEBUG.print(" in ");
	DEBUG.print(String(millis() - t));
	DEBUG.println(("ms"));
	return (temp < 0.01f ? -1.0f : temp);

}
float readHumi2() {
	float h = sht2.readHumidity();
	DEBUG.print(("Read Humi 2: "));
	DEBUG.print(String(h, 2));
	DEBUG.print(" in ");
	DEBUG.print(String(millis() - h));
	DEBUG.println(("ms"));
	return (h < 0.01f ? -1.0f : h);
}

int readLight() {
	return BH1750.BH1750_Read(BH1750_ADDRESS);
}