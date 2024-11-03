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
	char humidityString[6];
	char pressureString[7];

	dtostrf(h, 5, 1, humidityString);
	dtostrf(P0, 6, 1, pressureString);

	// Let's try to publish Temperature
	if ( !sendMQTT( mqtt_bme280Temperature, (char*) String(t).c_str(), true ) )
	{
    	//writeLogFile( F("Publish Temperature: failed"), 1 );
    	//checkStat = false;
  	}
    // Let's try to publish Humidity
  	if ( !sendMQTT( mqtt_bme280Humidity, humidityString, true ) )
  	{
  		//writeLogFile( F("Publish Humidity: failed"), 1 );
    	checkStat = false;
	}
    // Let's try to publish Pressure
    if ( !sendMQTT( mqtt_bme280Pressure, pressureString, true ) )
    {
    	//writeLogFile( F("Publish Pressure: failed"), 1 );
    	checkStat = false;
    }
    
    return checkStat;
}

void sendMeasuresWebhook()
{
	char* localURL;
 	String data;
	char data2[40];
    
	localURL = webLoc_server;
    
	sprintf(data2, "{\"t\":\"%.2f\",\"h\":\"%.2f\",\"p\":\"%.2f\"}",t,h,P0);
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
	sprintf(data, "{\"t\":\"%.2f\",\"h\":\"%.2f\",\"p\":\"%.2f\"}",t,h,P0);

	server.sendHeader( "Access-Control-Allow-Origin", "*" );
	server.send( 200, "application/json", data );
	//Serial.println( F("Sent measures") );
}



#endif