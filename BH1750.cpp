#include "BH1750.h"


bool BH1750Class::INIT(int address)
{
	Wire.begin(SDA, SCL);
	Wire.beginTransmission(address);
	Wire.write(0x10); // 1 [lux] aufloesung
	byte result = Wire.endTransmission();
	if (result == 0) {
		return true;
	}
	else {
		Serial.print("CAN'T INIT LIGHT SENSOR");
		return false;
	}
}


int BH1750Class::BH1750_Read(int address)
{
	byte buff[2];
	byte i = 0;
	Wire.beginTransmission(address);
	Wire.requestFrom(address, 2);
	//Serial.print("\n");
	while (Wire.available())
	{
		buff[i] = Wire.read();
		//Serial.print(buff[i], HEX);
		//Serial.print("-");
		i++;
	}
	byte result = Wire.endTransmission();
	if (result == 0) {
		uint16_t val;
		val = (uint16_t)(buff[0] << 8) | (uint16_t)(buff[1]);
		return int(val);
	}
	else {
		Serial.print("READ LIGHT ERROR");
		return -1;
	}
}

BH1750Class BH1750;