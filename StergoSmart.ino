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
	String message;

	Serial.begin ( SERIAL_BAUDRATE );
	delay(1000);
	//Serial.setDebugOutput(true);
  
	if ( !setupFS() )
		writeLogFile( F("No Filesystem"), 1, 3 );

	// We will Init config to gather needed data and choose next steps based on that config
	initConfig ( &message );
  
	deviceType = atoi(_deviceType);

	#ifdef MODUL_BME280
	setupBME280();
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

				getWeatherBME();
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
        			sendMeasuresWebhook();
        		}
      		}
		}
		#endif   											//=================================================
                      					
    	if ( mqtt_start == 1 )
    	{
			if ( !client.connected() )
        		MQTTreconnect();

      		#ifdef MODULE_WEATHER   						//===============================================
			String message;
    		if ( millis() - mqtt_previousMillis > mqtt_intervalHist )
        	{
        		mqtt_previousMillis = millis();
        		if ( !sendMeasuresMQTT() )
				{
					setupMQTT( &message, 1 );
					sendMeasuresMQTT();
				}	
        	}
      		#endif                                               //===============================================

        	client.loop();
      	}
		// Check if MQTT is turnued of because of too many tries
		if ( mqttTempDown == 1 )
		{
			// We will wait mqttTempDownInt and try again
			if ( ( millis() - lastmqttTempDownMillis > mqttTempDownInt ) )
    		{
				lastmqttTempDownMillis = millis();
				
				// Reset MQTT values
				mqtt_start = 1;
        		mqttTempDown = 0;
				#if ( DEBUG == 1 )
				writeLogFile( F("Resetting MQTT"), 1, 1);
				#endif
			}
		}
      
      	#ifdef MODULE_TICTACTOE									//=============================================== Exclude M-SEARCH over UDP 
		// We Have config setting for manualy start/stop Tic Tac Toe
		if ( tictac_start == 1 )
		{
			// Search for Devices in LAN
    		if ( ( millis() - previousSSDP > intervalSSDP ) || measureSSDPFirstRun )
			{
				previousSSDP = millis();
				measureSSDPFirstRun = false;
				requestSSDP(1); // This request should be done periodicaly / every 10min
    		}

			// Init M-SEARCH over UDP , SSDP Discovery Listen for TicTacToe
			handleSSDP(); //WE DON'T NEED SSDP in WiFi AP mode

			// Time interval to ask net devices to play tic tac toe
			if ( ( millis() - ticCallLMInterval > ticTacCallInterval ) || ticCallFirstRun )
			{
				ticCallLMInterval = millis();
				ticCallFirstRun = false;
				#if ( DEBUG == 1 )
				Serial.println("Inside TicTac Invite");
				#endif
				// Init Game
				inviteDeviceTicTacToe();
    		}
			// Time interval to check on inactivity of the game, auto reset
			if ( ( millis() - ticTacLastPlayed > ticTacLastPlayedInterval ) && gameStarted == 1 && turn > 0 )
			{
				writeLogFile( F("Inside LastPlayedTicTac measure and gameStarted == 1 and turn > 0"), 1, 1);
				resetTicTacToe();
    		}
			if ( ( millis() - ticTacLastPlayed > ticTacLastPlayedInterval ) && didIaskedToPlay )
			{
				writeLogFile( F("Inside LastPlayedTicTac measure and didIaskedToPlay == true"), 1, 1);
				resetTicTacToe();
    		}
		}
	  	#endif
	}

	#if ( ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 3 ) && ( STERGO_PLUG == 2 || STERGO_PLUG == 3 ) )  //===============================================
	// Button Handling - We have it Both in AP and STA 
	if ( millis() - keyPrevMillis >=  keySampleIntervalMs )
	{	// Move to SWitch.ino
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
