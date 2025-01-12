#ifndef MODULE_BME280
#define MODULE_BME280

#include <Arduino.h> // Include the Arduino core header file
#include <Adafruit_BME280.h>

// BME280 GPIOs 2 (SDA),0 (SCL) are used for BME280
#define GPIO_SDA 2
#define GPIO_SCL 0
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
extern Adafruit_BME280 bme;

bool setupBME280();
void getWeatherBME();

#endif