/* defConf ="deviceID": "1", "deviceType": "1", "deviceName": "", "moduleName": "", 
			"t_measure": 0, "p_measure": 0, "p_adjust": 0, "pa_unit": 0, "pl_adj": 0, "wifi_runAS": 1,
    		"wifi_hideAP": 0, "softAP_ssid": "StergoSmart", "softAP_pass": "123456789", "wifi_hostname": "StergoSmart",
    		"wifi_static": 0, "wifi_StaticIP": "", "wifi_SSID": "", "wifi_password": "", "wifi_gateway": "", "wifi_subnet": "",
    		"wifi_DNS": "", "mqtt_start": 0, "mqtt_interval": 60, "webLoc_start": 0, "webLoc_interval": 60, "tictac_start": 0,
    		"tictac_interval": 80, "tictac_webhook": 0, "tictac_discord": 0, "webLoc_server": "", "discord_url": "", "discord_avatar": "",
    		"mqtt_server": "", "mqtt_port": "", "mqtt_clientName": "", "mqtt_clientUsername": "", "mqtt_clientPassword": "",
    		"mqtt_myTopic": "", "mqtt_Humidity": "", "mqtt_Temperature": "", "mqtt_Pressure": "", "mqtt_switch": "",
    		"mqtt_switch2": ""
*/


