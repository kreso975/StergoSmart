#ifdef MODULE_DISPLAY

/* ======================================================================
Function: updateDisplay
Purpose : Main Display Constructor 
Input   : None
Output  : None
Comments: Updates the display with time, date, temperature, humidity, or message based on the current state.
TODO    : */
void updateDisplay()
{
	if ( displayON == 1 )	// if display is SET ON
	{
		unsigned long currentMillis = millis();
		if ( messageON )
		{
			if ( messageWinON )
				renderDisplayWin( currentMillis );				// Display Win message - moved to separate function
			else
				renderDisplayMessage( currentMillis );			// Display Message - moved to separate function
		}
		else
		{
			if ( currentMillis - dispPrevMils >= displayInterval )
			{
				FastLED.clearData();
				adjustedTime = now() + timeZoneOffset;	// Adjust time by Time Zone offset 
				dispPrevMils = currentMillis;
				
				if (currentMillis - lastDisplayChange >= displayRotateInterval)
				{
					lastDisplayChange = currentMillis;
					displayMode = (displayMode + 1) % 4; // Rotate between 4 display modes
				}
				
				switch (displayMode)
				{
					case 0:
							displayRotateInterval = 4000;              // 4 sec
							drawDate(tempBufferDate, 1, 0, displayColor);
							for (int i = 0; i < NUM_LEDS; i++) { leds[i] = tempBufferDate[i];}
							break;
					case 1:
							displayRotateInterval = 10000;             // 10 sec
							drawTime(1, 0, displayColor, true, false); // Display time
							break;
					case 2:
							displayRotateInterval = 5000;              // 5 sec
							drawTempHum(0, 0, displayColor, true);     // Display Temperature true
							break;
					case 3:
							displayRotateInterval = 4000;              // 4 sec
							drawTempHum(0, 0, displayColor, false);    // Display Humidity - false
							break;
				}
			}
		}
	}
	// Always show LEDs
	FastLED.show();
}

void drawLetter( int posx, int posy, char letter, CRGB color )
{
	if ((posx > -FontWidth) && (posx < kMatrixWidth))
		for (int x = 0; x < FontWidth; x++)
			for (int y = 0; y < FontHeight; y++)
				if (bitRead(pgm_read_byte(&(Font[letter][FontWidth - 1 - x])), y) == 1)
					leds[XYsafe(posx + x, posy + y)] = color;
}

void drawLetterBuf( CRGB *buffer, int posx, int posy, char letter, CRGB color )
{
	if ((posx > -FontWidth) && (posx < kMatrixWidth))
		for (int x = 0; x < FontWidth; x++)
			for (int y = 0; y < FontHeight; y++)
				if (bitRead(pgm_read_byte(&(Font[letter][FontWidth - 1 - x])), y) == 1)
					buffer[XYsafe(posx + x, posy + y)] = color;
}

void drawTempHum( int x, int y, CRGB colorText, bool isTemperature )
{
	char tmpStr[10];
	CRGB temperatureColor = CRGB(255, 0, 0);
	CRGB humidityColor = CRGB(0, 0, 255);
	temperatureColor.nscale8(maxBrightness);
	humidityColor.nscale8(maxBrightness);
	colorText.nscale8(maxBrightness);

	x = 4;
	if (isTemperature)
	{
		dtostrf(t, 3, 0, tmpStr);
		drawLetter(x, 0, 'C', temperatureColor);
		x += FontWidth - 3;
		drawLetter(x, -5, '.', temperatureColor); // Drawing '.' but move it up -5
		x += 6;
	}
	else
	{
		dtostrf(h, 3, 0, tmpStr);
		drawLetter(x, 1, '%', humidityColor);
		x += FontWidth + 1;
	}

	int length = strlen(tmpStr); // Get the length of the character array
	for (int i = 0; i < length; i++)
	{
		char letter = tmpStr[length - 1 - i]; // Reverse order
		drawLetter(x, y, letter, colorText);
		x += FontWidth + 1; // Move to the next position
	}
}

