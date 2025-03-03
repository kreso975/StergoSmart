#ifdef MODULE_DISPLAY
#include "../../settings.h"
#include "WS2812B_Matrix.h"

byte maxBrightness = 35;
// Display ON/OFF
byte displayON = 1;

// Params for width and height
const uint8_t kMatrixWidth = 32;
const uint8_t kMatrixHeight = 8;
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)

// Param for different pixel layouts
const bool kMatrixSerpentineLayout = true;
const bool kMatrixVertical = true;
byte kMatrixOrientation = 1; // O = Normal, 1 = Diagonal flip (0 and 256 are on opposite diagonal side), 2 = Vertical flip, 3 = Horizontal flip

CRGB leds_plus_safety_pixel[NUM_LEDS + 1];
CRGB *const leds(leds_plus_safety_pixel + 1);

CRGB displayColor = CRGB(255, 0, 0); // Red default

// TimeZone is adjustable in config.json
int timeZone = 1;
long timeZoneOffset; // Adjust for your timezone +1 - in Setup after config

unsigned long displayInterval = 1000; // Interval for display - default
unsigned long dispPrevMils = displayInterval;

unsigned long displayRotateInterval = 10000; // Interval for rotating display - default
unsigned long lastDisplayChange = 0;
byte displayMode = 0;

CRGB tempBufferText[NUM_LEDS];			// Buffer for text
CRGB tempBufferParticles[NUM_LEDS]; // Buffer for particles
CRGB tempBufferDate[NUM_LEDS];			// Buffer for date

CRGB *displayBuffer = nullptr; // Pointer to store the SCROLL MESSAGE buffer
int bufferSize = 0;
CRGB tempBufferMessage[NUM_LEDS]; // Buffer for message - not in use

// PARTICLES for FIREWORKS
// Global variables for initial velocity, gradual deceleration, and other parameters
const float INITIAL_VELOCITY = 1.5;
const float DECELERATION_FACTOR = 0.75;
const int PARTICLE_LIFE = 70;
const int PARTICLE_COUNT = 20;
const int EXPLOSION_FREQUENCY = 2; // 0 - 9 : 0 = no explosion, 9 = explosion every time
const int UPDATE_RATE = 50;

const char *messageDisplay = "";
static int scrollPosition = 0; // Make scrollPosition static
bool messageON = false;
bool messageWinON = false;			 // Use for Enter WIN and Fireworks
bool renderWIN = false;					 // Set renderWIN to false
const long intervalMessage = 30; // Update rate for displayMessage in milliseconds
unsigned long previousMillisMessage = 0;
unsigned long displayMessageLife = 10000; // Interval for Message Life
unsigned long prevMilMesLife = 0;

unsigned long previousMillisText = 0;
unsigned long previousMillisParticles = 0;
const long intervalText = 50;								// Update rate for drawText in milliseconds
const long intervalParticles = UPDATE_RATE; // Update rate for addParticles in milliseconds

// MQTT Topics used from config.json
char mqtt_Brightness[120];
char mqtt_Color[120];
char mqtt_displayON[120];

