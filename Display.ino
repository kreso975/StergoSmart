#ifdef MODULE_DISPLAY

void updateDisplay()
{
    unsigned long currentBlinkMillis = millis();
    if ( currentBlinkMillis - lastDisplayChange >= displayIntervalBlink )
    {
        lastDisplayChange = currentBlinkMillis;
        displayMode = ( displayMode + 1 ) % 4; // Rotate between 3 display modes
    }
    
    fill_solid( leds, NUM_LEDS, CRGB::Black ); // Clear the display
    
    switch ( displayMode )
    {
        case 0:
            displayTime(CRGB(255, 255, 255)); // How to use RGB 
            break;
        case 1:
            displayTemperature(CRGB::Red);
            break;
        case 2:
            displayHumidity(CRGB::Blue);
            break;
        case 3:
            displayMessage( CRGB(0, 255, 0) ); // Green color for message
            break;
    }

    // Handle fade in and fade out
    if ( fadeIn )
    {
        brightness += fadeAmount;
        if ( brightness >= BRIGHTNESS )
        {
            brightness = BRIGHTNESS;
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

void displayTime(CRGB color)
{
    char timeString[9];
    snprintf( timeString, sizeof(timeString), "%02d%c%02d%c%02d", hour(), (second() % 2 == 0) ? ':' : ' ', minute(), (second() % 2 == 0) ? ':' : ' ', second());
    // Display the time string on the LED matrix
    for ( int i = 0; i < 8; i++ )
        drawChar(timeString[i], i * 6, color); // Assuming each character is 6 pixels wide
}

void displayTemperature(CRGB color)
{
    char tempString[9];
    
    snprintf( tempString, sizeof(tempString), "Temp:%.1fC", t );
    
    // Display the temperature string on the LED matrix
    for ( int i = 0; i < 8; i++ )
    {
        drawChar(tempString[i], i * 6, color);
        // Assuming each character is 6 pixels wide
    }
}

void displayHumidity(CRGB color)
{
     char humString[9];
    snprintf(humString, sizeof(humString), "Hum:%.1f%%", h);
    
    // Display the humidity string on the LED matrix
    for ( int i = 0; i < 8; i++ )
    {
        drawChar(humString[i], i * 6, color);
        // Assuming each character is 6 pixels wide
    }
}

void displayMessage( CRGB color )
{
    for ( int i = 0; i < 32; i++ ) {
        int charPosition = (scrollPosition + i) % messageDisplayLength;
        drawChar( messageDisplay[charPosition], i * 6, color ); // Assuming each character is 6 pixels wide
    }
    scrollPosition++;
    if ( scrollPosition >= messageDisplayLength )
    {
        scrollPosition = 0;
    }
}

void drawChar( char c, int x, CRGB color )
{   
    if (c < 32 || c > 127)
        return;
        
    // Ignore non-printable characters
    for ( int i = 0; i < 6; i++ )
    {
        uint8_t line = Font[c - 32][i];
        for (int j = 0; j < 8; j++)
        {
            if (line & (1 << j))
            {
                leds[(x + i) + (j * 32)] = color; // Adjust for your matrix layout
            }
        }
    }
}
#endif