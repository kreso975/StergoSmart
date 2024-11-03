#ifdef MODUL_DHT

/* ======================================================================
Function: setupDHT
Purpose : Initialize DHT 11, 12, 22, 21
Input   : 
Output  : 
Comments: -
====================================================================== */
bool setupDHT()
{
	// Init DHT
	if ( !dht.begin() )
	{
		writeLogFile( F("No DHT sensor, check wiring!"), 1, 3 );
		detectModule = false;
		return false;
	}
	writeLogFile( F("DHT OK"), 1, 3 );
	detectModule = true;
	return true;
}

/* ======================================================================
Function: getWeatherDHT
Purpose : Read Sensor data
Input   : 
Output  : 
Comments: 
====================================================================== */
void getWeatherDHT()
{
	float h_tmp, t_tmp;
	t_tmp = dht.readTemperature();
    
    // Read temperature as Fahrenheit (isFahrenheit = true)
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