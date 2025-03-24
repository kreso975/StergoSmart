#ifndef MODULE_BME280_H
#define MODULE_BME280_H

#include <Arduino.h> // Include the Arduino core header file
#include <Wire.h>
#include <Bosch_BME280_Arduino.h>


extern bool writeLogFile(String message, int newLine, int output);
extern float Fahrenheit( float celsius );
extern float Kelvin( float celsius );
extern float iNHg( float hpa );
extern float Meter( float feet );


// BME280 GPIOs 2 (SDA),0 (SCL) are used for BME280
// 72 (2,0) 74 (4,5)
#define GPIO_SDA 4
#define GPIO_SCL 5
#define BMEaddr 0x76 //BME280 address not all running on same address 0x76 || 0x77

extern bool measureFirstRun;
extern bool detectModule;

extern byte pa_unit, t_measure, p_measure, p_adjust;
extern int pl_adj;
extern float h, t, p, dp, P0;

#define measureInterval 30e3                                // in miliseconds = 30 * 1000 (e3 = 3 zeros) = 15sec
extern unsigned long lastMeasureInterval;

#define intervalHist 1000 * 60 * 15                         // 4 measures / hours - orig 1000 * 60 * 15 - 15min
extern unsigned long previousMillis;

extern char mqtt_Humidity[120];
extern char mqtt_Temperature[120];
extern char mqtt_Pressure[120];

extern int mqtt_interval;
extern unsigned long mqtt_intervalHist;
extern unsigned long mqtt_previousMillis;

extern bool setupBME280();
extern void getWeatherBME();

#endif