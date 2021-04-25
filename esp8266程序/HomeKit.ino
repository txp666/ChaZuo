// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       HomeKit.ino
    Created:	2021/4/3 19:20:46
    Author:     DESKTOP-C3QC8JH\txp
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts


#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"
#include <SoftwareSerial.h>


SoftwareSerial mySerial(5, 4); // RX, TX软串口
char trans;
bool* a;
bool state=&a;
#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
#define PIN_SWITCH 12//继电器引脚
void setup() {
	Serial.begin(115200);
	mySerial.begin(9600);
	wifi_connect(); // in wifi_info.h
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
	my_homekit_setup();

}

void loop() {

	if (mySerial.available())
	{
		trans = char(mySerial.read());//作为字符串接收字符，强制类型转换
		Serial.println(trans);	
		if (trans == '1') state = 1; 
		if (trans == '0') state = 0;
		if (trans == '2') state = !state;//按键按下状态更改
		mySerial.println(state);
	}
	digitalWrite(PIN_SWITCH, state);
	my_homekit_loop();
	mySerial.println(state);
	
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;

static uint32_t next_heap_millis = 0;



//Called when the switch value is changed by iOS Home APP
void cha_switch_on_setter(const homekit_value_t value) {
	state = value.bool_value;
	cha_switch_on.value.bool_value = state;	
	//LOG_D("Switch: %s", on ? "ON" : "OFF");
	//digitalWrite(PIN_SWITCH, on ? LOW : HIGH);	
	
}

void my_homekit_setup() {
	pinMode(PIN_SWITCH, OUTPUT);
	digitalWrite(PIN_SWITCH, LOW);

	//Add the .setter function to get the switch-event sent from iOS Home APP.
	//The .setter should be added before arduino_homekit_setup.
	//HomeKit sever uses the .setter_ex internally, see homekit_accessories_init function.
	//Maybe this is a legacy design issue in the original esp-homekit library,
	//and I have no reason to modify this "feature".
	cha_switch_on.setter = cha_switch_on_setter;
	arduino_homekit_setup(&config);

	//report the switch value to HomeKit if it is changed (e.g. by a physical button)
	//bool switch_is_on = true/false;
	//cha_switch_on.value.bool_value = switch_is_on;
	//homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

void my_homekit_loop() {
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		// show heap info every 5 seconds
		next_heap_millis = t + 5 * 10;
		LOG_D("Free heap: %d, HomeKit clients: %d",
			ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}

