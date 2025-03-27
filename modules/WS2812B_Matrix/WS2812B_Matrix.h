#ifndef MODULE_DISPLAY_WS2812B_H
#define MODULE_DISPLAY_WS2812B_H

#include <Arduino.h>
#include <FastLED.h> // Include FastLED library
#include <vector>

// external functions
extern void sendJSONheaderReply(byte type, String message);
extern bool writeLogFile( String message, int newLine, int output);

// external variables
extern MQTTManager mqttManager;
extern time_t adjustedTime;

#define LED_PIN 0           // 74 = 0, 119 = 2
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define POWER_VOLTAGE 5             // set Voltage
#define MAX_POWER_MILLIAMPS 700     // set Milliamps

extern byte maxBrightness;
// Display ON/OFF
extern byte displayON;

// Params for width and height
extern const uint8_t kMatrixWidth;
extern const uint8_t kMatrixHeight;
extern byte kMatrixOrientation;

// Param for different pixel layouts
extern const bool kMatrixSerpentineLayout;
extern const bool kMatrixVertical;

extern CRGB leds_plus_safety_pixel[];

extern CRGB displayColor;         // Red default

// TimeZone is adjustable in config.json
extern int timeZone; 
extern long timeZoneOffset;                          // Adjust for your timezone +1 - in Setup after config

extern unsigned long displayInterval;         // Interval for display - default
extern unsigned long dispPrevMils;

extern unsigned long displayRotateInterval;  // Interval for rotating display - default
extern unsigned long lastDisplayChange;
extern byte displayMode;

extern CRGB tempBufferText[];            // Buffer for text
extern CRGB tempBufferParticles[];       // Buffer for particles
extern CRGB tempBufferDate[];            // Buffer for date

extern CRGB *displayBuffer;            // Pointer to store the SCROLL MESSAGE buffer
extern int bufferSize;
//CRGB tempBufferMessage[NUM_LEDS];       // Buffer for message - not in use


// PARTICLES for FIREWORKS
// Global variables for initial velocity, gradual deceleration, and other parameters
extern const float INITIAL_VELOCITY;
extern const float DECELERATION_FACTOR;
extern const int PARTICLE_LIFE;
extern const int PARTICLE_COUNT;
extern const int EXPLOSION_FREQUENCY; // 0 - 9 : 0 = no explosion, 9 = explosion every time
extern const int UPDATE_RATE;


extern const char* messageDisplay;
//static int scrollPosition;              // Make scrollPosition static
extern bool messageON;
extern bool messageWinON;                  // Use for Enter WIN and Fireworks
extern bool renderWIN;                     // Set renderWIN to false
extern const long intervalMessage;            // Update rate for displayMessage in milliseconds
extern unsigned long previousMillisMessage;
extern unsigned long displayMessageLife;   // Interval for Message Life
extern unsigned long prevMilMesLife;

extern unsigned long previousMillisText;
extern unsigned long previousMillisParticles;
extern const long intervalText;            // Update rate for drawText in milliseconds
extern const long intervalParticles; // Update rate for addParticles in milliseconds

//MQTT Topics used from config.json
extern char mqtt_Brightness[120];
extern char mqtt_Color[120];
extern char mqtt_displayON[120];

extern const byte W_coords[][2];
extern const byte I_coords[][2];
extern const byte N_coords[][2];
extern const uint8_t FontHeight;
extern const uint8_t FontWidth;
//const static uint8_t PROGMEM Font[][];



extern void updateDisplay();
extern void drawLetter(int posx, int posy, char letter, CRGB color, int orientation, CRGB *buffer = nullptr);
#ifdef MODULE_WEATHER										//=============== Weather Station  ==============
extern void drawTempHum( int x, int y, CRGB colorText, bool isTemperature );
#endif
extern void drawTime( int x, int y, CRGB colorTime, bool colon, bool seconds );
extern void drawDate( CRGB *buffer, int x, int y, CRGB colorDate );
extern void displayMessage(CRGB *buffer, CRGB colorScroll, const char *message, int numSpaces, int orientation);
extern void renderDisplayMessage( unsigned long currentMillis );
extern void drawWIN( const byte coords[][2], int size, CRGB *buffer, CRGB color );
extern void drawText( CRGB *buffer );
extern void renderDisplayWin( unsigned long currentMillis );

extern void resetParticles();
extern void addParticles( CRGB* buffer );

extern const char *convertToSingleByte(const char *input);
extern void displayState();

extern uint16_t XY(uint8_t x, uint8_t y);
extern uint16_t XYsafe(uint8_t x, uint8_t y);

extern void setupDisplay();

extern void callbackDisplayMQTT(char *topic, byte *payload, unsigned int length);

class Particle {
public:
    float x, y;
    CRGB color;
    float xvel, yvel;
    int life;

    Particle(float x, float y, CRGB color);

    void update(CRGB* buffer);
    void draw(CRGB* buffer);
    bool isDead();
    void slowDown();
};

#endif