void drawTime( int x, int y, CRGB colorTime, bool colon, bool seconds )
{
    int hours = hour(adjustedTime);
    int minutes = minute(adjustedTime);
    int secs = second(adjustedTime);

	 colorTime.nscale8(maxBrightness);
    
    // Display time on LED matrix with swapped positions
    x -= 0;
    drawLetter(x, y, minutes % 10 + '0', colorTime);
    x += FontWidth + 1;
    drawLetter(x, y, minutes / 10 + '0', colorTime);
    x += FontWidth;
    if ( colon )
    {
        if ( secs % 2 == 0 )
            drawLetter(x - 1, y, ':', colorTime);
        x += 4;
    }
    drawLetter(x, y, hours % 10 + '0', colorTime);
    x += FontWidth + 1;
    drawLetter(x, y, hours / 10 + '0', colorTime);
}

void drawDate( CRGB *buffer, int x, int y, CRGB colorDate )
{
	static int lastDays = -1;
	static int lastMonths = -1;
	static CRGB lastColorDate = CRGB::Black;
	static bool initialized = false;

	// Get current date components
	int days = day(adjustedTime);
	int months = month(adjustedTime);

	// Check if it's the first run or if the date or color has changed
	if (!initialized || days != lastDays || months != lastMonths || colorDate != lastColorDate)
	{
		// Update the buffer
		memset(buffer, 0, sizeof(CRGB) * NUM_LEDS); // Clear the buffer
		colorDate.nscale8(maxBrightness);

		// Display date on LED matrix
		int posX = x;
		drawLetterBuf(buffer, posX, y, months % 10 + '0', colorDate);
		posX += FontWidth + 1;
		drawLetterBuf(buffer, posX, y, months / 10 + '0', colorDate);
		posX += FontWidth + 1;
		drawLetterBuf(buffer, posX - 2, y + 1, '.', colorDate);
		posX += 3;
		drawLetterBuf(buffer, posX, y, days % 10 + '0', colorDate);
		posX += FontWidth + 1;
		drawLetterBuf(buffer, posX, y, days / 10 + '0', colorDate);

		// Update the last known values
		lastDays = days;
		lastMonths = months;
		lastColorDate = colorDate;
		initialized = true; // Mark as initialized
	}
}

/* =======displayMessage=================================================
Function: displayMessage
Purpose : Scroll text message on the LED matrix 
Input   : buffer: buffer where to write,
			 colorScroll: color for letters,
			 message: send or predefined,
			 numSpaces: number of spaces in front of the message
Output  : Scroll
Comments: 
TODO    : */
void displayMessage( CRGB colorScroll, const char *message, int numSpaces )
{
	static bool bufferInitialized = false;
	static String previousMessage = "";
	static int previousNumSpaces = 0;
	colorScroll.nscale8(maxBrightness);

	String convertedMessage = convertToSingleByte(message);

	// Check if the message or number of spaces has changed
	if (previousMessage != convertedMessage || previousNumSpaces != numSpaces)
	{
		if  (displayBuffer != nullptr )		// Free the old buffer if it exists
			delete[] displayBuffer;

		// Calculate the new buffer size
		bufferSize = (convertedMessage.length() + numSpaces) * 6 * kMatrixHeight;
		displayBuffer = new CRGB[bufferSize];  // Allocate memory for the new buffer

		for (int i = 0; i < bufferSize; i++)	// Initialize the buffer with the new message and spaces
			displayBuffer[i] = CRGB::Black;

		// Add spaces in front of the message
		for (int i = 0; i < numSpaces; i++)
		{
			int charPosition = i * 6;
			for (int x = 0; x < 6; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					int bufferIndex = (charPosition + x) + (y * (convertedMessage.length() + numSpaces) * 6);
					if (bufferIndex < bufferSize)
						displayBuffer[bufferIndex] = CRGB::Black;
				}
			}
		}

		for (int i = 0; i < convertedMessage.length(); i++)
		{
			char charToDisplay = convertedMessage.charAt(i);
			int charPosition = (i + numSpaces) * 6;
			for (int x = 0; x < 6; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					if (bitRead(pgm_read_byte(&(Font[charToDisplay][x])), y) == 1)
					{
						int bufferIndex = (charPosition + x) + (y * (convertedMessage.length() + numSpaces) * 6);
						if (bufferIndex < bufferSize)
							displayBuffer[bufferIndex] = colorScroll;
					}
				}
			}
		}

		previousMessage = convertedMessage; // Update the previous message
		previousNumSpaces = numSpaces;		// Update the previous number of spaces
		scrollPosition = 0;						// Reset the scroll position
	}

	// Copy the relevant part of the buffer to the LED matrix
	for (int x = 0; x < kMatrixWidth; x++)
	{
		for (int y = 0; y < kMatrixHeight; y++)
		{
			int bufferIndex = (scrollPosition + x) % ((convertedMessage.length() + numSpaces) * 6) + (y * (convertedMessage.length() + numSpaces) * 6);
			if ( bufferIndex < bufferSize )
				leds[XYsafe(kMatrixWidth - 1 - x, y)] += displayBuffer[bufferIndex]; // Use buffer instead of leds
		}
	}

	scrollPosition++;
	if (scrollPosition >= (convertedMessage.length() + numSpaces) * 6)
		scrollPosition = 0;
}

