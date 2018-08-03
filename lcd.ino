LiquidCrystal_I2C lcd(0x3f, 20, 04);
Ticker lcdShowTime;

void lcd_init() {
	//lcd.begin(12, 14);
	lcd.begin(LCD_SDA, LCD_SCL);
	lcd.backlight();

	flag_lcd_line_0 == SHOW_HUBID;
	lcd_show_line_0();

	lcd.setCursor(0, 2);
	lcd.print("TEMP    HUMI   LIGHT");

	static bool enable_timmer = false;
	if (!enable_timmer) {
		enable_timmer = true;
		lcdShowTime.attach_ms(1000, lcd_print_time);
	}
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

void lcd_print_time_set_flag() {
	flag_print_time = true;
}
void lcd_print_time() {
	flag_print_time = false;
	if (now() > 1000000000) {
		lcd.setCursor(0, 1);
		String timeStr = getTimeStr();
		lcd.print(timeStr);
		DEBUG.println(timeStr);
	}
	else {
		lcd.setCursor(0, 1);
		lcd.print("      Starting      ");
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

void lcd_print_sensor(float temp, float humi, int light) {
	//lcd_generate_frame(temp, humi, light);
	lcd.setCursor(1, 3);
	lcd.print(int(temp)); lcd.print("   ");
	lcd.setCursor(9, 3);
	lcd.print(int(humi)); lcd.print("   ");
	lcd.setCursor(16, 3);
	lcd.print(light); lcd.print("   ");
	return;
}

void lcd_show_line_0() {
	static int current_lcd_line_0 = -1;
	if (flag_lcd_line_0 == NOTI_CONNECTION_LOST) {
		if (current_lcd_line_0 != NOTI_CONNECTION_LOST) {
			current_lcd_line_0 = NOTI_CONNECTION_LOST;
			lcd.setCursor(0, 0);
			lcd.print("  CONNECTION  LOST  ");
		}
	}
	else if (flag_lcd_line_0 == NOTI_WATER_EMPTY) {
		if (current_lcd_line_0 != NOTI_WATER_EMPTY) {
			current_lcd_line_0 = NOTI_WATER_EMPTY;
			lcd.setCursor(0, 0);
			lcd.print(" ALARM WATER EMPTY! ");
		}
	}
	else if (current_lcd_line_0 != SHOW_HUBID) {
		current_lcd_line_0 = SHOW_HUBID;
		lcd.setCursor(0, 0);
		lcd.print("   MUSHROOM-" + HubID + "   ");
	}
}