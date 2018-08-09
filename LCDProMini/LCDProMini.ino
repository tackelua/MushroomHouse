//#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <TimeLib.h>
#include <Time.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

SoftwareSerial UART(D6, D7);
LiquidCrystal_I2C lcd(0x3f, 20, 04);
bool flag_time_set = false;
bool flag_show_start_ss = false;

void setup() {
	//WiFi.mode(WIFI_OFF);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	//delay(500);
	//digitalWrite(LED_BUILTIN, LOW);
	//delay(500);
	//digitalWrite(LED_BUILTIN, HIGH);
	//delay(500);
	//digitalWrite(LED_BUILTIN, LOW);
	Serial.begin(115200);
	Serial.setTimeout(200);
	Serial.println("ProMicro v1");
	UART.begin(115200);
	UART.setTimeout(200);
	lcd_init();
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
	if (UART.available() || Serial.available()) {
		String data;
		if (UART.available()) {
			data = UART.readStringUntil('\n');
			Serial.println("UART>");
			Serial.println(data);
		}
		if (Serial.available()) {
			data = Serial.readStringUntil('\n');
			Serial.println("Serial>");
			Serial.println(data);
		}

		DynamicJsonBuffer cmd(500);
		JsonObject& jsCmd = cmd.parseObject(data);
		if (jsCmd.success()) {
			digitalWrite(LED_BUILTIN, HIGH);
			String cmd = jsCmd["cmd"].asString();
			if (cmd == "ss") {
				if (!flag_show_start_ss) {
					flag_show_start_ss = true;
					lcd.setCursor(0, 2);
					lcd.print("TEMP    HUMI   LIGHT");
				}
				int temp = jsCmd["TEMP"].as<int>();
				int humi = jsCmd["HUMI"].as<int>();
				int light = jsCmd["LIGHT"].as<int>();
				lcd_print_sensor(temp, humi, light);
			}
			else if (cmd == "ts") {
				flag_time_set = true;
				long t = jsCmd["ts"].as<long>();
				setTime(t);
				delay(1);
				adjustTime(7 * SECS_PER_HOUR);
			}
			else if (cmd == "l0") {
				String data = jsCmd["dt"].asString();
				lcd.setCursor(0, 0);
				lcd.print(data);
			}
			else if (cmd == "uf") {
				lcd.setCursor(0, 0);
				lcd.print(" FIRMWARE  UPDATING ");
			}
			digitalWrite(LED_BUILTIN, LOW);
		}
	}

	if (flag_time_set) {
		static unsigned long t = millis() - 1000 - 1;
		if (millis() - t > 1000) {
			t = millis();
			lcd.setCursor(0, 1);
			String timeS = getTimeStr();
			lcd.print(timeS);
			UART.println(timeS);
			Serial.println(timeS);
		}
	}
}

void lcd_init() {
	lcd.begin();
	lcd.backlight();

	lcd.setCursor(0, 0);
	lcd.print("  SMART  MUSHROOM");
	lcd.setCursor(0, 1);
	lcd.print("   W E L C O M E");
}
void lcd_print_sensor(int temp, int humi, int light) {
	//lcd_generate_frame(temp, humi, light);
	lcd.setCursor(1, 3);
	lcd.print(temp); lcd.print("   ");
	lcd.setCursor(9, 3);
	lcd.print(humi); lcd.print("   ");
	lcd.setCursor(16, 3);
	lcd.print(light); lcd.print("   ");
	return;
}

String getTimeStr() {
	String strTime;
	strTime = day() < 10 ? String("0") + day() : day();
	strTime += "-";
	strTime += month() < 10 ? String("0") + month() : month();
	strTime += "-";
	strTime += year() < 10 ? String("0") + year() : year();
	strTime += "  ";

	strTime += hour() < 10 ? String("0") + hour() : hour();
	strTime += ":";
	strTime += minute() < 10 ? String("0") + minute() : minute();
	strTime += ":";
	strTime += second() < 10 ? String("0") + second() : second();

	return strTime;
}