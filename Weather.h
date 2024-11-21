
/*
* STERGO_PROGRAM :
 *
 * PowerSwitch                  = 0
 * StergoWeather BME280         = 1
 * ticTacToe                    = 2
 * StergoWeather+PowerSwitch    = 3
 * StergoWeather DHT22          = 4
 * StergoWeather DS18B20        = 5

  MODEL_NUMBER "v01" ( ESP8266 default 01S)
  MODEL_NUMBER "v02" ( LOLIN D1 mini)

  Screen or Led On device WS001 = Second 0 == device type
  FE: WS014 = WeatherStation 1 = LED 8x32, 4 = DHT22
*/

#define MODEL_PREFIX "WS0"

#if ( STERGO_PROGRAM == 1 )
  //#define MODEL_NAME "WS001"
  #define MODEL_NAME MODEL_PREFIX TOSTRING(STERGO_SCREEN) "1"
  #define MODULE_BME280
  #include <Adafruit_BME280.h>
  #include "BME280.h"
#endif
#if ( STERGO_PROGRAM_BOARD == 1 )
  #define MODEL_NUMBER "v01"
#elif ( STERGO_PROGRAM_BOARD == 2 )
  #define MODEL_NUMBER "v02"
#endif

#if ( STERGO_PROGRAM == 3 )
    #define MODEL_NAME MODEL_PREFIX TOSTRING(STERGO_SCREEN) "2"
    #define MODULE_BME280
    #include <Adafruit_BME280.h>
    #include "BME280.h"
#elif ( STERGO_PROGRAM == 4 )
    #define MODEL_NAME MODEL_PREFIX TOSTRING(STERGO_SCREEN) "3"
    #define MODULE_DHT
    #include <DHT.h>
    #include "DHTsensors.h"
#elif ( STERGO_PROGRAM == 5 )
    #define MODEL_NAME MODEL_PREFIX TOSTRING(STERGO_SCREEN) "4"
    #define MODULE_DS18B20
    #include <OneWire.h>
    #include <DallasTemperature.h>
    #include "DS18B20.h"
#endif

#define HISTORY_FILE "/history.json"
#define sizeHist 100                      // History size = nr of records (24h x 4pts)
