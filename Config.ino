/* ======================================================================
Function: initConfig
Purpose : read from LittleFS config.json 
Input   : message
Output  : true / false
Comments: 
TODO    : */
bool initConfig( String* message )
{
	File configfile = LittleFS.open( configFile, "r" );
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

	// Reserve the required space for jsonBuffer based on the file size
	DynamicJsonDocument jsonConfig( 6000 ); // Adding extra space for overhead
  	DeserializationError error = deserializeJson( jsonConfig, buf.get());
	
	if ( error )
	{
		writeLogFile( faParse + String(error.c_str()), 1, 3 );
		return false;
	}

	// Discord
	strlcpy(discord_url, jsonConfig["discord_url"].as<String>().c_str(), sizeof(discord_url));
	strlcpy(discord_avatar, jsonConfig["discord_avatar"].as<String>().c_str(), sizeof(discord_avatar));

	//---
	deviceType = jsonConfig["deviceType"].as<String>().toInt();
	strlcpy(deviceName, jsonConfig["deviceName"].as<String>().c_str(), sizeof(deviceName));
	
	// generate _devicename
	size_t len = 0;
	// Copy deviceName to _devicename, replacing spaces with '-'
	for (size_t i = 0; deviceName[i] != '\0' && len < sizeof(_devicename) - 1; i++)
    _devicename[len++] = (deviceName[i] == ' ') ? '-' : deviceName[i];

	// Append "-" and chipID to _devicename
	if (len < sizeof(_devicename) - 1)
    	_devicename[len++] = '-';

	strlcpy(_devicename + len, chipID.c_str(), sizeof(_devicename) - len);

	// Ensure null-terminated string
	_devicename[sizeof(_devicename) - 1] = '\0';
	
	// -----
	
	strlcpy(moduleName, jsonConfig["moduleName"].as<String>().c_str(), sizeof(moduleName));

	#ifdef MQTT_H													//================ MQTT Manager  ================
		#ifdef MODULE_WEATHER
		mqtt_interval = jsonConfig["mqtt_interval"].as<int>();
		mqtt_intervalHist = 1000 * mqtt_interval;
		mqtt_previousMillis = mqtt_intervalHist;     // time of last point added
		#endif
	// Extract values from jsonConfig and pass them to the setter methods
	mqttManager.setMqttStart(jsonConfig["mqtt_start"].as<byte>());
	mqttManager.setMqttPort(jsonConfig["mqtt_port"].as<int>());
	mqttManager.setMqttServer(jsonConfig["mqtt_server"].as<String>().c_str());
	mqttManager.setMqttClientName(jsonConfig["mqtt_clientName"].as<String>().c_str());
	mqttManager.setMqttClientUsername(jsonConfig["mqtt_clientUsername"].as<String>().c_str());
	mqttManager.setMqttClientPassword(jsonConfig["mqtt_clientPassword"].as<String>().c_str());
	mqttManager.setMqttMyTopic(jsonConfig["mqtt_myTopic"].as<String>().c_str());
	#endif															//================================================ 

	#ifdef WIFIMANAGER_H											//================ WiFi Manager  ================
	// Call the set methods with the extracted values
	char tmp[20]; 
	snprintf(tmp, sizeof(tmp), "%s_%s", jsonConfig["softAP_ssid"].as<const char*>(), chipID.c_str());
	// Pass the formatted string to the WiFiManager instance
	wifiManager.setSoftAPSSID(tmp);
	wifiManager.setSoftAPPassword(jsonConfig["softAP_pass"].as<const char*>());
	wifiManager.setWifiHostname(jsonConfig["wifi_hostname"].as<const char*>());
	wifiManager.setWifiSSID(jsonConfig["wifi_SSID"].as<const char*>());
	wifiManager.setWifiPassword(jsonConfig["wifi_password"].as<const char*>());
	wifiManager.setWifiStatic(jsonConfig["wifi_static"].as<byte>());
	wifiManager.setWifiStaticIP(jsonConfig["wifi_StaticIP"].as<const char*>());
	wifiManager.setWifiGateway(jsonConfig["wifi_gateway"].as<const char*>());
	wifiManager.setWifiSubnet(jsonConfig["wifi_subnet"].as<const char*>());
	wifiManager.setWifiDNS(jsonConfig["wifi_DNS"].as<const char*>());
	wifiManager.setWifiRunAS(jsonConfig["wifi_runAS"].as<byte>());
	#endif															//================================================
	
	#ifdef MODULE_WEATHER										//=============== Weather Station  ==============
		webLoc_start = jsonConfig["webLoc_start"].as<byte>();
		strlcpy(webLoc_server, jsonConfig["webLoc_server"].as<String>().c_str(), sizeof(webLoc_server));
		// WebHook updates for Void loop to take in account Config interval
		webLoc_interval = jsonConfig["webLoc_interval"].as<int>();
		webLoc_intervalHist = 1000 * webLoc_interval;
		webLoc_previousMillis = webLoc_intervalHist;     // time of last point added

		t_measure = jsonConfig["t_measure"].as<byte>();
		strlcpy(mqtt_Temperature, jsonConfig["mqtt_Temperature"].as<String>().c_str(), sizeof(mqtt_Temperature));
		
		#if defined( MODULE_DHT ) || defined( MODULE_BME280 ) //=============== MODULE DHT && BME  =============
			strlcpy(mqtt_Humidity, jsonConfig["mqtt_Humidity"].as<String>().c_str(), sizeof(mqtt_Humidity));
		#endif
		
		#ifdef MODULE_BME280								  		//=============== MODULE BME  ===================
			p_measure = jsonConfig["p_measure"].as<byte>();
			p_adjust = jsonConfig["p_adjust"].as<byte>();
			pa_unit = jsonConfig["pa_unit"].as<byte>();
			pl_adj = jsonConfig["pl_adj"].as<int>();
			strlcpy(mqtt_Pressure, jsonConfig["mqtt_Pressure"].as<String>().c_str(), sizeof(mqtt_Pressure));
		#endif
	#endif                                             //================================================

   #ifdef MODULE_SWITCH      									//=============== Power Switch ===================
		strlcpy(mqtt_switch, jsonConfig["mqtt_switch"].as<String>().c_str(), sizeof(mqtt_switch));
		strlcpy(mqtt_switch2, jsonConfig["mqtt_switch2"].as<String>().c_str(), sizeof(mqtt_switch2));
	#endif           										  		//================================================

	#ifdef MODULE_TICTACTOE									  	//=============== Tic Tac Toe  ===================
		// Tic Tac Toe config
		tictac_start  = jsonConfig["tictac_start"].as<byte>();
		tictac_interval = jsonConfig["tictac_interval"].as<byte>();
		ticTacCallInterval = 1000 * 60 * tictac_interval;	// 1000 * 60 * 60 - 60min
		ticCallLMInterval = ticTacCallInterval;     			// time of last point added
		tictac_webhook = jsonConfig["tictac_webhook"].as<byte>();
		tictac_discord = jsonConfig["tictac_discord"].as<byte>();
	#endif													  		//================================================

	#ifdef MODULE_DISPLAY									  	//===============   Display   ====================
		// Display config
		displayON = jsonConfig["displayON"].as<byte>();
		maxBrightness = jsonConfig["maxBrightness"].as<byte>();
		timeZone = jsonConfig["timeZone"].as<int>();
		
		String hexColor = String(jsonConfig["displayColor"]);
		displayColor = strtol(&hexColor[1], NULL, 16);
		strlcpy(mqtt_displayON, jsonConfig["mqtt_displayON"].as<String>().c_str(), sizeof(mqtt_displayON));
		strlcpy(mqtt_Brightness, jsonConfig["mqtt_Brightness"].as<String>().c_str(), sizeof(mqtt_Brightness));
		strlcpy(mqtt_Color, jsonConfig["mqtt_Color"].as<String>().c_str(), sizeof(mqtt_Color));
	#endif           										  		//================================================

	return true;
}

 /* ======================================================================
Function: writeToConfig
Purpose : Write to LittleFS config.json | Update config.json
Input   : message
Output  : true / false
Comments: 
TODO    : */
bool writeToConfig( String* message )
{ 
	writeLogFile( "Update Config", 1 );

	File configfile = LittleFS.open( configFile, "r" );
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

	DynamicJsonDocument jsonConfig(5000); // Adding extra space for overhead
  	DeserializationError error = deserializeJson( jsonConfig, buf.get());

	if ( error )
	{
		writeLogFile( faParse + String(error.c_str()), 1 );
		return false;
	}

	String stringArray[] = { "deviceType", "deviceName", "deviceID", "moduleName", "wifi_password", "wifi_StaticIP", "wifi_gateway", "wifi_subnet", "wifi_DNS",
  					"softAP_ssid", "softAP_pass", "wifi_SSID", "wifi_hostname", "mqtt_server", "mqtt_port", "mqtt_clientName", "mqtt_clientUsername", 
  					"mqtt_clientPassword", "mqtt_myTopic", "webLoc_server", "discord_url", "discord_avatar" };
	for (const auto& name : stringArray)
   	if ( server.hasArg(name) )
      	jsonConfig[name] = server.arg(name);
	
	String intArray[] = { "wifi_runAS", "wifi_static", "mqtt_start", "mqtt_interval", "webLoc_start", "webLoc_interval", 
						"tictac_start", "tictac_interval", "tictac_webhook", "tictac_discord" };
	for (const auto& name : intArray)
   	if ( server.hasArg(name) )
      	jsonConfig[name] = server.arg(name).toInt();

	
	#ifdef MODULE_WEATHER     											//=============== Weather Station ================
		
		String stringArray_2[] = { "mqtt_Temperature", "mqtt_Humidity", "mqtt_Pressure" };
		String intArray_2[] = { "t_measure", "p_measure", "p_adjust", "pa_unit", "pl_adj" };
		
		#ifdef MODULE_DS18B20 
		// stringArray_2 - use only 1st
		const byte NAMES_IN_USE_1 = 1;
		// intArray_2
		const byte NAMES_IN_USE_2 = 1;
		#endif
		#ifdef MODULE_DHT
		// stringArray_2 - use first 2
		const byte NAMES_IN_USE_1 = 2;
		// intArray_2
		const byte NAMES_IN_USE_2 = 1;
		#endif
		#ifdef MODULE_BME280
		// stringArray_2 - Use all
		const byte NAMES_IN_USE_1 = 3;
		// intArray_2
		const byte NAMES_IN_USE_2 = 5;
		#endif

		for ( byte i = 0; i < NAMES_IN_USE_1; i++ )
			if ( server.hasArg(stringArray_2[i]) )
				jsonConfig[stringArray_2[i]] = server.arg(stringArray_2[i]);

		for ( byte i = 0; i < NAMES_IN_USE_2; i++ )
			if ( server.hasArg(intArray_2[i]) )
				jsonConfig[intArray_2[i]] = server.arg(intArray_2[i]).toInt();
	#endif																	//================================================

	#ifdef MODULE_SWITCH     											//=============== Power Switch ===================
		String stringArray_3[] = {"mqtt_switch", "mqtt_switch2"};
		for ( const auto& name : stringArray_3 )
			if ( server.hasArg(name) )
				jsonConfig[name] = server.arg(name);
	#endif																	//================================================

	#ifdef MODULE_DISPLAY     											//=============== Display Clock ==================
		String stringArray_4[] = {"mqtt_displayON", "mqtt_Brightness", "mqtt_Color", "displayColor"};
		for ( const auto& name : stringArray_4 )
			if ( server.hasArg(name) )
				jsonConfig[name] = server.arg(name);
				
		String intArray_3[] = { "maxBrightness", "timeZone", "displayON" };
		for (const auto& name : intArray_3)
			if ( server.hasArg(name) )
				jsonConfig[name] = server.arg(name).toInt();
	#endif																	//================================================ 
  
	
	File file = LittleFS.open( configFile, "w" );
	if (!file)
	{
		writeLogFile( fWrite + String(configFile), 1 );
		*message = fWrite + String(configFile);
		return false;
	}
    
	if ( serializeJson(jsonConfig, file) == 0 )
	{
		writeLogFile( fOpen + String(configFile), 1 );
		*message = fOpen + String(configFile);
		file.close();
		return false;
	}
    
	file.close();
  
  	String msg;
	#ifdef MQTT_H													//================ MQTT Manager  ================
	// Let's check if we have change request for MQTT state (start/stop)
	if ( server.hasArg("MQTTstateOn") ) // Check if MQTT was started or Stopped
	{
		if ( server.arg("MQTTstateOn") == "1" )
		{
			// HERE WE MUST TELL THAT WE WILL START MQTT SERVICES
			mqttManager.setMqttStart(1);
			if ( mqttManager.setupMQTT( message, true ) )
			{
				return true;
			}
			else
			{
				*message = F("{\"Error\":\"Error MQTT start\"}");
				return false;
			}
		}
		else if ( server.arg("MQTTstateOn") == "0" )
		{
			mqttManager.setMqttStart(0);
			mqttManager.setupMQTT( message, false );
			
			return true;
		}
	}
	#endif															//================================================

	// WebHook
	#ifdef MODULE_WEATHER     									//=============== Weather Station ===============
	// Let's check if we have change request for WebHook state (start/stop)
	if ( server.hasArg("WhookStateOn") ) // Check if WebHook was started or Stopped
	{
		if ( server.arg("WhookStateOn") == "1" )
		{
			webLoc_start = 1;
			msg = F("Success WebHook Start");
			writeLogFile( msg, 1, 3 );
    		*message = F("{\"success\":\"") + msg + F("\"}");
			return true;
		}
		else if ( server.arg("WhookStateOn") == "0" )
		{
			webLoc_start = 0;
			msg = F("Success WebHook Stop");
			writeLogFile( msg, 1, 3 );
    		*message = F("{\"success\":\"") + msg + F("\"}");
			webLoc_previousMillis = webLoc_intervalHist;     // time of last point added
			return true;
		}
	}
	#endif															//================================================

	#ifdef MODULE_TICTACTOE									  	//=============== Tic Tac Toe  ===================
	// Let's check if we have change request for Tic Tac Toe state (start/stop)
	if ( server.hasArg("TicTacStateOn") ) // Check if TicTac was started or Stopped
	{
		if ( server.arg("TicTacStateOn") == "1" )
		{
			// HERE WE MUST TELL THAT WE WILL START/STOP TicTacToe SERVICE
			tictac_start = 1;
			msg = F("Success Tic Tac Toe Start");
			writeLogFile( msg, 1, 3 );
    		*message = F("{\"success\":\"") + msg + F("\"}");
			return true;
		}
		else if ( server.arg("TicTacStateOn") == "0" )
		{
			tictac_start = 0;
			msg = F("Success Tic Tac Toe Stop");
			writeLogFile( msg, 1, 3 );
    		*message = F("{\"success\":\"") + msg + F("\"}");
			ticCallLMInterval = ticTacCallInterval;
			return true;
		}
	}
	#endif															//================================================

	#ifdef MODULE_DISPLAY     									//=============== Display Clock ==================
	if ( server.hasArg("timeZone") ) // Check if TicTac was started or Stopped
	{
		String timeZoneStr = server.arg("timeZone");
		timeZoneOffset = 3600 * timeZoneStr.toInt();
	}
	// Let's check if we have change request for Tic Tac Toe state (start/stop)
	if ( server.hasArg("displayON") ) // Check if TicTac was started or Stopped
	{
		char payload[4]; // Ensure the array is large enough to hold the string representation
		if (server.arg("displayON") == "1")
		{
			// Set Updated Display values and do not reload config
			displayON = 1;
			itoa(displayON, payload, 10); // Convert byte to string
			msg = F("Success Display ON");
			writeLogFile(msg, 1, 3);
			*message = F("{\"success\":\"") + msg + F("\"}");

			#ifdef MQTT_H                  //================ MQTT Manager  ================
			if (mqttManager.getMqttStart())
				if (!mqttManager.sendMQTT(mqtt_displayON, payload, true))
					writeLogFile(F("Publish displayON: failed"), 1);
			#endif								//================================================

			return true;
		}
		else if (server.arg("displayON") == "0")
		{
			// Set Updated Display values and do not reload config
			displayON = 0;
			FastLED.clearData();
			itoa(displayON, payload, 10); // Convert byte to string
			msg = F("Success Display OFF");
			writeLogFile(msg, 1, 3);
			*message = F("{\"success\":\"") + msg + F("\"}");

			#ifdef MQTT_H                  //================ MQTT Manager  ================
			if (mqttManager.getMqttStart())
				if (!mqttManager.sendMQTT(mqtt_displayON, payload, true))
					writeLogFile(F("Publish displayON: failed"), 1);
			#endif								//================================================

			return true;
		}
	}
	#endif															//================================================

	// After every save of config Let's load it again
	if ( !initConfig( message ) )
	{
		// Should check what was response in message
		msg = F("Failed init config.json");
    	*message = F("{\"Error\":\"") + msg + F("\"}");
		return false;
	}
	else
	{
		msg = F("Saved and init Config");
		*message = F("{\"success\":\"") + msg + F("\"}");
		return true;
	}

	return true;
}
