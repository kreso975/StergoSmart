#define HISTORY_FILE "/history.json"

// 		MODEL_NUMBER "v01" ( ESP8266 default 01S)
//    MODEL_NUMBER "v02" ( LOLIN D1 mini)
#if ( STERGO_PROGRAM == 1 )
  #define MODEL_NAME "WS001"
#endif
#if ( STERGO_WEATHER == 1 )
  #define MODEL_NUMBER "v01"
#elif ( STERGO_WEATHER == 2 )
  #define MODEL_NUMBER "v02"
#endif

#if ( STERGO_PROGRAM == 3 )
  #define MODEL_NAME "WS002"
  #define MODEL_NUMBER "v01"
#endif

#define sizeHist 100                      // History size = nr of records (24h x 4pts)

// BME280 GPIOs 2 (SDA),0 (SCL) are used for BME280
#define GPIO_SDA 2
#define GPIO_SCL 0
#define BMEaddr 0x76 //BME280 address not all running on same address 0x76 || 0x77
  
bool detectModule = false;                // Was detectBME280 use True if moduledetection not needed


//*************************************************************************************************************
/*
 * p-adjust : adjust pressuer to location altitude by Sealevel adjustement | 0 = No | 1 = yes
 * pa_unit  : use meter or feet as unit | 0 = Meter | 1 = Feet
 * pl_adj   : value of adjustment
 * t = Temperature, h = Humidity, p = Pressure, dp = dewPoint, P0 = Adjusted Pressure to Location Altitude
 * t_measure : 0 = Meter | 1 = Feet;  p_measure : 0 = hPa | 1 = inhg;
 *
 * struct Config {
 *	  byte t_measure						  =	0;
 *	  byte p_measure						  =	0;
 *	  byte p_adjust						    =	1;
 *	  byte pa_unit						    =	0;
 *	  int pl_adj							    =	122;
 * };
 *
*/

byte pa_unit, t_measure, p_measure, p_adjust;
int pl_adj;
float h, t, p, dp, P0;
  
// Time Interval for reading data from Sensor
bool measureFirstRun = true;
#define measureInterval 30e3                                // in miliseconds = 30 * 1000 (e3 = 3 zeros) = 15sec
unsigned long lastMeasureInterval = measureInterval;        // time of last point added

// Time interval for Logging data into history.json
#define intervalHist 1000 * 60 * 15                         // 4 measures / hours - orig 1000 * 60 * 15 - 15min
unsigned long previousMillis = intervalHist;                // time of last point added

//MQTT Topics used from config.json
char mqtt_bme280Humidity[120];
char mqtt_bme280Temperature[120];
char mqtt_bme280Pressure[120];

Adafruit_BME280 bme;
