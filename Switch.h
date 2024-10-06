/*
  ALL #DEFINE SETTING ARE SETUP IN Config.h NOT HERE
*/

/*
 * Different models used for Plug / Switch
 * 
 * Native Board relay = 1   // Relay Switch - RS
 * Sonoff S26         = 2   // Plug Switch  - PS
 * Sonoff T4EU1C      = 3   // Light Switch - LS
 */
#if ( STERGO_PLUG == 1 && STERGO_PROGRAM == 0 )     //===============================================
  #define MODEL_NAME "RS001"
#elif ( STERGO_PLUG == 2 && STERGO_PROGRAM == 0 )   //===============================================
  #define MODEL_NAME "PS002"
#elif ( STERGO_PLUG == 3 && STERGO_PROGRAM == 0 )   //===============================================
  #define MODEL_NAME "LS001"
#endif                                              //===============================================

// ESP8266 01S        = 1
// LOLIN d1 Mini      = 2
#if ( STERGO_PLUG_BOARD == 1 )                      //===============================================
  #define MODEL_NUMBER "v01"
#elif ( STERGO_PLUG_BOARD == 2 )                    //===============================================
  #define MODEL_NUMBER "v02"
#endif                                              //===============================================

#if ( STERGO_PLUG == 1 )                  //===============================================
  #define LED  1
  #define LED2 2
  #define RELAY 0                         // relay connected to  GPIO0
#elif ( STERGO_PLUG == 2 )                //===============================================
  #define LED  13                         // BLUE light
  #define RELAY 12                        // relay connected to  GPIO12 && RED light
  #define BUTTON01 0
#elif ( STERGO_PLUG == 3 )                //===============================================
  #define LED  13                         // BLUE LIGHT
  #define RELAY 12                        // relay connected to  GPIO12 && RED light
  #define BUTTON01 0
#endif                                    //===============================================


// Editable if needed
// --------------------------------------------------------------------------------

// Button press timings
#if ( STERGO_PLUG != 1 && ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 3 ) )
  unsigned long keyPrevMillis = 0;
  const unsigned long keySampleIntervalMs = 25;
  byte longKeyPressCountMax = 80;    // 80 * 25 = 2000 ms
  byte longKeyPressCount = 0;

  byte prevKeyState = HIGH;          // button is active low
  bool ledStatus = false;            // button is connected to pin 2 and GND
#endif

//MQTT Topics used from config.json
char mqtt_switch[120];              //"/home/ESP8266-2/switch";
char mqtt_switch2[120];             //"/home/ESP8266-2/switch2";

#define POWERON "{\"POWER\":\"ON\"}"
#define POWEROFF "{\"POWER\":\"OFF\"}"
    
long lastMsg = 0;
char msg[50];
int value = 0;

bool relay01State = false;
