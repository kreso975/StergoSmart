/*
 * StergoSmart
 * 
 * Smart Home IOT - Weather and Switches with GUI - ESP8266
 *
 *
 * ESP8266 + BME280 + BOOTSTRAP + SPIFFS + GOOGLE CHARTS + MQTT + WebHooks
 * + CAPTIVE PORTAL + SSDP + OTA
 *
 *
 * Copyright (C) 2018-2024 Kresimir Kokanovic - https://github.com/kreso975/StergoSmart
 *
 * Apache-2.0 license - https://github.com/kreso975/StergoSmart#Apache-2.0-1-ov-file
 */


#include "Config.h"

void setup()
{
	Serial.begin ( SERIAL_BAUDRATE );
	delay(1000);
	//Serial.setDebugOutput(true);
  
	if ( !setupFS() )
	{
		//Serial.println( F("No Filesystem") );
		writeLogFile( F("No Filesystem"), 1, 3 );
	}

	String message;

	// We will Init config to gather needed data and choose next steps based on that config
	initConfig( &message );
  
	deviceType = atoi(_deviceType);

	#ifdef MODUL_BME280
	if ( setupBME280() )
		detectModule = true;
	#endif

	#ifdef MODULE_SWITCH
	setupSwitch();
	#endif

	WiFiManager();

	if ( WiFi.getMode() == 1 )
	{
		MDNS.begin( wifi_hostname );

		timeClient.begin();
		timeClient.setUpdateInterval(60000);
		//timeClient.setTimeOffset(timeClient.adjustDstEurope());
		timeClient.update();

		/*
    	String statusMessage = "Year is: " + String(timeClient.getYear()) + "\n" + "Month is: " + String(timeClient.getMonth()) + "\n" + "Day is: " + String(timeClient.getDay());
    	Serial.println( statusMessage );
    	Serial.println(timeClient.getFormattedTime());
  		*/

		setupHttpServer();
		// If we are connected to WLAN we cen start SSDP (Simple Service Discovery Protocol) service
    	setupSSDP();

		// Seed for generating Random number
		randomSeed(analogRead(0));

		if ( mqtt_start == 1 )
			setupMQTT( &message, 1 );


		#if ( EXCLUDED_CODE == 9 )      //===============================================
    	// test for SSL
    	/*
      	for ( int i = 1; i<=5; i++ )
      	{
      	connectToSecureAndTest();
      	delay(2000);
      	}
    	*/
    	#endif                           //===============================================
	}
	else
	{
		setupHttpServer();
	}


	
  
  	#ifdef MODULE_TICTACTOE   			//=============================================== Exclude M-SEARCH over UDP 
	// Start ntpUDP
	// We need it for M-SEARCH over UDP - SSDP discovery
	ntpUDP.begin( LOCAL_UDP_PORT );
	#endif

	
}


// nothing can run if we are in 1st setup cycle
void loop()
{
	if ( WiFi.getMode() >= 2 )          // Needed only in WiFi AP mode
		dnsServer.processNextRequest();

	server.handleClient();
	
	// IN AP MODE we don't have Date & Time so we don't do anything except setup Device
	if ( WiFi.getMode() == 1 )
	{
		// If BME280 is detected
		#ifdef MODUL_BME280   								//===============================================
		if ( detectModule )
		{
    		if ( ( millis() - lastMeasureInterval > measureInterval ) || measureFirstRun )
    		{
				lastMeasureInterval = millis();
				measureFirstRun = false;

				getWeather();
				MainSensorConstruct();
			}

			if ( webLoc_start == 1 )
    		{
    			if ( millis() - webLoc_previousMillis > webLoc_intervalHist )
        		{
					Serial.println("Inside true webLoc loop");
        			webLoc_previousMillis = millis();
        			sendWebhook( 1 );
        		}
      		}
		}
		#endif   											//=================================================

    	                       					
    	if ( mqtt_start == 1 )
    	{
      		#if ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 3 )   //===============================================
			String message;
    		if ( millis() - mqtt_previousMillis > mqtt_intervalHist )
        	{
        		mqtt_previousMillis = millis();
        		if ( !sendMQTT() )
				{
					setupMQTT( &message, 1 );
					sendMQTT();
				}	
        	}
      		#endif                                               //===============================================
      
        	if (!client.connected())
        		MQTTreconnect();

        	client.loop();
      	}
      
      	#ifdef MODULE_TICTACTOE									//=============================================== Exclude M-SEARCH over UDP 
		// Search for Devices in LAN
    	if ( ( millis() - previousSSDP > intervalSSDP ) || measureSSDPFirstRun )
		{
			previousSSDP = millis();
			measureSSDPFirstRun = false;
			requestSSDP(1); // This request should be done periodicaly / every 10min
    	}


		// Time interval to ask net devices to play tic tac toe
		if ( ( millis() - ticCallLMInterval > ticTacCallInterval ) || ticCallFirstRun )
		{
			ticCallLMInterval = millis();
			ticCallFirstRun = false;
			// Init Game
			inviteDeviceTicTacToe();
    	}
		// Time interval to check on inactivity of the game, auto reset
		if ( ( millis() - ticTacLastPlayed > ticTacLastPlayedInterval ) && gameStarted == 1 && turn > 0 )
		{
			Serial.println("Inside LastPlayedTicTac measure and gameStarted == 1 and turn > 0");
			resetTicTacToe();
    	}
		if ( ( millis() - ticTacLastPlayed > ticTacLastPlayedInterval ) && didIaskedToPlay )
		{
			Serial.println("Inside LastPlayedTicTac measure and didIaskedToPlay == true");
			resetTicTacToe();
    	}
		

		// Init M-SEARCH over UDP , SSDP Discovery Listen for TicTacToe
		handleSSDP(); //WE DON'T NEED SSDP in WiFi AP mode
	  	#endif

	}

	#if ( ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 3 ) && ( STERGO_PLUG == 2 || STERGO_PLUG == 3 ) )  //===============================================
	// Button Handling - We have it Both in AP and STA
	if ( millis() - keyPrevMillis >=  keySampleIntervalMs )
	{
		keyPrevMillis = millis();

		byte currKeyState = digitalRead( BUTTON01 );
        
		if ( prevKeyState == HIGH && currKeyState == LOW )
			keyPress();
		else if ( prevKeyState == LOW && currKeyState == HIGH )
			keyRelease();
		else if ( currKeyState == LOW )
			longKeyPressCount++;

		prevKeyState = currKeyState;
	}
	#endif                                                                   //===============================================
}
