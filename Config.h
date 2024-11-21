/*
 * For What Device We need to Compile
 * 
 * STERGO_PROGRAM :
 *
 * PowerSwitch                  = 0
 * StergoWeather BME280         = 1
 * ticTacToe                    = 2
 * StergoWeather+PowerSwitch    = 3
 * StergoWeather DHT22          = 4
 * StergoWeather DS18B20        = 5
 */
#define STERGO_PROGRAM 1
// Screen or Led On device WS001 = Second 0 == device type
// FE: WS014 = WeatherStation 1 = LED 8x32, 4 = DHT22
#define STERGO_SCREEN 0
/*
 * STERGO_PROGRAM_BOARD :
 * 
 * ESP8266 default 01S = 1   // v01
 * LOLIN D1 mini       = 2   // v02
 */
#define STERGO_PROGRAM_BOARD 2

/*
 * STERGO_PLUG :
 * 
 * Native Board relay = 1   // Relay Switch - RS
 * Sonoff S26         = 2   // Plug Switch  - PS
 * Sonoff T4EU1C      = 3   // Light Switch - LS
 */
#define STERGO_PLUG 3

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Firmware Version always part of this file
#define FW_VERSION "000.05.103"  // Check releaseLog for details
#define MODEL_FRENDLY_NAME "Stergo Smart"
#define COMPANY_URL "http://www.stergo.hr"

// Exluded Code tu be included
// 0 = None to be included
#define EXCLUDED_CODE 0

// 1 true | 0 false  / Serial.print 
#define DEBUG 1

//=================================================================


#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESP8266WebServer.h"
#include "ESP8266httpUpdate.h"
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include "ArduinoJson.h"    //v5.13.5  
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ESP8266SSDP.h>    // SSDP (Simple Service Discovery Protocol) service
#include "Filesystem.h"     // 

#if ( STERGO_PROGRAM == 0 )  // Power Plug | Switch
    #define MODULE_SWITCH
    #include "Switch.h"
    #include "SSDP.h"
    #define MODULE_TICTACTOE
    #include "TicTacToe.h"
#elif ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 4 || STERGO_PROGRAM == 5 )  // Weather Station BME280 | DHT | DS18B20
    #define MODULE_WEATHER
    #include "Weather.h"
    #define MODULE_TICTACTOE
    #include "TicTacToe.h"
    #include "SSDP.h"
#elif ( STERGO_PROGRAM == 3 )  // Weather Station and Switch BME280
    #define MODULE_WEATHER
    #include "Weather.h"
    #define MODULE_SWITCH
    #include "SSDP.h"
    #include "Switch.h"
#elif ( STERGO_PROGRAM == 2 )  // TicTacToe
    #define MODULE_TICTACTOE
    #include "TicTacToe.h"
    #include "SSDP.h"
#elif ( STERGO_PROGRAM == 9 )  // EXCLUDED CODE
    #include <WiFiClientSecure.h>
    #include "Switch.h"
#endif
    
#if ( STERGO_SCREEN == 1 )
    #include <Adafruit_GFX.h>
    #include <Adafruit_NeoMatrix.h>
    #include <Adafruit_NeoPixel.h>
    #define MODULE_DISPLAY
    #include "Display.h"
#endif

extern "C" {
#include "user_interface.h"
}


/*********************************************************************************************************/
/* MQTT Server address
 * TODO : WebClient - In devices - Run Services (on | off )
 *                    Check if MQTT Broker is available, Check if MQTT is set - give alerts (graphic)
 *********************************************************************************************************/
#include "MQTT.h"

#define PRODUCT MODEL_NAME MODEL_NUMBER
#define FIRMWARE PRODUCT "-" FW_VERSION
String SERIAL_NUMBER = String(PRODUCT) + "-" + String(ESP.getChipId());

#define SERIAL_BAUDRATE 9600
#define WEBSERVER_PORT 80
#define NTPSERVER "europe.pool.ntp.org"
#define SSDPPORT 1900
#define SSDPADRESS 239, 255, 255, 250

