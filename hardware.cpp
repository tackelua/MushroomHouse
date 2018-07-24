#include "hardware.h"

bool stt_pump_mix = OFF;
bool stt_pump_floor = OFF;
bool stt_fan_mix = OFF;
bool stt_fan_wind = OFF;
bool stt_light = OFF;
bool stt_led = HIGH;

extern void out(int pin, bool status);
void hardware_init() {
	pinMode(PUMP_MIX, OUTPUT);
	pinMode(PUMP_FLOOR, OUTPUT);
	pinMode(FAN_MIX, OUTPUT);
	pinMode(FAN_WIND, OUTPUT);
	pinMode(LIGHT, OUTPUT);
	pinMode(LED_STT, OUTPUT);

	pinMode(SS_LIGHT, INPUT);

	out(PUMP_MIX, OFF);
	out(PUMP_FLOOR, OFF);
	out(FAN_MIX, OFF);
	out(FAN_WIND, OFF);
	out(LIGHT, OFF);
	out(LED_STT, ON);
}