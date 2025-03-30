/*
 * StergoSmart
 * 
 * Smart Home IoT - Weather, Clock and Switches with GUI
 *
 * Components:
 *   - ESP8266 + ESP32
 *   - BME280 + DHT + DS18B20
 *   - CAPTIVE PORTAL + BOOTSTRAP + LittleFS + APEX CHARTS
 *   - MQTT + WebHooks + HTTP
 *   - SSDP + UDP + NTP + OTA
 *   - TicTacToe
 *   - LED Matrix Display 8x32
 *   - Discord Integration
 *
 * Copyright (C) 2018-2025 Kresimir Kokanovic
 * GitHub: https://github.com/kreso975/StergoSmart
 * License: Apache-2.0 (https://github.com/kreso975/StergoSmart#Apache-2.0-1-ov-file)
 */


#include "Config.h"

void setup()
{
	String message;

	#if ( DEBUG == 1 )
	Serial.begin ( SERIAL_BAUDRATE );
	#endif
	delay(1000);
	
	if ( !setupFS() )
		writeLogFile( F("No Filesystem"), 1, 3 );

	// We will Init config to load needed data and choose next steps based on that config
	initConfig ( &message );

	#ifdef MODULE_WEATHER   							//===================  MODULE WEATHER ===========
	setupWeather();
	#endif													//===============================================
	
	#ifdef MODULE_SWITCH
	setupSwitch();
	#endif

	#ifdef MODULE_DISPLAY
	FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
	FastLED.setMaxPowerInVoltsAndMilliamps(POWER_VOLTAGE, MAX_POWER_MILLIAMPS); // check config
	FastLED.setBrightness(maxBrightness);

	timeZoneOffset = 3600 * timeZone;	// Used for accurate display of local Time
	setupDisplay(); // Set init values
	#endif

	// Start WiFi
	wifiManager.manageWiFi();

	if ( WiFi.getMode() == WIFI_STA )
	{
		#if (DEBUG == 1)
		writeLogFile(F("WiFi Connected, IP: "), 0, 1);
		writeLogFile(WiFi.localIP().toString(), 1, 1);
		#endif
		MDNS.begin( wifiManager.getHostname() );

		// Start ntpUDP
		// We need it for M-SEARCH over UDP - SSDP discovery and NTP time sync
		ntpUDP.begin( LOCAL_UDP_PORT );

		timeClient.begin();
		timeClient.update();
		setTime(timeClient.getEpochTime());
		startTime = now();
		DST = isDST() ? 1 : 0;					// Check if DST is active	

		setupHttpServer();
		// If we are connected to WLAN we can start SSDP (Simple Service Discovery Protocol) service
    	setupSSDP();

		// Seed for generating Random number
		randomSeed(analogRead(0));

		#ifdef MQTT_H										//=================== MQTT Manager ==============
		if ( mqttManager.getMqttStart() )
			mqttManager.setupMQTT( &message, true );
		#endif												//===============================================
	}
	else
	{
		#if (DEBUG == 1)
		writeLogFile(F("Setting AP"), 1, 1);
		#endif
		setupHttpServer();
	}
}


void loop()
{
	// WiFi AP mode
	// IN AP MODE we don't have Date & Time so we don't do anything except setup Device
	if ( WiFi.getMode() == WIFI_AP )
	{
		dnsServer.processNextRequest();
		// If in AP mode we will check if we need to restart the device
		wifiManager.checkAPRestart();
	}
		
	server.handleClient();

	if ( WiFi.getMode() == WIFI_STA )
	{
		// Check and update time
		if ( timeClient.update() )
		{
			setTime(timeClient.getEpochTime());
			DST = isDST() ? 1 : 0;					// Check if DST is active	
		}
			
  
		// We will not run anything if we are in WIN message mode
		// Skip operations if in WIN message mode (dirty fix)
		#ifdef MODULE_DISPLAY						//=================  MODULE DISPLAY =============
		if ( !messageWinON )
		{
		#endif											//===============================================
  
			#ifdef MODULE_WEATHER   				//===================  MODULE WEATHER ===========
			updateWeather();
			#endif   									//===============================================
  
			#ifdef MQTT_H								//=================== MQTT Manager ==============
			mqttManager.updateMQTT();
			#endif   									//===============================================
  
			#ifdef MODULE_TICTACTOE					//=================  MODULE TICTACTOE ===========
			updateTicTacToe();
			#endif   									//===============================================
  
		#ifdef MODULE_DISPLAY						//=================  MODULE DISPLAY =============
		}

		updateDisplay();
		#endif											//===============================================
  }
  

	// This should be renamed to updateSwitch and handled in Switch.ino
	#if ( defined( MODULE_SWITCH ) && ( STERGO_PLUG == 2 || STERGO_PLUG == 3 ) )  //===============================================
	checkSwitchButton();	// Check Button State - long press
	#endif                                                                   	   //===============================================
}
