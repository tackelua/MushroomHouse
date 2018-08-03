unsigned long myChannelNumber = 548289;
const char * myWriteAPIKey = "2IYKQV667YMMYC28";

WiFiClient  thingspkeak_client;

void ThingSpeak_init() {
	ThingSpeak.begin(thingspkeak_client);
}
void thingspeak_update(float temp, float humi, float light) {
	bool write = false;
	if (temp != -1) {
		write = true;
		ThingSpeak.setField(1, temp);
	}
	if (humi != -1) {
		write = true;
		ThingSpeak.setField(2, humi);
	}
	if (light != -1) {
		write = true;
		ThingSpeak.setField(3, light);
	}
	if (write) {
		ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
	}
}