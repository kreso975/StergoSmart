/*
======================================================================
Function: setupMQTT
Input: what == 0 | 1 / stop | start mqtt functionality
TODO: 
Comments:*/
bool setupMQTT( String* message, int what )
{ 
  String msg; 
  if ( what == 1 )
  {
    // If we submited again to start MQTT and it's already running
    if ( !client.connected() )
    {
  		client.setServer( mqtt_server, mqtt_port );

      #if defined( MODULE_SWITCH ) || defined( MODULE_DISPLAY ) //======================================================
    	client.setCallback(callbackMQTT);                         // Only Switch & Display listens, Weather just publishes
  		#endif                                                    //======================================================
      
  		if ( !client.connected() )
  		{
    		if ( MQTTreconnect() )
    		{
          msg = F("Success MQTT Start");
    			*message = F("{\"success\":\"") + msg + F("\"}");
    			writeLogFile( msg, 1, 3 );
      		return true;
    		}
  		}
    }
    else
    {
      msg = F("Success MQTT already running");
    	*message = F("{\"success\":\"") + msg + F("\"}");
      writeLogFile( msg, 1, 3 );
      return true;
    }
	}
	else if ( what == 0 )
	{
		if ( client.connected() )
			client.disconnect();

    msg = F("Success MQTT Stop");
    *message = F("{\"success\":\"") + msg + F("\"}");
    writeLogFile( msg, 1, 3 );
		
    return true;
	}
  
  //*message = F("Error MQTT service");
	return false; 
	
}

bool MQTTreconnect()
{
  // Loop until we're reconnected || return false
  while ( !client.connected() )
  {
    //writeLogFile( F("MQTT Reconnecting"), 1, 3 );
    if ( client.connect( mqtt_clientName, mqtt_clientUsername, mqtt_clientPassword ) )
    {
      //writeLogFile( F("MQTT connected"), 1 );
      client.publish( mqtt_myTopic, "connected" );            // Once connected, publish an announcement...
      #ifdef MODULE_SWITCH                                    //===============================================
      client.subscribe(mqtt_switch);                          // We will subsribe only if Switch
      #endif                                                  //===============================================
      #ifdef MODULE_DISPLAY                                   //===============================================
      client.subscribe(mqtt_Brightness);                      // mqtt_Brightness == maxBrightness
      client.subscribe(mqtt_Color);                           // mqtt_Color == RGB() or HSV()
      client.subscribe(mqtt_displayON);                       // mqtt_displayON == ON | OFF
      #endif                                                  //===============================================
    }
    else
    {
      if ( mqtt_connectTry > 0 )
      {
        mqtt_connectTry--;
        // Wait 2 seconds before retrying
        delay(2000);
      }
      else
      {
        //No success We shut down MQTT - but Need to find a way to retry in an hour, also send Discord if its setup
        mqtt_start = 0;
        mqttTempDown = 1; // So that we know reconnect issue and we need to try later
        writeLogFile( F("MQTT NoT Reconnected"), 1, 3 );
        // here I must find a way to send back to web page an ERROR (descriptive) - if it was started from web page
        return false;
      }
    }
  }
 
  return true;
}

/* ==========================================================================================
Function: sendMQTT
Purpose : send MQTT payload 
Input   : Topic, Payload, retain
Output  : Success = True | Fail = False
Comments: 
TODO    :  */
bool sendMQTT ( char* Topic, char* Payload, bool retain )
{
  bool checkStat = true;

  if ( client.connected() )
  {
    // Let's try to publish
    if ( !client.publish( Topic, Payload, retain ) )
    {
      //writeLogFile( F("Publish Temperature: failed"), 1 );
      checkStat = false;
    }
        
    return checkStat;
  }
  else
  {
    // We are not connected, Write it in Log
    #if ( DEBUG == 1 )
    writeLogFile( F("MQTT Pub Not Connected"), 1, 3 );
    #endif
    return false;
  }  
  return checkStat;
}

// Only for Switches
#if defined( MODULE_SWITCH ) || defined( MODULE_DISPLAY )         //===============================================
void callbackMQTT( char* topic, byte* payload, unsigned int length )
{
  #ifdef MODULE_SWITCH
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
      // No current purpose Except for ESP8266 01S turning light onboard on/off
      #ifdef LED2
      if ((char)payload[0] == '1')  // Turn the LED on/off
        digitalWrite( LED2, 0);  
      else
        digitalWrite( LED2, 1);
      #endif
    }
  #endif
  #ifdef MODULE_DISPLAY
    if ( String(topic) == String(mqtt_displayON) )
    { 
      //displayON = (byte)payload[0];
      if( (char)payload[0] == '1' )
        displayON = 1;
      if( (char)payload[0] == '0' )
        displayON = 0;
      
    }
    if ( String(topic) == String(mqtt_Brightness) )
    {
      payload[length] = '\0'; // Null-terminate the payload
      maxBrightness = atoi((char*)payload);
    }
    if ( String(topic) == String(mqtt_Color) )
    {
      // Null-terminate the payload
      payload[length] = '\0';
      // Convert payload to String
      String hexColor = String((char*)payload);
      displayColor = strtol(&hexColor[1], NULL, 16);
    }
  #endif

}
#endif                                                                 //===============================================
