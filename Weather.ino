#include "Config.h"
#ifdef MODULE_WEATHER

/* ======================================================================
Function: updateWeather
Purpose : Main loop handler for weather-related operations
Input   : None
Output  : None
Comments:
          - Handles periodic weather sensor readings.
          - Updates web-based location data at predefined intervals.
          - Publishes data via MQTT at defined intervals, with error handling.
TODO    : Optimize intervals or modularize components further if necessary.
*/
void updateWeather()
{
	// If MODULE WEATHER is detected on Setup
	if ( detectModule )
	{
		if ( ( millis() - lastMeasureInterval > measureInterval ) || measureFirstRun )
		{
			lastMeasureInterval = millis();
			measureFirstRun = false;

			// read Weather Sensor based on MODULE
			readWeather();
			
			MainSensorConstruct();
		}

		if ( webLoc_start == 1 )
		{
			if ( millis() - webLoc_previousMillis > webLoc_intervalHist )
			{	
				#if ( DEBUG == 1 )                      // -------------------------------------------
					writeLogFile(F("Inside true webLoc loop"), 1, 1);
				#endif
				webLoc_previousMillis = millis();
				sendMeasures(true);
			}
		}

		if ( mqtt_start == 1 )
		{
			String message;
			if ( millis() - mqtt_previousMillis > mqtt_intervalHist )
			{
				mqtt_previousMillis = millis();
				if (!sendMeasuresMQTT())
				{
					mqttManager.setupMQTT(&message, 1);
					sendMeasuresMQTT();
				}
			}
			
		}
	}
}

//Feet to Meter conversion
float Meter( float feet ) { return feet * 0.3048; }

//Celsius to Fahrenheit conversion
float Fahrenheit( float celsius ) { return 1.8 * celsius + 32; }

//Celsius to Kelvin conversion
float Kelvin( float celsius ) { return celsius + 273.15; }

//hPascals to inhg conversion
float iNHg( float hpa ) { return hpa * 0.02952998; }

bool setupWeather()
{
	#ifdef MODULE_BME280
	if ( setupBME280() )
		return true;
	#endif
	#ifdef MODULE_DHT
	if ( setupDHT() )
		return true;
	#endif
	#ifdef MODULE_DS18B20
	setupDS18B20();
	return true;
	#endif

	return false;
}

