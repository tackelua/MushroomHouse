#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <TimeLib.h>
#include <Time.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG Serial
#define ON	LOW
#define OFF	HIGH

#define RESET	D5

bool flag_time_set = false;
bool flag_show_start_ss = false;
unsigned long t_system_active;

#define __VERSION__	"0.2"
String _firmwareVersion = __VERSION__ " " __DATE__ " " __TIME__;

String HubID = "17C80";
const char* mqtt_server = "mic.duytan.edu.vn";
const char* mqtt_user = "Mic@DTU2017";
const char* mqtt_password = "Mic@DTU2017!@#";
const uint16_t mqtt_port = 1883;

SoftwareSerial UART(D6, D7);
LiquidCrystal_I2C lcd(0x3f, 20, 04);

String mqtt_Message;
WiFiClient mqtt_espClient;
PubSubClient mqtt_client(mqtt_espClient);


void setup() {
	//WiFi.mode(WIFI_OFF);
	pinMode(RESET, OUTPUT);
	digitalWrite(RESET, HIGH);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, ON);

	Serial.begin(74880);
	Serial.setTimeout(200);
	Serial.println("ProMicro v1");
	UART.begin(115200);
	UART.setTimeout(200);
	lcd_init();
	mqtt_init();
	WiFi.mode(WIFI_STA);
	WiFi.begin("IoT Wifi", "mic@dtu12345678()");
	digitalWrite(LED_BUILTIN, OFF);
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

		data.trim();
		DynamicJsonBuffer cmd(500);
		JsonObject& jsCmd = cmd.parseObject(data);
		if (jsCmd.success()) {
			digitalWrite(LED_BUILTIN, ON);
			String cmd = jsCmd["cmd"].asString();
			if (cmd == "ss") {
				t_system_active = millis();
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
				//adjustTime(7 * SECS_PER_HOUR);
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
			digitalWrite(LED_BUILTIN, OFF);
		}
	}

	if (flag_time_set) {
		static unsigned long t = millis() - 1000 - 1;
		if (millis() - t >= 1000) {
			t = millis();
			lcd.setCursor(0, 1);
			String timeS = getTimeStr();
			lcd.print(timeS);
			UART.println(timeS);
			Serial.println(timeS);
		}
	}

	if (millis() - t_system_active > 3 * 60000) {
		t_system_active = millis();
		reset_esp32();
	}

	mqtt_loop();
}

void lcd_init() {
	lcd.begin();
	lcd.backlight();

	lcd.setCursor(0, 0);
	lcd.print("  SMART  MUSHROOM   ");
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

int wifi_quality() {
	int dBm = WiFi.RSSI();
	if (dBm <= -100)
		return 0;
	else if (dBm >= -50)
		return 100;
	else
		return int(2 * (dBm + 100));
}

void updateFirmware(String url) {
	ESPhttpUpdate.rebootOnUpdate(true);

	t_httpUpdate_return ret = ESPhttpUpdate.update(url);

	switch (ret) {
	case HTTP_UPDATE_FAILED:
		DEBUG.printf("HTTP_UPDATE_FAILD Error (%d): %s\r\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
		break;

	case HTTP_UPDATE_NO_UPDATES:
		DEBUG.println(("HTTP_UPDATE_NO_UPDATES"));
		break;

	case HTTP_UPDATE_OK:
		DEBUG.println(("HTTP_UPDATE_OK"));
		delay(2000);
		ESP.restart();
		break;
	}
}

void reset_esp32() {
	digitalWrite(RESET, LOW);
	delay(100);
	digitalWrite(RESET, HIGH);
	mqtt_publish("Mushroom/Terminal/" + HubID, "WEMOS RESET MUSHROOM", false);
}

void mqtt_callback(char* topic, uint8_t* payload, unsigned int length) {
	String topicStr = topic;

	DEBUG.println(("\r\n>>>"));
	//DEBUG.println(topicStr);
	DEBUG.print(("Message arrived: "));
	DEBUG.print(topicStr);
	DEBUG.print(("["));
	DEBUG.print(String(length));
	DEBUG.println(("]"));

	mqtt_Message = "";
	digitalWrite(LED_BUILTIN, ON);
	for (uint i = 0; i < length; i++) {
		//DEBUG.print((char)payload[i]);
		mqtt_Message += (char)payload[i];
	}
	digitalWrite(LED_BUILTIN, OFF);

	DEBUG.println(mqtt_Message);

	if (mqtt_Message == "/restart-w") {
		reset_esp32();
	}
	else if (mqtt_Message == "/restart-ws") {
		ESP.reset();
		delay(500);
	}
	else if (mqtt_Message.startsWith("/uf-w")) {
		String url = mqtt_Message.substring(6);
		url.trim();
		mqtt_publish("Mushroom/Terminal/" + HubID, "WEMOS UPDATING FIRMWARE\r\n" + url, false);
		updateFirmware(url);
	}
	yield();
}

void mqtt_reconnect() {  // Loop until we're reconnected
	while (!mqtt_client.connected()) {
		DEBUG.print(("Attempting MQTT connection..."));
		if (mqtt_client.connect(String(HubID + "-WS").c_str(), mqtt_user, mqtt_password, ("Mushroom/DEBUG/" + HubID).c_str(), 0, true, String("{\"HUB_ID\":\"" + HubID + "-WS" + "\",\"STATUS\":\"OFFLINE\"}").c_str())) {
			DEBUG.print((" Connected."));
			String h_online = "{\"HUB_ID\":\"" + HubID + "-WS" + "\",\"STATUS\":\"ONLINE\",\"FW_VER\":\"" + _firmwareVersion + "\",\"WIFI\":\"" + WiFi.SSID() + "\",\"SIGNAL\":" + String(wifi_quality()) + "}";
			mqtt_client.publish(("Mushroom/DEBUG/" + HubID).c_str(), h_online.c_str(), true);
			String sub_topic = "Mushroom/Terminal/" + HubID;
			mqtt_client.subscribe(sub_topic.c_str());
		}
		else {
			DEBUG.print(("failed, rc="));
			DEBUG.println(String(mqtt_client.state()));
			return;
		}
	}
}

void mqtt_init() {
	mqtt_Message.reserve(MQTT_MAX_PACKET_SIZE); //tao buffer khoang trong cho mqtt_Message
	mqtt_client.setServer(mqtt_server, mqtt_port);
	mqtt_client.setCallback(mqtt_callback);
}

void mqtt_loop() {
	if (!WiFi.isConnected()) {
		return;
	}
	if (!mqtt_client.connected()) {
		mqtt_reconnect();
	}
	mqtt_client.loop();
	delay(1);
}

bool mqtt_publish(String topic, String payload, bool retain) {
	if (!mqtt_client.connected()) {
		return false;
	}
	DEBUG.print(("MQTT publish to topic: "));
	DEBUG.println(topic);
	DEBUG.println(payload);
	DEBUG.println();

	digitalWrite(LED_BUILTIN, ON);
	bool ret = mqtt_client.publish(topic.c_str(), payload.c_str(), retain);
	delay(1);
	digitalWrite(LED_BUILTIN, OFF);
	return ret;
}