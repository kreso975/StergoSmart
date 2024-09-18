/*
   ESP8266 + BME280 + BOOTSTRAP + SPIFFS + GOOGLE CHARTS + MQTT
   + CAPTIVE PORTAL + SSDP + OTA
   Copyright (C) 2018 Kreso

   MIT
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

  /*Serial.println();
    Serial.println("Module ID: " + String(deviceType) + "\nModule Name: " + String(moduleName) + "\nTemp unit: " + String(temperature_unit));
    Serial.println("Hum unit: " + String(humidity_unit) + "\nPres unit: " + String(pressure_unit) + "\nPres adj: " + String(pressure_adjust));
    Serial.println("Pres alt h: " + String(pressure_location_adj));*/

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

	

  /*
    String statusMessage = "Year is: " + String(timeClient.getYear()) + "\n" + "Month is: " + String(timeClient.getMonth()) + "\n" + "Day is: " + String(timeClient.getDay());
    Serial.println( statusMessage );
    Serial.println(timeClient.getFormattedTime());
  */

	if ( mqtt_start )
		setupMQTT( &message, 1 );
  

	ntpUDP.begin( LOCAL_UDP_PORT );

  /*Serial.println();
    Serial.println("MQTT Server: " + String(mqtt_server) + "\nMQTT PORT: " + String(mqtt_port) + "\nMQTT cNAme: " + String(mqtt_clientName));
    Serial.println("MQTT cUserN: " + String(mqtt_clientUsername) + "\nMQTT cPass: " + String(mqtt_clientPassword) + "\nMQTT myTopic: " + String(mqtt_myTopic));
    Serial.println("MQTT UM: " + String(mqtt_bme280Humidity) + "\nMQTT Temp: " + String(mqtt_bme280Temperature) + "\nMQTT Pres: " + String(mqtt_bme280Pressure));*/

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
      		//mqtt_interval = atoi(_mqttInterval);
      		//Serial.println("MQTT Inerval is: " + String(mqtt_interval));
      		//delay(100);
      
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


      
      	#if ( STERGO_PROGRAM == 9 )   //=============================================== Exckude SSDP
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