void readWeather()
{
	#ifdef MODULE_BME280
	getWeatherBME();
	#endif
	#ifdef MODULE_DHT
	getWeatherDHT();
	#endif
	#ifdef MODULE_DS18B20
	getWeatherDS18B20();
	#endif
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

Comments: -  */
bool updateHistory( int z = 0 )
{
	// Default empty JSON structure
	const char* json = (z == 1) ? "{\"sensor\":[]}" : "";

	// Open the file for writing
	File file = LittleFS.open(HISTORY_FILE, "w");
	if (!file) {
		 //writeLogFile(F("Failed to open history file"), 1, 3);  // Added error logging
		 return false;
	}

	// Log deletion of history if applicable
	if (z == 1) {
		 writeLogFile(F("Delete History"), 1, 3);
	}

	// Write to the file
	file.print(json);
	file.close();

	return true;
}

/* ======================================================================
Function: sendMeasuresMQTT
Purpose : Publish measures to MQTT
Input   : 
Output  : Return true / false
Comments: 
TODO    : */
bool sendMeasuresMQTT()
{
	bool checkStat = true;
	
	// Let's try to publish Temperature
	if ( !mqttManager.sendMQTT( mqtt_Temperature, (char*) String(t).c_str(), true ) )
	{
    	//writeLogFile( F("Publish Temperature: failed"), 1 );
    	checkStat = false;
  	}

   #if defined( MODULE_DHT ) || defined( MODULE_BME280 )
	char humidityString[6];
	dtostrf(h, 5, 1, humidityString);
    
	// Let's try to publish Humidity
  	if ( !mqttManager.sendMQTT( mqtt_Humidity, humidityString, true ) )
  	{
  		//writeLogFile( F("Publish Humidity: failed"), 1 );
    	checkStat = false;
	}
   #endif
    
	#ifdef MODULE_BME280
	char pressureString[7];
	dtostrf(P0, 6, 1, pressureString);
    
	// Let's try to publish Pressure
   if ( !mqttManager.sendMQTT( mqtt_Pressure, pressureString, true ) )
   {
   	//writeLogFile( F("Publish Pressure: failed"), 1 );
    	checkStat = false;
   }
   #endif
   return checkStat;
}

/* ==========================================================================
Function: sendMeasures
Purpose : Publish temperature, humidity, air pressure to JSON HTTP || WebHook
Input   : -
Output  : HTTP JSON
Comments: -  */
void sendMeasures(bool webhook)
{
	const char* localURL = webLoc_server; // Only used if webhook is true
	char data[50]; // Allocate a buffer for the data

	#ifdef MODULE_DS18B20
	snprintf(data, sizeof(data), PSTR("{\"t\":\"%.2f\"}"), t);
	#endif
	#ifdef MODULE_DHT
	snprintf(data, sizeof(data), PSTR("{\"t\":\"%.2f\",\"h\":\"%.2f\"}"), t, h);
	#endif
	#ifdef MODULE_BME280
	snprintf(data, sizeof(data), PSTR("{\"t\":\"%.2f\",\"h\":\"%.2f\",\"p\":\"%.2f\"}"), t, h, P0);
	#endif

	if ( webhook )
		sendWebhook( localURL, data );
	else
		sendJSONheaderReply(3, data); // 3 is an indicator of JSON already formatted reply
}

/* ======================================================================
Function: MainSensorConstruct
Purpose : brain of load / save data logic
Input   : -
Output  : - history.json
Comments: -
ToDo	: -  */
bool MainSensorConstruct()
{
	File file = LittleFS.open(HISTORY_FILE, "r");
	if (!file)
	{
		if (updateHistory(1)) // Create proper initial History File
			writeLogFile(F("Delete History 1"), 1, 1);
	}
	else
	{
		size_t size = file.size();
		#if (DEBUG == 1)
		//String logMessage = String(F("File size: ")) + String(size);
		//writeLogFile(logMessage, 1, 2);
		#endif

		if ( size < 4 )
		{
			if (updateHistory(1)) // Create proper initial History File
				writeLogFile(F("Delete History 2"), 1, 1);
		}
		else
		{
			std::unique_ptr<char[]> buf(new char[size]);
			file.readBytes(buf.get(), size);
			file.close();

			// Reserve the required space for jsonBuffer based on the file size
			DynamicJsonDocument jsonBuffer(size + 500); // Adding extra space for overhead
			DeserializationError error = deserializeJson(jsonBuffer, buf.get());

			if (error)
			{
				writeLogFile(error.c_str(), 1, 1);
				return false;
			}
			else
			{
				JsonArray sensor = jsonBuffer["sensor"];
				JsonArray Sensordata = sensor.createNestedArray();

				unsigned long currentMillis = millis();
				if (currentMillis - previousMillis > intervalHist)
				{
					long int tps = now();
					previousMillis = currentMillis;

					if (tps > 0 && tps < 2085974400)
					{
						Sensordata.add(tps);			// Timestamp
						Sensordata.add(round2(t)); // Temperature

						#if defined(MODULE_DHT) || defined(MODULE_BME280)
						Sensordata.add(round2(h)); // Humidity
						#endif

						#ifdef MODULE_BME280
						Sensordata.add(round2(P0)); // Pressure
						#endif

						if (sensor.size() > sizeHist)
							sensor.remove(0); // - remove first record / oldest

						File file = LittleFS.open(HISTORY_FILE, "w");
						if (!file)
						{
							if (updateHistory(1)) // Create proper initial History File
								writeLogFile(F("Delete History 3"), 1, 1);
						}
						else
						{
							if (serializeJson(jsonBuffer, file) == 0)
							{
								writeLogFile(fWrite + HISTORY_FILE, 1, 1);
								file.close();
								return false;
							}
						}
					}
				}
			}
		}
		file.close();
	}
	return true;
}

#endif