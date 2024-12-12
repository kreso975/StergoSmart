#ifdef MODULE_DISPLAY

void updateDisplay() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastDisplayChange >= displayRotateInterval) {
        lastDisplayChange = currentMillis;
        displayMode = (displayMode + 1) % 4; // Rotate between 3 display modes
    }

    
    fill_solid(leds, NUM_LEDS, CRGB::Black); // Clear the display
    
    switch (displayMode)
    {
        case 0:
            displayRotateInterval = 4000;
            drawDate(1, 0, displayColor );             // Display date
            break;
        case 1:
            displayRotateInterval = 10000;
            drawTime(1, 0, displayColor, true, false); // Display time
            break;
        case 2:
            displayRotateInterval = 5000;
            drawTempHum(0, 0, displayColor, true);       // Display Temperature true
            break;
        case 3:
            displayRotateInterval = 4000;
            drawTempHum(0, 0, displayColor, false);       // Display Humidity - false
            break;
    }
}


void drawLetter(int posx, int posy, char letter, CRGB color) {
    if ((posx > -FontWidth) && (posx < kMatrixWidth)) {
        for (int x = 0; x < FontWidth; x++) {
            for (int y = 0; y < FontHeight; y++) {
                if (bitRead(pgm_read_byte(&(Font[letter][FontWidth - 1 - x])), y) == 1) {
                    leds[XYsafe(posx + x, posy + y)] = color;
                }
            }
        }
    }
}

void drawTempHum(int x, int y, CRGB color, bool isTemperature)
{
    char tmpStr[10];

    x = 4;
    if (isTemperature)
    {
        dtostrf(t, 3, 0, tmpStr);
        drawLetter(x, 1, 'C', CRGB(255, 0, 0));
        x += FontWidth - 3;
        drawLetter(x, -5, '.', CRGB(255, 0, 0));    // Drawing '.' but move it up -5
        x += 6;
    }
    else
    {
        dtostrf(h, 3, 0, tmpStr);
        drawLetter(x, 1, '%', CRGB(0, 0, 255));
        x += FontWidth + 1;
    }

    int length = strlen(tmpStr); // Get the length of the character array
    for (int i = 0; i < length; i++)
    {
        char letter = tmpStr[length - 1 - i]; // Reverse order
        drawLetter(x, y, letter, color);
        x += FontWidth + 1; // Move to the next position
    }
}


void drawTime(int x, int y, CRGB color, bool colon, bool seconds)
{
    int hours = hour(adjustedTime);
    int minutes = minute(adjustedTime);
    int secs = second(adjustedTime);
    
    // Clear the LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black); // Clear the display
    
    // Display time on LED matrix with swapped positions
    x -= 0;
    drawLetter(x, y, minutes % 10 + '0', color);
    x += FontWidth + 1;
    drawLetter(x, y, minutes / 10 + '0', color);
    x += FontWidth;
    if ( colon )
    {
        if ( secs % 2 == 0 )
            drawLetter(x - 1, y, ':', color);
        x += 4;
    }
    drawLetter(x, y, hours % 10 + '0', color);
    x += FontWidth + 1;
    drawLetter(x, y, hours / 10 + '0', color);

    /*
    if (seconds)
        leds[XYsafe(secs * kMatrixWidth / 60, kMatrixHeight - 1)] = color;
    */
}


void drawDate(int x, int y, CRGB color)
{ 
    struct tm *ptm = gmtime((time_t*)&adjustedTime);

    // Get current date components
    int day = ptm->tm_mday;
    int month = ptm->tm_mon + 1; // tm_mon is 0-11, so we add 1

    // Clear the display
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // Display date on LED matrix
    x -= 0;
    drawLetter(x, y, month % 10 + '0', color);
    x += FontWidth + 1;
    drawLetter(x, y, month / 10 + '0', color);
    x += FontWidth + 1;
    drawLetter(x-2, y+1, '.', color);
    x += 4;
    drawLetter(x, y, day % 10 + '0', color);
    x += FontWidth + 1;
    drawLetter(x, y, day / 10 + '0', color);
}