//set local UDP port - using it for device 2 device communication (WiFi)
#define LOCAL_UDP_PORT 4210
#define NUMBER_OF_FOUND_SSDP 6
IPAddress foundSSDPdevices[NUMBER_OF_FOUND_SSDP];
int actualSSDPdevices = 0;

#define configFile "/config.json"
#define LOG_FILE "/log.json"

byte logOutput = 0;  // 0 = Serial, 1 = LogFile
#define sizeLog 30   // Log size = nr of records

// Optimize string space in flash, avoid duplication
String JST = "{\r\n";
String JSE = "\r\n}\r\n";
String JSE2 = "\r\n},\r\n";
String QCQ = "\":\"";
String QC = "\":";
String QCN = ",\r\n\"";
String QCNL = "\",\r\n\"";

String fOpen = "Fail to open ";
String fWrite = "Fail to write ";
#define nLOG "New Log file"
#define fsLarge " file size is too large"  //Used to be String
#define faParse "Fail to parse json "      //Used to be String


/******************************************************************************************************
 *  CAPTIVE PORTAL - SETUP
 *
 *  TODO: 
 *  
 *
 * hostname (wifi_hostname) for mDNS. Should work at least on windows. Try http://stergoweather.local *
 * Set these to your desired softAP credentials. They are not configurable at runtime                 *
 ******************************************************************************************************/
char softAP_ssid[20] = "StergoSmart_ap";  //This value shows only if SPIFFS not loaded
char softAP_pass[20] = "123456789";       //
#define DNS_PORT 53

// Interval For restart device if in AP mode and pass & Gateway are set in config.json
#define ap_intervalHist 1000 * 60 * 5                // 1000 * 60 * 5 = 5min
unsigned long ap_previousMillis = ap_intervalHist;     // time of last point added

/** Should I connect to W LAN asap? */
bool connect;

byte mqtt_start, webLoc_start, wifi_runAS, wifi_static, deviceType;
byte randNumber;

char _deviceType[2] = "1";  // Module device number - like BME280 = 1, Dalas = 2 and similar CHANGED SHOULD SWITCH TO MODEL_NAME + MODEL_NUMBER
char deviceName[20] = ""; 
char _devicename[28] = "";

//htaccess - Not used anywhere in the code - maybe one day implement credentials for HTTP
//char htaccess_username[20];
//char htaccess_password[20];

// Wifi Connection
char wifi_hostname[20] = "StergoSmart";
char wifi_ssid[20] = "";
char wifi_password[20] = "";
char wifi_StaticIP[16] = "";
char wifi_gateway[16] = "";
char wifi_subnet[16] = "";
char wifi_DNS[16] = "";


/******************************************************************************************************
 *  Time Intervals
 *
 *  TODO: 
 *
 ******************************************************************************************************/
// UpTime - Device
time_t upTime;

char webLoc_server[120];
// Time Interval for sending Wb Location data / targeting scripts to send data via HTTP POST method
int webLoc_interval = 1000 * 60 * 1;                    // 1000 * 60 * 1 = 1min
unsigned long webLoc_intervalHist;
unsigned long webLoc_previousMillis = webLoc_interval;  // time of last point added

char discord_url[130];
char discord_avatar[120];

// Module
char moduleName[20] = "Test";


// create Objects
WiFiClient espClient;
HTTPClient http;
PubSubClient client(espClient);

#if ( EXCLUDED_CODE == 9 )  //===============================================
// TEST WITH SSL
#define host "nas.local"
#define httpsPort 8081
#define fingerprint "71 0A 87 39 FA D0 38 FA 74 97 A9 41 67 3F 49 B1 1D FA 2E 01"
WiFiClientSecure wiFiClient;
#endif  //===============================================

DNSServer dnsServer;
FSInfo fs_info;

File fsUploadFile;

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, NTPSERVER, 0, 60000);

ESP8266WebServer server(WEBSERVER_PORT);
 
// Buffer for ArduinoJson
//String jsonLogBuffer = "{\"log\":[]}";                      // JSON log export buffer
