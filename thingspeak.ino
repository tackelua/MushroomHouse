unsigned long myChannelNumber = 548289;
const char * myWriteAPIKey = "2IYKQV667YMMYC28";

enum thingspeak_fields {
	ts_ss_temp = 1,
	ts_ss_humi,
	ts_ss_light,
	ts_pump_mix,
	ts_pump_floor,
	ts_fan_mix,
	ts_fan_wind,
	ts_light
};

WiFiClient  thingspkeak_client;

void ThingSpeak_init() {
	ThingSpeak.begin(thingspkeak_client);
}
void thingspeak_update(float temp, float humi, float light) {
	bool write = false;
	if (temp != -1) {
		write = true;
		ThingSpeak.setField(ts_ss_temp, temp);
	}
	if (humi != -1) {
		write = true;
		ThingSpeak.setField(ts_ss_humi, humi);
	}
	if (light != -1) {
		write = true;
		ThingSpeak.setField(ts_ss_light, light);
	}
	if (write) {
		ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
	}
}

void thingspeak_log_control(int pin, bool status) {
	//return;
	if (WiFi.isConnected()) {
		switch (pin)
		{
		case PUMP_MIX:
			ThingSpeak.setField(ts_pump_mix, status);
			ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
			return;
		case PUMP_FLOOR:
			ThingSpeak.setField(ts_pump_floor, status);
			ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
			return;
		case FAN_MIX:
			ThingSpeak.setField(ts_fan_mix, status);
			ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
			return;
		case FAN_WIND:
			ThingSpeak.setField(ts_fan_wind, status);
			ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
			return;
		case LIGHT:
			ThingSpeak.setField(ts_light, status);
			ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
			return;

		default:
			break;
		}
	}
}