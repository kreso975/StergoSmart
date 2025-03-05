// settings.h
#ifndef SETTINGS_H
#define SETTINGS_H
/*
 * For What Device We need to Compile
 * 
 * STERGO_PROGRAM :
 *
 * PowerSwitch                  = 0
 * StergoWeather BME280         = 1
 * TicTacToe                    = 2
 * StergoWeather+PowerSwitch    = 3
 * StergoWeather DHT22          = 4
 * StergoWeather DS18B20        = 5
 */
#define STERGO_PROGRAM 1

// Screen or Led On device WS001 = Second 0 == device type
// example: WS014 = WeatherStation 1 = LED 8x32, 4 = DHT22
#define STERGO_SCREEN 1

/*
 * STERGO_PROGRAM_BOARD :
 * 
 * ESP8266 default 01S = 1   // v01
 * LOLIN D1 mini       = 2   // v02
 * ESP32C6             = 3   // v03
 */
#define STERGO_PROGRAM_BOARD 2

/*
 * STERGO_PLUG :
 * 
 * Native Board relay = 1   // Relay Switch - RS
 * Sonoff S26         = 2   // Plug Switch  - PS
 * Sonoff T4EU1C      = 3   // Light Switch - LS
 */
#define STERGO_PLUG 1

// Firmware Version always part of this file
#define FW_VERSION "000.06.004"                 // Check releaseLog for details
#define MODEL_FRENDLY_NAME "Stergo Smart"
#define COMPANY_URL "http://www.stergo.hr"

// Excluded Code to be included
// 0 = None to be included
#define EXCLUDED_CODE 1

// 1 true | 0 false  / Serial.print 
#define DEBUG 0


#endif // SETTINGS_H