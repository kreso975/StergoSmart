#ifdef MODULE_DS18B20

#include "DS18B20.h"

bool detectModule = false;
bool measureFirstRun = true;

byte t_measure;
float t;
unsigned long lastMeasureInterval = measureInterval;
unsigned long previousMillis = intervalHist;

char mqtt_Temperature[120];

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

extern bool writeLogFile(String message, int newLine, int output);
extern float Fahrenheit( float celsius );
extern float Kelvin( float celsius );

/* ======================================================================
Function: setupDS18B20
Purpose : Initialize DS18B20
Input   : 
Output  : 
Comments: -
====================================================================== */
bool setupDS18B20()
{
	// Init DHT - need to find a way to know is it running properly
	sensors.begin();
	delay(500);
	sensors.requestTemperatures(); // Send the command to get temperatures 
	if ( !sensors.getDeviceCount() > 0 ) 
	{ 
		writeLogFile( F("DS18B20 sensor is not connected!"), 1, 3 );
		//detectModule = false;
		return false;
	} 
	
	writeLogFile( F("DS18B20 OK"), 1, 3 );
	detectModule = true;
	delay(2000);
	return true;
}

/* ======================================================================
Function: getWeatherDS18B20
Purpose : Read Sensor data
Input   : 
Output  : 
Comments: 
====================================================================== */
void getWeatherDS18B20()
{
    sensors.requestTemperatures();
	float t_tmp;
	t_tmp = sensors.getTempCByIndex(0);
    
    //float f = sensors.getTempFByIndex(0);
}
#endif