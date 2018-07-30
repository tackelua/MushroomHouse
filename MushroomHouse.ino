
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
#include <LiquidCrystal_I2C.h>
#include <Ticker.h>
#include <WiFiManager.h>
#include <ThingSpeak.h>

#define __VERSION__  "3.1.9"

String _firmwareVersion = __VERSION__ " " __DATE__ " " __TIME__;

String HubID;

void updateTimeStamp(unsigned long interval);
bool control(int pin, bool status, bool update_to_server, bool isCommandFromApp);


bool flag_SmartConfig = false;
bool flag_error_wifi = false;
bool flag_water_empty = false;


void setup()
{
	delay(50);
	Serial.begin(115200);
	Serial.setTimeout(20);

	pinMode(RELAY5, OUTPUT);
	pinMode(RELAY7, OUTPUT);
	pinMode(RELAY8, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	while (true)
	{
		DEBUG.println("HIGH");
		digitalWrite(RELAY5, HIGH);
		digitalWrite(RELAY7, HIGH);
		digitalWrite(RELAY8, HIGH);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(2000);

		DEBUG.println("LOW");
		digitalWrite(RELAY5, LOW);
		digitalWrite(RELAY7, LOW);
		digitalWrite(RELAY8, LOW);
		digitalWrite(LED_BUILTIN, LOW);
		delay(2000);
	}

	DEBUG.print(("\r\nFirmware Version: "));
	DEBUG.println(_firmwareVersion);

	HubID = getMacAddress();
	lcd_init();
	hardware_init();

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
	update_sensor(10000);
	auto_control();
}

