unsigned long t_pump1_change;
unsigned long t_pump2_change;
unsigned long t_fan_mix_change;
unsigned long t_fan_wind_change;
unsigned long t_light_change;
String CMD_ID = "         ";


#pragma region functions

//void DEBUG.print(String x, bool isSendToMQTT = false);
//void DEBUG.print(String x, bool isSendToMQTT) {
//	DEBUG.print(x);
//	if (isSendToMQTT) {
//		mqtt_publish("Mushroom/Debug/" + HubID, x, false);
//	}
//}
//void DEBUG.println(String x = "", bool isSendToMQTT = false);
//void DEBUG.println(String x, bool isSendToMQTT) {
//	DEBUG.println(x);
//	if (x = "") return;
//	if (isSendToMQTT) {
//		mqtt_publish("Mushroom/Debug/" + HubID, x, false);
//		mqtt_publish("Mushroom/Debug/" + HubID, "\r\n", false);
//	}
//}

bool smart_config() {
	Serial.println(("SmartConfig started."));
	unsigned long t = millis();
	WiFi.beginSmartConfig();
	while (1) {
		stt_led = !stt_led;
		out(LED_STT, stt_led);
		delay(200);
		if (WiFi.smartConfigDone()) {
			Serial.println(("SmartConfig: Success"));
			Serial.print(("RSSI: "));
			Serial.println(WiFi.RSSI());
			WiFi.printDiag(Serial);
			WiFi.stopSmartConfig();
			break;
		}
		if ((millis() - t) > (5 * 60000)) {
			Serial.println(("ESP restart"));
			ESP.restart();
			return false;
		}
		myBtn.read();
		if (myBtn.wasPressed()) {
			Serial.println("Log out Smart Config");
			WiFi.stopSmartConfig();
			break;
			//WiFi.waitForConnectResult();
			//return false;
		}
	}

	WiFi.reconnect();
	if (WiFi.waitForConnectResult() == WL_CONNECTED)
	{
		Serial.println(("connected\n"));
		Serial.print(("IP: "));
		Serial.println(WiFi.localIP());
		return true;
	}
	else {
		Serial.println(("SmartConfig Fail\n"));
	}
	return false;
}

void wifi_init() {
	//WiFi.disconnect();
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	WiFi.mode(WIFI_STA);
	//WiFi.begin("Gith", "12345678");
	WiFi.begin("DTU");

	DEBUG.println();
	WiFi.printDiag(DEBUG);
	DEBUG.println();

	DEBUG.println(("\nConnecting..."));
	
	if (WiFi.waitForConnectResult() == WL_CONNECTED)
	{
		DEBUG.println(("connected\n"));
	}
	else
	{
		//WiFiManager wifiManager;
		//wifiManager.autoConnect(String("SMARTMUSHROOM-" + HubID).c_str());
		smart_config();
	}

	DEBUG.print("\r\nIP Address: ");
	DEBUG.println(WiFi.localIP());
	DEBUG.print("RSSI: ");
	DEBUG.println(WiFi.RSSI());
}

void wifi_loop() {
	if (WiFi.isConnected() == false) {
		static unsigned long t_pre_check_wifi_connected = -5000 - 1;
		if ((t_pre_check_wifi_connected == 0) || ((millis() - t_pre_check_wifi_connected) > 5000)) {
			t_pre_check_wifi_connected = millis();
			DEBUG.println("Connecting wifi...");
			if (WiFi.waitForConnectResult() == WL_CONNECTED)
			{
				DEBUG.println(("connected\n"));
				DEBUG.println(WiFi.localIP());
			}
		}
	}

	if (flag_SmartConfig) {
		if (WiFi.smartConfigDone()) {
			DEBUG.println(("SmartConfig: Success"));
			DEBUG.print(("RSSI: "));
			DEBUG.println(WiFi.RSSI());
			WiFi.printDiag(DEBUG);
			WiFi.stopSmartConfig();
		}

	}
}