/* ======================================================================
Function: updateDisplay
Purpose : Main Display Constructor
Input   : None
Output  : None
Comments: Updates the display with time, date, temperature, humidity, or message based on the current state.
TODO    : */
void updateDisplay()
{
	if (displayON == 1) // if display is SET ON
	{
		unsigned long currentMillis = millis();
		if (messageON)
		{
			if (messageWinON)
				renderDisplayWin(currentMillis); // Display Win message - moved to separate function
			else
				renderDisplayMessage(currentMillis); // Display Message - moved to separate function
		}
		else
		{
			if (currentMillis - dispPrevMils >= displayInterval)
			{
				FastLED.clearData();
				adjustedTime = now() + timeZoneOffset; // Adjust time by Time Zone offset
				dispPrevMils = currentMillis;

				if (currentMillis - lastDisplayChange >= displayRotateInterval)
				{
					lastDisplayChange = currentMillis;
					displayMode = (displayMode + 1) % 4; // Rotate between 4 display modes
				}

				switch (displayMode)
				{
					case 0:
						displayRotateInterval = 4000; // 4 sec
						drawDate(tempBufferDate, 1, 0, displayColor);
						for (int i = 0; i < NUM_LEDS; i++)
						{
							leds[i] = tempBufferDate[i];
						}
						break;
					case 1:
						displayRotateInterval = 10000;						 // 10 sec
						drawTime(1, 0, displayColor, true, false); // Display time
						break;
					case 2:
						displayRotateInterval = 5000;					 // 5 sec
						drawTempHum(0, 0, displayColor, true); // Display Temperature true
						break;
					case 3:
						displayRotateInterval = 4000;						// 4 sec
						drawTempHum(0, 0, displayColor, false); // Display Humidity - false
						break;
				}
			}
		}
	}
	// Always show LEDs
	FastLED.show();
}

void drawLetter(int posx, int posy, char letter, CRGB color, int orientation, CRGB *buffer)
{
	if ((posx > -FontWidth) && (posx < kMatrixWidth))
	{
		for (int x = 0; x < FontWidth; x++)
		{
			for (int y = 0; y < FontHeight; y++)
			{
				bool bit = bitRead(pgm_read_byte(&(Font[letter][FontWidth - 1 - x])), y);
				int drawX = posx + x;
				int drawY = posy + y;

				// Handle different orientations
				switch (orientation)
				{
					case 0: // Normal orientation
						if (bit == 1)
							(buffer ? buffer[XYsafe(drawX, drawY)] : leds[XYsafe(drawX, drawY)]) = color;
						break;
					case 1: // Diagonal flip (0 and 256 are on opposite diagonal side)
						if (bit == 1)
							(buffer ? buffer[XYsafe(kMatrixWidth - 1 - drawX, kMatrixHeight - 1 - drawY)] : leds[XYsafe(kMatrixWidth - 1 - drawX, kMatrixHeight - 1 - drawY)]) = color;
						break;
					case 2: // Vertical flip
						if (bit == 1)
							(buffer ? buffer[XYsafe(drawX, kMatrixHeight - 1 - drawY)] : leds[XYsafe(drawX, kMatrixHeight - 1 - drawY)]) = color;
						break;
					case 3: // Horizontal flip
						if (bit == 1)
							(buffer ? buffer[XYsafe(kMatrixWidth - 1 - drawX, drawY)] : leds[XYsafe(kMatrixWidth - 1 - drawX, drawY)]) = color;
						break;
				}
			}
		}
	}
}

void drawTempHum(int x, int y, CRGB colorText, bool isTemperature)
{
	char tmpStr[10];
	// colorText.nscale8(maxBrightness);

	x = 4;
	if (isTemperature)
	{
		dtostrf(t, 3, 0, tmpStr);
		drawLetter(x, 0, 'C', colorText, kMatrixOrientation);

		x += FontWidth - 3;
		drawLetter(x, -5, '.', colorText, kMatrixOrientation); // Drawing '.' but move it up -5

		x += 6;
	}
	else
	{
		dtostrf(h, 3, 0, tmpStr);
		drawLetter(x, 1, '%', colorText, kMatrixOrientation);
		x += FontWidth + 1;
	}

	int length = strlen(tmpStr); // Get the length of the character array
	for (int i = 0; i < length; i++)
	{
		char letter = tmpStr[length - 1 - i]; // Reverse order
		drawLetter(x, y, letter, colorText, kMatrixOrientation);
		x += FontWidth + 1; // Move to the next position
	}
}

