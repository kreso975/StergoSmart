/*
 * StergoSmart
 * 
 * Smart Home IOT - Weather and Switches with GUI - ESP8266
 *
 *
 * ESP8266 + BME280 + DHT + DS18B20 + BOOTSTRAP + SPIFFS + GOOGLE CHARTS + MQTT + WebHooks
 * + CAPTIVE PORTAL + SSDP + OTA + TicTacToe + LED Matrix Display 8x32 + Discord
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
	#endif
	delay(1000);
	//Serial.setDebugOutput(true);
  
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
	#endif


	WiFiManager();

	if ( WiFi.getMode() == 1 )
	{
		MDNS.begin( wifi_hostname );

		timeClient.begin();
		timeClient.update();
		setTime(timeClient.getEpochTime());
		startTime = now();

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
		timeClient.update();
		if ( timeClient.getEpochTime() != now() )
			setTime(timeClient.getEpochTime());

		#ifdef MODULE_WEATHER   							//===============================================
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
		// Check if MQTT is turned OFF because of too many tries
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
      
      	#ifdef MODULE_TICTACTOE														//=============================================== 
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
				#if ( DEBUG == 1 )
				writeLogFile( F("Inside LastPlayedTicTac measure and gameStarted == 1 and turn > 0"), 1, 1);
				#endif
				resetTicTacToe();
    		}
			if ( ( millis() - ticTacLastPlayed > ticTacLastPlayedInterval ) && didIaskedToPlay )
			{
				#if ( DEBUG == 1 )
				writeLogFile( F("Inside LastPlayedTicTac measure and didIaskedToPlay == true"), 1, 1);
				#endif
				resetTicTacToe();
    		}
		}
	  	#endif

		#ifdef MODULE_DISPLAY
		unsigned long curDispMil = millis();
		if ( curDispMil - dispPrevMils >= displayInterval )
		{
			adjustedTime = now() + timeZoneOffset;	// Adjust time by Time Zone offset 
			dispPrevMils = curDispMil;
			updateDisplay();
			for (int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8_video(maxBrightness); }
		}
		FastLED.show();
		#endif
	}

	#if ( defined( MODULE_SWITCH ) && ( STERGO_PLUG == 2 || STERGO_PLUG == 3 ) )  //===============================================

	checkSwitchButton();	// Check Button State - long press

	#endif                                                                   	  //===============================================
}
