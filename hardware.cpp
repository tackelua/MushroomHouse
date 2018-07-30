#include "hardware.h"
#include "Sensor.h"

bool stt_pump_mix = OFF;
bool stt_pump_floor = OFF;
bool stt_fan_mix = OFF;
bool stt_fan_wind = OFF;
bool stt_light = OFF;
bool stt_led = ON;
bool stt_lcd_backlight = ON;

Button myBtn(BUTTON, true, true, 20);

extern void out(int pin, bool status);
void hardware_init() {
	pinMode(PUMP_BOTH, OUTPUT);
	pinMode(PUMP_MIX, OUTPUT);
	pinMode(PUMP_FLOOR, OUTPUT);
	pinMode(FAN_MIX, OUTPUT);
	pinMode(FAN_WIND, OUTPUT);
	pinMode(LIGHT, OUTPUT);
	pinMode(LED_STT, OUTPUT);

	//pinMode(SS_LIGHT, INPUT);

	out(PUMP_MIX, OFF);
	out(PUMP_FLOOR, OFF);
	out(FAN_MIX, OFF);
	out(FAN_WIND, OFF);
	out(LIGHT, OFF);
	out(LED_STT, ON);

	BH1750.INIT(BH1750_ADDRESS);
}
String getMacAddress() {
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	char baseMacChr[18] = { 0 };
	sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
	String id = String(baseMacChr);
	id = id.substring(id.length() - 5);
	id.toUpperCase();

	DEBUG.print("HID = ");
	DEBUG.println(id);
	DEBUG.println();
	return id;
}