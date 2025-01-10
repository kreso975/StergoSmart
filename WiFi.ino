/* ======================== OTA Manager ==============================
Function: firmwareOnlineUpdate
Purpose : brain of OTA behaviour
Input   : byte what -> 1 == SPIFFS, 2 == Firmware
Output  : String Json message format 
Comments: - Firmware Update Version Check done server side 
            http://192.168.1.101/StergoWeather/firmwareCheck.php
          
          - naming convention for firmware file, 
          WS001 = Device, v01 = Device version number, 000.05.000 = firmware version number
          
          WS001v01-000.05.000.bin
          WS002v02-000.05.000.bin

          - 1. Manual upload
            2. Auto check online

          FIRMWARE is a construct MODEL_NAME + MODEL_NUMBER + FW_VERSION */
String firmwareOnlineUpdate(byte what) {
  String message;
  String prefix;
  t_httpUpdate_return ret;

  #if defined(ESP8266)
    ESPhttpUpdate.rebootOnUpdate(false);
    ESPhttpUpdate.closeConnectionsOnUpdate(false);
  #elif defined(ESP32)
    httpUpdate.rebootOnUpdate(false);
  #endif

  if (what == 1) {
    prefix = "LittleFS";
    #if defined(ESP8266)
      ret = ESPhttpUpdate.updateFS(espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", SERIAL_NUMBER);
    #elif defined(ESP32)
      // Use the Update class for ESP32
      WiFiClient client;
      if (client.connect("192.168.1.101", 80)) {
        client.print(String("GET ") + "/StergoWeather/firmwareCheck.php" + " HTTP/1.1\r\n" +
                     "Host: " + "192.168.1.101" + "\r\n" +
                     "Connection: close\r\n\r\n");
        while (client.connected()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") {
            break;
          }
        }
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) {
          Update.printError(Serial);
        }
        while (client.available()) {
          uint8_t buff[128];
          int len = client.read(buff, sizeof(buff));
          if (len <= 0) {
            break;
          }
          Update.write(buff, len);
        }
        if (!Update.end(true)) {
          Update.printError(Serial);
        }
      }
      ret = HTTP_UPDATE_OK;
    #endif
  } else if (what == 2) {
    prefix = "Firmware";
    #if defined(ESP8266)
      ret = ESPhttpUpdate.update(espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", FIRMWARE);
    #elif defined(ESP32)
      ret = httpUpdate.update(espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", FIRMWARE);
    #endif
  }

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      #if defined(ESP8266)
        writeLogFile("HTTP UPDATE FAIL " + String(ESPhttpUpdate.getLastErrorString()), 1, 3);
      #elif defined(ESP32)
        writeLogFile("HTTP UPDATE FAIL " + String(httpUpdate.getLastErrorString()), 1, 3);
      #endif
      message = F("{\"Error\":\"Failed\"}");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      writeLogFile(prefix + F(" Up2Date"), 1, 3);
      message = F("{\"Info\":\"No Updates\"}");
      break;
    case HTTP_UPDATE_OK:
      writeLogFile(prefix + F(" Update"), 1, 3);
      message = F("{\"success\":\"Updating ..\"}");
      break;
  }

  return message;
}

void wifiScanResult(int networksFound)
{ 
  String json;
  
  json = "{\"result\": [";
  //Serial.printf("%d network(s) found\n", networksFound);
  for (int i = 0; i < networksFound; i++)
  {
    json += "{\"ssid\": \""+WiFi.SSID(i)+"\",\"encType\": \"" + WiFi.encryptionType(i) + "\",\"chann\": "+WiFi.channel(i)+",\"rssi\": "+WiFi.RSSI(i)+"}";
    ( i < networksFound-1 ) ? json += "," : json +="";

    //Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
  }
  json += "], \"status\": \"OK\"}";

  sendJSONheaderReply( 3, json );
}

/* ======================================================================
Function: wifiScanJSON
Purpose : scan Wifi Access Point and return JSON code
Input   : -
Output  : HTTP JSON
Comments: - */
void wifiScanJSON()
{
  #if defined(ESP8266)
    WiFi.scanNetworksAsync(wifiScanResult);
  #elif defined(ESP32)
    WiFi.scanNetworks();
    int n = WiFi.scanComplete();
    wifiScanResult(n);
  #endif
}
