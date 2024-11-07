//Constants
#define DHTPIN 2            // what pin we're connected to
//#define DHTTYPE DHT11     // DHT 11
#define DHTTYPE DHT22       // DHT 22 (AM2302), AM2321
//#define DHTTYPE DHT21     // DHT 21 (AM2301)

bool detectModule = false;                                  // Was detect DHT use True if moduledetection not needed

byte t_measure;
float h, t, dp;
  
// Time Interval for reading data from Sensor
bool measureFirstRun = true;
#define measureInterval 30e3                                // in miliseconds = 30 * 1000 (e3 = 3 zeros) = 15sec
unsigned long lastMeasureInterval = measureInterval;        // time of last point added

// Time interval for Logging data into history.json
#define intervalHist 1000 * 60 * 15                         // 4 measures / hours - orig 1000 * 60 * 15 - 15min
unsigned long previousMillis = intervalHist;                // time of last point added

//MQTT Topics used from config.json
char mqtt_Temperature[120];
char mqtt_Humidity[120];

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor