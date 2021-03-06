#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <DHT.h>
#include "TSL2561.h"
#include <WiFiClientSecure.h>
#include <ssl_client.h>
#include <ESP8266WebServer.h>
#include <SPIFFS.h>
#include <vfs_api.h>
#include <FSImpl.h>
#include <FS.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <Time.h>
#include <SHT1x.h>
#include <Button.h>
#include "Sensor.h"
#include "hardware.h" 
#include "mqtt_helper.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <ESP32httpUpdate.h> 
#include <WebServer-esp32/src/WebServer.h>
#include <DNSServer.h>
#include <WIFIMANAGER-ESP32/WiFiManager.h> 
//#include "esp_system.h"

#define __VERSION__  "3.1.25b2 testing"

String _firmwareVersion = __VERSION__ " " __DATE__ " " __TIME__;

String HubID;

void updateTimeStamp(unsigned long interval);
void control(int pin, bool status, bool update_to_server, bool isCommandFromApp);

HardwareSerial LCD_UART(2);


enum lcd_line_0 {
	SHOW_HUBID = 0,
	NOTI_CONNECTION_LOST,
	NOTI_WATER_EMPTY
};
bool flag_SmartConfig = false;
bool flag_error_wifi = false;
bool flag_water_empty = false;
bool flag_print_time = false;

bool flag_schedule_pump_floor = false;

int flag_lcd_line_0 = SHOW_HUBID;
bool ENABLE_SYSTEM_BY_CONTROL = false;
unsigned long t_ENABLE_SYSTEM;

void wait(unsigned long ms) {
	delay(ms);
}

void setup()
{
	wait(50);
	DEBUG.begin(115200);
	DEBUG.setTimeout(20);
	LCD_UART.begin(115200, SERIAL_8N1, 16, 17); //baud rate, 8 data bits - no parity - 2 stop bits, RX pin, TX pin
	LCD_UART.print("{\"cmd\":\"l0\",\"dt\":\" SMART MUSHROOM\"}");


	DEBUG.print(("\r\nFirmware Version: "));
	DEBUG.println(_firmwareVersion);

	HubID = getMacAddress();
	//HubID = "17C80";
	hardware_init();
	//lcd_init();
	load_library();

	wifi_init();
	out(LED_STT, OFF);
	DEBUG.println(("LED_STT OFF"));

	updateTimeStamp(0);
	mqtt_init();
	t_ENABLE_SYSTEM = millis();
	mqtt_publish("Mushroom/DEBUG/" + HubID, "SYSTEM READY");
}

void loop()
{
	wifi_loop();
	mqtt_loop();

	if (!ENABLE_SYSTEM_BY_CONTROL) {
		if (millis() - t_ENABLE_SYSTEM > 30000) {
			ENABLE_SYSTEM_BY_CONTROL = true;
		}
		wait(1);
		return;
	}

	led_loop();
	updateTimeStamp(3600000);
	serial_command_handle();
	button_handle();
	warming_alarm();

	update_sensor(update_sensor_interval);
	auto_control();

}

