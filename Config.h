#ifndef CONFIG_H
#define CONFIG_H

#if defined(ESP8266)                                         // -----------------  ESP8266  -----------------
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include "ESP8266WebServer.h"
  #include "ESP8266httpUpdate.h"
  #include <ESP8266mDNS.h>
  #include <ESP8266SSDP.h>            // SSDP (Simple Service Discovery Protocol) service
  extern "C" {
    #include "user_interface.h"
  }
  #include <pgmspace.h>
  #define F(string_literal) (FPSTR(PSTR(string_literal)))
#elif defined(ESP32)                                         // -----------------  ESP32  -----------------
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <FS.h>                     // File System for Web Server Files
  #include <WebServer.h>
  #include <HTTPUpdate.h>
  #include <ESPmDNS.h>
  #include <ESP32SSDP.h>              // 2.0.0 https://github.com/luc-github/ESP32SSDP/releases
  #include "esp_system.h"
  #include <esp_netif.h>
  #include "esp_wifi.h"
  #define F(string_literal) (string_literal)
#endif                                                       // -------------------------------------------

#include <DNSServer.h>
#include <LittleFS.h>
#include <TimeLib.h>
#include <NTPClient_Generic.h>          // https://github.com/khoih-prog/NTPClient_Generic
#include "ArduinoJson.h"  // 6.21.5
#include <WiFiUdp.h>

#if defined(ESP8266)                                         // -----------------  ESP8266  -----------------
  String chipID = String(ESP.getChipId());
#elif defined(ESP32)                                         // -----------------  ESP32  -------------------
  uint64_t chipIDmac = ESP.getEfuseMac();
  String chipID = String((uint16_t)(chipIDmac >> 32), HEX) + String((uint32_t)chipIDmac, HEX);
#endif                                                       // ---------------------------------------------

// ----------------------------------------------------------------------------------------------------------

// Main settings for StergoSmart
#include "settings.h"

#include "Filesystem.h"     //
#include "./src/WiFiManager/WiFiManager.h"

#if ( STERGO_PROGRAM == 0 )  // Power Plug | Switch
    #define MODULE_MQTT
    #define MODULE_SWITCH
    #include "Switch.h"
    #include "SSDP.h"
    #define MODULE_TICTACTOE
#elif ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 4 || STERGO_PROGRAM == 5 )  // Weather Station BME280 | DHT | DS18B20
    #define MODULE_MQTT
    #define MODULE_WEATHER
    #include "Weather.h"
    #define MODULE_TICTACTOE
    #include "SSDP.h"
#elif ( STERGO_PROGRAM == 3 )  // Weather Station and Switch BME280
    #define MODULE_MQTT
    #define MODULE_WEATHER
    #include "Weather.h"
    #define MODULE_SWITCH
    #include "SSDP.h"
    #include "Switch.h"
#elif ( STERGO_PROGRAM == 2 )  // TicTacToe
    #define MODULE_TICTACTOE
    #include "SSDP.h"
    #define MODEL_NAME "TT001"
#endif

#if ( STERGO_SCREEN == 1 )
    #define MODULE_DISPLAY
    #define MODULE_MQTT
#endif

#ifdef MODULE_MQTT                  //================ MQTT Manager  ================
#include "./src/MQTTManager/MQTTManager.h"
MQTTManager mqttManager;
#endif	                            //================================================

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Macro generate model version number based on STERGO_PROGRAM_BOARD
#define MODEL_NUMBER_HELPER(x) "v0" #x
#define MODEL_NUMBER_HELPER2(x) MODEL_NUMBER_HELPER(x)
#define MODEL_NUMBER MODEL_NUMBER_HELPER2(STERGO_PROGRAM_BOARD)

#define PRODUCT MODEL_NAME MODEL_NUMBER
#define FIRMWARE PRODUCT "-" FW_VERSION
String SERIAL_NUMBER = String(PRODUCT) + "-" + chipID;

#define SERIAL_BAUDRATE 9600
#define DNS_PORT 53
#define WEBSERVER_PORT 80
#define NTPSERVER "europe.pool.ntp.org"
#define NTP_UPDATE 60000  // Update every 60 seconds

#define configFile "/config.json"
#define LOG_FILE "/log.json"

byte logOutput = 0;  // 0 = Serial, 1 = LogFile
#define sizeLog 30   // Log size = nr of records

String fOpen = "Fail to open ";
String fWrite = "Fail to write ";
#define nLOG "New Log file"
#define fsLarge " file size is too large"  //Used to be String
#define faParse "Fail to parse json "      //Used to be String

byte deviceType, randNumber;

char deviceName[20] = ""; 
char _devicename[28] = "";

char discord_url[130];
char discord_avatar[120];

// Module
char moduleName[20] = "Test";

// create Objects
WiFiClient espClient;
HTTPClient http;

DNSServer dnsServer;
#if defined(ESP8266)                                    //====================== ESP8266 ================
  FSInfo fs_info;
#elif defined(ESP32)                                    //======================== ESP32 ================
  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();
#endif                                                  //===============================================

File fsUploadFile;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTPSERVER, 0, NTP_UPDATE); // 60 * 60 * 1000 == 1 hour

// startTime - When Device Started
time_t startTime;
time_t adjustedTime;

#if defined(ESP8266)                                    //====================== ESP8266 ================
  ESP8266WebServer server(WEBSERVER_PORT);
#elif defined(ESP32)                                    //======================== ESP32 ================
  WebServer server(WEBSERVER_PORT);
#endif                                                  //===============================================

// Create an instance of the WiFiManager class
WiFiManager wifiManager;

// Load Modules
#ifdef MODULE_DISPLAY                             //====================== MODULE_DISPLAY ================
  //#include "Display.h"
  #include "./modules/myFonts.h"
  #include "./modules/WS2812B_Matrix/WS2812B_Matrix.h"
  #include "./modules/WS2812B_Matrix/WS2812B_Matrix.cpp"
#endif                                            //======================================================

// TicTacToe after Display because it setup some variables. Also possible to include .h but that is going to 2nd phase
#ifdef MODULE_TICTACTOE                           //==================== MODULE_TICTACTOE ================
  #include "./modules/TicTacToe/TicTacToe.h"
  #include "./modules/TicTacToe/TicTacToe.cpp"
#endif                                            //======================================================

#endif // CONFIG_H