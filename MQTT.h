/*********************************************************************************************************/
/* MQTT Server address
 * TODO : WebClient - In devices - Run Services (on | off )
 *                    Check if MQTT Broker is available, Check if MQTT is set - give alerts (graphic)
 *********************************************************************************************************/ 
int mqtt_connectTry = 3;	// How many time we will try to connect to MQTT before we will put it at sleep = 3

char mqtt_server[20];
int mqtt_port;
char mqtt_clientName[23];
char mqtt_clientUsername[50];
char mqtt_clientPassword[50];
char mqtt_myTopic[120];


#if ( STERGO_PROGRAM == 0 )           // Power Plug | Switch
  char mqtt_switch[120];              //"/home/ESP8266-2/switch";
  char mqtt_switch2[120];             //"/home/ESP8266-2/switch2";
#elif ( STERGO_PROGRAM == 1 )         // Weather Station
  char mqtt_bme280Humidity[120];
  char mqtt_bme280Temperature[120];
  char mqtt_bme280Pressure[120];
#elif ( STERGO_PROGRAM == 3 )
  char mqtt_switch[120];              //"/home/ESP8266-2/switch";
  char mqtt_switch2[120];             //"/home/ESP8266-2/switch2";
  char mqtt_bme280Humidity[120];
  char mqtt_bme280Temperature[120];
  char mqtt_bme280Pressure[120];
#endif
