#ifdef MODULE_DISPLAY

void updateDisplay()
{
    unsigned long currentBlinkMillis = millis();
    if ( currentBlinkMillis - lastDisplayChange >= displayIntervalBlink )
    {
        lastDisplayChange = currentBlinkMillis;
        displayMode = ( displayMode + 1 ) % 5; // Rotate between 3 display modes
    }
    
    fill_solid( leds, NUM_LEDS, CRGB::Black ); // Clear the display
    
    //writeLogFile( F("Display Mode: ") + String(displayMode), 1, 1 );
    switch ( displayMode )
    {
        case 0:
            displayTime(false, CRGB(255, 255, 255)); // How to use RGB 
            break;
        case 1:
            displayTime(true, CRGB(255, 255, 255)); // How to use RGB 
            break;
        case 2:
            displayTemperature(CRGB::Red);
            break;
        case 3:
            displayHumidity(CRGB::Blue);
            break;
        case 4:
            displayMessage( CRGB(0, 255, 0) ); // Green color for message
            break;
    }

    // Handle fade in and fade out
    if ( fadeIn )
    {
        brightness += fadeAmount;
        if ( brightness >= maxBrightness )
        {
            brightness = maxBrightness;
            fadeIn = false;
        }
    }
    else
    {
        brightness -= fadeAmount;
        if ( brightness <= 0 )
        {
            brightness = 0;
            fadeIn = true;
        }
    }
    FastLED.setBrightness(brightness);
}

void displayTime(bool displayDate, CRGB color)
{
    char displayString[11]; // Adjust size to accommodate date string
    
    // Get the current time and adjust for time zone offset
    time_t adjustedTime = now() + timeZoneOffset;
    int adjustedHour = hour(adjustedTime);
    int adjustedMinute = minute(adjustedTime);
    int adjustedSecond = second(adjustedTime);
    int adjustedDay = day(adjustedTime);
    int adjustedMonth = month(adjustedTime);

    if (displayDate) 
        snprintf(displayString, sizeof(displayString), "%02d.%02d.", adjustedDay, adjustedMonth);
    else 
        snprintf(displayString, sizeof(displayString), "%02d%c%02d%c%02d", adjustedHour, (adjustedSecond % 2 == 0) ? ':' : ' ', adjustedMinute, (adjustedSecond % 2 == 0) ? ':' : ' ', adjustedSecond);
    
    // Log the display string
    //String logMessage = displayDate ? "Display Date: " : "Display Time: ";
    //logMessage += String(displayString);
    //writeLogFile(logMessage, 1, 1);
    
    // Display the string on the LED matrix
    for (byte i = 0; i < 10; i++) { // Adjust loop to match displayString length
        drawChar(displayString[i], i * 6, color); // Assuming each character is 6 pixels wide
    }
}



void displayTemperature(CRGB color)
{
    char tempString[20];
    snprintf(tempString, sizeof(tempString), "%.2f℃", t);

    //writeLogFile( String(tempString), 1, 1 );
    // Display the temperature string on the LED matrix
    for ( byte i = 0; i < 8; i++ )
        drawChar(tempString[i], i * 6, color); // Assuming each character is 6 pixels wide
}

void displayHumidity(CRGB color)
{
    char humString[9];
    
    snprintf(humString, sizeof(humString), "%.1f%%", h);
    //writeLogFile( F("Display Hum: ") + String(humString), 1, 1 );
    // Display the humidity string on the LED matrix
    for ( byte i = 0; i < 8; i++ )
        drawChar(humString[i], i * 6, color); // Assuming each character is 6 pixels wide
}


void displayMessage( CRGB color )
{
    unsigned long currentMillis = millis();
    if ( currentMillis - previousScrollMillis >= scrollInterval )
    {
        previousScrollMillis = currentMillis;
        for ( byte i = 0; i < 32; i++ )
        {
            int charPosition = (scrollPosition + i) % messageDisplayLength;
            drawChar(messageDisplay[charPosition], i * 6, color); // Assuming each character is 6 pixels wide
        }

        scrollPosition++; 
        if ( scrollPosition >= messageDisplayLength )
            scrollPosition = 0;
    }
}

void drawChar( char c, int x, CRGB color )
{   
    const static uint8_t PROGMEM Font[256][6] = {};
    if (c < 32 || c > 127)
        return;
        
    // Ignore non-printable characters
    for ( byte i = 0; i < 6; i++ )
    {
        //uint8_t line = Font[c - 32][i]; // Check what is better for us - at the moment without PROGMEM is crashing 
        uint8_t line = pgm_read_byte(&Font[c - 32][i]);
        for (byte j = 0; j < 8; j++)
        {
            if (line & (1 << j))
            {
                leds[(x + i) + (j * 32)] = color; // Adjust for your matrix layout
            }
        }
    }
}

void displayState()
{
    // Get arg/params posted and change settings
    // timeZoneOffset, brightness, messageDisplay, displayMode
    //String what = server.arg("state");
}
#endif