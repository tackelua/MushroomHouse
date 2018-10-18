unsigned long t_pump_mix_change = -1;
unsigned long t_pump_floor_change = -1;
unsigned long t_fan_mix_change = -1;
unsigned long t_fan_wind_change = -1;
unsigned long t_light_change = -1;
//unsigned long t_lcd_backlight_change = -1;
String CMD_ID = "         ";

//extern LiquidCrystal_I2C lcd;
extern PubSubClient mqtt_client;

#pragma region functions

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

bool smart_config() {
	Serial.println(("SmartConfig started."));
	unsigned long t = millis();
	WiFi.beginSmartConfig();
	while (1) {
		stt_led = !stt_led;
		out(LED_STT, stt_led);
		wait(200);
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

void print_wifi_details() {
	DEBUG.println();
	DEBUG.print("Connected to ");
	DEBUG.println(WiFi.SSID());
	DEBUG.print("SSID: ");
	DEBUG.println(WiFi.SSID());
	DEBUG.print("IP Address: ");
	DEBUG.println(WiFi.localIP());
	DEBUG.print("RSSI: ");
	DEBUG.println(WiFi.RSSI());
	DEBUG.println();
}
void wifi_init() {
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	WiFi.mode(WIFI_STA);

	WiFiManager wifiManager;
	wifiManager.autoConnect(String("MUSHROOMHOUSE-" + HubID).c_str());

	//DEBUG.println(("\nWiFi attempting to connect to MIC"));
	//WiFi.begin("IoT Wifi", "mic@dtu12345678()");
	//WiFi.printDiag(DEBUG);
	//DEBUG.println();

	unsigned long t = millis();
	while (WiFi.status() != WL_CONNECTED && (millis() - t) < 10000) {
		wait(500);
		out(LED_STT, !stt_led);
	}

	DEBUG.println("Connecting ");
	t = millis();
	while (WiFi.localIP() == INADDR_NONE && (millis() - t) < 3000) {
		wait(100);
		out(LED_STT, !stt_led);
	}
	if (WiFi.isConnected()) {
		print_wifi_details();
		out(LED_STT, false);
		return;
	}
}

bool isWaterEmpty() {
	bool s = digitalRead(SS_WATER_LOW);
	s = !s; //cái này đổi do mức logic cảm biến dùng cái khác
	return s;
}

void wifi_loop() {
	if (WiFi.isConnected() == false || mqtt_client.connected() == false) {
		flag_lcd_line_0 = NOTI_CONNECTION_LOST;
	}
	else {
		flag_lcd_line_0 = SHOW_HUBID;
	}

	if (WiFi.isConnected() == false) {
		DEBUG.println(("\nWiFi attempting to connect..."));
		if (WiFi.waitForConnectResult() == WL_CONNECTED) {
			DEBUG.println();
			DEBUG.print("Connected to ");
			DEBUG.println(WiFi.SSID());
			DEBUG.print("IP Address: ");
			DEBUG.println(WiFi.localIP());
			DEBUG.print("RSSI: ");
			DEBUG.println(WiFi.RSSI());
		}
		//else {
		//	wifi_init();
		//}
	}
}

void led_loop() {
	if (!mqtt_client.connected()) { //loi ket noi den server
		static unsigned long t = millis();
		if ((millis() - t) > 1000) {
			t = millis();
			out(LED_STT, !stt_led);
		}
	}
	else if (isWaterEmpty()) { //water empty 
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
	wait(1);
}

String http_request(String host, uint16_t port = 80, String url = "/") {
	Serial.println("\r\nGET " + host + ":" + String(port) + url);
	WiFiClient client;
	client.setTimeout(100);
	if (!client.connect(host.c_str(), port)) {
		Serial.println("connection failed");
		wait(1000);
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
		wait(1);
	}

	// Read all the lines of the reply from server and print them to Serial
	//while (client.available()) {
	//  String line = client.readStringUntil('\r');
	//  Serial.print(line);
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
	wait(1);
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



				//gửi cho pro mini
				{
					StaticJsonBuffer<500> jsBuffer;
					JsonObject& jsProMicro = jsBuffer.createObject();
					jsProMicro["cmd"] = "ts";
					jsProMicro["ts"] = now();
					String strProMicro;
					jsProMicro.printTo(strProMicro);
					LCD_UART.println(strProMicro);
					DEBUG.println(strProMicro);
				}
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


				//gửi cho pro mini
				{
					StaticJsonBuffer<500> jsBuffer;
					JsonObject& jsProMicro = jsBuffer.createObject();
					jsProMicro["cmd"] = "ts";
					jsProMicro["ts"] = now();
					String strProMicro;
					jsProMicro.printTo(strProMicro);
					LCD_UART.println(strProMicro);
					DEBUG.println(strProMicro);
				}
				return;
			}
		}
	}
	else {
		if ((millis() - t_pre_update) > interval) {
			lcd_init();

			updateTimeStamp();
		}
	}
	if (!wasSync) {
		updateTimeStamp();
	}
	wait(1);
}

