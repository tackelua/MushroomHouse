LiquidCrystal_I2C lcd(0x3f, 20, 04);
Ticker lcdShowTime;
int current_lcd_line_0 = -1;

int lcd_print(String buffer) {
	lcd.print(buffer);
	return lcd.getWriteError();
}
void lcd_init() {
	//lcd.begin(12, 14);
	DEBUG.println("LCD Init");
	//lcd_result_transmit = 0;
	digitalWrite(SDA, LOW);
	digitalWrite(SCL, LOW);
	delay(10);
	lcd.begin();
	//lcd.begin(LCD_SDA, LCD_SCL);
	lcd.backlight();

	flag_lcd_line_0 == SHOW_HUBID;
	current_lcd_line_0 = -1;
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
		if (lcd_print(timeStr)) {
			DEBUG.println("LCD ERROR");
			mqtt_publish("DEBUG" + HubID, "LCD ERROR");
			lcd_init();
		}
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
	if (flag_lcd_line_0 == NOTI_CONNECTION_LOST) {
		if (current_lcd_line_0 != NOTI_CONNECTION_LOST) {
			current_lcd_line_0 = NOTI_CONNECTION_LOST;
			lcd.setCursor(0, 0);
			lcd.print("  CONNECTION  LOST  ");		
			
			StaticJsonBuffer<200> jsBuffer;
			JsonObject& jsProMicro = jsBuffer.createObject();
			jsProMicro["cmd"] = "l0";
			jsProMicro["dt"] = "  CONNECTION  LOST  ";
			String strProMicro;
			jsProMicro.printTo(strProMicro);
			ProMicro.println(strProMicro);
			DEBUG.println(strProMicro);
		}
	}
	else if (flag_lcd_line_0 == NOTI_WATER_EMPTY) {
		if (current_lcd_line_0 != NOTI_WATER_EMPTY) {
			current_lcd_line_0 = NOTI_WATER_EMPTY;
			lcd.setCursor(0, 0);
			lcd.print(" ALARM WATER EMPTY! ");

			StaticJsonBuffer<200> jsBuffer;
			JsonObject& jsProMicro = jsBuffer.createObject();
			jsProMicro["cmd"] = "l0";
			jsProMicro["dt"] = " ALARM WATER EMPTY! ";
			String strProMicro;
			jsProMicro.printTo(strProMicro);
			ProMicro.println(strProMicro);
			DEBUG.println(strProMicro);
		}
	}
	else if (current_lcd_line_0 != SHOW_HUBID) {
		current_lcd_line_0 = SHOW_HUBID;
		lcd.setCursor(0, 0);
		lcd.print("   MUSHROOM-" + HubID + "   ");

		StaticJsonBuffer<200> jsBuffer;
		JsonObject& jsProMicro = jsBuffer.createObject();
		jsProMicro["cmd"] = "l0";
		jsProMicro["dt"] = " ALARM WATER EMPTY! ";
		String strProMicro;
		jsProMicro.printTo(strProMicro);
		ProMicro.println(strProMicro);
		DEBUG.println(strProMicro);
	}
}

//void lcd_repair() {
//	if (lcd_result_transmit != 0) {
//		switch (lcd_result_transmit)
//		{
//		case 1:
//			DEBUG.println("LCD_ERROR: data too long to fit in transmit buffer");
//			mqtt_publish("DEBUG" + HubID, "LCD_ERROR: data too long to fit in transmit buffer");
//			break;
//		case 2:
//			DEBUG.println("LCD_ERROR: received NACK on transmit of address");
//			mqtt_publish("DEBUG" + HubID, "LCD_ERROR: received NACK on transmit of address");
//			break;
//		case 3:
//			DEBUG.println("LCD_ERROR: received NACK on transmit of data");
//			mqtt_publish("DEBUG" + HubID, "LCD_ERROR: received NACK on transmit of data");
//			break;
//		case 4:
//			DEBUG.println("LCD_ERROR: other error");
//			mqtt_publish("DEBUG" + HubID, "LCD_ERROR: other error");
//			break;
//		default:
//			break;
//		}
//		lcd_init();
//	}
//}
