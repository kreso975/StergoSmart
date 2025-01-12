#ifndef MODULE_DISPLAY_WS2812B
#define MODULE_DISPLAY_WS2812B

#include <Arduino.h>
#include <FastLED.h> // Include FastLED library
#include <vector>

void updateDisplay();
void drawLetter( int posx, int posy, char letter, CRGB color );
void drawLetterBuf( CRGB *buffer, int posx, int posy, char letter, CRGB color );
void drawTempHum( int x, int y, CRGB colorText, bool isTemperature );
void drawTime( int x, int y, CRGB colorTime, bool colon, bool seconds );
void drawDate( CRGB *buffer, int x, int y, CRGB colorDate );
void displayMessage( CRGB colorScroll, const char *message, int numSpaces );
void renderDisplayMessage( unsigned long currentMillis );

void drawWIN( const byte coords[][2], int size, CRGB *buffer, CRGB color );
void drawText( CRGB *buffer );
void renderDisplayWin( unsigned long currentMillis );

void resetParticles();
void addParticles( CRGB* buffer );

String convertToSingleByte( String input );
void displayState();

uint16_t XY(uint8_t x, uint8_t y);
uint16_t XYsafe(uint8_t x, uint8_t y);

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