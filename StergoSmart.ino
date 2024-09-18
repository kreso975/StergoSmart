/*
 * StergoSmart
 * 
 * Smart Home IOT - Weather and Switches with GUI - ESP8266
 *
 *
 * ESP8266 + BME280 + BOOTSTRAP + SPIFFS + GOOGLE CHARTS + MQTT
 * + CAPTIVE PORTAL + SSDP + OTA
 *
 *
 * Copyright (C) 2018 Kresimir Kokanovic - https://github.com/kreso975/StergoSmart
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
		writeLogFile( F("No Filesystem"), 1 );
	}

	String message;

	// We will Init config to gather needed data and choose next steps based on that config
	initConfig( &message );
  
	deviceType = atoi(_deviceType);

	#if ( STERGO_PROGRAM == 1 )       //===============================================
		if ( setupBME280() )
			detectModule = true;
	#elif ( STERGO_PROGRAM == 0 )     //===============================================
		setupSwitch();
	#elif ( STERGO_PROGRAM == 3 )
		if ( setupBME280() )
			detectModule = true;

		setupSwitch();
	#endif                            //===============================================

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

		#if ( STERGO_PROGRAM == 9 )      //===============================================
    	// test for SSL
    	/*
      	for ( int i = 1; i<=5; i++ )
      	{
      	connectToSecureAndTest();
      	delay(2000);
      	}
    	*/
    	#endif                            //===============================================
	}
	else
	{
		setupHttpServer();
	}


	if ( mqtt_start )
		setupMQTT( &message, 1 );
  
	// Start ntpUDP
	// We need it for M-SEARCH over UDP - SSDP discovery
	//ntpUDP.begin( LOCAL_UDP_PORT );

}


// nothing can run if we are in 1st setup cycle
void loop()
{
	if ( WiFi.getMode() >= 2 )          // Needed only in WiFi AP mode
		dnsServer.processNextRequest();

	server.handleClient();

	// IN AP MODE we don't have Date & Time so we don't do anything except setup Device
	// And if Module (like BME280 present
	if ( WiFi.getMode() == 1 && detectModule )
	{

    	#if ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 3 )   //===============================================
    	if ( ( millis() - lastMeasureInterval > measureInterval ) || measureFirstRun )
    	{
			lastMeasureInterval = millis();
			measureFirstRun = false;

			if ( detectModule )
				getWeather();

			// This should go in detectModule
			MainSensorConstruct();
		}
		#endif                          					//=================================================

    	if ( mqtt_start )
    	{
      		#if ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 3 )   //===============================================
    		if ( millis() - mqtt_previousMillis > mqtt_intervalHist )
        	{
        		mqtt_previousMillis = millis();
        		sendMQTT();
        	}
      		#endif                                               //===============================================
      
        	if (!client.connected())
        		MQTTreconnect();

        	client.loop();
      	}


      
      	#if ( STERGO_PROGRAM == 9 )   //=============================================== Exclude M-SEARCH over UDP 
		// Search for Devices in LAN
    	if ( ( millis() - previousSSDP > intervalSSDP ) || measureSSDPFirstRun )
		{
			previousSSDP = millis();
			measureSSDPFirstRun = false;
			requestSSDP(1, 0); // This request should be done periodicaly / every 10min
			requestSSDP(2, 0); // This request should be done periodicaly / every 10min
    	}

		// Init M-SEARCH over UDP , SSDP Discovery
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
