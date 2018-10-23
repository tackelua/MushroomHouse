// 
// 
// 

#include "mqtt_helper.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "hardware.h"

extern String HubID;
extern String _firmwareVersion;
extern void updateFirmware(String url);
extern String CMD_ID;
extern bool flag_schedule_pump_floor;


const char* mqtt_server = "mic.duytan.edu.vn";
const char* mqtt_user = "Mic@DTU2017";
const char* mqtt_password = "Mic@DTU2017!@#";
const uint16_t mqtt_port = 1883;

const String on_ = "on";
const String off_ = "off";

int TEMP_MAX, TEMP_MIN, HUMI_MAX, HUMI_MIN, LIGHT_MAX, LIGHT_MIN;
long DATE_HAVERST_PHASE;
bool library = false;

extern bool stt_pump_mix, stt_fan_mix, stt_light;
extern void control(int pin, bool status, bool update_to_server, bool isCommandFromApp);
extern void create_logs(String relayName, bool status, bool isCommandFromApp);
extern void send_status_to_server();
extern void out(int pin, bool status);
extern bool ENABLE_SYSTEM_BY_CONTROL;
extern void wait(unsigned long ms);
extern void control_handle(String cmd);
extern void save_library();


time_t update_sensor_interval = 30000;
time_t update_sensor_t; //millis()
time_t delay_update_sensor_t;

String mqtt_Message;

WiFiClient mqtt_espClient;
PubSubClient mqtt_client(mqtt_espClient);

#pragma region parseTopic
void handleTopic__Mushroom_Commands_HubID() {
	StaticJsonBuffer<500> jsonBuffer;
	JsonObject& commands = jsonBuffer.parseObject(mqtt_Message);

	String HUB_ID = commands["HUB_ID"].as<String>();

	CMD_ID = commands["CMD_ID"].as<String>();
	bool isCommandFromApp = false;
	static bool firsControlFromRetain = true;

	CMD_ID.trim();
	if (CMD_ID.startsWith("HW-")) {
		//DEBUG.print(("Command ID "));
		//DEBUG.print(CMD_ID);
		//DEBUG.print((" was excuted."));
		if (!firsControlFromRetain) {
			DEBUG.println(("Skipped\r\n"));
			return;
		}
		firsControlFromRetain = false;
	}
	else {
		isCommandFromApp = true;
		ENABLE_SYSTEM_BY_CONTROL = true;
	}

	if (isCommandFromApp || firsControlFromRetain) {
		delay_update_sensor_t = millis();
		//gi?m update_sensor_interval
		update_sensor_interval = 5000;
		update_sensor_t = millis();

		String pump_mix_stt = commands["MIST"].as<String>();
		extern bool skip_auto_pump_mix;
		if (pump_mix_stt == on_ && stt_pump_mix == false)
		{
			skip_auto_pump_mix = true;
			control(PUMP_MIX, true, true, isCommandFromApp);
			//create_logs("Pump_Mix", true, isCommandFromApp);

			//pump_mix_change = control(PUMP_FLOOR, true, false, isCommandFromApp);
			//create_logs("Pump_Floor", true, isCommandFromApp);
			flag_schedule_pump_floor = true;
		}
		else if (pump_mix_stt == off_ && stt_pump_mix == true)
		{
			skip_auto_pump_mix = true;
			control(PUMP_MIX, false, true, isCommandFromApp);
			//create_logs("Pump_Mix", false, isCommandFromApp);

			control(PUMP_FLOOR, false, true, isCommandFromApp);
			//create_logs("Pump_Floor", false, isCommandFromApp);
		}

		String light_stt = commands["LIGHT"].as<String>();
		extern bool skip_auto_light;
		if (light_stt == on_ && stt_light == false)
		{
			skip_auto_light = true;
			control(LIGHT, true, true, isCommandFromApp);
		}
		else if (light_stt == off_ && stt_light == true)
		{
			skip_auto_light = true;
			control(LIGHT, false, true, isCommandFromApp);
		}

		String fan_stt = commands["FAN"].as<String>();
		extern bool skip_auto_fan_mix;
		if (fan_stt == on_ && stt_fan_mix == false)
		{
			skip_auto_fan_mix = true;
			control(FAN_MIX, true, true, isCommandFromApp);

			control(FAN_WIND, true, true, isCommandFromApp);
		}
		else if (fan_stt == off_ && stt_fan_mix == true)
		{
			skip_auto_fan_mix = true;
			control(FAN_MIX, false, true, isCommandFromApp);

			control(FAN_WIND, false, true, isCommandFromApp);
		}
	}
}

