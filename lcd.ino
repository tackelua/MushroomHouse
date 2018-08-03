LiquidCrystal_I2C lcd(0x3f, 20, 04);
Ticker lcdShowTime;

void lcd_init() {
	//lcd.begin(12, 14);
	lcd.begin(LCD_SDA, LCD_SCL);
	lcd.backlight();
	lcd.setCursor(3, 0);
	lcd.print("MUSHROOM-" + HubID);
	static bool enable_timmer = false;
	if (!enable_timmer) {
		enable_timmer = true;
		lcdShowTime.attach_ms(1000, lcd_print_time_set_flag);
	}
}

String getTimeStr() {
	String strTime;
	strTime = hour() < 10 ? String("0") + hour() : hour();
	strTime += ":";
	strTime += minute() < 10 ? String("0") + minute() : minute();
	strTime += ":";
	strTime += second() < 10 ? String("0") + second() : second();
	return strTime;
}

void lcd_print_time_set_flag() {
	flag_print_time = true;
}
void lcd_print_time() {
	flag_print_time = false;
	lcd.setCursor(6, 1);
	if (now() > 1000000000) {
		lcd.print(getTimeStr());
	}
	else {
		lcd.print("Starting");
	}
	//if ((hour() > 6 && hour() < 18)) {
	//	if (lcd_backlight) {
	//		lcd_backlight = false;
	//		lcd.noBacklight();
	//	}
	//}
	//else {
	//	if (!lcd_backlight) {
	//		lcd_backlight = true;
	//		lcd.backlight();
	//	}
	//}
}

void lcd_generate_frame(float temp, float humi, int light) {
	//lcd.begin(LCD_SDA, LCD_SCL);
	lcd.clear();
	lcd.setCursor(3, 0);
	lcd.print("MUSHROOM-" + HubID);
	lcd_print_time();

	lcd.setCursor(0, 2);
	lcd.print("TEMP    HUMI   LIGHT");
	lcd.setCursor(1, 3);
	lcd.print(int(temp)); lcd.print("   ");
	lcd.setCursor(9, 3);
	lcd.print(int(humi)); lcd.print("   ");
	lcd.setCursor(16, 3);
	lcd.print(light); lcd.print("   ");
}
void lcd_print_sensor(float temp, float humi, int light) {
	lcd_generate_frame(temp, humi, light);
	return;


	static bool frame = false;
	static unsigned long t = millis();
	if (millis() - t > 60000) {
		t = millis();
		frame = false;
		lcd_init();
		lcd_print_time();
	}

	if (!frame) {
		frame = true;
		lcd.setCursor(0, 2);
		lcd.print("TEMP    HUMI   LIGHT");
	}
	lcd.setCursor(1, 3);
	lcd.print(int(temp)); lcd.print("   ");
	lcd.setCursor(9, 3);
	lcd.print(int(humi)); lcd.print("   ");
	lcd.setCursor(16, 3);
	lcd.print(light); lcd.print("   ");
}