void displayMessage(CRGB color, int numSpaces)
{
    static bool bufferInitialized = false;
    static CRGB* displayBuffer = nullptr; // Pointer to store the buffer
    static int bufferSize = 0;
    static const char* previousMessage = nullptr;
    static int previousNumSpaces = 0;

    // Check if the message or number of spaces has changed
    if (previousMessage != messageDisplay || previousNumSpaces != numSpaces)
    {
        // Free the old buffer if it exists
        if (displayBuffer != nullptr)
            delete[] displayBuffer;

        // Calculate the new buffer size
        bufferSize = (strlen(messageDisplay) + numSpaces) * 6 * kMatrixHeight;
        displayBuffer = new CRGB[bufferSize]; // Allocate memory for the new buffer

        // Initialize the buffer with the new message and spaces
        for (int i = 0; i < bufferSize; i++)
            displayBuffer[i] = CRGB::Black;

        for (int i = 0; i < strlen(messageDisplay) + numSpaces; i++)
        {
            char charToDisplay = (i < numSpaces) ? ' ' : messageDisplay[i - numSpaces];
            int charPosition = i * 6;
            for (int x = 0; x < FontWidth; x++)
            {
                for (int y = 0; y < FontHeight; y++)
                {
                    if (bitRead(pgm_read_byte(&(Font[charToDisplay][x])), y) == 1)
                    {
                        int bufferIndex = (charPosition + x) + (y * (strlen(messageDisplay) + numSpaces) * 6);
                        if (bufferIndex < bufferSize)
                            displayBuffer[bufferIndex] = color;
                    }
                }
            }
        }

        previousMessage = messageDisplay; // Update the previous message
        previousNumSpaces = numSpaces; // Update the previous number of spaces
        scrollPosition = 0; // Reset the scroll position
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black); // Clear the display

    // Copy the relevant part of the buffer to the LED matrix
    for (int x = 0; x < kMatrixWidth; x++)
    {
        for (int y = 0; y < kMatrixHeight; y++)
        {
            int bufferIndex = (scrollPosition + x) % ((strlen(messageDisplay) + numSpaces) * 6) + (y * (strlen(messageDisplay) + numSpaces) * 6);
            if (bufferIndex < bufferSize)
                leds[XYsafe(kMatrixWidth - 1 - x, y)] = displayBuffer[bufferIndex];
        }
    }

    scrollPosition++;
    if (scrollPosition >= (strlen(messageDisplay) + numSpaces) * 6)
        scrollPosition = 0;
}


/* ======================================================================
Function: displayState
Purpose : fast change for Display Params - no save 
Input   : POST
Output  : json
Comments: 
TODO    : */
void displayState()
{
    if ( server.args() == 0 )
    {
        char data[50];
        char buffer[8];
        sprintf(buffer, "#%02X%02X%02X", displayColor.r, displayColor.g, displayColor.b);
        sprintf(data, "{\"Brightness\":%d,\"color\":\"%s\",\"timeZone\":%d}", maxBrightness, buffer, 1);

        // 3 is indicator of JSON already formated reply
        sendJSONheaderReply( 3, data );
    }
    else
    {
        // Get arg/params posted and change settings
        // timeZoneOffset, brightness, messageDisplay, displayMode
        if ( server.hasArg("brightness") )
        {
            maxBrightness = (byte)server.arg("brightness").toInt();
            writeLogFile( F("Updated brightness to ") + String(maxBrightness), 1, 1 );
            
        }

        if ( server.hasArg("messageDisplay") )
        {
            messageDisplay = server.arg("messageDisplay").c_str();
            writeLogFile( F("Updated messageDisplay to ") + String(messageDisplay), 1, 1 );
        }

        if ( server.hasArg("color") )
        {
            //need to decide how to store color as a string or ...
            String hexColor = server.arg("color");
            // Example: "#A12345"
            hexColor.remove(0, 1); // Remove the '#'character
            uint32_t colorValue = strtoul(hexColor.c_str(), NULL, 16);
            displayColor = CRGB((colorValue >> 16) & 0xFF, (colorValue >> 8) & 0xFF, colorValue & 0xFF);
            writeLogFile( F("Color: ") + server.arg("color"), 1, 1 );

        }
        sendJSONheaderReply( 1, "Updated" );
    }
}


uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false)
  {
    if (kMatrixVertical == false)
      i = (y * kMatrixWidth) + x;
    else
      i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
  }

  if( kMatrixSerpentineLayout == true)
  {
    if (kMatrixVertical == false)
    {
      if( y & 0x01)
      {
        // Odd rows run backwards
        uint8_t reverseX = (kMatrixWidth - 1) - x;
        i = (y * kMatrixWidth) + reverseX;
      } 
      else
      {
        // Even rows run forwards
        i = (y * kMatrixWidth) + x;
      }
    }
    else
    { // vertical positioning
      if ( x & 0x01)
        i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
      else
        i = kMatrixHeight * (kMatrixWidth - x) - (y+1);
    }
  }
  
  return i;
}

uint16_t XYsafe( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return XY(x,y);
}

#endif