#define HISTORY_FILE "/history.json"

#if ( STERGO_PROGRAM == 1 )
  #define MODEL_NAME "WS001"
#elif ( STERGO_PROGRAM == 3 )
  #define MODEL_NAME "WS002"
#endif

// 			MODEL_NUMBER "v01" ( ESP8266 default 01S)
//      MODEL_NUMBER "v02" ( LOLIN D1 mini)
#define MODEL_NUMBER "v01"

#define sizeHist 100                      // History size = nr of records (24h x 4pts)

// BME280 GPIOs 2 (SDA),0 (SCL) are used for BME280
#define GPIO_SDA 2
#define GPIO_SCL 0
#define BMEaddr 0x76 //BME280 address not all running on same address
  
bool detectModule = false;                // Was detectBME280 use True if moduledetection not needed

//*************************************************************************************************************
/*
 * p-adjust : adjust pressuer to location altitude by Sealevel adjustement | 0 = No | 1 = yes
 * pa_unit  : use meter or feet as unit | 0 = Meter | 1 = Feet
 * pl_adj   : value of adjustment
 * t = Temperature, h = Humidity, p = Pressure, dp = dewPoint, P0 = Adjusted Pressure to Location Altitude
 * t_measure : 0 = Meter | 1 = Feet;  p_measure : 0 = hPa | 1 = inhg;
 */
/*
struct Config {
  	byte deviceID						  =	1;
	byte deviceType						  =	1;
	char deviceName[16]					=	"";
	char moduleName[20]					=	"Weather Station - WS001V01";
	byte t_measure						  =	0;
	byte p_measure						  =	0;
	byte p_adjust						    =	1;
	byte pa_unit						    =	0;
	int pl_adj							    =	122;
	byte wifi_runAS						  =	1;
	byte wifi_hideAP					  =	0;
	char softAP_ssid[20]				=	"StergoSmart_ap";            //This value shows only if SPIFFS not loaded
	char softAP_pass[20]				=	"987654321";                 //
	char wifi_hostname[20]			=	"StergoSmart";
	byte wifi_static					  =	0;
	char wifi_StaticIP[16]			=	"";
	char wifi_ssid[20]					=	"GoAway"; // SSID in app / web
 	char wifi_password[20]			=	"wireless01";
	char wifi_gateway[16]				=	"";
	char wifi_subnet[16] 				=	"";
	char wifi_DNS[16]					  =	"";
	byte mqtt_start						  =	0;
	int mqtt_interval					  =	60;
	byte webLoc_start					  =	0;
	int webLoc_interval					=	60;
	char mqtt_server[20]				=	"192.168.1.101";
	int mqtt_port						    =	"21883";
	char mqtt_clientName[23] 			    =	"Garaza_1521214361";
	char mqtt_clientUsername[50]		  =	"8b16e04c-cd07-45c1-9525-d7831574da47";
	char mqtt_clientPassword[50]		  =	"r:feed36bdad2010009cfc0230efa46652";
	char mqtt_myTopic[120]				    =	"/home/ESP8266-1/state";
	char mqtt_bme280Humidity[120]		  =	"qiot/things/admin/Garaza/GarazaHumidity";
	char mqtt_bme280Temperature[120]	=	"qiot/things/admin/Garaza/GarazaTemperature";
	char mqtt_bme280Pressure[120]     =	"qiot/things/admin/Garaza/GarazaPressure";
};
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
  
Adafruit_BME280 bme;
