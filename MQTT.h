/*********************************************************************************************************/
/* MQTT Server address
 * TODO : WebClient - In devices - Run Services (on | off )
 *                    Check if MQTT Broker is available, Check if MQTT is set - give alerts (graphic)
 *********************************************************************************************************/ 
byte mqtt_connectTry = 3;	// How many time we will try to connect to MQTT before we will put it at sleep = 3
byte mqttTempDown = 0; // If we coudn't reconnect, mqtt_start is set to 0, and mqttTempDown is set to 1. 
// Time interval for try to start MQTT again
#define mqttTempDownInt 1000 * 60 * 15                                 // 4 measures / hours - orig 1000 * 60 * 15 - 15 min
unsigned long lastmqttTempDownMillis = mqttTempDownInt;                // time of last point added

byte mqtt_start = 0;
char mqtt_server[20];
int mqtt_port;
char mqtt_clientName[23];
char mqtt_clientUsername[50];
char mqtt_clientPassword[50];
char mqtt_myTopic[120];

// Time Interval for sending MQTT data
int mqtt_interval = 120;
unsigned long mqtt_intervalHist;
unsigned long mqtt_previousMillis;     // time of last point added