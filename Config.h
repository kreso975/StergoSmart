//
// "StergoWeather" = 1, "PowerSwitch" = 0, "ticTacToe" = 2, StergoWeather+PowerSwitch = 3, "EXCLUDED CODE" = 9 );
//
#define STERGO_PROGRAM 0

/*
 * Different models used for Plug / Switch
 * 
 * Native Board relay = 1
 * Sonoff S26         = 2
 * Sonoff T4EU1C      = 3   //Light Switch
 */
#if (STERGO_PROGRAM == 0 || STERGO_PROGRAM == 3)  // DON'T TOUCH
#define STERGO_PLUG 1                             // CHANGE THIS
#endif


// Firmware Version always part of this file
#define FW_VERSION "000.05.101"  // Check releaseLog for details
#define MODEL_FRENDLY_NAME "Stergo Smart"
#define COMPANY_URL "http://www.stergo.hr"

#include <ESP8266WiFi.h>
#include "ESP8266WebServer.h"
#include "ESP8266httpUpdate.h"
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include "ArduinoJson.h"  //v5.13.5
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ESP8266SSDP.h>

#if (STERGO_PROGRAM == 0)  // Power Plug | Switch
#include "Switch.h"
#include "TicTacToe.h"
#elif (STERGO_PROGRAM == 1)  // Weather Station
#include <Adafruit_BME280.h>
#include "BME280.h"
#elif (STERGO_PROGRAM == 2)  // TicTacToe
#include "TicTacToe.h"
#elif (STERGO_PROGRAM == 3)
#include <Adafruit_BME280.h>
#include "BME280.h"
#include "Switch.h"
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

#define SERIAL_BAUDRATE 115200
#define WEBSERVER_PORT 80
#define NTPSERVER "europe.pool.ntp.org"
#define SSDPPORT 1900
#define SSDPADRESS 239, 255, 255, 250

//set local UDP port - using it for device 2 device communication (WiFi)
#define LOCAL_UDP_PORT 4210
#define NUMBER_OF_FOUND_SSDP 6
IPAddress foundSSDPdevices[NUMBER_OF_FOUND_SSDP];
int actualSSDPdevices = 0;

//String toStringIp( IPAddress ip );
//bool checkPing( IPAddress remote_ip );

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
long lastConnectTry = 0;
/** Current WLAN status */
byte status = WL_IDLE_STATUS;

byte mqtt_start, webLoc_start, wifi_runAS, wifi_static, deviceType;

char _deviceType[2] = "1";  // Module device number - like BME280 = 1, Dalas = 2 and similar CHANGED SHOULD SWITCH TO MODEL_NAME + MODEL_NUMBER
char deviceName[16] = "";

//htaccess
char htaccess_username[20];
char htaccess_password[20];

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

// Time interval for udp SSDP M-SEARCH
bool measureSSDPFirstRun = true;
const long intervalSSDP = 1000 * 60 * 10;   // 1000 * 60 * 10 - 10min
unsigned long previousSSDP = intervalSSDP;  // time of last point added

// Time Interval for sending MQTT data
int mqtt_interval = 120;
unsigned long mqtt_intervalHist;
unsigned long mqtt_previousMillis;     // time of last point added


// Time Interval for sending Wb Location data / targeting scripts to send data via HTTP POST method
int webLoc_interval = 60;
const long webLoc_intervalHist = 1000 * webLoc_interval;
unsigned long webLoc_previousMillis = webLoc_intervalHist;  // time of last point added

// Module
char moduleName[20] = "Test";


// create Objects
WiFiClient espClient;
PubSubClient client(espClient);

#if (STERGO_PROGRAM == 9)  //===============================================
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
