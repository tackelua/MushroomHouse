
#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <DHT.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESP8266WebServer.h>
#include <SPIFFS.h>
#include <vfs_api.h>
#include <FSImpl.h>
#include <FS.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <ssl_client.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <TimeLib.h>
#include <Time.h>
#include <SHT1x.h>
#include <Button.h>
#include "Sensor.h"
#include "hardware.h"
#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
#include "mqtt_helper.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32httpUpdate.h>
#include "LiquidCrystal_I2C_m.h"
#include <Ticker.h>
#include <WiFiManager.h>
#include <ThingSpeak.h>

#define __VERSION__  "3.1.14"

String _firmwareVersion = __VERSION__ " " __DATE__ " " __TIME__;

String HubID;

void updateTimeStamp(unsigned long interval);
bool control(int pin, bool status, bool update_to_server, bool isCommandFromApp);


enum lcd_line_0 {
	SHOW_HUBID = 0,
	NOTI_CONNECTION_LOST,
	NOTI_WATER_EMPTY
};
bool flag_SmartConfig = false;
bool flag_error_wifi = false;
bool flag_water_empty = false;
bool flag_print_time = false;
int flag_lcd_line_0 = SHOW_HUBID;


void setup()
{
	delay(50);
	DEBUG.begin(115200);
	DEBUG.setTimeout(20);

	DEBUG.print(("\r\nFirmware Version: "));
	DEBUG.println(_firmwareVersion);

	HubID = getMacAddress();
	hardware_init();
	lcd_init();

	//pinMode(LED_BUILTIN, OUTPUT);
	//while (true) {
	//	digitalWrite(RELAY1, ON);
	//	digitalWrite(RELAY2, ON);
	//	digitalWrite(RELAY3, ON);
	//	digitalWrite(RELAY4, ON);
	//	digitalWrite(RELAY5, ON);
	//	digitalWrite(RELAY6, ON);
	//	digitalWrite(RELAY7, ON);
	//	digitalWrite(LED_BUILTIN, ON);
	//	delay(1500);
	//
	//	digitalWrite(RELAY1, OFF);
	//	digitalWrite(RELAY2, OFF);
	//	digitalWrite(RELAY3, OFF);
	//	digitalWrite(RELAY4, OFF);
	//	digitalWrite(RELAY5, OFF);
	//	digitalWrite(RELAY6, OFF);
	//	digitalWrite(RELAY7, OFF);
	//	digitalWrite(LED_BUILTIN, OFF);
	//	delay(1000);
	//}

	wifi_init();
	out(LED_STT, OFF);
	DEBUG.println(("LED_STT OFF"));

	updateTimeStamp(0);
	mqtt_init();
	ThingSpeak_init();
}

void loop()
{
	wifi_loop();
	led_loop();
	updateTimeStamp(3600000);
	mqtt_loop();
	serial_command_handle();
	button_handle();
	warming_alarm();
	update_sensor(10000);
	auto_control();
	lcd_repair();
}

