#ifndef WEATHER_H
#define WEATHER_H

/*
* STERGO_PROGRAM :
 *
 * PowerSwitch                  = 0
 * StergoWeather BME280         = 1
 * ticTacToe                    = 2
 * StergoWeather+PowerSwitch    = 3
 * StergoWeather DHT22          = 4
 * StergoWeather DS18B20        = 5

  MODEL_NUMBER "v01" ( ESP8266 default 01S )
  MODEL_NUMBER "v02" ( LOLIN D1 mini )
  MODEL_NUMBER "v03" ( ESP32 )
  Screen or Led On device WS001 = Second 0 == device type
  FE: WS014 = WeatherStation 1 = LED 8x32, 4 = DHT22
*/
#include "Config.h"
#define MODEL_PREFIX "WS0"

#if ( STERGO_PROGRAM == 1 )
  //#define MODEL_NAME "WS001"
  #define MODEL_NAME MODEL_PREFIX TOSTRING(STERGO_SCREEN) "1"
  #define MODULE_BME280
  #include "./modules/BME280/BME280.h"
  #include "./modules/BME280/BME280.cpp"
#endif

#if ( STERGO_PROGRAM == 3 )
    #define MODEL_NAME MODEL_PREFIX TOSTRING(STERGO_SCREEN) "2"
    #define MODULE_BME280
    #include "./modules/BME280/BME280.h"
    #include "./modules/BME280/BME280.cpp"
#elif ( STERGO_PROGRAM == 4 )
    #define MODEL_NAME MODEL_PREFIX TOSTRING(STERGO_SCREEN) "4"
    #define MODULE_DHT
    #include "./modules/DHTsensors/DHTsensors.h"
    #include "./modules/DHTsensors/DHTsensors.cpp"
#elif ( STERGO_PROGRAM == 5 )
    #define MODEL_NAME MODEL_PREFIX TOSTRING(STERGO_SCREEN) "5"
    #define MODULE_DS18B20
    #include "./modules/DS18B20/DS18B20.h"
    #include "./modules/DS18B20/DS18B20.cpp"
#endif

#define HISTORY_FILE "/history.json"
#define sizeHist 100                      // History size = nr of records (24h x 4pts)

void sendMeasures(bool webhook = false);

#endif // WEATHER_H