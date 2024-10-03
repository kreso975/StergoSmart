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
		return false;
	}
	writeLogFile( F("BME280 OK"), 1, 3 );
	return true;
}

//Feet to Meter conversion
float Meter( float feet ) { return feet * 0.3048; }

//Celsius to Fahrenheit conversion
float Fahrenheit( float celsius ) { return 1.8 * celsius + 32; }

//Celsius to Kelvin conversion
float Kelvin( float celsius ) { return celsius + 273.15; }

//hPascals to inhg conversion
float iNHg( float hpa ) { return hpa * 0.02952998; }

/* ======================================================================
Function: getWeather
Purpose : Read Sensor data
Input   : 
Output  : 
Comments: - It's hardcoded to BME280, need to add support for other
====================================================================== */
void getWeather()
{
	float h_tmp, t_tmp;
	t_tmp = bme.readTemperature();
	h_tmp = round( bme.readHumidity() * 100 ) / 100.0F;
	//t_tmp = 21.23;
	//h_tmp = 34.56;

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

/* ======================================================================
Function: updateHistory
Purpose : Save || Erase Log file
Input   : z = 0 (saveHistory); z = 1 (eraseHistory)
Output  : true / false
TODO:   : check if jsonDataBuffer not EMPTY before write to file. After
          write, check if history file is not empty (write again)
          After history delete, should write last value of sensors

          Should be done similar like wifiManager - logic in one function calling
          sub functions so that we can go over again if needed


Comments: -
====================================================================== */
bool updateHistory( int z = 0 )
{
	String json;

	File file = SPIFFS.open( HISTORY_FILE, "w" );
	if ( !file )
		return false;

	// Clear/Delete history
	if ( z == 1 )
	{
		json = "{\"sensor\":[]}"; // length = 13
		writeLogFile( F("Delete History"), 1, 3 );
	}
		

	file.print( json );
  	file.close();

    //writeLogFile( F("Update History"), 1 );
    return true;
}

/* ======================================================================
Function: sendMeasures
Purpose : 
Input   : -
Output  : HTTP JSON
Comments: -
====================================================================== */
void sendMeasures()
{
	/*
	String json = "{\"t\":\"" + String(t) + "\",";
           json += "\"h\":\"" + String(h) + "\",";
           json += "\"p\":\"" + String(P0) + "\"}";
    */
	char data[40];
	sprintf(data, "{\"t\":\"%.2f\",\"h\":\"%.2f\",\"p\":\"%.2f\"}",t,h,P0);

	server.sendHeader( "Access-Control-Allow-Origin", "*" );
	server.send( 200, "application/json", data );
	//Serial.println( F("Sent measures") );
}
  
/* ======================================================================
Function: MainSensorConstruct
Purpose : brain of load / save data logic
Input   : -
Output  : - history.json
Comments: -
ToDo	: - Add Params so other Sensors can be included, now is just
			BME280
====================================================================== */
bool MainSensorConstruct()
{
    //writeLogFile( F("In MainSensorConstruct"), 0 );
    //unsigned int Jsonlength = jsonDataBuffer.length();
    //writeLogFile( "Before load size: " + String(Jsonlength), 1 );

    DynamicJsonBuffer jsonBuffer(12000);

	File file = SPIFFS.open( HISTORY_FILE, "r" );
	if (!file)
	{
		//writeLogFile( fOpen + HISTORY_FILE, 1 );
		if ( updateHistory( 1 ) ) // Create proper initial History File
			writeLogFile( F("Delete History 1"), 1, 3 );
	}
	else
	{
		size_t size = file.size();
		if ( size < 4 )
		{
			if ( updateHistory( 1 ) ) // Create proper initial History File
				writeLogFile( F("Delete History 2"), 1, 3 );
		}
        else
        {
			std::unique_ptr<char[]> buf (new char[size]);
			file.readBytes(buf.get(), size);
			file.close();
			
			JsonObject& root = jsonBuffer.parseObject(buf.get());

			if ( !root.success() )
			{
				//writeLogFile( faParse + HISTORY_FILE, 1 );
				return false;
			}
			else
            {
				JsonArray& sensor = root["sensor"];
        		JsonArray& Sensordata = sensor.createNestedArray();	
				
				unsigned long currentMillis = millis();
				// should add on startup to insert record
				if ( currentMillis - previousMillis > intervalHist )
				{
					//writeLogFile( F("In check intervalHist - 1st: "), 0 );
					long int tps = timeClient.getEpochTime();
					previousMillis = currentMillis;

					if ( tps > 0 )
					{
						Sensordata.add(tps);  // Timestamp
						Sensordata.add(t);    // Temperature
						Sensordata.add(h);    // Humidity
						Sensordata.add(P0);   // Pressure

						if ( sensor.size() > sizeHist )
						{
							//writeLogFile( F("tps - Root size greater then sizeHist"), 0 );
							sensor.remove(0); // - remove first record / oldest
						}

						File file = SPIFFS.open( HISTORY_FILE, "w" );
						if (!file)
						{
							//writeLogFile( fOpen + HISTORY_FILE, 1 );
							if ( updateHistory( 1 ) ) // Create proper initial History File
								writeLogFile( F("Delete History 3"), 1 );
						}
						else
						{
							if ( root.printTo(file) == 0 )
							{
								// Should do something if this happened!!!!
								writeLogFile( fWrite + HISTORY_FILE, 1 );
								//*message = fWrite + HISTORY_FILE;
								file.close();
								return false;
							}
						}
            		}  // END of tps
        		}
			}
		}
		file.close();
    }
    return true;
}

#endif