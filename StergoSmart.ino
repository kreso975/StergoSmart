/*
 * StergoSmart
 * 
 * Smart Home IOT - Weather and Switches with GUI - ESP8266
 *
 *
 * ESP8266 + BME280 + DHT + DS18B20 + BOOTSTRAP + SPIFFS + APEX CHARTS + MQTT + WebHooks
 * + CAPTIVE PORTAL + SSDP + NTP + OTA + TicTacToe + LED Matrix Display 8x32 + Discord
 *
 *
 * Copyright (C) 2018-2024 Kresimir Kokanovic - https://github.com/kreso975/StergoSmart
 *
 * Apache-2.0 license - https://github.com/kreso975/StergoSmart#Apache-2.0-1-ov-file
 */


#include "Config.h"

void setup()
{
	String message;

	#if ( DEBUG == 1 )
	Serial.begin ( SERIAL_BAUDRATE );
	delay(1000);
	//Serial.setDebugOutput(true);
	#endif
	
	if ( !setupFS() )
		writeLogFile( F("No Filesystem"), 1, 3 );

	// We will Init config to load needed data and choose next steps based on that config
	initConfig ( &message );
  
	deviceType = atoi(_deviceType);

	#ifdef MODULE_WEATHER
	setupWeather();
	#endif
	#ifdef MODULE_SWITCH
	setupSwitch();
	#endif

	#ifdef MODULE_DISPLAY
	FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
   FastLED.setMaxPowerInVoltsAndMilliamps(POWER_VOLTAGE, MAX_POWER_MILLIAMPS); // check config
   FastLED.setBrightness(maxBrightness);

	timeZoneOffset = 3600 * timeZone;
	#endif

	WiFiManager();

	if ( WiFi.getMode() == 1 )
	{
		MDNS.begin( wifi_hostname );

		timeClient.begin();
		timeClient.update();
		setTime(timeClient.getEpochTime());
		startTime = now();

		setupHttpServer();
		// If we are connected to WLAN we can start SSDP (Simple Service Discovery Protocol) service
    	setupSSDP();

		// Seed for generating Random number
		randomSeed(analogRead(0));

		if ( mqtt_start == 1 )
			setupMQTT( &message, 1 );

		// Start ntpUDP
		// We need it for M-SEARCH over UDP - SSDP discovery
		ntpUDP.begin( LOCAL_UDP_PORT );
	}
	else
	{
    	ap_previousMillis = millis();
		setupHttpServer();
	}
	
}


// nothing can run if we are in 1st setup cycle
void loop()
{
	// WiFi AP mode
	// IN AP MODE we don't have Date & Time so we don't do anything except setup Device
	if ( WiFi.getMode() == 2 )
	{
		dnsServer.processNextRequest();

		//IF AP is running for 5 min and we have in cofig setup Password and Gateway
		//We will restart device and try to connect to WiFi STA again
		if ( ( millis() - ap_previousMillis > ap_intervalHist ) && strcmp( wifi_ssid, "" )  && strcmp( wifi_password, "" ) )
		{
			writeLogFile( F("AP 5min restart"), 1, 3 );
			delay(500);
			ESP.restart();
		}
	}
		
	server.handleClient();

	if ( WiFi.getMode() == 1 )
	{
		//timeClient.update();
		//if ( timeClient.getEpochTime() != now() )
		//	setTime(timeClient.getEpochTime());
		#ifdef MODULE_DISPLAY						//=================  MODULE DISPLAY =============
		if ( !messageWinON )
		{
		#endif											//===============================================
			
			
			#ifdef MODULE_WEATHER   				//===================  MODULE WEATHER ===========
			updateWeather();
			#endif   									//===============================================
													
			updateMQTT();
			
			#ifdef MODULE_TICTACTOE					//=================  MODULE TICTACTOE =========== 
			updateTicTacToe();
			#endif										//===============================================

		
		#ifdef MODULE_DISPLAY						//=================  MODULE DISPLAY =============
			updateDisplay();
		}
		else
		{
			updateDisplay();
		}
		#endif											//===============================================
	}

	#if ( defined( MODULE_SWITCH ) && ( STERGO_PLUG == 2 || STERGO_PLUG == 3 ) )  //===============================================
	checkSwitchButton();	// Check Button State - long press
	#endif                                                                   	  //===============================================
}
