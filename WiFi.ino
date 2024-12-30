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
String firmwareOnlineUpdate( byte what )
{
  String message;
  String prefix;

  t_httpUpdate_return ret;
  
  // We will send all mesages then restart
  ESPhttpUpdate.rebootOnUpdate(false);
  // This is needed not to close TCP before messages are sent
  ESPhttpUpdate.closeConnectionsOnUpdate(false);

  if ( what == 1 )
  {
    prefix = "Spiffs";
    ret = ESPhttpUpdate.updateFS( espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", SERIAL_NUMBER );
    //t_httpUpdate_return ret = ESPhttpUpdate.updateFS( espClient, "http://192.168.1.101/StergoWeather/firmware/TT001v01-000.05.102.spiffs.bin" );
  }
  else if ( what == 2 )
  {
    prefix = "Firmware";
    ret = ESPhttpUpdate.update( espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", FIRMWARE );
    //t_httpUpdate_return ret = ESPhttpUpdate.update( espClient, "http://192.168.1.101/StergoWeather/firmware/temp/StergoSmart.ino.bin", FIRMWARE );
  }
  
  switch( ret ) {
    case HTTP_UPDATE_FAILED:
      //Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      writeLogFile( "HTTP UPDATE FAIL "+String(ESPhttpUpdate.getLastErrorString()), 1, 3 );
      message = F("{\"Error\":\"Failed\"}");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      writeLogFile( prefix + F(" Up2Date"), 1, 3 );
      message = F("{\"Info\":\"No Updates\"}");
      break;
    case HTTP_UPDATE_OK:
      writeLogFile( prefix + F(" Update"), 1, 3 );
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
  // New Async scan Method
  WiFi.scanNetworksAsync(wifiScanResult);
}