// void freeDisplayMessageBuffer() { if (displayBuffer != nullptr) { delete[] displayBuffer; displayBuffer = nullptr; } }

void renderDisplayMessage( unsigned long currentMillis )
{
	//
	// DRAW TEXT SCROLL
	if ( currentMillis - previousMillisMessage >= intervalMessage )
	{
		FastLED.clearData();
		previousMillisMessage = currentMillis;
		//memset( tempBufferMessage, 0, sizeof(tempBufferMessage) ); // Clear temp buffer
		displayMessage( CRGB::Red, messageDisplay, 6); // Update temp buffer | using buffer if we want add something over / effect
		
		//for (int i = 0; i < NUM_LEDS; i++)	// Fill LEDS from buffer
		//	leds[i] = tempBufferMessage[i];
	}
	
	if ( currentMillis - prevMilMesLife >= displayMessageLife )
	{
		server.begin(); // We have stop it when set messageON = true in displayState()
		//memset( tempBufferMessage, 0, sizeof(tempBufferMessage) ); // Clear temp buffer
		//freeDisplayMessageBuffer(); 
		messageON = false; // Set messageON to false after 10 seconds 
	}
}

/* =======Particle======================================================
Class   : Particle
Purpose : Represents a particle with position, color, velocity, and life attributes
Members :
    - float x, y: Position of the particle
    - CRGB color: Color of the particle
    - float xvel, yvel: Velocity of the particle in the x and y directions
    - int life: Life duration of the particle

Methods :
    - Particle(float x, float y, CRGB color): Constructor to initialize the particle with position, color, and random velocity
    - void update(CRGB* buffer): Updates the particle's position, slows it down, and decreases its life
    - void draw(CRGB* buffer): Draws the particle on the buffer and updates its position
    - bool isDead(): Checks if the particle's life has ended
    - void slowDown(): Slows down the particle's velocity

Comments: 
    - The constructor initializes the particle's position, color, and velocity based on global constants.
    - The update method calls the draw method, slows down the particle, and decreases its life.
    - The draw method ensures the particle stays within bounds and updates its position.
    - The isDead method checks if the particle's life has ended.
    - The slowDown method reduces the particle's velocity using a global deceleration factor. */
class Particle
{
public:
	float x, y;
	CRGB color;
	float xvel, yvel;
	int life;

	Particle(float x, float y, CRGB color)
	{
		this->x = x;
		this->y = y;
		this->color = color;
		float velocityFactor = 100.0 / INITIAL_VELOCITY;
		this->xvel = random(-100, 100) / velocityFactor; // Use global initial velocity
		this->yvel = random(-100, 100) / velocityFactor; // Use global initial velocity
		this->life = PARTICLE_LIFE;					       // Use global particle life
	}

	void update(CRGB* buffer)
	{
		draw(buffer);
		slowDown();
		life -= 5;
	}