/* ======================================================================
Function: initConfig
Purpose : read from SPIFF config.json 
Input   : message
Output  : true / false
Comments: 
TODO    : FIX strcpy replace with strlcpy
		  strlcpy(config.hostname,           // <- destination
          root["hostname"] | "example.com",  // <- source
          sizeof(config.hostname));          // <- destination's capacity */
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

	DynamicJsonDocument  jsonConfig(8000);
  	DeserializationError error = deserializeJson( jsonConfig, buf.get());

	if ( error )
	{
		writeLogFile( faParse + String(error.c_str()), 1 );
		return false;
	}

	mqtt_start = jsonConfig["mqtt_start"];
  	// MQTT updates for Void loop to take in account Config interval
	mqtt_interval = jsonConfig["mqtt_interval"];
	mqtt_intervalHist = 1000 * mqtt_interval;
	mqtt_previousMillis = mqtt_intervalHist;     // time of last point added

	mqtt_port = jsonConfig["mqtt_port"];
	strcpy(mqtt_server, jsonConfig["mqtt_server"]);
	strcpy(mqtt_clientName, jsonConfig["mqtt_clientName"]);
	strcpy(mqtt_clientUsername, jsonConfig["mqtt_clientUsername"]);
	strcpy(mqtt_clientPassword, jsonConfig["mqtt_clientPassword"]);
	strcpy(mqtt_myTopic, jsonConfig["mqtt_myTopic"]);
 	
	//---
	webLoc_start = jsonConfig["webLoc_start"];
	strcpy(webLoc_server, jsonConfig["webLoc_server"]);
	// WebHook updates for Void loop to take in account Config interval
	webLoc_interval = jsonConfig["webLoc_interval"];
	webLoc_intervalHist = 1000 * webLoc_interval;
	webLoc_previousMillis = webLoc_intervalHist;     // time of last point added
 	//---

	// Discord
	strcpy(discord_url, jsonConfig["discord_url"]);
	strcpy(discord_avatar, jsonConfig["discord_avatar"]);
	//---
	strcpy(_deviceType,  jsonConfig["deviceType"]);
	
	strcpy(deviceName,  jsonConfig["deviceName"]);
	// This must be placed somwhere else
	String _tmp = deviceName;
	_tmp.replace(' ', '-');
	_tmp += "-";
	_tmp += ESP.getChipId();
	_tmp.toCharArray(_devicename, sizeof(_devicename));
	// -----
	
	strcpy(moduleName, jsonConfig["moduleName"]);

	wifi_runAS = jsonConfig["wifi_runAS"];
	strcpy( wifi_hostname, jsonConfig["wifi_hostname"] );
	strcpy( softAP_ssid, jsonConfig["softAP_ssid"] );
	strcpy( softAP_pass, jsonConfig["softAP_pass"] );
	strcpy( wifi_ssid, jsonConfig["wifi_SSID"] );
	strcpy( wifi_password, jsonConfig["wifi_password"] );
	wifi_static = jsonConfig["wifi_static"];
	strcpy( wifi_StaticIP, jsonConfig["wifi_StaticIP"] );
	strcpy( wifi_gateway, jsonConfig["wifi_gateway"] );
	strcpy( wifi_subnet, jsonConfig["wifi_subnet"] );
	strcpy( wifi_DNS, jsonConfig["wifi_DNS"] );
    
	#ifdef MODULE_WEATHER     								  //=============== Weather Station  ==============
		t_measure = jsonConfig["t_measure"];
		strcpy(mqtt_Temperature, jsonConfig["mqtt_Temperature"]);
		
		#if defined( MODULE_DHT ) || defined( MODULE_BME280 ) //=============== MODULE DHT && BME  =============
			strcpy(mqtt_Humidity, jsonConfig["mqtt_Humidity"]);
		#endif
		
		#ifdef MODULE_BME280								  //=============== MODULE BME  ===================
			p_measure = jsonConfig["p_measure"];
			p_adjust = jsonConfig["p_adjust"];
			pa_unit = jsonConfig["pa_unit"];
			pl_adj = jsonConfig["pl_adj"];
			strcpy(mqtt_Pressure, jsonConfig["mqtt_Pressure"]);
		#endif
	#endif                                                    //================================================

    #ifdef MODULE_SWITCH      								  //=============== Power Switch ===================
		strcpy(mqtt_switch, jsonConfig["mqtt_switch"]);
		strcpy(mqtt_switch2, jsonConfig["mqtt_switch2"]);
	#endif           										  //================================================

	#ifdef MODULE_TICTACTOE									  //=============== Tic Tac Toe  ===================
		// Tic Tac Toe config
		tictac_start  = jsonConfig["tictac_start"];
		tictac_interval = jsonConfig["tictac_interval"];
		ticTacCallInterval = 1000 * 60 * tictac_interval;	// 1000 * 60 * 60 - 60min
		ticCallLMInterval = ticTacCallInterval;     // time of last point added
		tictac_webhook = jsonConfig["tictac_webhook"];
		tictac_discord = jsonConfig["tictac_discord"];
	#endif													  //================================================

	#ifdef MODULE_DISPLAY									  //===============   Display   ====================
		// Display config
		displayON = jsonConfig["displayON"];
		maxBrightness = jsonConfig["maxBrightness"];
		timeZone = jsonConfig["timeZone"];
		
		String hexColor = String(jsonConfig["displayColor"]);
		displayColor = strtol(&hexColor[1], NULL, 16);
		strcpy(mqtt_displayON, jsonConfig["mqtt_displayON"]);
		strcpy(mqtt_Brightness, jsonConfig["mqtt_Brightness"]);
		strcpy(mqtt_Color, jsonConfig["mqtt_Color"]);
	#endif           										  //================================================

	return true;
}

 /* ======================================================================
Function: writeToConfig
Purpose : Write to SPIFF config.json | Update config.json
Input   : message
Output  : true / false
Comments: 
TODO    : 
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

  	DynamicJsonDocument  jsonConfig(8000);
  	DeserializationError error = deserializeJson( jsonConfig, buf.get());

	if ( error )
	{
		writeLogFile( faParse + String(error.c_str()), 1 );
		return false;
	}

	String stringArray[] = { "deviceType", "deviceName", "deviceID", "moduleName", "wifi_password", "wifi_StaticIP", "wifi_gateway", "wifi_subnet", "wifi_DNS",
  					"softAP_ssid", "softAP_pass", "wifi_SSID", "wifi_hostname", "mqtt_server", "mqtt_port", "mqtt_clientName", "mqtt_clientUsername", 
  					"mqtt_clientPassword", "mqtt_myTopic", "webLoc_server", "discord_url", "discord_avatar" };
	const byte NAMES_IN_USE_1 = 22;
	
	String intArray[] = { "wifi_runAS", "wifi_static", "mqtt_start", "mqtt_interval", "webLoc_start", "webLoc_interval", 
						"tictac_start", "tictac_interval", "tictac_webhook", "tictac_discord" };
	const byte NAMES_IN_USE_2 = 10;

	for ( byte i = 0; i < NAMES_IN_USE_1; i++ )
		if ( server.hasArg(stringArray[i]) )  jsonConfig[stringArray[i]] = server.arg(stringArray[i]);

	for ( byte i = 0; i < NAMES_IN_USE_2; i++ )
		if ( server.hasArg(intArray[i]) )  jsonConfig[intArray[i]] = server.arg(intArray[i]).toInt();

	
	#ifdef MODULE_WEATHER     											//=============== Weather Station ==============
		
		String stringArray_2[] = { "mqtt_Temperature", "mqtt_Humidity", "mqtt_Pressure" };
		String intArray_2[] = { "t_measure", "p_measure", "p_adjust", "pa_unit", "pl_adj" };
		
		#ifdef MODULE_DS18B20 
		// stringArray_2 - use only 1st
		const byte NAMES_IN_USE_3 = 1;
		// intArray_2
		const byte NAMES_IN_USE_4 = 1;
		#endif
		#ifdef MODULE_DHT
		// stringArray_2 - use first 2
		const byte NAMES_IN_USE_3 = 2;
		// intArray_2
		const byte NAMES_IN_USE_4 = 1;
		#endif
		#ifdef MODULE_BME280
		// stringArray_2 - Use all
		const byte NAMES_IN_USE_3 = 3;
		// intArray_2
		const byte NAMES_IN_USE_4 = 5;
		#endif

		for ( byte i = 0; i < NAMES_IN_USE_3; i++ )
			if ( server.hasArg(stringArray_2[i]) )  jsonConfig[stringArray_2[i]] = server.arg(stringArray_2[i]);

		for ( byte i = 0; i < NAMES_IN_USE_4; i++ )
			if ( server.hasArg(intArray_2[i]) )  jsonConfig[intArray_2[i]] = server.arg(intArray_2[i]).toInt();
	#endif

	#ifdef	MODULE_SWITCH     											//=============== Power Switch ==============
		String stringArray_3[] = {"mqtt_switch", "mqtt_switch2"};
		const byte NAMES_IN_USE_5 = 2;
		
		for ( byte i = 0; i < NAMES_IN_USE_5; i++ )
			if ( server.hasArg(stringArray_3[i]) )  jsonConfig[stringArray_3[i]] = server.arg(stringArray_3[i]);
	#endif

	#ifdef	MODULE_DISPLAY     											//=============== Display Clock ==============
		String stringArray_4[] = {"mqtt_displayON", "mqtt_Brightness", "mqtt_Color", "displayColor"};
		const byte NAMES_IN_USE_6 = 4;
		String intArray_3[] = { "maxBrightness", "timeZone", "displayON" };
		const byte NAMES_IN_USE_7 = 3;
		
		for ( byte i = 0; i < NAMES_IN_USE_6; i++ )
			if ( server.hasArg(stringArray_4[i]) )  jsonConfig[stringArray_4[i]] = server.arg(stringArray_4[i]);
		for ( byte i = 0; i < NAMES_IN_USE_7; i++ )
			if ( server.hasArg(intArray_3[i]) )  jsonConfig[intArray_3[i]] = server.arg(intArray_3[i]).toInt();
	#endif 
  
	
	File file = SPIFFS.open( configFile, "w" );
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
	// Let's check if we have change request for MQTT state (start/stop)
	if ( server.hasArg("MQTTstateOn") ) // Check if MQTT was started or Stopped
	{
		if ( server.arg("MQTTstateOn") == "1" )
		{
			// HERE WE MUST TELL THAT WE WILL START MQTT SERVICES
			mqtt_start = 1;
			if ( setupMQTT( message, 1 ) )
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
			mqtt_start = 0;
			
			setupMQTT( message, 0 );
			// reset interval times
			mqtt_previousMillis = mqtt_intervalHist;     // time of last point added
			return true;
		}
	}

	// Let's check if we have change request for MWebHookQTT state (start/stop)
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

	#ifdef MODULE_TICTACTOE
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
	#endif
	#ifdef MODULE_DISPLAY
	if ( server.hasArg("timeZone") ) // Check if TicTac was started or Stopped
	{
		String timeZoneStr = server.arg("timeZone");
		timeZoneOffset = 3600 * timeZoneStr.toInt();
	}
	// Let's check if we have change request for Tic Tac Toe state (start/stop)
	if ( server.hasArg("displayON") ) // Check if TicTac was started or Stopped
	{
		char payload[4]; // Ensure the array is large enough to hold the string representation
		if ( server.arg("displayON") == "1" )
		{
			// Set Updated Display values and do not reload config
			displayON = 1;
			itoa(displayON, payload, 10); // Convert byte to string
			msg = F("Success Display ON");
			writeLogFile( msg, 1, 3 );
    		*message = F("{\"success\":\"") + msg + F("\"}");
			
			if ( mqtt_start == 1 )
                if ( !sendMQTT( mqtt_displayON, payload, true ) )
                    writeLogFile( F("Publish displayON: failed"), 1 );
			
			return true;
		}
		else if ( server.arg("displayON") == "0" )
		{
			// Set Updated Display values and do not reload config
			displayON = 0;
			itoa(displayON, payload, 10); // Convert byte to string
			msg = F("Success Display OFF");
			writeLogFile( msg, 1, 3 );
    		*message = F("{\"success\":\"") + msg + F("\"}");

			if ( mqtt_start == 1 )
                if ( !sendMQTT( mqtt_displayON, payload, true ) )
                    writeLogFile( F("Publish displayON: failed"), 1 );
			return true;
		}
	}
	#endif

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
