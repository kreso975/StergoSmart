// settings.h
#ifndef SETTINGS_H
#define SETTINGS_H
/*
 * For What Device We need to Compile
 * 
 * STERGO_PROGRAM :
 *
 * PowerSwitch                  = 0
 * StergoWeather BME280         = 1 // Must configure in modules BME280/BME280.h set GPIO_SDA and GPIO_SCL
 * TicTacToe                    = 2
 * StergoWeather+PowerSwitch    = 3
 * StergoWeather DHT22          = 4
 * StergoWeather DS18B20        = 5
 */
#define STERGO_PROGRAM 1

// BME280 GPIOs 2 (SDA),0 (SCL) are used for BME280
// 72 (2,0) 74 (4,5)
#define GPIO_SDA 2
#define GPIO_SCL 0
#define BMEaddr 0x76 //BME280 address not all running on same address 0x76 || 0x77

// DHT MODELS
//#define DHTTYPE "DHT22"
//#define DHTTYPE DHT21     // DHT 21 (AM2301
#define DHTPIN 0
#define DHTTYPE DHT22       // DHT 22 (AM2302), AM2321

// Screen or Led On device WS001 = Second 0 == device type
// example: WS014 = WeatherStation 1 = LED 8x32, 4 = DHT22
#define STERGO_SCREEN 1
#define LED_PIN 0
// Matrix LED orientation
// 78 bedroom clock = 0, 74 = 1
// 0 = Normal
// 1 = Diagonal flip (0 and 256 are on opposite diagonal side)
#define MATRIX_ORIENTATION 1

/*
 * STERGO_PROGRAM_BOARD :
 * 
 * ESP8266 default 01S = 1   // v01
 * LOLIN D1 mini       = 2   // v02
 * ESP32C6             = 3   // v03
 * ESP32C3             = 4   // v04
 * ESP32dev            = 5   // v05
 */
#define STERGO_PROGRAM_BOARD 2

/*
 * STERGO_PLUG :
 * 
 * Native Board relay = 1   // Relay Switch       - RS
 * Sonoff S26         = 2   // Plug Switch        - PS
 * Sonoff T4EU1C      = 3   // Light Switch       - LS
 * Izymo Transmitter  = 4   // Izymo Transmitter  - IT
 */
#define STERGO_PLUG 4

// Firmware Version always part of this file
#define FW_VERSION "000.06.005"
#define MODEL_FRENDLY_NAME "Stergo Smart"
#define COMPANY_URL "http://www.stergo.hr"


// 1 true | 0 false  / Serial.print 
#define DEBUG 0


#endif // SETTINGS_H