extern PubSubClient mqtt_client;
void led_loop() {
	if (flag_SmartConfig) {
		static unsigned long t = millis();
		if ((millis() - t) > 100) {
			t = millis();
			out(LED_STT, !stt_led);
		}
	}
	else if (!mqtt_client.connected()) { //loi ket noi den server
		static unsigned long t = millis();
		if ((millis() - t) > 1000) {
			t = millis();
			out(LED_STT, !stt_led);
		}
	}
	else if (digitalRead(SS_WATER_LOW)) { //water empty 
		static unsigned long t = millis();
		if (stt_led) {
			if ((millis() - t) > 1000) {
				t = millis();
				out(LED_STT, !stt_led);
			}
		}
		else {
			if ((millis() - t) > 3000) {
				t = millis();
				out(LED_STT, !stt_led);
			}
		}
	}
	else { //normal led is on
		if (!stt_led) {
			out(LED_STT, stt_led);
		}
	}
}

String http_request(String host, uint16_t port = 80, String url = "/") {
	Serial.println("\r\nGET " + host + ":" + String(port) + url);
	WiFiClient client;
	client.setTimeout(100);
	if (!client.connect(host.c_str(), port)) {
		Serial.println("connection failed");
		delay(1000);
		return "";
	}
	client.print(String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"Connection: close\r\n\r\n");
	unsigned long timeout = millis();
	while (client.available() == 0) {
		if (millis() - timeout > 2000) {
			Serial.println(">>> Client Timeout !");
			client.stop();
			return "";
		}
		delay(1);
	}

	// Read all the lines of the reply from server and print them to Serial
	//while (client.available()) {
	//	String line = client.readStringUntil('\r');
	//	Serial.print(line);
	//}
	//Serial.println();
	//Serial.println();
	String body;
	if (client.available()) {
		body = client.readString();
		int pos_body_begin = body.indexOf("\r\n\r\n");
		if (pos_body_begin > 0) {
			body = body.substring(pos_body_begin + 4);
		}
	}
	client.stop();
	body.trim();
	return body;
}

void updateTimeStamp(unsigned long interval = 0) {
	if (!WiFi.isConnected()) {
		return;
	}
	delay(1);
	static unsigned long t_pre_update = 0;
	static bool wasSync = false;
	if (interval == 0) {
		{
			DEBUG.println("Update timestamp");
			String strTimeStamp;
			strTimeStamp = http_request("gith.cf", 80, "/timestamp");
			int ln = strTimeStamp.indexOf("\r\n");
			if (ln > -1) {
				strTimeStamp = strTimeStamp.substring(ln + 2);
			}
			DEBUG.println(strTimeStamp);
			strTimeStamp.trim();
			long ts = strTimeStamp.toInt();
			if (ts > 1000000000) {
				t_pre_update = millis();
				wasSync = true;
				setTime(ts);
				adjustTime(7 * SECS_PER_HOUR);
				DEBUG.println(("Time Updated "));
				DEBUG.println(ts);
				DEBUG.println();
				return;
			}
		}

		String strTimeStamp = http_request("mic.duytan.edu.vn", 88, "/api/GetUnixTime");
		DEBUG.println(strTimeStamp);
		DynamicJsonBuffer timestamp(500);
		JsonObject& jsTimeStamp = timestamp.parseObject(strTimeStamp);
		if (jsTimeStamp.success()) {
			time_t ts = String(jsTimeStamp["UNIX_TIME"].asString()).toInt();
			if (ts > 1000000000) {
				t_pre_update = millis();
				wasSync = true;
				setTime(ts);
				adjustTime(7 * SECS_PER_HOUR);
				DEBUG.println(("Time Updated\r\n"));
				return;
			}
		}
	}
	else {
		if ((millis() - t_pre_update) > interval) {
			updateTimeStamp();
		}
	}
	if (!wasSync) {
		updateTimeStamp();
	}
	delay(1);
}

