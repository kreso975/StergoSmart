#ifndef MODULE_DS18B20_H
#define MODULE_DS18B20_H

#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
#define ONE_WIRE_BUS 2   

extern bool measureFirstRun;
extern bool detectModule;

extern byte t_measure;
extern float t;

#define measureInterval 30e3                                // in miliseconds = 30 * 1000 (e3 = 3 zeros) = 15sec
extern unsigned long lastMeasureInterval;

#define intervalHist 1000 * 60 * 15                         // 4 measures / hours - orig 1000 * 60 * 15 - 15min
extern unsigned long previousMillis;

extern char mqtt_Humidity[120];

//DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

extern bool setupDS18B20();
extern void getWeatherDS18B20();

#endif