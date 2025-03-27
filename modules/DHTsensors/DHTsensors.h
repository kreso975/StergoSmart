#ifndef MODULE_DHT_H
#define MODULE_DHT_H

#include <Arduino.h> // Include the Arduino core header file
#include <DHT.h>

// 
#define DHTPIN 0            // what pin we're connected to
//#define DHTTYPE DHT11     // DHT 11
#define DHTTYPE DHT22       // DHT 22 (AM2302), AM2321
//#define DHTTYPE DHT21     // DHT 21 (AM2301)

extern bool writeLogFile(String message, int newLine, int output);
extern float Fahrenheit( float celsius );
extern float Kelvin( float celsius );

extern bool measureFirstRun;
extern bool detectModule;

extern byte t_measure;
extern float h;
extern float t;
extern float dp;

#define measureInterval 30e3                                // in miliseconds = 30 * 1000 (e3 = 3 zeros) = 15sec
extern unsigned long lastMeasureInterval;

#define intervalHist 1000 * 60 * 15                         // 4 measures / hours - orig 1000 * 60 * 15 - 15min
extern unsigned long previousMillis;

extern char mqtt_Humidity[120];
extern char mqtt_Temperature[120];

extern int mqtt_interval;
extern unsigned long mqtt_intervalHist;
extern unsigned long mqtt_previousMillis;

// Time Interval for sending Wb Location data / targeting scripts to send data via HTTP POST method
extern byte webLoc_start;
extern char webLoc_server[120];
extern int webLoc_interval;                                 // 1000 * 60 * 1 = 1min
extern unsigned long webLoc_intervalHist;
extern unsigned long webLoc_previousMillis;                 // time of last point added
//DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

extern bool setupDHT();
extern void getWeatherDHT();

#endif