//====================================================================
float temp;
float humi;
int light;
void update_sensor(unsigned long period) {
	//check waterEmpty nếu thay đổi thì update ngay lập tức
	static bool waterEmpty_old = false;
	static bool waterEmpty_new;
	waterEmpty_new = !digitalRead(SS_WATER_LOW);
	if (waterEmpty_new != waterEmpty_old) {
		waterEmpty_old = waterEmpty_new;
		update_sensor(0);
	}

	//update sensors data to server every period milli seconds
	static unsigned long preMillis = millis();
	if ((millis() - preMillis) > period) {
		preMillis = millis();
		temp = readTemp1();
		//
		//mqtt_loop();
		//serial_command_handle();
		//
		humi = readHumi1();
		//
		//mqtt_loop();
		//serial_command_handle();
		//
		light = readLight();
		StaticJsonBuffer<200> jsBuffer;
		JsonObject& jsData = jsBuffer.createObject();
		jsData["HUB_ID"] = HubID;
		jsData["TEMP"] = (abs(temp) > 200.0 ? -1 : (int)temp);
		jsData["HUMI"] = (abs(humi) > 200.0 ? -1 : (int)humi);
		jsData["LIGHT"] = light == 1024 ? -1 : light;

		bool waterEmpty = digitalRead(SS_WATER_LOW);
		jsData["WATER_EMPTY"] = waterEmpty ? "YES" : "NO";

		int dBm = WiFi.RSSI();
		int quality;
		if (dBm <= -100)
			quality = 0;
		else if (dBm >= -50)
			quality = 100;
		else
			quality = 2 * (dBm + 100);
		jsData["RSSI"] = quality;

		String data;
		data.reserve(120);
		jsData.printTo(data);
		mqtt_publish(("Mushroom/Sensor/" + HubID), data, true);
	}
}

void out(int pin, bool status) {
	switch (pin)
	{
	case PUMP_MIX:
		DEBUG.print("PUMP_MIX: ");
		DEBUG.println(status ? "ON" : "OFF");
	case PUMP_FLOOR:
		DEBUG.print("PUMP_FLOOR: ");
		DEBUG.println(status ? "ON" : "OFF");
	case FAN_MIX:
		DEBUG.print("FAN_MIX: ");
		DEBUG.println(status ? "ON" : "OFF");
	case FAN_WIND:
		DEBUG.print("FAN_WIND: ");
		DEBUG.println(status ? "ON" : "OFF");
	case LIGHT:
		DEBUG.print("LIGHT: ");
		DEBUG.println(status ? "ON" : "OFF");

	default:
		break;
	}
	if (pin == LED_STT) {
		stt_led = status;
	}
	digitalWrite(pin, status);
}

bool create_logs(String relayName, bool status, bool isCommandFromApp) {
	StaticJsonBuffer<200> jsLogBuffer;
	JsonObject& jsLog = jsLogBuffer.createObject();
	jsLog["HUB_ID"] = HubID;
	String content = relayName;
	content += status ? " on" : " off";
	jsLog["Content"] = content;
	jsLog["From"] = isCommandFromApp ? "APP" : "HUB";
	jsLog["Timestamp"] = String(now());

	String jsStrLog;
	jsStrLog.reserve(150);
	jsLog.printTo(jsStrLog);
	bool res = mqtt_publish("Mushroom/Logs/" + HubID, jsStrLog);
	String strTime = (F("["));
	strTime += (hour() < 10 ? String("0") + hour() : hour());
	strTime += (F(":"));
	strTime += (minute() < 10 ? String("0") + minute() : minute());
	strTime += (F(":"));
	strTime += (second() < 10 ? String("0") + second() : second());
	strTime += (F("] "));
	return res;
}

