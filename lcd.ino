LiquidCrystal_I2C lcd(0x3f, 20, 04);
Ticker lcdShowTime;

void lcd_init() {
	//lcd.begin(12, 14);
	lcd.begin(LCD_SDA, LCD_SCL);
	lcd.backlight();
	lcd.setCursor(3, 0);
	lcd.print("MUSHROOM-" + HubID);
	lcdShowTime.attach_ms(1000, lcd_print_time);
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
void lcd_print_time() {
	lcd.setCursor(6, 1);
	lcd.print(getTimeStr());
}

void lcd_print_sensor(float temp, float humi) {
	//char lineData[20] = { 0 };
	//sprintf(lineData, "TEMP: %4d   HUMI: %4d", temp, humi);
	//sprintf(lineData, "TEMP: %4.2f    HUMI: %4.2f", temp, humi);

	lcd.setCursor(6, 2);
	lcd.print("TEMP: " + String(temp, 0));
	lcd.setCursor(6, 3);
	lcd.print("TEMP: " + String(humi, 0));
}