#ifdef MODULE_BME280
#include "BME280.h"
#include <Wire.h>

//*************************************************************************************************************
/*
 * p-adjust : adjust pressuer to location altitude by Sealevel adjustement | 0 = No | 1 = yes
 * pa_unit  : use meter or feet as unit | 0 = Meter | 1 = Feet
 * pl_adj   : value of adjustment
 * t = Temperature, h = Humidity, p = Pressure, dp = dewPoint, P0 = Adjusted Pressure to Location Altitude
 * t_measure : 0 = Meter | 1 = Feet;  p_measure : 0 = hPa | 1 = inhg;
 *
 * struct Config {
 *	  byte t_measure						    =	0;
 *	  byte p_measure						    =	0;
 *	  byte p_adjust						        =	1;
 *	  byte pa_unit                              =	0;
 *	  int pl_adj							    =	122;
 * };
 *
*/

bool detectModule = false;
bool measureFirstRun = true;

byte pa_unit, t_measure, p_measure, p_adjust;
int pl_adj;
float h, t, p, dp, P0;
unsigned long lastMeasureInterval = 30e3; // in milliseconds = 30 * 1000 (e3 = 3 zeros) = 15sec
unsigned long previousMillis = 1000 * 60 * 15; // 4 measures / hours - orig 1000 * 60 * 15 - 15min
char mqtt_Humidity[120];
char mqtt_Temperature[120];
char mqtt_Pressure[120];

Adafruit_BME280 bme;

extern bool writeLogFile(String message, int newLine, int output);
extern float Fahrenheit( float celsius );
extern float Kelvin( float celsius );
extern float iNHg( float hpa );
extern float Meter( float feet );

/* ======================================================================
Function: setupBME280
Purpose : Initialize BME280
Input   : 
Output  : 
Comments: - */
bool setupBME280()
{
    Wire.begin(GPIO_SDA, GPIO_SCL); // 0, 2  
    Wire.setClock(100000);

    // Init BME280
    if (!bme.begin(BMEaddr))
    {
        writeLogFile(F("No BME280 sensor, check wiring!"), 1, 3);
        detectModule = false;
        return false;
    }
    writeLogFile(F("BME280 OK"), 1, 3);
    detectModule = true;
    return true;
}

/* ======================================================================
Function: getWeatherBME
Purpose : Read Sensor data
Input   : 
Output  : 
Comments: */
void getWeatherBME()
{
    float h_tmp, t_tmp;
    t_tmp = bme.readTemperature();
    h_tmp = round(bme.readHumidity() * 100) / 100.0F;

    // If we start getting totally wrong readings
    if ((t_tmp == 0 && h_tmp == 0) || isnan(h_tmp) || isnan(t_tmp) || t_tmp > 100 || t_tmp < -60)
    {
        writeLogFile(F("Fail read BME280!"), 1, 3);
        return;
    }
    else
    {
        t = t_tmp;
        h = h_tmp;
        dp = t - 0.36 * (100.0 - h);
        p = bme.readPressure();

        if (t_measure == 1)
            t = Fahrenheit(t);

        float R = 8.31432;
        float M = 0.0289644;
        float g = 9.80665;
        float T = Kelvin(t); // Kelvin

        int tmp_pl_adj;
        if (p_adjust == 1)
        {
            if (pa_unit == 1)
                tmp_pl_adj = Meter(pl_adj);
            else
                tmp_pl_adj = pl_adj;

            float Adjust = exp(-1 * g * M * tmp_pl_adj / (R * T));
            P0 = (p / Adjust) / 100.0F;
        }
        else
            P0 = p / 100.0F;

        if (p_measure == 1)
            P0 = iNHg(P0);
    }
}
#endif