void drawTime(int x, int y, CRGB colorTime, bool colon, bool seconds)
{
	int hours = hour(adjustedTime);
	int minutes = minute(adjustedTime);
	int secs = second(adjustedTime);

	// colorTime.nscale8(maxBrightness);

	// Display time on LED matrix with swapped positions
	x -= 0;
	drawLetter(x, y, minutes % 10 + '0', colorTime, kMatrixOrientation);
	x += FontWidth + 1;
	drawLetter(x, y, minutes / 10 + '0', colorTime, kMatrixOrientation);
	x += FontWidth;
	if (colon)
	{
		if (secs % 2 == 0)
			drawLetter(x - 1, y, ':', colorTime, kMatrixOrientation);
		x += 4;
	}
	drawLetter(x, y, hours % 10 + '0', colorTime, kMatrixOrientation);
	x += FontWidth + 1;
	drawLetter(x, y, hours / 10 + '0', colorTime, kMatrixOrientation);
}

void drawDate(CRGB *buffer, int x, int y, CRGB colorDate)
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
		// colorDate.nscale8(maxBrightness);

		// Display date on LED matrix
		int posX = x;
		drawLetter(posX, y, months % 10 + '0', colorDate, kMatrixOrientation, buffer);
		posX += FontWidth + 1;
		drawLetter(posX, y, months / 10 + '0', colorDate, kMatrixOrientation, buffer);
		posX += FontWidth + 1;
		drawLetter(posX - 2, y + 1, '.', colorDate, kMatrixOrientation, buffer);
		posX += 3;
		drawLetter(posX, y, days % 10 + '0', colorDate, kMatrixOrientation, buffer);
		posX += FontWidth + 1;
		drawLetter(posX, y, days / 10 + '0', colorDate, kMatrixOrientation, buffer);

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
void displayMessage(CRGB *buffer, CRGB colorScroll, const char *message, int numSpaces, int orientation)
{
	static bool bufferInitialized = false;
	static CRGB *displayBuffer = nullptr; // Pointer to store the buffer
	static int bufferSize = 0;
	static char previousMessage[256] = "";
	static int previousNumSpaces = 0;

	const char *convertedMessage = convertToSingleByte(message);

	// Check if the message or number of spaces has changed
	if (strcmp(previousMessage, convertedMessage) != 0 || previousNumSpaces != numSpaces)
	{
		if (displayBuffer != nullptr) // Free the old buffer if it exists
			delete[] displayBuffer;

		// Calculate the new buffer size
		bufferSize = (strlen(convertedMessage) + numSpaces) * 6 * kMatrixHeight;
		displayBuffer = new CRGB[bufferSize]; // Allocate memory for the new buffer

		for (int i = 0; i < bufferSize; i++) // Initialize the buffer with the new message and spaces
			displayBuffer[i] = CRGB::Black;

		// Add spaces in front of the message
		for (int i = 0; i < numSpaces; i++)
		{
			int charPosition = i * 6;
			for (int x = 0; x < 6; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					int bufferIndex = (charPosition + x) + (y * (strlen(convertedMessage) + numSpaces) * 6);
					if (bufferIndex < bufferSize)
						displayBuffer[bufferIndex] = CRGB::Black;
				}
			}
		}

		// Render message in the appropriate order
		if (orientation == 1)
		{
			for (int i = strlen(convertedMessage) - 1; i >= 0; i--)
			{
				char charToDisplay = convertedMessage[i];
				int charPosition = (strlen(convertedMessage) - 1 - i + numSpaces) * 6;
				for (int x = 0; x < 6; x++)
				{
					for (int y = 0; y < 8; y++)
					{
						if (bitRead(pgm_read_byte(&(Font[charToDisplay][5 - x])), 7 - y) == 1)
						{ // Apply vertical and horizontal flip
							int bufferIndex = (charPosition + x) + (y * (strlen(convertedMessage) + numSpaces) * 6);
							if (bufferIndex < bufferSize)
								displayBuffer[bufferIndex] = colorScroll;
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < strlen(convertedMessage); i++)
			{
				char charToDisplay = convertedMessage[i];
				int charPosition = (i + numSpaces) * 6;
				for (int x = 0; x < 6; x++)
				{
					for (int y = 0; y < 8; y++)
					{
						if (bitRead(pgm_read_byte(&(Font[charToDisplay][x])), y) == 1)
						{
							int bufferIndex = (charPosition + x) + (y * (strlen(convertedMessage) + numSpaces) * 6);
							if (bufferIndex < bufferSize)
								displayBuffer[bufferIndex] = colorScroll;
						}
					}
				}
			}
		}

		// Copy convertedMessage to previousMessage
		strncpy(previousMessage, convertedMessage, sizeof(previousMessage) - 1);
		// Ensure null termination
		previousMessage[sizeof(previousMessage) - 1] = '\0';

		// Update the previous message
		previousNumSpaces = numSpaces;																				// Update the previous number of spaces
		scrollPosition = (orientation == 1) ? (strlen(convertedMessage) + numSpaces) * 6 - 1 : 0; // Set the scroll position based on orientation
	}

	// Copy the relevant part of the buffer to the LED matrix
	for (int x = 0; x < kMatrixWidth; x++)
	{
		for (int y = 0; y < kMatrixHeight; y++)
		{
			int bufferIndex = (scrollPosition + x) % ((strlen(convertedMessage) + numSpaces) * 6) + (y * (strlen(convertedMessage) + numSpaces) * 6);
			if (bufferIndex < bufferSize)
				buffer[XYsafe(kMatrixWidth - 1 - x, y)] += displayBuffer[bufferIndex]; // Use buffer instead of leds
		}
	}

	if (orientation == 1)
	{
		scrollPosition--;
		if (scrollPosition < 0)
			scrollPosition = (strlen(convertedMessage) + numSpaces) * 6 - 1;
	}
	else
	{
		scrollPosition++;
		if (scrollPosition >= (strlen(convertedMessage) + numSpaces) * 6)
			scrollPosition = 0;
	}
}

void freeDisplayMessageBuffer()
{
	if (displayBuffer != nullptr)
	{
		delete[] displayBuffer;
		displayBuffer = nullptr;
	}
}

void renderDisplayMessage(unsigned long currentMillis)
{
	//
	// DRAW TEXT SCROLL
	if (currentMillis - previousMillisMessage >= intervalMessage)
	{
		FastLED.clearData();
		previousMillisMessage = currentMillis;
		memset(tempBufferMessage, 0, sizeof(tempBufferMessage));												// Clear temp buffer
		displayMessage(tempBufferMessage, displayColor, messageDisplay, 6, kMatrixOrientation);	// Update temp buffer | using buffer if we want add something over / effect

		for (int i = 0; i < NUM_LEDS; i++) // Fill LEDS from buffer
			leds[i] = tempBufferMessage[i];
	}

	if (currentMillis - prevMilMesLife >= displayMessageLife)
	{
		server.begin();																					 // We have stop it when set messageON = true in displayState()
		memset(tempBufferMessage, 0, sizeof(tempBufferMessage)); // Clear temp buffer
		freeDisplayMessageBuffer();
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

Particle::Particle(float x, float y, CRGB color)
{
	this->x = x;
	this->y = y;
	this->color = color;
	float velocityFactor = 100.0 / INITIAL_VELOCITY;
	this->xvel = random(-100, 100) / velocityFactor; // Use global initial velocity
	this->yvel = random(-100, 100) / velocityFactor; // Use global initial velocity
	this->life = PARTICLE_LIFE;											 // Use global particle life
}

void Particle::update(CRGB *buffer)
{
	draw(buffer);
	slowDown();
	life -= 5;
}

void Particle::draw(CRGB *buffer)
{
	// Adjust Particle brightness - 0 to 255
	color.nscale8(min(maxBrightness * 4, 255));

	// Ensure particles stay within bounds
	if (x >= 0 && x < kMatrixWidth && y >= 0 && y < kMatrixHeight)
	{
		int index = XYsafe(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
		if (index != -1)
			buffer[index] += color; // Use additive blending
	}
	x += xvel;
	y += yvel;

	// Mark particle as dead if out of bounds
	if (x < 0 || x >= kMatrixWidth || y < 0 || y >= kMatrixHeight)
		life = -1;
}

bool Particle::isDead()
{
	return life < 0;
}

void Particle::slowDown()
{
	xvel *= DECELERATION_FACTOR; // Use global deceleration factor
	yvel *= DECELERATION_FACTOR; // Use global deceleration factor
}

std::vector<Particle> particles;

void resetParticles()
{
	particles.clear(); // Clear the particles vector
	// Reinitialize any other necessary variables here
}

void addParticles(CRGB *buffer)
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

void drawWIN(const byte coords[][2], int size, CRGB *buffer, CRGB color, int orientation)
{
	for (int i = 0; i < size; i++)
	{
		int x = coords[i][0];
		int y = coords[i][1];

		// Handle different orientations
		switch (orientation)
		{
			case 0: // Normal orientation
				buffer[XYsafe(x, y)] = color;
				break;
			case 1: // Diagonal flip
				buffer[XYsafe(kMatrixWidth - 1 - x, kMatrixHeight - 1 - y)] = color;
				break;
			case 2: // Vertical flip
				buffer[XYsafe(x, kMatrixHeight - 1 - y)] = color;
				break;
			case 3: // Horizontal flip
				buffer[XYsafe(kMatrixWidth - 1 - x, y)] = color;
				break;
		}
	}
}

void drawText(CRGB *buffer)
{
	static uint8_t brightness = 30;
	static bool increasing = true;

	// Draw flipped "WIN" in the center with varying brightness
	static CRGB color = CRGB(255, 0, 0);
	static uint8_t lastBrightness = 30;
	if (brightness != lastBrightness)
	{
		color = CRGB(255, 0, 0);
		color.nscale8(brightness);
		lastBrightness = brightness;
	}

	// Draw letters using the optimized function
	drawWIN(W_coords, sizeof(W_coords) / sizeof(W_coords[0]), buffer, color, kMatrixOrientation);
	drawWIN(I_coords, sizeof(I_coords) / sizeof(I_coords[0]), buffer, color, kMatrixOrientation);
	drawWIN(N_coords, sizeof(N_coords) / sizeof(N_coords[0]), buffer, color, kMatrixOrientation);

	// Adjust brightness
	if (increasing)
	{
		brightness += 30;
		if (brightness >= min(maxBrightness * 4, 255))
			increasing = false;
	}
	else
	{
		brightness -= 30;
		if (brightness <= 30)
			increasing = true;
	}
}

void renderDisplayWin(unsigned long currentMillis)
{
	FastLED.clearData();
	// DRAW WIN WITH FIREWORKS
	if (renderWIN)
	{
		if (currentMillis - previousMillisText >= intervalText)
		{
			writeLogFile("renderWIN", 1, 1);
			previousMillisText = currentMillis;
			memset(tempBufferText, 0, sizeof(tempBufferText)); // Clear temp buffer
			drawText(tempBufferText);													 // Update temp buffer
		}
	}

	// Update addParticles at its own interval
	if (currentMillis - previousMillisParticles >= intervalParticles)
	{
		writeLogFile("render Particle", 1, 1);
		previousMillisParticles = currentMillis;
		memset(tempBufferParticles, 0, sizeof(tempBufferParticles)); // Clear temp buffer
		addParticles(tempBufferParticles);													 // Update temp buffer
	}

	// Combine buffers
	for (int i = 0; i < NUM_LEDS; i++)
		leds[i] = tempBufferText[i] + tempBufferParticles[i]; // Combine buffers tempBufferText[i] +

	if (currentMillis - prevMilMesLife >= displayMessageLife)
	{
		FastLED.clearData();
		memset(tempBufferText, 0, sizeof(tempBufferText));					 // Clear temp buffer
		memset(tempBufferParticles, 0, sizeof(tempBufferParticles)); // Clear temp buffer
		messageON = false;																					 // Set messageON to false after 10 seconds
		messageWinON = false;
		renderWIN = true;
		resetParticles(); // Clear the particles vector
		server.begin();		// We have stop it when set messageON = true in displayState()
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
		sprintf(data, "{\"displayON\":%d,\"Brightness\":%d,\"RGB\":\"%s\",\"timeZone\":%d}", displayON, maxBrightness, buffer, 1);

		// 3 is indicator of JSON already formated reply
		sendJSONheaderReply(3, data);
	}
	else
	{
		// Get arg/params posted and change settings
		// timeZoneOffset, brightness, messageDisplay, displayMode
		// WE WILL USE THIS FOR MAX BRIGHTNESS - For now not at all
		/*
		if (server.hasArg("Brightness"))
		{
			maxBrightness = server.arg("Brightness").toInt();
			itoa(maxBrightness, buffer, 10); // Convert
			writeLogFile(F("Updated Brightness to ") + String(maxBrightness), 1, 1);
			if (mqtt_start == 1)
				if (!mqttManager.sendMQTT(mqtt_Brightness, buffer, true))
					writeLogFile(F("Publish Brightness: failed"), 1);


		}
		*/
		if (server.hasArg("messageDisplay"))
		{
			messageDisplay = server.arg("messageDisplay").c_str();
			// writeLogFile( F("Updated messageDisplay to ") + String(messageDisplay), 1, 1 );
			// renderWIN = true;
			// messageWinON = true;
			messageON = true;
			server.stop(); // Stopping webServer because it scrambles scroll buffer if accessed during scroll
			prevMilMesLife = millis();
		}

		if (server.hasArg("RGB"))
		{
			const char *hexColor = server.arg("RGB").c_str();
			unsigned long tempDisplayColor; // Temporary variable for calculations

			// Check if hexColor has a '#' in front and convert to long integer
			if (hexColor[0] == '#')
				tempDisplayColor = strtol(&hexColor[1], NULL, 16);
			else
				tempDisplayColor = strtol(hexColor, NULL, 16);

			// Set the global displayColor variable
			displayColor = tempDisplayColor;

			// Extract individual red, green, and blue components using bitwise operations
			int red = (tempDisplayColor >> 16) & 0xFF;
			int green = (tempDisplayColor >> 8) & 0xFF;
			int blue = tempDisplayColor & 0xFF;

			// Calculate the brightness as the maximum of the red, green, and blue components
			maxBrightness = max(max(red, green), blue);

			// Convert the brightness to a string for MQTT payload
			char brightnessBuffer[10];
			itoa(maxBrightness, brightnessBuffer, 10);

			// Send the color and brightness values via MQTT
			if (mqtt_start == 1)
			{
				if (!mqttManager.sendMQTT(mqtt_Color, (char *)hexColor, true)) // Cast to char*
					writeLogFile(F("Publish Color: failed"), 1);
				if (!mqttManager.sendMQTT(mqtt_Brightness, brightnessBuffer, true))
					writeLogFile(F("Publish Brightness: failed"), 1);
			}

			sendJSONheaderReply(1, "Updated");
		}
	}
}

/* ======================================================================
Function: convertToSingleByte
Purpose : convert extended ASCII 2byte to SingleByte
Input   : String input (letter)
Output  : String
Comments:
TODO    : */
const char *convertToSingleByte(const char *input)
{
	static char output[256] = "";
	int len = strlen(input);
	int j = 0;
	for (int i = 0; i < len; i++)
	{
		char c = input[i];
		if (c == 0xC4 || c == 0xC5)
		{
			char nextChar = input[i + 1];
			switch (nextChar)
			{
			case 0x8D:
				output[j++] = (char)0xE8;
				i++;
				break; // č (C4)
			case 0x8C:
				output[j++] = (char)0xC8;
				i++;
				break; // Č (C4)
			case 0x87:
				output[j++] = (char)0xE6;
				i++;
				break; // ć (C4)
			case 0x86:
				output[j++] = (char)0xC6;
				i++;
				break; // Ć (C4)
			case 0xBE:
				output[j++] = (char)0x9E;
				i++;
				break; // ž (C5)
			case 0xBD:
				output[j++] = (char)0x8E;
				i++;
				break; // Ž (C5)
			case 0xA1:
				output[j++] = (char)0x9A;
				i++;
				break; // š (C5)
			case 0xA0:
				output[j++] = (char)0x8A;
				i++;
				break; // Š (C5)
			case 0x91:
				output[j++] = (char)0xF0;
				i++;
				break; // đ (C5)
			case 0x90:
				output[j++] = (char)0xD0;
				i++;
				break; // Đ (C5)
			default:
				output[j++] = c;
				break;
			}
		}
		else
		{
			output[j++] = c;
		}
	}
	output[j] = '\0'; // Null-terminate the output string
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
	if (x >= kMatrixWidth)
		return -1;
	if (y >= kMatrixHeight)
		return -1;
	if (x < 0)
		return -1;
	if (y < 0)
		return -1;
	return XY(x, y);
}

void setupDisplay()
{
	// MQTT subscribe to:
	subscriptionList.push_back(mqtt_displayON);	// mqtt_displayON == ON | OFF
	subscriptionList.push_back(mqtt_Brightness);	// mqtt_Brightness == maxBrightness
	subscriptionList.push_back(mqtt_Color);		// mqtt_Color == RGB() or HSV()

	mqttManager.registerCallback(callbackDisplayMQTT, 2); // register Display callback for MQTT
}

void callbackDisplayMQTT(char *topic, byte *payload, unsigned int length)
{
	if (strcmp(topic, mqtt_displayON) == 0)
	{
		// displayON = (byte)payload[0];
		if ( payload[0] == '1' )
		{
			displayON = 1;
		}
		else if ( payload[0] == '0' )
		{
			FastLED.clearData();
			displayON = 0;
		}
	}

	/*
	if (strcmp(topic, mqtt_Brightness) == 0)
	{
		payload[length] = '\0'; // Null-terminate the payload
		maxBrightness = atoi((char *)payload);
	}
	*/

	if (strcmp(topic, mqtt_Color) == 0)
    {
        // Null-terminate the payload
        payload[length] = '\0';

        unsigned long tempDisplayColor; // Temporary variable for calculations

        // Convert payload to long integer, handling both '#' and non-'#' formats
        if (payload[0] == '#')
            tempDisplayColor = strtol((char*)&payload[1], NULL, 16);
        else
            tempDisplayColor = strtol((char*)payload, NULL, 16);

        // Set the global displayColor variable
        displayColor = tempDisplayColor;

        // Extract individual red, green, and blue components using bitwise operations
        int red = (tempDisplayColor >> 16) & 0xFF;
        int green = (tempDisplayColor >> 8) & 0xFF;
        int blue = tempDisplayColor & 0xFF;

        // Create a CRGB object with the extracted color value
        CRGB rgbColor = CRGB(red, green, blue);

        // Calculate the brightness as the maximum of the red, green, and blue components
        maxBrightness = max(max(red, green), blue);

        // Convert the brightness to a string for MQTT payload
        char brightnessBuffer[10];
        itoa(maxBrightness, brightnessBuffer, 10);

        // Send the brightness value via MQTT
        if (mqtt_start == 1)
        {
            if (!mqttManager.sendMQTT(mqtt_Brightness, brightnessBuffer, true))
                writeLogFile(F("Publish Brightness: failed"), 1);
        }
    }
}

#endif