void send_status_to_server();
bool control(int pin, bool status, bool update_to_server, bool isCommandFromApp);
bool control(int pin, bool status, bool update_to_server, bool isCommandFromApp) { //status = true -> ON; false -> OFF
	if ((pin == PUMP_MIX) && (stt_pump_mix != status)) {
		t_pump1_change = millis();
		stt_pump_mix = status;		
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			create_logs("Pump_Mix", status, isCommandFromApp);
			send_status_to_server();
		}
		return true;
	}
	if ((pin == PUMP_FLOOR) && (stt_pump_floor != status)) {
		t_pump2_change = millis();
		stt_pump_floor = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			create_logs("Pump_Floor", status, isCommandFromApp);
			send_status_to_server();
		}
		return true;
	}
	if ((pin == FAN_MIX) && (stt_fan_mix != status)) {
		t_fan_mix_change = millis();
		stt_fan_mix = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			create_logs("Fan_Mix", status, isCommandFromApp);
			send_status_to_server();
		}
		return true;
	}
	if ((pin == FAN_WIND) && (stt_fan_wind != status)) {
		t_fan_wind_change = millis();
		stt_fan_wind = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			create_logs("Fan_Wind", status, isCommandFromApp);
			send_status_to_server();
		}
		return true;
	}
	if ((pin == LIGHT) && (stt_light != status)) {
		t_light_change = millis();
		stt_light = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			create_logs("Light", status, isCommandFromApp);
			send_status_to_server();
		}
		return true;
	}

	return false;
}

void send_status_to_server() {
	DEBUG.println(("send_status_to_server"));

	StaticJsonBuffer<200> jsBuffer;
	JsonObject& jsStatus = jsBuffer.createObject();
	jsStatus["HUB_ID"] = HubID;
	jsStatus["MIST"] = stt_pump_mix ? on_ : off_;
	jsStatus["FAN_MIX"] = stt_fan_mix ? on_ : off_;
	jsStatus["LIGHT"] = stt_light ? on_ : off_;
	jsStatus["CMD_ID"] = "HW-" + String(now());;

	String jsStatusStr;
	jsStatusStr.reserve(150);
	jsStatus.printTo(jsStatusStr);
	mqtt_publish("Mushroom/Commands/" + HubID, jsStatusStr, true);

}