void handleTopic__Mushroom_Library_HubID() {
	const size_t bufferSize = JSON_OBJECT_SIZE(7) + 120;
	DynamicJsonBuffer jsonBuffer(bufferSize);
	//StaticJsonBuffer<500> jsonBuffer;
	JsonObject& lib = jsonBuffer.parseObject(mqtt_Message);
	if (lib.success()) {
		TEMP_MAX = lib["TEMP_MAX"];
		TEMP_MIN = lib["TEMP_MIN"];
		HUMI_MAX = lib["HUMI_MAX"];
		HUMI_MIN = lib["HUMI_MIN"];
		LIGHT_MAX = lib["LIGHT_MAX"];
		LIGHT_MIN = lib["LIGHT_MIN"];
		DATE_HAVERST_PHASE = lib["DATE_HAVERST_PHASE"];
		library = true;

		//save preferences for load_library
		save_library();

		String d;
		d += ("TEMP_MAX = " + String(TEMP_MAX)) + "\r\n";
		d += ("TEMP_MIN = " + String(TEMP_MIN)) + "\r\n";
		d += ("HUMI_MAX = " + String(HUMI_MAX)) + "\r\n";
		d += ("HUMI_MIN = " + String(HUMI_MIN)) + "\r\n";
		d += ("LIGHT_MAX = " + String(LIGHT_MAX)) + "\r\n";
		d += ("LIGHT_MIN = " + String(LIGHT_MIN)) + "\r\n";
		d += ("DATE_HAVERST_PHASE = " + String(DATE_HAVERST_PHASE));

		DEBUG.println(d);
		//mqtt_publish("Mushroom/DEBUG/" + HubID, mqtt_Message);
		//mqtt_publish("Mushroom/DEBUG/" + HubID, d);
	}
	else {
		mqtt_publish("Mushroom/DEBUG/" + HubID, "Parse library fail");
	}

}
#pragma endregion

int wifi_quality() {
	int dBm = WiFi.RSSI();
	if (dBm <= -100)
		return 0;
	else if (dBm >= -50)
		return 100;
	else
		return int(2 * (dBm + 100));
}

void mqtt_callback(char* topic, uint8_t* payload, unsigned int length) {
	//ulong t = millis();
	//DEBUG.print(("\r\n#1 FREE RAM : "));
	//DEBUG.println(ESP.getFreeHeap());

	String topicStr = topic;

	DEBUG.println(("\r\n>>>"));
	//DEBUG.println(topicStr);
	DEBUG.print(("Message arrived: "));
	DEBUG.print(topicStr);
	DEBUG.print(("["));
	DEBUG.print(String(length));
	DEBUG.println(("]"));

	mqtt_Message = "";
	out(LED_STT, ON);
	for (uint i = 0; i < length; i++) {
		//DEBUG.print((char)payload[i]);
		mqtt_Message += (char)payload[i];
	}
	out(LED_STT, OFF);

	DEBUG.println(mqtt_Message);

	//control pump_mix, light, fan
	if (topicStr == String("Mushroom/Commands/" + HubID))
	{
		handleTopic__Mushroom_Commands_HubID();
	}

	else if (topicStr == String("Mushroom/Library/" + HubID)) {
		handleTopic__Mushroom_Library_HubID();
	}

	else if (topicStr == "Mushroom/Terminal/" + HubID) {
		if (mqtt_Message == "/restart") {
			mqtt_publish("Mushroom/Terminal/" + HubID, "Restarting");
			DEBUG.println("\r\nRestart\r\n");
			ESP.restart();
			wait(100);
		}
		if (mqtt_Message.startsWith("/uf")) {
			String url;
			url = mqtt_Message.substring(3);
			url.trim();

			if (!url.startsWith("http")) {
				url = "http://gith.cf/files/MushroomHouse.bin";
			}
			mqtt_publish("Mushroom/Terminal/" + HubID, "Updating new firmware " + url);
			DEBUG.print(("\nUpdating new firmware: "));
			updateFirmware(url);
			DEBUG.println(("DONE!"));
		}
		else if (mqtt_Message.indexOf("/get version") > -1) {
			//StaticJsonBuffer<500> jsBuffer;
			DynamicJsonBuffer jsBuffer(500);
			JsonObject& jsData = jsBuffer.createObject();
			jsData["HUB_ID"] = HubID;
			jsData["FW Version"] = _firmwareVersion;

			String data;
			data.reserve(100);
			jsData.printTo(data);
			mqtt_publish("Mushroom/Terminal/" + HubID, data);
		}
		else if (mqtt_Message.indexOf("/get library") > -1) {
			//StaticJsonBuffer<500> jsBuffer;
			DynamicJsonBuffer jsBuffer(500);
			JsonObject& jsLib = jsBuffer.createObject();
			jsLib["library"] = library;
			jsLib["TEMP_MAX"] = TEMP_MAX;
			jsLib["TEMP_MIN"] = TEMP_MIN;
			jsLib["HUMI_MAX"] = HUMI_MAX;
			jsLib["HUMI_MIN"] = HUMI_MIN;
			jsLib["LIGHT_MAX"] = LIGHT_MAX;
			jsLib["LIGHT_MIN"] = LIGHT_MIN;
			jsLib["DATE_HAVERST_PHASE"] = DATE_HAVERST_PHASE;

			String libs;
			libs.reserve(100);
			jsLib.printTo(libs);
			mqtt_publish("Mushroom/Terminal/" + HubID, libs);
		}

		String mqtt_cmd = mqtt_Message;
		mqtt_cmd.toUpperCase();
		control_handle(mqtt_cmd);
	}

	else if (topicStr == "Mushroom/Terminal") {
		StaticJsonBuffer<550> jsonBuffer;
		JsonObject& terminal = jsonBuffer.parseObject(mqtt_Message);
		/*
		Mushroom/Terminal
		{
			"Command" : "FOTA",
			"Hub_ID" : "17C80",
			"Version" : "",
			"Url" : "http://autominer.xyz/files/MushroomHouse.bin"
		}
		*/
		if (terminal.success()) {
			String command = terminal["Command"].as<String>();
			if (command == "FOTA") {
				DEBUG.println(("Update firmware function"));
				String hub = terminal["Hub_ID"].as<String>();
				if ((hub == HubID) || (hub == "all")) {
					DEBUG.println("Thoa man dieu kien HubID");
					String ver = terminal["Version"].as<String>();
					if (ver != _firmwareVersion) {
						DEBUG.println("Thoa man dieu kien version");
						String url = terminal["Url"].as<String>();
						mqtt_publish("Mushroom/Terminal/" + HubID, "Updating new firmware " + ver);
						DEBUG.print(("\nUpdating new firmware: "));
						DEBUG.println(ver);
						DEBUG.println(url);
						updateFirmware(url);
						DEBUG.println(("DONE!"));
					}
				}
			}
		}
	}

	//DEBUG.print(("#2 FREE RAM : "));
	//DEBUG.println(ESP.getFreeHeap());

	//DEBUG.println("Time mqtt_callback " + String(millis() - t));
}

