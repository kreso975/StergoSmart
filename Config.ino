 /* ======================================================================
Function: initConfig
Purpose : read from SPIFF config.json 
Input   : message
Output  : true / false
Comments: 
TODO    : FIX strcpy replace with strlcpy
		strlcpy(config.hostname,             // <- destination
          root["hostname"] | "example.com",  // <- source
          sizeof(config.hostname));          // <- destination's capacity
====================================================================== */
bool initConfig( String* message )
{
	File configfile = SPIFFS.open( configFile, "r" );
	if (!configfile)
	{
		// Cannot open file - check if not exist to pull one from web or sim
		writeLogFile( fOpen + String(configFile), 1 );
		return false;
	}

	// Allocate a buffer to store contents of the file.
	size_t size = configfile.size();
	std::unique_ptr<char[]> buf(new char[size]);
	configfile.readBytes(buf.get(), size);
	configfile.close();

	DynamicJsonBuffer jsonConfig;
	JsonObject& configMain = jsonConfig.parseObject(buf.get());
	//configMain.printTo(Serial);

	if ( !configMain.success() )
	{
		writeLogFile( faParse + String(configFile), 1 );
		return false;
	}

	mqtt_start = atoi(configMain["mqtt_start"]);

  // MQTT updates for Void loop to take in account Config interval
	mqtt_interval = atoi(configMain["mqtt_interval"]);
	mqtt_intervalHist = 1000 * mqtt_interval;
	mqtt_previousMillis = mqtt_intervalHist;     // time of last point added
  //---
  
	webLoc_start = atoi(configMain["webLoc_start"]);
	webLoc_interval = atoi(configMain["webLoc_interval"]);
	mqtt_port = atoi(configMain["mqtt_port"]);
	strcpy(mqtt_server, configMain["mqtt_server"]);
	strcpy(mqtt_clientName, configMain["mqtt_clientName"]);
	strcpy(mqtt_clientUsername, configMain["mqtt_clientUsername"]);
	strcpy(mqtt_clientPassword, configMain["mqtt_clientPassword"]);
	strcpy(mqtt_myTopic, configMain["mqtt_myTopic"]);

	strcpy(_deviceType,  configMain["deviceType"]);
	strcpy(deviceName,  configMain["deviceName"]);
	strcpy(moduleName, configMain["moduleName"]);

	wifi_runAS = atoi(configMain["wifi_runAS"]);
	strcpy( wifi_hostname, configMain["wifi_hostname"] );
	strcpy( softAP_ssid, configMain["softAP_ssid"] );
	strcpy( softAP_pass, configMain["softAP_pass"] );
	strcpy( wifi_ssid, configMain["wifi_SSID"] );
	strcpy( wifi_password, configMain["wifi_password"] );
	wifi_static = atoi(configMain["wifi_static"]);
	strcpy( wifi_StaticIP, configMain["wifi_StaticIP"] );
	strcpy( wifi_gateway, configMain["wifi_gateway"] );
	strcpy( wifi_subnet, configMain["wifi_subnet"] );
	strcpy( wifi_DNS, configMain["wifi_DNS"] );
    
	#if ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 3 )      //=============== Weather Station ==============
		t_measure = atoi(configMain["t_measure"]);
		p_measure = atoi(configMain["p_measure"]);
		p_adjust = atoi(configMain["p_adjust"]);
		pa_unit = atoi(configMain["pa_unit"]);
		pl_adj = atoi(configMain["pl_adj"]);
		strcpy(mqtt_bme280Humidity, configMain["mqtt_bme280Humidity"]);
		strcpy(mqtt_bme280Temperature, configMain["mqtt_bme280Temperature"]);
		strcpy(mqtt_bme280Pressure, configMain["mqtt_bme280Pressure"]);
	#endif                                                  //===============================================

  #if ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 3 )      //=============== Power Switch ==============
		strcpy(mqtt_switch, configMain["mqtt_switch"]);
		strcpy(mqtt_switch2, configMain["mqtt_switch2"]);
	#endif                                                  //===============================================

	return true;
}

 /* ======================================================================
Function: writeToConfig
Purpose : Write to SPIFF config.json | Update config.json
Input   : message
Output  : true / false
Comments: 
TODO    : FIX Start MQTT - move it to MQTT file
====================================================================== */
bool writeToConfig( String* message )
{ 
	writeLogFile( "Update Config", 1 );

	File configfile = SPIFFS.open( configFile, "r" );
	if ( !configfile )
	{
		// Cannot open file - check if not exist to pull one from web or sim
		writeLogFile( fOpen + String(configFile), 1 );
		return false;
	}

	// Allocate a buffer to store contents of the file.
	size_t size = configfile.size();
	std::unique_ptr<char[]> buf(new char[size]);
	configfile.readBytes(buf.get(), size);
	configfile.close();

	DynamicJsonBuffer jsonConfig;
	JsonObject& configMain = jsonConfig.parseObject(buf.get());
	//configMain.printTo(Serial);

	if ( !configMain.success() )
	{
		writeLogFile( faParse + String(configFile), 1 );
		return false;
	}

	String stringArray[] = { "deviceType", "deviceName", "deviceID", "moduleName", "wifi_password", "wifi_StaticIP", "wifi_gateway", "wifi_subnet", "wifi_DNS",
  					"softAP_ssid", "softAP_pass", "wifi_SSID", "wifi_hostname", "mqtt_server", "mqtt_port", "mqtt_clientName", "mqtt_clientUsername", 
  					"mqtt_clientPassword", "mqtt_myTopic", "webLoc_server" };
	const byte NAMES_IN_USE_1 = 20;
	
	String intArray[] = { "wifi_runAS", "wifi_static", "mqtt_start", "mqtt_interval", "webLoc_start", "webLoc_interval" };
	const byte NAMES_IN_USE_2 = 6;

	for ( byte i = 0; i < NAMES_IN_USE_1; i++ )
		if ( server.hasArg(stringArray[i]) )  configMain[stringArray[i]] = server.arg(stringArray[i]);

	for ( byte i = 0; i < NAMES_IN_USE_2; i++ )
		if ( server.hasArg(intArray[i]) )  configMain[intArray[i]] = server.arg(intArray[i]).toInt();

	
	#if ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 3 )      				//=============== Weather Station ==============
		String stringArray_2[] = {"mqtt_bme280Humidity", "mqtt_bme280Temperature", "mqtt_bme280Pressure"};
		const byte NAMES_IN_USE_3 = 3;
		String intArray_2[] = {"t_measure", "p_measure", "p_adjust", "pa_unit", "pl_adj"};
		const byte NAMES_IN_USE_4 = 5;

		for ( byte i = 0; i < NAMES_IN_USE_3; i++ )
			if ( server.hasArg(stringArray_2[i]) )  configMain[stringArray_2[i]] = server.arg(stringArray_2[i]);

		for ( byte i = 0; i < NAMES_IN_USE_4; i++ )
			if ( server.hasArg(intArray_2[i]) )  configMain[intArray_2[i]] = server.arg(intArray_2[i]).toInt();
	#endif 
  
	
	File file = SPIFFS.open( configFile, "w" );
	if (!file)
	{
		writeLogFile( fWrite + String(configFile), 1 );
		*message = fWrite + String(configFile);
		return false;
	}
    
	if ( configMain.prettyPrintTo(file) == 0 )
	{
		writeLogFile( fOpen + String(configFile), 1 );
		*message = fOpen + String(configFile);
		file.close();
		return false;
	}
    
	file.close();
	//configMain.printTo(Serial);
	//return true;

	// Let's check if we have change request for MQTT state (start/stop)
	if ( server.hasArg("stateOn") ) // Check if MQTT was started or Stopped
	{
		if ( server.arg("stateOn") == "1" )
		{
			// HERE WE MUST TELL THAT WE WILL START MQTT SERVICES
			if ( !initConfig( message ) )
			{
				// Should check what was response in message
				*message = F("Error init MQTT");
				return false;
			}
			mqtt_start = 1;
			if ( setupMQTT( message, 1 ) )
			{
				*message = F("Success MQTT start");
				return true;
			}
			else
			{
				*message = F("Error MQTT start");
				return false;
			}
		}
		else if ( server.arg("stateOn") == "0" )
		{
			mqtt_start = 0;
			setupMQTT( message, 0 );

			*message = F("Success MQTT stop");
		}
	}

	return true;
}
