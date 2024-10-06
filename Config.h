/*
 * For What Device We need to Compile
 * 
 * STERGO_PROGRAM :
 *
 * PowerSwitch                  = 0
 * StergoWeather                = 1
 * ticTacToe                    = 2
 * StergoWeather+PowerSwitch    = 3
 */
#define STERGO_PROGRAM 0

/*
 * Different models used for Weather
 * 
 * STERGO_WEATHER :
 * 
 * ESP8266 default 01S = 1   // WS001
 * LOLIN D1 mini       = 2   // WS002
 */
#define STERGO_WEATHER 2

/*
 * Different models used for Plug / Switch
 * 
 * STERGO_PLUG :
 * 
 * Native Board relay = 1   // Relay Switch - RS
 * Sonoff S26         = 2   // Plug Switch  - PS
 * Sonoff T4EU1C      = 3   // Light Switch - LS
 */
#define STERGO_PLUG 3

// ESP8266 01S        = 1
// LOLIN d1 Mini      = 2
#define STERGO_PLUG_BOARD 1


// Firmware Version always part of this file
#define FW_VERSION "000.05.101"  // Check releaseLog for details
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

#if (STERGO_PROGRAM == 0)  // Power Plug | Switch
#define MODULE_SWITCH
#define MODULE_TICTACTOE
#include "Switch.h"
#include "SSDP.h"
#include "TicTacToe.h"
#elif (STERGO_PROGRAM == 1)  // Weather Station
#define MODULE_WEATHER
#define MODUL_BME280
#define MODULE_TICTACTOE
#include <Adafruit_BME280.h>
#include "BME280.h"
#include "SSDP.h"
#include "TicTacToe.h"
#elif (STERGO_PROGRAM == 3)  // Weather Station and Switch
#define MODULE_WEATHER
#define MODUL_BME280
#define MODULE_SWITCH
#include <Adafruit_BME280.h>
#include "BME280.h"
#include "SSDP.h"
#include "Switch.h" 
#elif (STERGO_PROGRAM == 2)  // TicTacToe
#define MODULE_TICTACTOE
#include "SSDP.h"
#include "TicTacToe.h"
#elif (STERGO_PROGRAM == 9)  // EXCLUDED CODE
#include <WiFiClientSecure.h>
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

String PRODUCT = String(MODEL_NAME) + String(MODEL_NUMBER);
String FIRMWARE = PRODUCT + "-" + String(FW_VERSION);
String SERIAL_NUMBER = PRODUCT + "-" + String(ESP.getChipId());

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

/** Should I connect to W LAN asap? */
bool connect;
/** Last time I tried to connect to WLAN */
//long lastConnectTry = 0;
/** Current WLAN status */
//byte status = WL_IDLE_STATUS;  // nowhere in the code used. WiFi.status is more readable

byte mqtt_start, webLoc_start, wifi_runAS, wifi_static, deviceType;
byte randNumber;

char _deviceType[2] = "1";  // Module device number - like BME280 = 1, Dalas = 2 and similar CHANGED SHOULD SWITCH TO MODEL_NAME + MODEL_NUMBER
char deviceName[20] = ""; 
char _devicename[28] = "";

//htaccess - Not used anywhere in the code
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
int webLoc_interval = 1000 * 60 * 1;
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