bool skip_auto_light = false;
bool skip_auto_pump1 = false;
bool skip_auto_fan = false;
void auto_control() {
	//https://docs.google.com/document/d/1d2rM6dZIEmlLwflTyprBfNNIQCff1AfebeyYhX7ptJw/edit
	//https://prnt.sc/j2oxmu https://snag.gy/6E7xhU.jpg

	//+ PUMP_MIX tự tắt sau 1.5 phút
	if ((millis() - t_pump1_change) > (90000)) {
		skip_auto_pump1 = false;
		if (stt_pump_mix) {
			DEBUG.println("AUTO PUMP_MIX OFF");
			control(PUMP_MIX, false, true, false);
		}
	}
	if ((millis() - t_pump2_change) > 90000) {
		if (stt_pump_floor) {
			DEBUG.println("AUTO PUMP_FLOOR OFF");
			control(PUMP_FLOOR, false, true, false);
		}
	}
	//+ FAN_MIX tự tắt sau 1 tiếng
	if ((millis() - t_fan_mix_change) > (1 * 1000 * SECS_PER_HOUR)) {
		skip_auto_fan = false;
		if (stt_fan_mix) {
			DEBUG.println("AUTO FAN_MIX OFF");
			control(FAN_MIX, false, true, false);
		}
	}
	//+ LIGHT tự tắt sau 3 tiếng
	if ((millis() - t_light_change) > (3 * 1000 * SECS_PER_HOUR)) {
		skip_auto_light = false;
		if (stt_light) {
			DEBUG.println("AUTO LIGHT OFF");
			control(LIGHT, false, true, false);
		}
	}
	//==============================================================
	//1/ Bật tắt đèn
	if (!skip_auto_light) {
		if (library && (light < LIGHT_MIN) && (hour() >= 6) && (hour() <= 18)) {
			DEBUG.println("AUTO LIGHT ON");
			control(LIGHT, true, true, false);
		}
		else if (library && (light > LIGHT_MAX)) {
			DEBUG.println("AUTO LIGHT OFF");
			control(LIGHT, false, true, false);
		}
	}
	//-------------------

	//2. Bật tắt phun sương
	//a. Phun trực tiếp vào phôi vào lúc 6h và 16h
	if (((hour() == 6) || (hour() == 16)) && (minute() == 0) && (second() == 0)) {
		skip_auto_pump1 = true;
		skip_auto_fan = true;
		DEBUG.println("AUTO PUMP_MIX ON");
		DEBUG.println("AUTO PUMP_FLOOR ON");
		control(PUMP_FLOOR, true, true, false);
		control(PUMP_MIX, true, true, false);
		DEBUG.println("AUTO FAN_MIX ON");
		control(FAN_MIX, true, true, false);
	}

	//b. Phun sương làm mát, duy trì độ ẩm. Thời gian bật: 3 phút, mỗi lần bật cách nhau 1 giờ.
	if (!skip_auto_pump1 && library && ((int(temp) > TEMP_MAX) || (int(humi) < HUMI_MIN)) && ((millis() - t_pump1_change) > 3600000) && !stt_pump_mix) {
		DEBUG.println("AUTO PUMP_MIX ON");
		control(PUMP_MIX, true, true, false);
		DEBUG.println("AUTO FAN_MIX ON");
		control(FAN_MIX, true, true, false);
	}
	//-------------------

	//c. Bật tắt quạt
	if (!skip_auto_fan && library && (((int)humi > HUMI_MAX) || ((int)temp > TEMP_MAX)) && ((millis() - t_fan_mix_change) < 3600000) && !stt_fan_mix) {
		DEBUG.println("AUTO FAN_MIX ON");
		control(FAN_MIX, true, true, false);
	}
	delay(1);
}

void updateFirmware(String url) {
	ESPhttpUpdate.rebootOnUpdate(true);

	if ((WiFi.status() == WL_CONNECTED)) {

		t_httpUpdate_return ret = ESPhttpUpdate.update(url);

		switch (ret) {
		case HTTP_UPDATE_FAILED:
			DEBUG.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
			break;

		case HTTP_UPDATE_NO_UPDATES:
			DEBUG.println("HTTP_UPDATE_NO_UPDATES");
			break;

		case HTTP_UPDATE_OK:
			DEBUG.println("HTTP_UPDATE_OK");
			break;
		}
	}
}

void serial_command_handle() {
	if (Serial.available()) {
		String Scmd = Serial.readString();
		Scmd.trim();
		DEBUG.println(("\r\n>>>"));
		DEBUG.println(Scmd);

		control_handle(Scmd);
	}

	delay(1);
}

void control_handle(String cmd) {
	cmd.toUpperCase();
	if (cmd.indexOf("LIGHT ON") > -1) {
		skip_auto_light = true;
		control(LIGHT, true, true, true);
	}
	if (cmd.indexOf("LIGHT OFF") > -1) {
		skip_auto_light = true;
		control(LIGHT, false, true, true);
	}

	if (cmd.indexOf("MIST ON") > -1) {
		skip_auto_pump1 = true;
		control(PUMP_MIX, true, true, true);
	}
	if (cmd.indexOf("MIST OFF") > -1) {
		skip_auto_pump1 = true;
		control(PUMP_MIX, false, true, true);
	}

	if (cmd.indexOf("FAN_MIX ON") > -1) {
		skip_auto_fan = true;
		control(FAN_MIX, true, true, true);
	}
	if (cmd.indexOf("FAN_MIX OFF") > -1) {
		skip_auto_fan = true;
		control(FAN_MIX, false, true, true);
	}
	if (cmd.indexOf("RESET WIFI") > -1) {
		DEBUG.println(("Reset Wifi"));
		WiFi.disconnect();
	}
}

#pragma endregion


