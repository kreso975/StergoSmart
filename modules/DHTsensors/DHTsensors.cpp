#include "../../settings.h"
#ifdef MODULE_DHT
#include "DHTsensors.h"

bool detectModule = false;
bool measureFirstRun = true;

byte t_measure;
float h;
float t;
float dp;
unsigned long lastMeasureInterval = measureInterval;
unsigned long previousMillis = intervalHist;

char mqtt_Temperature[120];
char mqtt_Humidity[120];

int mqtt_interval = 120;
unsigned long mqtt_intervalHist;
unsigned long mqtt_previousMillis;

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

extern bool writeLogFile(String message, int newLine, int output);
extern float Fahrenheit( float celsius );
extern float Kelvin( float celsius );


/* ======================================================================
Function: setupDHT
Purpose : Initialize DHT 11, 12, 22, 21
Input   : 
Output  : 
Comments: - */
bool setupDHT()
{
	// Init DHT - need to find a way to know is it running properly
	dht.begin();
	float t = dht.readTemperature(); // Check if any reads failed and exit early (to try again) 
	if ( isnan(t) )
	{
		writeLogFile( F("DHT sensor is not connected!"), 1, 3 );
		//detectModule = false;
		return false;
	}
	writeLogFile( F("DHT OK"), 1, 3 );
	delay(2000);
	detectModule = true;
	return true;
}

/* ======================================================================
Function: getWeatherDHT
Purpose : Read Sensor data
Input   : 
Output  : 
Comments: */
void getWeatherDHT()
{
	float h_tmp, t_tmp;
	t_tmp = dht.readTemperature();
    
    // Read temperature as Fahrenheit ( t_measure == 1 )
    //float f = dht.readTemperature(true);

	h_tmp = round( dht.readHumidity() * 100 ) / 100.0F;
	
	// If we start getting totaly wrong readings
	if ( ( t_tmp == 0 && h_tmp == 0 ) || isnan(h_tmp) || isnan(t_tmp) || t_tmp > 100 || t_tmp < -60 )
	{
		writeLogFile( F("Fail read DHT!"), 1, 3 );
		return;
	}
	else
	{
		t = t_tmp;
		h = h_tmp;
		dp = t-0.36*(100.0-h);

		if ( t_measure == 1 )
			t = Fahrenheit(t);
	
	    float T = Kelvin(t); // Kelvin             
  	}
}

#endif