//====================================================================
extern bool stt_alarm;
void warming_alarm() {
	static bool waterEmpty_pre = false;
	static bool waterEmpty;
	waterEmpty = isWaterEmpty();

	if (waterEmpty) {
		if (stt_pump_mix == ON || stt_pump_floor == ON) {
			control(PUMP_FLOOR, false, true, false);
			control(PUMP_MIX, false, true, false);
		}

		flag_lcd_line_0 = NOTI_WATER_EMPTY;
		static unsigned long t = millis() - 15000;
		if (stt_alarm && (millis() - t > 3000)) {
			t = millis();
			stt_alarm = false;
			digitalWrite(ALARM_SIRENS, stt_alarm);
		}
		if (!stt_alarm && (millis() - t > 10000)) {
			t = millis();
			stt_alarm = true;
			digitalWrite(ALARM_SIRENS, stt_alarm);
		}
	}
	else {
		if (stt_alarm) {
			stt_alarm = false;
			digitalWrite(ALARM_SIRENS, stt_alarm);
		}
	}
	lcd_show_line_0();
}
int temp = 0;
int humi = 0;
int light = 0;
void update_sensor(unsigned long period) {
	//reset update_sensor_interval
	if (millis() - update_sensor_t > 30000) {
		update_sensor_interval = 30000;
		update_sensor_t = millis();
	}

	//reset delay_update_sensor
	if (millis() - delay_update_sensor_t < 2000) {
		return;
	}

	static int sensor_fail = 0;
	//check waterEmpty nếu thay đổi thì update ngay lập tức
	static bool flag_update_now = true;
	static bool waterEmpty_pre = false;
	static bool waterEmpty;
	waterEmpty = isWaterEmpty();
	if (waterEmpty != waterEmpty_pre) {
		waterEmpty_pre = waterEmpty;
		flag_update_now = true;
	}

	//update sensors data to server every period milli seconds
	static unsigned long preMillis = millis() - period - 1;
	if (flag_update_now || (millis() - preMillis) > period) {
		flag_update_now = false;
		preMillis = millis();
		float ftemp, fhumi, flight;
		ftemp = readTemp1();
		wait(10);
		//
		//mqtt_loop();
		//serial_command_handle();
		//
		fhumi = readHumi1();
		wait(10);
		//
		//mqtt_loop();
		//serial_command_handle();
		//
		flight = readLight();

		int itemp = round(ftemp);
		int ihumi = round(fhumi);
		int ilight = round(flight);

		unsigned long t_read_sensors = millis() - preMillis;

		if (itemp <= 0 || itemp > 100) {
			wait(50);
			ftemp = readTemp1();
			itemp = ftemp;
			itemp = (itemp > 100 || itemp <= 0) ? -1 : itemp;
		}

		if (ihumi <= 0 || ihumi > 100) {
			wait(50);
			fhumi = readHumi1();
			ihumi = fhumi;
			ihumi = (ihumi > 100 || ihumi <= 0) ? -1 : ihumi;
		}

		if (ilight <= 0 || ilight > 100) {
			wait(50);
			flight = readLight();
			ilight = flight;
			ilight = (ilight > 20000 || ilight < 0 || ilight == 703) ? -1 : ilight;
		}

		if (itemp != -1) {
			temp = itemp;
		}
		else {
			temp = 0;
			sensor_fail++;
		}
		if (ihumi != -1) {
			humi = ihumi;
		}
		else {
			humi = 0;
			sensor_fail++;
		}
		if (ilight != -1) {
			light = ilight;
		}
		else {
			light = 0;
		}
		//lcd_print_sensor(temp, humi, light);

		if (sensor_fail > 10) {
			mqtt_publish("Mushroom/DEBUG/" + HubID, "Restart because sensor fail too much");
			ESP.restart();
		}

		if (temp > 0 && temp < 60 && humi > 0 && humi < 100) {
			sensor_fail = 0;
		}

		StaticJsonBuffer<500> jsBuffer;
		JsonObject& jsData = jsBuffer.createObject();
		jsData["HUB_ID"] = HubID;
		jsData["TEMP"] = temp;
		jsData["HUMI"] = humi;
		jsData["LIGHT"] = light;
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

		//make compatible cmd LCD_UART
		{
			jsData["cmd"] = "ss"; //sensor
			String dtProMicro;
			jsData.printTo(dtProMicro);
			LCD_UART.println(dtProMicro);
			DEBUG.println(dtProMicro);
		}

		mqtt_publish(("Mushroom/Sensor/" + HubID), data, true);

		thingspeak_update(ftemp, fhumi, flight);
		//mqtt_publish("Mushroom/DEBUG/" + HubID, "Sensor: " + String(ftemp) + " " + String(fhumi) + " " + String(flight) + "\nsensor_fail: " + String(sensor_fail));

		//mqtt_publish("Mushroom/DEBUG/" + HubID, "Time Read Sensor : " + String(t_read_sensors));
	}
}