void mqtt_reconnect() {  // Loop until we're reconnected
	while (!mqtt_client.connected()) {
		DEBUG.print(("Attempting MQTT connection..."));
		//boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
		if (mqtt_client.connect(HubID.c_str(), mqtt_user, mqtt_password, ("Mushroom/Status/" + HubID).c_str(), 0, true, String("{\"HUB_ID\":\"" + HubID + "\",\"STATUS\":\"OFFLINE\"}").c_str())) {
			DEBUG.print((" Connected."));
			String h_online = "{\"HUB_ID\":\"" + HubID + "\",\"STATUS\":\"ONLINE\",\"FW_VER\":\"" + _firmwareVersion + "\",\"WIFI\":\"" + WiFi.SSID() + "\",\"SIGNAL\":" + String(wifi_quality()) + "}";
			//String h_online = HubID + " online";
			//mqtt_client.publish(("Mushroom/Status/" + HubID).c_str(), (HubID + " online").c_str(), true);
			mqtt_client.publish(("Mushroom/Status/" + HubID).c_str(), h_online.c_str(), true);

			mqtt_client.publish(("Mushroom/SetWifi/" + HubID).c_str(), "Success");
			mqtt_client.subscribe(("Mushroom/Library/" + HubID).c_str());
			mqtt_client.subscribe(("Mushroom/Commands/" + HubID).c_str());
			mqtt_client.subscribe("Mushroom/Terminal");
			mqtt_client.subscribe(("Mushroom/Terminal/" + HubID).c_str());
		}
		else {
			DEBUG.print(("failed, rc="));
			DEBUG.println(String(mqtt_client.state()));
			return;
			//DEBUG.println((" try again"));
			//wait(500);
		}
	}
}

void mqtt_init() {
	//http.setReuse(true);
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
	wait(1);
}

bool mqtt_publish(String topic, String payload, bool retain) {
	if (!mqtt_client.connected()) {
		return false;
	}
	DEBUG.print(("MQTT publish to topic: "));
	DEBUG.println(topic);
	DEBUG.println(payload);
	DEBUG.println();

	out(LED_STT, ON);
	bool ret = mqtt_client.publish(topic.c_str(), payload.c_str(), retain);
	wait(1);
	out(LED_STT, OFF);
	return ret;
}