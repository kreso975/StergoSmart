#ifdef MODUL_DS18B20

/* ======================================================================
Function: setupDS18B20
Purpose : Initialize DS18B20
Input   : 
Output  : 
Comments: -
====================================================================== */
bool setupDS18B20()
{
	// Init DHT
	if ( !sensors.begin() )
	{
		writeLogFile( F("No DS18B20 sensor, check wiring!"), 1, 3 );
		detectModule = false;
		return false;
	}
	writeLogFile( F("DS18B20 OK"), 1, 3 );
	detectModule = true;
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