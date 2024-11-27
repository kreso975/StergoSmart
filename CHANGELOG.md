## **TODO:**
  - Filesystem/setupFS() - Implement fail over if SPIFFS get corrupted - copy content from web loc or Upload or 
    https://github.com/spacehuhn/esp8266_deauther/blob/master/esp8266_deauther/webfiles.h or start updateOTA for SPIFFS
  - Firmware update OTA, with more info - Your firmware is up to date, Firmware XXX available etc
 
 <br><br>
 
## **CHANGES:**
**000.05.104** - in development
- wifiScanJSON() rewritten, using asyn method WiFi.scanNetworksAsync(wifiScanResult)
- firmwareOnlineUpdate() updated
- code CleanUP
- Fixed Interval For restart device if in AP mode and pass & Gateway are set in config.json | default 5min
- Prep for Display 8x32 WS2812B using FastLed.h

**000.05.103**
- added support for DHT sensors DHT11, DHT12, DHT21, DHT22  
- added support for DS18B20 sensor  
- rename Variables mqtt_bme280Humidity, mqtt_bme280Temperature, mqtt_bme280Pressure into generic mqtt_Humidity, mqtt_Temperature, mqtt_Pressure  
- reorganized Weather structure  
- New precompile definitions MODULE_WEATHER, MODULE_BME280, MODULE_DHT, MODULE_DS18B20  
- Restart device after 5 min if APmode && WiFi pass & Gateway are set in config.json (when wifi connection is lost - we need to try go to STA)  
- Start ntpUDP always if WiFi is STA  
- fixed old mesures.json link to measures.json  
- Config.ino added return message for success init config after save/update  
- Upgrade ArduinoJson v5.13.5 to v6.21.5  
- new #definition STERGO_SCREEN - future use of Screens  
- reorganized handleHttp. Added More application/json responses. new function sendJSONheaderReply  

**000.05.102**
- SSDP cleanUp code
- modified Config.h, new #definitions STERGO_PROGRAM_BOARD
- enable Weather module to play TicTacToe over UDP with other devices found in local network
- Switches added #define STERGO_PROGRAM_BOARD, cleanUp Switch.h
- CleanUp MQTT from switch old code
- MQTT cleanUP sendMQTT, payload moved to Modules
- Main lopp cleanUP MQTT time intervals to check in Modules not in MQTT.ino
- Discord Webhook URL should point to script that actualy sends Discord Webhook - we just send payload
- BME280 cleanUP code
- SPIFFS OTA UPDATE
- MQTT.h topics VARs moved to BME or Switch where they belong
- MQTT if there is problem connecting to Broker after 3 retries, wait 1h and try again. 
- modified and cleanUP in handleHttp.ino
- Added setting for Tic Tac Toe into config.json
- UI added Tic Tac Toe Options under Device Tab


**000.05.101**  
- CleanUp code. Moved SSDP from WiFi.ino to SSDP.ino
- Remove some duplicated code in Configs
- Finished Tic Tac Toe auto play for some devices | Switch or Tic Tac Toe single
- Implemented WebHook for Sensors
- MQTT cleanup

**000.05.100**  
- Added 'retain: **true**' in MQTT publish

**000.05.099**  
- Added Switch MQTT basic On/Off only for 1 switch, fix and modifications on index.html. No IP harcoded any more. 

**000.05.098**  
- Added Switch MQTT Topics save in config, fix mqtt time interval used from config.json  

**000.05.097**  
- Added MODEL_NUMER v02 in BME280.h, Simplify MODEL_NUMER define in Switch.h

**000.05.096**  
- MQTT config write fix. Fix mqtt nr of trys to reconnect by moving up position of include mqtt.h file.  
- Config.ino initConfig() removed check if filesize > 1520. I don't think we care how big is
 
**000.05.095**  
- hitory.json change to normal instead prettyPrintTo  

**000.05.094**  
- minor fixes, added var webLoc_server  

**000.05.093**  
- bug fix for sensor write to history.json, sendMeasures() moved from handleHttp to BME280  

**000.05.092**  
- rewritten MainSensorConstruct, removed jsonLogBuffer, writing History immediately, BME280 rewritten  

**000.05.091**  
- rewritten logWriting, removed logBuffer, writing Log immediately. Filesystem rewritten, removed initLog()  

**000.05.090**  
- rewritten config.json , functions initConfig,  writeToConfig, removed function initDeviceSetup - useing writeToConfig  

**000.05.088**  
- added check in BME280 read - getWeather isnan(h_tmp) || isnan(t_tmp)  

**000.05.087**  
- added STERGO_PROGRAM 3 == WeatherStation + Switch : If BME280 installed onto switch || Plug - Needs deeper Test || HTML SPIFFS NOT addapted  

**000.05.086**  
- added Functionality for ESPhttpUpdate.updateSpiffs but not active - tested  

**000.05.085**  
- Removed delay(500), didn't help - Flash mode: DIO, CPU Freq: 160  

**000.05.083/084**  
- Added delay(500) before BME280 history write / trying to solve issue when loosing history data file  

**000.05.082**  
- Small fix in MainSensorConstruct() : loadHistory : Do updateHistory() based on jsonDataBuffer  

**000.05.081**  
- Small fixes in getWeather()  

**000.05.080**  
- logic changed in getWeather() / in IF put check measure data - false readings or device error  

**000.05.079**  
- Firmware update done!!. Using remote Php script for checking against new version.  

**000.05.078**  
- Support for T4EU1C.  

**000.05.077**  
- Support for Sonoff S26, Button support added. Still need to fix lights. Small fix on MQTT save config  

**000.05.071**  
- Added code for Sonoff S26 supporting Button. Need finish Button functionality. Possible support for T4EU light 1wire switch.  

**000.05.070**
- Optimized code, removed serving history from buffer/only from static. Added some code for TICTACTOE - not finished yet  
- Changes in setupMQTT() - optimized code. Fix BME280 if t==0 || h==0 to &&.  

**0.5.65**  
- Added/extracted/moved MQTT conf into MQTT.h file. Addopted code for Handling MQTT compile based on config setuf of device  

**0.5.64**  
- Updated Adafruit_BME280.h library from 1.1.0 to 2.0.0  

**0.5.63**  
- Minor changes  

**0.5.61**  
- Minor changes. Log write for FileUpload.  

**0.5.6**  
- Combined code for WS and Switch. var compile for = ( "StergoWeather" = 1, "PowerSwitch" = 0 ); in #define STERGO_PROGRAM  0  
- Small changes on SSDP  

**0.5.51**  
- SPIFFS sized to 256K, Code reduced to 372k, OTA possible  

**0.5.4**  
- SPIFF Web files compressed to gzip , and served compressed  

**0.5.35**  
- Check history.json / jsonDataBuffer file size - to prevent writing in file { } empty / changes  
- Filesystem - updateHistory(), functions - MainSensorConstruct() if History file less then 4 , update it  

**0.5.34**  
- SSDP change: SSDP.setName( String(deviceName) );  

**0.5.33**  
- SSDP changes: SSDP.setSerialNumber, SSDP.setModelName, SSDP.setModelNumber, SSDP.setModelURL  
- Main File: added constants: MODEL_NAME, MODEL_NUMBER  
- added releaseLog file as part of Project - trace changes in code  
                    
         
**0.5.32**  
- SSDP.setModelURL("Test"); TEMP fixed wrong generation of url  