void out(int pin, bool status) {
	if (pin == LED_STT) {
		stt_led = status;
	}
	digitalWrite(pin, status);

	switch (pin)
	{
	case PUMP_MIX:
		digitalWrite(PUMP_BOTH, status);
		DEBUG.print("PUMP_MIX: ");
		DEBUG.println(status ? "ON" : "OFF");
	case PUMP_FLOOR:
		digitalWrite(PUMP_BOTH, status);
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

	//t_lcd_backlight_change = millis();
	//stt_lcd_backlight = true;
	//lcd.backlight();
}

void create_logs(String relayName, bool status, bool isCommandFromApp) {
	if (!mqtt_client.connected()) {
		return;
	}
	StaticJsonBuffer<500> jsLogBuffer;
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
}

void send_status_to_server();
void control(int pin, bool status, bool update_to_server, bool isCommandFromApp);
void control(int pin, bool status, bool update_to_server, bool isCommandFromApp) { //status = true -> ON; false -> OFF
	if ((pin == PUMP_MIX)/* && (stt_pump_mix != status)*/) {
		if (isWaterEmpty() && status == ON) {
			mqtt_publish(("Mushroom/DEBUG/" + HubID).c_str(), "WATER EMPTY, CAN NOT ON PUMP_MIX");
			return;
		}
		t_pump_mix_change = millis();
		stt_pump_mix = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			send_status_to_server();
			create_logs("Pump_Mix", status, isCommandFromApp);
		}
	}
	else if ((pin == PUMP_FLOOR) /*&& (stt_pump_floor != status)*/) {
		if (isWaterEmpty() && status == ON) {
			mqtt_publish(("Mushroom/DEBUG/" + HubID).c_str(), "WATER EMPTY, CAN NOT ON PUMP_FLOOR");
			return ;
		}
		t_pump_floor_change = millis();
		stt_pump_floor = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			send_status_to_server();
			create_logs("Pump_Floor", status, isCommandFromApp);
		}
	}
	else if ((pin == FAN_MIX) /*&& (stt_fan_mix != status)*/) {
		t_fan_mix_change = millis();
		stt_fan_mix = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			send_status_to_server();
			create_logs("Fan_Mix", status, isCommandFromApp);
		}
	}
	else if ((pin == FAN_WIND) /*&& (stt_fan_wind != status)*/) {
		t_fan_wind_change = millis();
		stt_fan_wind = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			send_status_to_server();
			create_logs("Fan_Wind", status, isCommandFromApp);
		}
	}
	else if ((pin == LIGHT) /*&& (stt_light != status)*/) {
		t_light_change = millis();
		stt_light = status;
		out(pin, status ? ON : OFF);
		if (update_to_server) {
			send_status_to_server();
			create_logs("Light", status, isCommandFromApp);
		}
	}
}

void send_status_to_server() {
	if (!mqtt_client.connected()) {
		return;
	}
	static unsigned long num_update = 0;
	DEBUG.println(("send_status_to_server"));

	StaticJsonBuffer<500> jsBuffer;
	JsonObject& jsStatus = jsBuffer.createObject();
	jsStatus["HUB_ID"] = HubID;
	jsStatus["MIST"] = stt_pump_mix ? on_ : off_;
	jsStatus["FAN"] = stt_fan_mix ? on_ : off_;
	jsStatus["LIGHT"] = stt_light ? on_ : off_;
	jsStatus["CMD_ID"] = "HW-" + String(++num_update);

	String jsStatusStr;
	jsStatusStr.reserve(150);
	jsStatus.printTo(jsStatusStr);
	mqtt_publish("Mushroom/Commands/" + HubID, jsStatusStr, true);
}

