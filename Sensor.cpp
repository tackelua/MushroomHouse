// 
// 
// 
#include "hardware.h"
#include "Sensor.h"
#include <Wire.h>
#include <SHT1x.h>
#include "TSL2561.h"
//#include <DHT.h>
//#include "BH1750.h"

static SHT1x sht1(SHT1_DAT, SHT1_CLK);
static TSL2561 tsl(TSL2561_ADDR_FLOAT);

void sensor_init() {
	//BH1750.INIT(BH1750_ADDRESS);

	if (tsl.begin()) {
		DEBUG.println("Found TSL2561 sensor");
	}
	else {
		DEBUG.println("No TSL2561 sensor?");
	}

	//tsl.setGain(TSL2561_GAIN_0X);						// set no gain (for bright situtations)
	tsl.setGain(TSL2561_GAIN_16X);						// set 16x gain (for dim situations)

	// Changing the integration time gives you a longer time over which to sense light
	// longer timelines are slower, but are good in very low light situtations!
	//tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);		// shortest integration time (bright light)
	tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);		// medium integration time (medium light)
	//tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);		// longest integration time (dim light)
}
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
	ulong t = millis();
	float h = sht1.readHumidity();
	DEBUG.print(("Read Humi 1: "));
	DEBUG.print(String(h, 2));
	DEBUG.print(" in ");
	DEBUG.print(String(millis() - t));
	DEBUG.println(("ms"));
	return (h < 0.01f ? -1.0f : h);
}

//use BH1750
//int readLight() {
//	return BH1750.BH1750_Read(BH1750_ADDRESS);
//}

int readLight() {

	uint32_t lum = tsl.getFullLuminosity();
	uint16_t ir, full;
	ir = lum >> 16;
	full = lum & 0xFFFF;

	DEBUG.print("Visible: "); DEBUG.print(full - ir);   DEBUG.print("\t");

	uint32_t lux = tsl.calculateLux(full, ir);
	DEBUG.print("Lux: "); DEBUG.println(lux);
	if (lux == 703) {
		sensor_init();
		delay(10);
		uint32_t lum = tsl.getFullLuminosity();
		uint16_t ir, full;
		ir = lum >> 16;
		full = lum & 0xFFFF;

		DEBUG.print("Visible: "); DEBUG.print(full - ir);   DEBUG.print("\t");

		uint32_t llux = tsl.calculateLux(full, ir);
		DEBUG.print("Lux: "); DEBUG.println(llux);

		if (llux == 703) {
			return (-1);
		}
		return llux;
	}
	return lux;
}