#ifdef MODUL_DHT

/* ======================================================================
Function: setupDHT
Purpose : Initialize DHT 11, 22, 21
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