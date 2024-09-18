/* ======================================================================
Function: setupSwitch
Purpose : Initialize Switch
Input   : 
Output  : 
Comments: -
====================================================================== */
bool setupSwitch()
{
  digitalWrite( RELAY, HIGH );
  pinMode( RELAY, OUTPUT );
  
  #if ( STERGO_PLUG == 2 || STERGO_PLUG == 3 )
    pinMode( LED, OUTPUT );
    pinMode( BUTTON01, INPUT );
    digitalWrite( LED, HIGH );
  #endif

  return true;
}

void turnSwitchState( int state )
{
  if ( state == 1 ) // Turn the Relay on/off
  {
    digitalWrite( RELAY, HIGH );

    relay01State = true;
    if (mqtt_start == 1)
    {
      sendMQTT ();
    }
    server.sendHeader( "Access-Control-Allow-Origin", "*" );
    server.send( 200, "application/json", POWERON );
  }  
  else
  {
    digitalWrite( RELAY, LOW );

    relay01State = false;
    if (mqtt_start == 1)
    {
      sendMQTT ();
    }
    server.sendHeader( "Access-Control-Allow-Origin", "*" );
    server.send( 200, "application/json", POWEROFF );
  }

    //return true;
}

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
    {
      server.sendHeader( "Access-Control-Allow-Origin", "*" );
      server.send( 200, "application/json", POWERON );
    }
    else
    {
      server.sendHeader( "Access-Control-Allow-Origin", "*" );
      server.send( 200, "application/json", POWEROFF );
    }
  }
}


#if ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 3 )
// Sonoff S26 && Sonoff T4EU1C
#if ( STERGO_PLUG == 2 || STERGO_PLUG == 3 )                  //===============================================

// called when button is kept pressed for less than 2 seconds
void shortKeyPress()
{
  if ( relay01State )
    turnSwitchState( 0 );
  else
    turnSwitchState( 1 );
}


// called when button is kept pressed for more than 2 seconds
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

// called when key goes from not pressed to pressed
void keyPress()
{
  //Serial.println("key press");
  longKeyPressCount = 0;
}

// called when key goes from pressed to not pressed
void keyRelease()
{
  //Serial.println("key release");
    
  if ( longKeyPressCount >= longKeyPressCountMax )
    longKeyPress();
  else
    shortKeyPress();
}
#endif                                                    //===============================================

#endif