#ifndef _HARDWARE_h
#define _HARDWARE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "esp_system.h"
#include <Button.h>

#define DEBUG Serial

#define RELAY1			13
#define RELAY2			14
#define RELAY3			27	
#define RELAY4			26
#define RELAY5			25
#define RELAY6			33
#define RELAY7			32

#define PUMP_BOTH		RELAY1 //Bơm phun nước cho MIX và FLOOR
#define PUMP_MIX		RELAY2 //Van phun sương
#define PUMP_FLOOR		RELAY3 //Van phun ẩm sàn nhà
#define	LIGHT			RELAY4 //Đèn chiếu sáng
#define FAN_MIX			RELAY5 //Quạt đối lưu
#define FAN_WIND		RELAY6 //Quạt thông gió
#define ALARM_SIRENS	RELAY6 //Còi báo động

#define LED_STT			23
#define BUTTON			4

#define LCD_SCL			SCL
#define LCD_SDA			SDA

#define SHT1_DAT		18
#define SHT1_CLK		19

#define SS_WATER_LOW	5

//======================================
#define ON	HIGH 
#define OFF	LOW

extern bool stt_pump_mix;
extern bool stt_pump_floor;
extern bool stt_fan_mix;
extern bool stt_fan_wind;
extern bool stt_light;
extern bool stt_led;
extern bool stt_lcd_backlight;

extern String HubID;
extern Button myBtn;

void hardware_init();
String getMacAddress();
#endif