	void draw( CRGB* buffer )
	{
		// adjust Particle brightness - 0 to 255
		color.nscale8(min(maxBrightness * 4, 255));

		// Ensure particles stay within bounds
		if ( x >= 0 && x < kMatrixWidth && y >= 0 && y < kMatrixHeight )
		{
			int index = XYsafe(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
			if ( index != -1 )
				buffer[index] += color; // Use additive blending
		}
		x += xvel;
		y += yvel;

		// Mark particle as dead if out of bounds
		if ( x < 0 || x >= kMatrixWidth || y < 0 || y >= kMatrixHeight )
			life = -1;
	}

	bool isDead() {
		return life < 0;
	}

	void slowDown() {
		xvel *= DECELERATION_FACTOR; // Use global deceleration factor
		yvel *= DECELERATION_FACTOR; // Use global deceleration factor
	}
};

std::vector<Particle> particles;

void resetParticles()
{
  particles.clear(); // Clear the particles vector
  // Reinitialize any other necessary variables here
}

void addParticles( CRGB* buffer )
{
	//  Continuously generate explosion particles, concentrating between rows 3 and 6
	if (random(10) < EXPLOSION_FREQUENCY)
	{
		int x = random(kMatrixWidth);
		int y = random(1, 6); // Concentrate explosions between rows 1 and 6
		CRGB color = CRGB(random(256), random(256), random(256));

		for (int i = 0; i < PARTICLE_COUNT; i++)
			particles.push_back(Particle(x, y, color));
	}

	for (size_t i = 0; i < particles.size(); i++)
	{
		particles[i].update(buffer); // Update each particle

		if (particles[i].isDead())
		{
			particles.erase(particles.begin() + i);
			i--;
		}
	}
}

void drawWIN( const byte coords[][2], int size, CRGB *buffer, CRGB color )
{
	for (int i = 0; i < size; i++)
		buffer[XYsafe(coords[i][0], coords[i][1])] = color;
}

void drawText( CRGB *buffer )
{
	static uint8_t brightness = 30;
	static bool increasing = true;

	// Draw flipped "WIN" in the center with varying brightness
	static CRGB color = CRGB(255, 0, 0);
	static uint8_t lastBrightness = 30;
	if ( brightness != lastBrightness )
	{
		color = CRGB(255, 0, 0);
		color.nscale8(brightness);
		lastBrightness = brightness;
	}

	// Draw letters using the optimized function
	drawWIN(W_coords, sizeof(W_coords) / sizeof(W_coords[0]), buffer, color);
	drawWIN(I_coords, sizeof(I_coords) / sizeof(I_coords[0]), buffer, color);
	drawWIN(N_coords, sizeof(N_coords) / sizeof(N_coords[0]), buffer, color);

	// Adjust brightness
	if ( increasing )
	{
		brightness += 30;
		if ( brightness >= min(maxBrightness*4, 255) )
			increasing = false;
	}
	else
	{
		brightness -= 30;
		if ( brightness <= 30 )
			increasing = true;
	}
}

void renderDisplayWin( unsigned long currentMillis )
{
	FastLED.clearData();
	// DRAW WIN WITH FIREWORKS
	if ( renderWIN )
	{
		if ( currentMillis - previousMillisText >= intervalText )
		{
			writeLogFile( "renderWIN", 1, 1 );
			previousMillisText = currentMillis;
			memset( tempBufferText, 0, sizeof(tempBufferText) ); // Clear temp buffer
			drawText(tempBufferText); // Update temp buffer
		}
	}
	
	// Update addParticles at its own interval
	if ( currentMillis - previousMillisParticles >= intervalParticles )
	{
		writeLogFile( "render Particle", 1, 1 );
		previousMillisParticles = currentMillis;
		memset( tempBufferParticles, 0, sizeof(tempBufferParticles) ); // Clear temp buffer
		addParticles( tempBufferParticles ); 									// Update temp buffer
	}
	
	// Combine buffers
	for (int i = 0; i < NUM_LEDS; i++)
		leds[i] = tempBufferText[i] + tempBufferParticles[i]; // Combine buffers tempBufferText[i] + 

	if ( currentMillis - prevMilMesLife >= displayMessageLife )
	{
		FastLED.clearData();
		memset( tempBufferText, 0, sizeof(tempBufferText) ); // Clear temp buffer
		memset( tempBufferParticles, 0, sizeof(tempBufferParticles) ); // Clear temp buffer
		messageON = false; // Set messageON to false after 10 seconds
		messageWinON = false;
		renderWIN = true;
		resetParticles(); // Clear the particles vector
		server.begin(); // We have stop it when set messageON = true in displayState()
	}
}
/* ======================================================================
Function: displayState
Purpose : fast update for Display Params - return JSON on status no save 
Input   : POST || Blank == return JSON
Output  : nothing || return JSON
Comments: 
TODO    : */
void displayState()
{
	char buffer[8];

	if (server.args() == 0)
	{
		char data[90];
		sprintf(buffer, "#%02X%02X%02X", displayColor.r, displayColor.g, displayColor.b);
		sprintf(data, "{\"displayON\":%d,\"Brightness\":%d,\"displayColor\":\"%s\",\"timeZone\":%d}", displayON, maxBrightness, buffer, 1);

		// 3 is indicator of JSON already formated reply
		sendJSONheaderReply(3, data);
	}
	else
	{
		// Get arg/params posted and change settings
		// timeZoneOffset, brightness, messageDisplay, displayMode
		if (server.hasArg("brightness"))
		{
			maxBrightness = server.arg("brightness").toInt();
			itoa(maxBrightness, buffer, 10); // Convert
			writeLogFile(F("Updated brightness to ") + String(maxBrightness), 1, 1);
			if (mqtt_start == 1)
				if (!sendMQTT(mqtt_Brightness, buffer, true))
					writeLogFile(F("Publish Brightness: failed"), 1);
		}

		if (server.hasArg("messageDisplay"))
		{
			messageDisplay = server.arg("messageDisplay").c_str();
			// writeLogFile( F("Updated messageDisplay to ") + String(messageDisplay), 1, 1 );
			messageON = true;
			server.stop(); // Stopping webServer because it scrambles scroll buffer if accessed during scroll
			prevMilMesLife = millis();
		}

		if (server.hasArg("color"))
		{
			String hexColor = server.arg("color");
			// Example: "#A12345"
			hexColor.toCharArray(buffer, sizeof(buffer));
			hexColor.remove(0, 1); // Remove the '#'character
			displayColor = strtoul(hexColor.c_str(), NULL, 16);
			if (mqtt_start == 1)
				if (!sendMQTT(mqtt_Color, buffer, true))
					writeLogFile(F("Publish Color: failed"), 1);
		}
		sendJSONheaderReply(1, "Updated");
	}
}

/* ======================================================================
Function: convertToSingleByte
Purpose : convert extended ASCII 2byte to SingleByte 
Input   : String input (letter)
Output  : String 
Comments: 
TODO    : */
String convertToSingleByte( String input )
{
   String output = "";
   for (int i = 0; i < input.length(); i++)
   {
      char c = input.charAt(i);
      if (c == 0xC4 || c == 0xC5)
      {
         char nextChar = input.charAt(i + 1);
         switch (nextChar)
         {
            case 0x8D: output += (char)0xE8; i++; break; // č (C4)
            case 0x8C: output += (char)0xC8; i++; break; // Č (C4)
            case 0x87: output += (char)0xE6; i++; break; // ć (C4)
            case 0x86: output += (char)0xC6; i++; break; // Ć (C4)
            case 0xBE: output += (char)0x9E; i++; break; // ž (C5)
            case 0xBD: output += (char)0x8E; i++; break; // Ž (C5)
            case 0xA1: output += (char)0x9A; i++; break; // š (C5)
            case 0xA0: output += (char)0x8A; i++; break; // Š (C5)
            case 0x91: output += (char)0xF0; i++; break; // đ (C5)
            case 0x90: output += (char)0xD0; i++; break; // Đ (C5)
            default: output += c; break;
         }
      }
      else
      {
         output += c;
      }
   }
   return output;
}

uint16_t XY(uint8_t x, uint8_t y)
{
	uint16_t i;
	if (kMatrixSerpentineLayout == false)
	{
		if (kMatrixVertical == false)
			i = (y * kMatrixWidth) + x;
		else
			i = kMatrixHeight * (kMatrixWidth - (x + 1)) + y;
	}

	if (kMatrixSerpentineLayout == true)
	{
		if (kMatrixVertical == false)
		{
			if (y & 0x01)
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
			if (x & 0x01)
				i = kMatrixHeight * (kMatrixWidth - (x + 1)) + y;
			else
				i = kMatrixHeight * (kMatrixWidth - x) - (y + 1);
		}
	}

	return i;
}

uint16_t XYsafe(uint8_t x, uint8_t y)
{
	if ( x >= kMatrixWidth ) return -1;
	if ( y >= kMatrixHeight ) return -1;
	if ( x < 0 ) return -1;
	if ( y < 0 ) return -1;
	return XY(x, y);
}

#endif