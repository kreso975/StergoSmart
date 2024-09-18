/*
 * Different models used for Plug / Switch
 * 
 * Native Board relay = 1   // Relay Switch - RS
 * Sonoff S26         = 2   // Plug Switch  - PS
 * Sonoff T4EU1C      = 3   // Light Switch - LS
 */
#if ( STERGO_PLUG == 1 )                  //===============================================
  #define LED  1
  #define LED2 2
  #define RELAY 0                         // relay connected to  GPIO0
#elif ( STERGO_PLUG == 2 )                //===============================================
  #define LED  13
  #define LED2 2                          // only because of legacy - FIX MQTT first then this can be removed
  #define RELAY 12                        // relay connected to  GPIO12
  #define BUTTON01 0
#elif ( STERGO_PLUG == 3 )                //===============================================
  #define LED  13
  #define LED2 2                          // only because of legacy - FIX MQTT first then this can be removed
  #define RELAY 12                        // relay connected to  GPIO12
  #define BUTTON01 0
#endif                                    //===============================================

#if ( STERGO_PLUG == 1 && STERGO_PROGRAM == 0 )     //===============================================
  #define MODEL_NAME "RS001"
  #define MODEL_NUMBER "v01"
#elif ( STERGO_PLUG == 2 && STERGO_PROGRAM == 0 )   //===============================================
  #define MODEL_NAME "PS002"
  #define MODEL_NUMBER "v01"
#elif ( STERGO_PLUG == 3 && STERGO_PROGRAM == 0 )   //===============================================
  #define MODEL_NAME "LS001"
  #define MODEL_NUMBER "v01"
#endif                                              //===============================================


// Button press timings
#if ( STERGO_PLUG != 1 && STERGO_PROGRAM == 0 )
  unsigned long keyPrevMillis = 0;
  const unsigned long keySampleIntervalMs = 25;
  byte longKeyPressCountMax = 80;    // 80 * 25 = 2000 ms
  byte longKeyPressCount = 0;

  byte prevKeyState = HIGH;          // button is active low
  bool ledStatus = false;            // button is connected to pin 2 and GND
#endif

#define POWERON "{\"POWER\":\"ON\"}"
#define POWEROFF "{\"POWER\":\"OFF\"}"
    
long lastMsg = 0;
char msg[50];
int value = 0;

bool relay01State = false;