bool skip_auto_light = false;
bool skip_auto_pump_mix = false;
bool skip_auto_fan_mix = false;
bool skip_auto_fan_wind = false;
void auto_control() {
	//https://docs.google.com/document/d/1wSJvCkT_4DIpudjprdOUVIChQpK3V6eW5AJgY0nGKGc/edit
	//https://prnt.sc/j2oxmu https://snag.gy/6E7xhU.jpg

	//auto trở lại sau khi điều khiển 5 phút
	unsigned long t_auto_return = 1 * 60000;
	if (skip_auto_light && (millis() - t_light_change) > t_auto_return) {
		skip_auto_light = false;
	}
	if (skip_auto_pump_mix && (millis() - t_pump_mix_change) > t_auto_return) {
		skip_auto_pump_mix = false;
	}
	if (skip_auto_fan_mix && (millis() - t_fan_mix_change) > t_auto_return) {
		skip_auto_fan_mix = false;
	}
	if (skip_auto_fan_wind && (millis() - t_fan_wind_change) > t_auto_return) {
		skip_auto_fan_wind = false;
	}
	//==============================================================

	//+ LIGHT tự tắt sau 60 phút
	if ((millis() - t_light_change) > (60 * 1000 * SECS_PER_MIN)) {
		skip_auto_light = false;
		if (stt_light) {
			DEBUG.println("AUTO LIGHT OFF");
			control(LIGHT, false, true, false);
		}
	}

	bool pump_floor_on = false;
	//+ PUMP_MIX tự tắt sau 180s
	if (hour() >= 11 && hour() < 15) {
		if ((millis() - t_pump_mix_change) > (6 * 60 * 1000)) {
			skip_auto_pump_mix = false;
			if (stt_pump_mix) {
				DEBUG.println("AUTO PUMP_MIX OFF");
				control(PUMP_MIX, false, true, false);

				if (flag_schedule_pump_floor) {
					pump_floor_on = true;
				}
			}
		}
	}
	else if ((millis() - t_pump_mix_change) > (180 * 1000)) {
		skip_auto_pump_mix = false;
		if (stt_pump_mix) {
			DEBUG.println("AUTO PUMP_MIX OFF");
			control(PUMP_MIX, false, true, false);

			if (flag_schedule_pump_floor) {
				pump_floor_on = true;
			}
		}
	}

	if (pump_floor_on) {
		DEBUG.println("AUTO PUMP_FLOOR OFF");
		control(PUMP_FLOOR, true, true, false);
	}
	//+ PUMP_FLOOR tự tắt sau 90s
	if ((millis() - t_pump_floor_change) > 90000) {
		if (stt_pump_floor) {
			DEBUG.println("AUTO PUMP_FLOOR OFF");
			control(PUMP_FLOOR, false, true, false);
		}
	}
	//+ FAN_MIX tự tắt sau 185s
	if (hour() >= 11 && hour() < 15) {
		if ((millis() - t_fan_mix_change) > (6 * 60 * 1000) + 5) {
			skip_auto_fan_mix = false;
			if (stt_fan_mix) {
				DEBUG.println("AUTO FAN_MIX OFF");
				control(FAN_MIX, false, true, false);
			}
		}
	}
	else if ((millis() - t_fan_mix_change) > 185000) {
		skip_auto_fan_mix = false;
		if (stt_fan_mix) {
			DEBUG.println("AUTO FAN_MIX OFF");
			control(FAN_MIX, false, true, false);
		}
	}
	//+ FAN_WIND tự tắt sau 10 phút
	if ((millis() - t_fan_wind_change) > (10 * 1000 * SECS_PER_MIN)) {
		skip_auto_fan_wind = false;
		if (stt_fan_wind) {
			DEBUG.println("AUTO FAN_WIND OFF");
			control(FAN_WIND, false, true, false);
		}
	}

	////+ LCD BACKLIGHT tự tắt sau 2 phút
	//if (stt_lcd_backlight && (millis() - t_lcd_backlight_change) > (2 * 1000 * SECS_PER_MIN)) {
	//	DEBUG.println("AUTO LCD BACKLIGHT OFF");
	//	lcd.noBacklight();
	//}
	//==============================================================
	//1/ Bật tắt đèn
	if (!skip_auto_light) {
		if (library && (!stt_light) && (light != -1 && light < LIGHT_MIN) && (hour() >= 6) && (hour() <= 18)) {
			DEBUG.println("AUTO LIGHT ON");
			control(LIGHT, true, true, false);
		}
		else if (library && (light != -1 && light > LIGHT_MAX) && stt_light) {
			DEBUG.println("AUTO LIGHT OFF");
			control(LIGHT, false, true, false);
		}
	}
	//-------------------

	//2. Bật tắt phun sương
	//a. Phun trực tiếp vào phôi vào lúc 6h và 16h
	if (((hour() == 6) || (hour() == 16)) && (minute() == 0) && (second() == 0 || second() == 1)) {
		skip_auto_pump_mix = true;
		skip_auto_fan_mix = true;
		DEBUG.println("AUTO FAN_MIX ON");
		control(FAN_MIX, true, true, false);
		DEBUG.println("AUTO PUMP_MIX ON");
		control(PUMP_MIX, true, true, false);


		//DEBUG.println("AUTO PUMP_FLOOR ON");
		//control(PUMP_FLOOR, true, true, false);
		//DEBUG.println("AUTO FAN_WIND ON");
		//control(FAN_WIND, true, true, false);
	}

	//b. Phun sương làm mát, duy trì độ ẩm. Thời gian bật: 90s, mỗi lần check điều kiện cách nhau 30 phút.
	if (!skip_auto_pump_mix && library && ((temp != -1 && temp > TEMP_MAX) && (humi != -1 && humi < HUMI_MIN)) && ((millis() - t_pump_mix_change) > (30 * 1000 * SECS_PER_MIN))/* && !stt_pump_mix*/) {
		if (now() > DATE_HAVERST_PHASE) {
			DEBUG.println("AUTO PUMP_MIX ON");
			control(PUMP_MIX, true, true, false);
			DEBUG.println("AUTO FAN_MIX ON");
			control(FAN_MIX, true, true, false);

			flag_schedule_pump_floor = true;
		}
		else {
			DEBUG.println("AUTO PUMP_FLOOR ON");
			control(PUMP_FLOOR, true, true, false);
			DEBUG.println("AUTO FAN_MIX ON");
			control(FAN_MIX, true, true, false);
		}
	}
	//-------------------

	//c. Bật tắt quạt
	//Bật quạt FAN_MIX mỗi 15 phút
	if (!skip_auto_fan_mix && ((millis() - t_fan_mix_change) > (15 * 1000 * SECS_PER_MIN)) && !stt_fan_mix) {
		DEBUG.println("AUTO FAN_MIX ON");
		control(FAN_MIX, true, true, false);
	}

	//FAN_WIND bật nếu thỏa điều kiện, mỗi lần check cách nhau 30 phút
	if (!skip_auto_fan_wind && library && ((humi != -1 && humi > HUMI_MAX) || (temp != -1 && temp > TEMP_MAX)) && ((millis() - t_fan_wind_change) > (30 * 1000 * SECS_PER_MIN)) && !stt_fan_wind) {
		DEBUG.println("AUTO FAN_WIND ON");
		control(FAN_WIND, true, true, false);
	}

	//TESTCASE 4, mỗi lần check cách nhau 30 phút
	if (library && ((humi != -1 && humi < HUMI_MIN) && (temp != -1 && temp < TEMP_MIN)) && ((millis() - t_pump_mix_change) > (30 * 1000 * SECS_PER_MIN))) {
		DEBUG.println("AUTO FAN_WIND OFF");
		control(FAN_WIND, false, true, false);

		DEBUG.println("AUTO PUMP_MIX ON");
		control(PUMP_MIX, true, true, false);

		DEBUG.println("AUTO FAN_MIX ON");
		control(FAN_MIX, true, true, false);
	}

	wait(1);
}

