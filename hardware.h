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

#define RELAY1			14
#define RELAY2			27
#define RELAY3			26	
#define RELAY4			25
#define RELAY5			33
#define RELAY6			32
#define RELAY7			35
#define RELAY8			34

#define PUMP_MIX		RELAY1 //Phun sương
#define PUMP_FLOOR		RELAY2 //Phun ẩm sàn nhà
#define FAN_MIX			RELAY3 //Quạt đối lưu
#define FAN_WIND		RELAY4 //Quạt thông gió
#define	LIGHT			RELAY5 //Đèn chiếu sáng

#define LED_STT			23
#define BUTTON			16 //RX2

#define LCD_SCL			SCL
#define LCD_SDA			SDA

#define SHT1_DAT		17 //TX2
#define SHT1_CLK		5
#define SHT2_DAT		18
#define SHT2_CLK		19

#define SS_LIGHT		4
#define SS_WATER_LOW	13

//======================================
#define ON	HIGH 
#define OFF	LOW

extern bool stt_pump_mix;
extern bool stt_pump_floor;
extern bool stt_fan_mix;
extern bool stt_fan_wind;
extern bool stt_light;
extern bool stt_led;

extern String HubID;
extern Button myBtn;

void hardware_init();
String getMacAddress();
#endif

