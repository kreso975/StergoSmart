/* ======================================================================
Function: setupMQTT
Input: what == 0 | 1 / stop | start mqtt functionality
TODO: 
Comments: 
====================================================================== */
bool setupMQTT( String* message, int what )
{  
  if ( what == 1 )
  {
    // If we submited again to start MQTT and it's already running
    if ( !client.connected() )
    {
  		client.setServer( mqtt_server, mqtt_port );

  		#if ( STERGO_PROGRAM == 0  || STERGO_PROGRAM == 3 )     //====================================================
    	client.setCallback(callbackMQTT);                       // Only Switch needs listening, Weather just publishes
  		#endif                                                  //====================================================
      
  		if ( !client.connected() )
  		{
    		if ( MQTTreconnect() )
    		{
    			*message = F("Success MQTT Start");
    			writeLogFile( *message, 1 );
      			return true;
    		}
  		}
    }
    else
    {
      *message = F("Success MQTT already running");
      writeLogFile( *message, 1 );
      return true;
    }
	}
	else if ( what == 0 )
	{
		if ( client.connected() )
			client.disconnect();

		return true;
	}
  
  	*message = F("Error MQTT service");
	return false; 
	
}

bool MQTTreconnect()
{
  // Loop until we're reconnected || return false
  while ( !client.connected() )
  {
    writeLogFile( F("MQTT Reconnecting"), 1 );
    //Serial.println( String(mqtt_clientName) + "  -  " + String(mqtt_clientUsername) + "  -  " + String(mqtt_clientPassword) );
    if ( client.connect( mqtt_clientName, mqtt_clientUsername, mqtt_clientPassword ) )
    {
      //writeLogFile( F("MQTT connected"), 1 );
      client.publish( mqtt_myTopic, "connected" );            // Once connected, publish an announcement...
      #if ( STERGO_PROGRAM == 0  || STERGO_PROGRAM == 3 )     //===============================================
      client.subscribe(mqtt_switch);                          // We will subsribe only if Switch
      #endif                                                  //===============================================
    }
    else
    {
      if ( mqtt_connectTry > 0 )
      {
        mqtt_connectTry--;
        
        //writeLogFile( statusMessage, 1 );
        // Wait 2 seconds before retrying
        delay(2000);
      }
      else
      {
        //No success
        mqtt_start = 0;
        // here I must find a way to send back to web page an ERROR (descriptive) - if it was started from web page
        return false;
      }
    }
  }
 
  return true;
}


// Called from StergoSmart.ino = Triggered regarding time interval.
// Need fix to start using TimeInterval from config
bool sendMQTT ()
{
  #if ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 3 )               //===============================================
    char humidityString[6];
    char pressureString[7];

    if ( client.connected() )
    {
      dtostrf(h, 5, 1, humidityString);
      dtostrf(P0, 6, 1, pressureString);

      //writeLogFile( F("Sending payload: "), 1 );
      //String temp = "";temp += t;temp += "";
      //String hum = "";hum += humidityString;hum += "";
      //String pres = "";pres += P0;pres += "";

      // Let's try to publish Temperature
      if ( client.publish( mqtt_bme280Temperature, (char*) String(t).c_str(), true ) )
      {
        //writeLogFile( F("Publish Temperature: ok"), 1 );
      }
      else
      {
        //writeLogFile( F("Publish Temperature: failed"), 1 );
      }
      
      // Let's try to publish Humidity
      if ( client.publish( mqtt_bme280Humidity, humidityString, true ) )
      {
        //writeLogFile( F("Publish Humidity: ok"), 1 );
      }
      else
      {
        //writeLogFile( F("Publish Humidity: failed"), 1 );
      }
    
      // Let's try to publish Pressure
      if ( client.publish( mqtt_bme280Pressure, pressureString, true ) )
      {
        //writeLogFile( F("Publish Pressure: ok"), 1 );
      }
      else
      {
        //writeLogFile( F("Publish Pressure: failed"), 1 );
      }
    
      return true;
    }
    else
    {
      // We are not connected, Write it in Log
      writeLogFile( F("MQTT Pub No Connect"), 1 );
    }
    
  #endif                                                           //===============================================

  #if ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 3 )               //===============================================
    if ( client.connected() )
    {
      // Prep publish for PowerState change
      // Let's try to publish Temperature
      if ( client.publish( mqtt_switch, (char*) String(relay01State).c_str(), true ) )
      {
        //writeLogFile( F("MQTT Switch: ok"), 1 );
      }
      else
      {
        //writeLogFile( F("MQTT Switch: failed"), 1 );
      } 
    }
  #endif                                                           //===============================================

  return false;
}

                                                                   // Only for Switches
#if ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 3 )                 //===============================================
void callbackMQTT( char* topic, byte* payload, unsigned int length )
{
    if ( String(topic) == String(mqtt_switch) )
    {
      if ( (char)payload[0] == '1' ) {          // Turn the Relay on/off
        digitalWrite( RELAY, HIGH );
        relay01State = true;
      } 
      if ( (char)payload[0] == '0' ) {
        digitalWrite( RELAY, LOW );
        relay01State = false;
      }   
    }
    else if ( String(topic) == String(mqtt_switch2) )
    {
      if ((char)payload[0] == '1')  // Turn the LED on/off
        digitalWrite( LED2, 0);  
      else
        digitalWrite( LED2, 1);
    }
     
}
#endif                                                                 //===============================================
