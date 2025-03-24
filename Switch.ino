#ifdef MODULE_SWITCH


/* ======================================================================
Function: setupSwitch
Purpose : Initialize Switch
Input   : 
Output  : 
Comments: -  */
bool setupSwitch()
{
  digitalWrite( RELAY, HIGH );
  pinMode( RELAY, OUTPUT );
  
  #if ( STERGO_PLUG == 2 || STERGO_PLUG == 3 )
    pinMode( LED, OUTPUT );
    pinMode( BUTTON01, INPUT );
    digitalWrite( LED, HIGH );
  #endif

  // Add MQTT subscriptions
  subscriptionList.push_back(mqtt_switch);
  //subscriptionList.push_back(mqtt_switch2);
  
  mqttManager.registerCallback(callbackSwitchMQTT, 1); 		// register Switch callback for MQTT

  return true;
}

/* ======================================================================
Function: turnSwitchState
Purpose : Turn the Switch on/off
Input   : int state | 0 == OFF or 1 == ON
Output  : digitalWrite( RELAY, HIGH || LOW )
Comments: -  */
void turnSwitchState( int state )
{
  if ( state == 1 ) // Turn the Relay on/off
  {
    digitalWrite( RELAY, HIGH );

    relay01State = true;
    if ( mqttManager.getMqttStart() )
      sendSwitchMQTT();

    sendJSONheaderReply( 3, POWERON );
  }  
  else
  {
    digitalWrite( RELAY, LOW );

    relay01State = false;
    if ( mqttManager.getMqttStart() )
      sendSwitchMQTT();

    sendJSONheaderReply( 3, POWEROFF );
  }

    //return true;
}

/* ======================================================================
Function: checkSwitchState
Purpose : Get state of the Switch on/off 
Input   : -
Output  : JSON {\"POWER\":\"ON || OFF\"}
Comments: -  */
void checkSwitchState()
{    
  // Get arg from client browser GET
  String what = server.arg("state");
  //int att = server.arg("what").toInt();
    
  if ( what == "ON" )
    turnSwitchState(1);
  else if ( what == "OFF" )
    turnSwitchState(0);
  else
  {
    if ( relay01State )
      sendJSONheaderReply( 3, POWERON );
    else
      sendJSONheaderReply( 3, POWEROFF );
  }
}

/* ======================================================================
Function: sendSwitchMQTT
Purpose : Send state of the Switch on/off 
Input   : -
Output  : bool true || false on success
Comments: -  */
bool sendSwitchMQTT()
{
  bool checkStat = true;
  // We are sending Switch state on change
  if ( !mqttManager.sendMQTT( mqtt_switch, (char*) String(relay01State).c_str(), true ) )
  {
    #if ( DEBUG == 1 )
    writeLogFile( F("MQTT Switch: failed"), 1 );
    #endif
    checkStat = false;
  }
  return checkStat;
}


// Sonoff S26 && Sonoff T4EU1C
#if ( STERGO_PLUG == 2 || STERGO_PLUG == 3 )                  //===============================================

/* ======================================================================
Function: checkSwitchButton
Purpose : Check for Button Long or Short Press  
Input   : -
Output  : 
Comments: -  */
void checkSwitchButton()
{
  // Button Handling - We have it Both in AP and STA 
	// This can be moved into Switch.ino
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
}

/* ======================================================================
Function: shortKeyPress
Purpose : when button is kept pressed for less than 2 seconds 
Input   : -
Output  : 
Comments: -  */
void shortKeyPress()
{
  if ( relay01State )
    turnSwitchState( 0 );
  else
    turnSwitchState( 1 );
}

/* ======================================================================
Function: longKeyPress
Purpose : when button is kept pressed for more than 2 seconds 
Input   : -
Output  : 
Comments: -  */
void longKeyPress()
{
  if ( ledStatus )
  {
    digitalWrite( LED, HIGH );
    ledStatus = false;
  }
  else
  {
    digitalWrite( LED, LOW );
    ledStatus = true;
  }
  //writeLogFile( F("longKeyPress"), 1 );
}

/* ======================================================================
Function: keyPress
Purpose : when key goes from not pressed to pressed
Input   : -
Output  : 
Comments: -  */
void keyPress()
{
  //Serial.println("key press");
  longKeyPressCount = 0;
}

/* ======================================================================
Function: keyRelease
Purpose : when key goes from pressed to not pressed
Input   : -
Output  : 
Comments: -  */
void keyRelease()
{
  //Serial.println("key release");
    
  if ( longKeyPressCount >= longKeyPressCountMax )
    longKeyPress();
  else
    shortKeyPress();
}
#endif                                                    // Button Control===============================================


void callbackSwitchMQTT(char *topic, byte *payload, unsigned int length)
{
	#ifdef MODULE_SWITCH
	if (String(topic) == String(mqtt_switch))
	{
		if ((char)payload[0] == '1')
		{ // Turn the Relay on/off
			digitalWrite(RELAY, HIGH);
			relay01State = true;
		}
		if ((char)payload[0] == '0')
		{
			digitalWrite(RELAY, LOW);
			relay01State = false;
		}
	}
	else if (String(topic) == String(mqtt_switch2))
	{
		// No current purpose Except for ESP8266 01S turning light onboard on/off
		#ifdef LED2
		if ((char)payload[0] == '1') // Turn the LED on/off
			digitalWrite(LED2, 0);
		else
			digitalWrite(LED2, 1);
		#endif
	}
	#endif
}

#endif