#ifdef MODULE_WEATHER

//Feet to Meter conversion
float Meter( float feet ) { return feet * 0.3048; }

//Celsius to Fahrenheit conversion
float Fahrenheit( float celsius ) { return 1.8 * celsius + 32; }

//Celsius to Kelvin conversion
float Kelvin( float celsius ) { return celsius + 273.15; }

//hPascals to inhg conversion
float iNHg( float hpa ) { return hpa * 0.02952998; }


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

bool sendMeasuresMQTT()
{
	bool checkStat = true;
	
	// Let's try to publish Temperature
	if ( !sendMQTT( mqtt_Temperature, (char*) String(t).c_str(), true ) )
	{
    	//writeLogFile( F("Publish Temperature: failed"), 1 );
    	//checkStat = false;
  	}

    #if defined( MODULE_DHT ) || defined( MODULE_BME280 )
	char humidityString[6];
	dtostrf(h, 5, 1, humidityString);
    
	// Let's try to publish Humidity
  	if ( !sendMQTT( mqtt_Humidity, humidityString, true ) )
  	{
  		//writeLogFile( F("Publish Humidity: failed"), 1 );
    	checkStat = false;
	}
    #endif
    
	#ifdef MODULE_BME280
	char pressureString[7];
	dtostrf(P0, 6, 1, pressureString);
    
	// Let's try to publish Pressure
    if ( !sendMQTT( mqtt_Pressure, pressureString, true ) )
    {
    	//writeLogFile( F("Publish Pressure: failed"), 1 );
    	checkStat = false;
    }
    #endif
    return checkStat;
}

void sendMeasuresWebhook()
{
	char* localURL;
 	String data;
	char data2[40];
    
	localURL = webLoc_server;
    
    #ifdef MODULE_DS18B20
    sprintf(data2, "{\"t\":\"%.2f\"}",t);
    #endif
    #ifdef MODULE_DHT
    sprintf(data2, "{\"t\":\"%.2f\",\"h\":\"%.2f\"}",t,h);
    #endif
    #ifdef MODULE_BME280
	sprintf(data2, "{\"t\":\"%.2f\",\"h\":\"%.2f\",\"p\":\"%.2f\"}",t,h,P0);
    #endif
	
    data = String(data2);
	sendWebhook( localURL, data );
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

    #ifdef MODULE_DS18B20
    sprintf(data, "{\"t\":\"%.2f\"}",t);
    #endif
    #ifdef MODULE_DHT
    sprintf(data, "{\"t\":\"%.2f\",\"h\":\"%.2f\"}",t,h);
    #endif
    #ifdef MODULE_BME280
	sprintf(data, "{\"t\":\"%.2f\",\"h\":\"%.2f\",\"p\":\"%.2f\"}",t,h,P0);
    #endif


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
ToDo	: - 
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

            #if defined( MODULE_DHT ) || defined( MODULE_BME280 )
						Sensordata.add(h);    // Humidity
            #endif

            #ifdef MODULE_BME280
						Sensordata.add(P0);   // Pressure
            #endif

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