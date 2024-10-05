## **TODO:**
  - Filesystem/setupFS() - Implement fail over if SPIFFS get corrupted - copy content from web loc or Upload or 
    https://github.com/spacehuhn/esp8266_deauther/blob/master/esp8266_deauther/webfiles.h or start updateOTA for SPIFFS
  - Firmware update OTA, with more info - Your firmware is up to date, Firmware XXX available etc
 
 <br><br>
 
## **CHANGES:**
**000.05.102** - in development
- SSDP cleanUp code
- modified Config.h, new #definitions
- enable Weather module to play TicTacToe over UDP with other devices found in local network
- Switches added #define STERGO_PLUG_BOARD, cleanUp Switch.h
- CleanUp MQTT from switch old code
- MQTT cleanUP sendMQTT, payload moved to Modules
- Main lopp cleanUP MQTT time intervals to check in Modules not in MQTT.ino
- Discord Webhook URL should point to script that actualy sends Discord Webhook - we just send payload
- BME280 cleanUP code


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
