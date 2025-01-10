#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
#define ONE_WIRE_BUS 2     

bool detectModule = false;                                  // Was detectDS18B20 use True if moduledetection not needed

byte t_measure;
float t;
  
// Time Interval for reading data from Sensor
bool measureFirstRun = true;
#define measureInterval 30e3                                // in miliseconds = 30 * 1000 (e3 = 3 zeros) = 15sec
unsigned long lastMeasureInterval = measureInterval;        // time of last point added

// Time interval for Logging data into history.json
#define intervalHist 1000 * 60 * 15                         // 4 measures / hours - orig 1000 * 60 * 15 - 15min
unsigned long previousMillis = intervalHist;                // time of last point added

//MQTT Topics used from config.json
char mqtt_Temperature[120];


// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);