void updateFirmware(String url) {
	{
		StaticJsonBuffer<500> jsBuffer;
		JsonObject& jsProMicro = jsBuffer.createObject();
		jsProMicro["cmd"] = "uf";
		String strProMicro;
		jsProMicro.printTo(strProMicro);
		LCD_UART.println(strProMicro);
		DEBUG.println(strProMicro);
	}
	//lcd.begin();
	////lcd.begin(LCD_SDA, LCD_SCL);
	//lcd.setCursor(3, 0);
	//lcd.print("MUSHROOM-" + HubID);
	//lcd.setCursor(1, 2);
	//lcd.print("FIRMWARE  UPDATING");



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

	wait(1);
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
		skip_auto_pump_mix = true;
		control(PUMP_MIX, true, true, true);
	}
	if (cmd.indexOf("MIST OFF") > -1) {
		skip_auto_pump_mix = true;
		control(PUMP_MIX, false, true, true);
	}

	if (cmd.indexOf("FAN_MIX ON") > -1) {
		skip_auto_fan_mix = true;
		control(FAN_MIX, true, true, true);
	}
	if (cmd.indexOf("FAN_MIX OFF") > -1) {
		skip_auto_fan_mix = true;
		control(FAN_MIX, false, true, true);
	}
	if (cmd.indexOf("RESET WIFI") > -1) {
		DEBUG.println(("Reset Wifi"));
		WiFi.disconnect();
	}
}

#pragma endregion



