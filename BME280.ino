#ifdef MODUL_BME280

/* ======================================================================
Function: setupBME280
Purpose : Initialize BME280
Input   : 
Output  : 
Comments: -
====================================================================== */
bool setupBME280()
{
	Wire.begin( GPIO_SDA, GPIO_SCL ); // 0, 2  
	Wire.setClock( 100000 );

	// Init BME280
	if ( !bme.begin( BMEaddr ) )
	{
		writeLogFile( F("No BME280 sensor, check wiring!"), 1, 3 );
		detectModule = false;
		return false;
	}
	writeLogFile( F("BME280 OK"), 1, 3 );
	detectModule = true;
	return true;
}

/* ======================================================================
Function: getWeatherBME
Purpose : Read Sensor data
Input   : 
Output  : 
Comments: 
====================================================================== */
void getWeatherBME()
{
	float h_tmp, t_tmp;
	t_tmp = bme.readTemperature();
	h_tmp = round( bme.readHumidity() * 100 ) / 100.0F;

	// If we start getting totaly wrong readings
	if ( ( t_tmp == 0 && h_tmp == 0 ) || isnan(h_tmp) || isnan(t_tmp) || t_tmp > 100 || t_tmp < -60 )
	{
		writeLogFile( F("Fail read BME280!"), 1, 3 );
		return;
	}
	else
	{
		//writeLogFile( F("Sensor data is Valid!"), 1 );
		t = t_tmp;
		h = h_tmp;
		dp = t-0.36*(100.0-h);
		p = bme.readPressure();
		//p = 101345;

		if ( t_measure == 1 )
			t = Fahrenheit(t);
      
	    /* Full formula
	     *  
	     *  P = P0 * exp (-g * M * (h-h0) / (R * T))
	     *  
	     *  h is the altitude at which we want to calculate the pressure, expressed in meters.
	     *  P is the air pressure at altitude h.
	     *  P0 is the pressure at the reference level h0. 
	     *     In our pressure calculator it is assumed that the reference level is located 
	     *     as sea level, so h0 = 0.
	     *  T is the temperature at altitude h, expressed in Kelvins.
	     *  g is the gravitational acceleration. For Earth, g = 9.80665 m/s^2.
	     *  M is the molar mass of air. For Earthly air, M = 0.0289644 kg/mol.
	     *  R is the universal gas constant. Its value is equal to R = 8.31432 N*m /(mol*K).
	     */
     
	    float R = 8.31432;
	    float M = 0.0289644;
	    float g = 9.80665;
	
	    float T = Kelvin(t); // Kelvin
	    
	    int tmp_pl_adj;
	    if ( p_adjust == 1 )
    	{
      		if ( pa_unit == 1 )
        		tmp_pl_adj = Meter( pl_adj );
      		else
        		tmp_pl_adj = pl_adj;

      		float Adjust =  exp( -1 * g * M * tmp_pl_adj / (R * T) );
      		P0 = ( p / Adjust ) / 100.0F;
    	}
    	else
      		P0 = p  / 100.0F;

    	if ( p_measure == 1 )
      		P0 = iNHg(P0);
                   
  	}
  
}

#endif