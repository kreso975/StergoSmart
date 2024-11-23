/* ======================== WiFiManager ==============================
Function: WiFiManager
Purpose : brain of WiFi behaviour
Input   : -
Output  : - WiFi Mode: 1 (STA), WiFi Mode: 2 (AP), WiFi Mode: 3 (AP_STA)
Comments: -
====================================================================== */

/*
WiFi.waitForConnectResult();
* WL_CONNECTED after successful connection is established 
* WL_NO_SSID_AVAIL in case configured SSID cannot be reached 
* WL_CONNECT_FAILED if password is incorrect 
* WL_IDLE_STATUS when Wi-Fi is in process of changing between statuses 
* WL_DISCONNECTED if module is not configured in station mode
*/

// Disconects STA 
bool disconnectSTA()
{ 
  WiFi.disconnect(true);
  return true;
}

bool startWPS()
{ // only push-button configuration (WPS_TYPE_PBC mode) is supported
  // Begin WPS (press WPS button on your router)
  if ( WiFi.beginWPSConfig() )
    return true;
  else
    return false;
}

bool startSTA( int STAmode = 0 )
{ // Try to get error log or something like - Wrong Password - No SSID etc
  WiFi.mode( WIFI_STA );

  if ( STAmode == 0 )
  {
    WiFi.begin( wifi_ssid, wifi_password );
  }
  else if ( STAmode == 1 )
  {
    IPAddress _ip,_gw,_sn,_dns;
        _ip.fromString(wifi_StaticIP);
        _gw.fromString(wifi_gateway);
        _sn.fromString(wifi_subnet);
        _dns.fromString(wifi_DNS);
        
    WiFi.config( _ip, _dns, _gw, _sn );
    WiFi.begin( wifi_ssid, wifi_password );
  }

  int cnt = 1;
  while ( WiFi.status() != WL_CONNECTED )
  {
    if ( cnt > 60 )
      return false;
      
    delay(500);
    //Serial.print('.');
    writeLogFile( ".", 0, 1 );
    cnt ++;
  }
  
  writeLogFile( F("STA connected"), 1, 3 );
  writeLogFile( F("WiFi Mode: ") + String(WiFi.getMode()), 1, 3 );
  writeLogFile( F("IP: ") + WiFi.localIP().toString(), 1, 3 );
  
  return true;
}

bool startAP()
{
  //writeLogFile( F("Starting AP"), 1 );
  
  WiFi.mode( WIFI_AP );
  String tmp = String(softAP_ssid) + "_" + String(ESP.getChipId());  
  WiFi.softAP( tmp.c_str(), softAP_pass );
  delay(500);                                                             // Without delay I've seen the IP address blank

  writeLogFile( F("AP connected"), 1, 3 );
  writeLogFile( F("WiFi Mode: ") + String(WiFi.getMode()), 1, 3 );
  writeLogFile( F("IP: ") + WiFi.softAPIP().toString(), 1, 3 );
  
  return true;
}

/* ======================== WiFiManager ==============================
Function: WiFiManager
Purpose : brain of WiFi behaviour
Input   : -
Output  : - WiFi Mode: 1 (STA), WiFi Mode: 2 (AP), WiFi Mode: 3 (AP_STA)
Comments: - */
void WiFiManager()
{
  wifi_station_set_hostname( wifi_hostname );         // This line allows access device by it's name - StergoWeather1.local (define in config.json)
  WiFi.hostname( wifi_hostname );                     // DHCP Hostname (useful for finding device for static lease)

  // 1st check if wifi_ssid && wifi_password is setup  != ""
  // Each attemp must be broken to separate functions - some of them might be called from different steps
  // Should I allow connecting to network without pass? NO
  if ( strcmp( wifi_ssid, "" )  && strcmp( wifi_password, "" ) )
  {
    // We can try to connect to STA but
    // Dynamic IP STA connection
    if ( wifi_static == 0 )
    {
      // let's first solve entire case with dynamic IP
      if ( startSTA() ) { }
      else
      {
        writeLogFile( F("Problem connect to STA"), 1 );

        // We Need to Bring up AP
        disconnectSTA();
        startAP();
      }   
    }
    else
    { // Use static ip check if != "" (Not Empty), strcmp == 0 (false) if it's equal
      if ( strcmp( wifi_StaticIP, "" ) ) // Seams as not needed - && strcmp( wifi_gateway, "" ) && strcmp( wifi_subnet, "" ) && strcmp( wifi_DNS, "" )
      {
        if ( startSTA( 1 ) ) // Connecting with static IP
        {
          writeLogFile( F("Succes connect with Static IP"), 1, 3 );
        }
        else // PROBLEM
        {
          writeLogFile( F("Problem connect with Static IP"), 1, 3 ); // decide what to do 
          if ( startSTA() ) { }                                      // We Can try again with dynamic IP
          else                                                       // and if that does not work - go to AP
          {
            writeLogFile( F("Problem connect to STA"), 1, 3 );
 
            // We Need to Bring up AP
            disconnectSTA();
            startAP();
          }  
        }
      }
      else
      {
        //Something wrong is with one of IP's == EMPTY
        // decide what to do
      }
    }
  }
  else
  {
    // No STA Data - Run just AP
    startAP();
  }
}

/* ======================== OTA Manager ==============================
Function: firmwareOnlineUpdate
Purpose : brain of OTA behaviour
Input   : -
Output  : - 
Comments: - Firmware Update Version Check done server side 
            http://192.168.1.101/StergoWeather/firmwareCheck.php
          
          - naming convetion for firmware file, 
          WS001 = Device, v01 = Device version number, 000.05.000 = firmware version number
          

          WS001v01-00005000.bin
          WS002v02-00005000.bin

          - 1. Manual upload
            2. Auto check online

          firmware should be check based on module,
          ??? not on device (extensions are added to module - code should auto recognize them)

          FIRMWARE is a construct MODEL_NAME + MODEL_NUMBER + FW_VERSION */
String firmwareOnlineUpdate( byte what )
{
  /* CODE NEEDS TO BE CHECKED - Based on theese callback We can return Value to Browser whether Success or Error | on success
   *  we can start reload browser process, on Error show in Info bar Error
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);
  */
  String message;

  t_httpUpdate_return ret;
  //This WORKS!!! YEAH
  if ( what == 1 )
  {
    t_httpUpdate_return ret = ESPhttpUpdate.updateFS( espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", SERIAL_NUMBER );
    //t_httpUpdate_return ret = ESPhttpUpdate.updateFS( espClient, "http://192.168.1.101/StergoWeather/firmware/TT001v01-000.05.102.spiffs.bin" );
  }
  else if ( what == 2 )
  {
    t_httpUpdate_return ret = ESPhttpUpdate.update( espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", FIRMWARE );
  }
  
  //t_httpUpdate_return ret = ESPhttpUpdate.update( espClient, "http://192.168.1.101/StergoWeather/firmware/temp/StergoSmart.ino.bin", FIRMWARE );
  switch( ret ) {
    case HTTP_UPDATE_FAILED:
      #if ( DEBUG == 1 )
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      #endif
      writeLogFile( F("HTTP_UPDATE_FAIL"), 1, 3 );
      message = "{\"Error\":\"Failed\"}";
      break;
    case HTTP_UPDATE_NO_UPDATES:
      writeLogFile( F("Firmware Up2Date"), 1, 3 );
      message = "{\"Info\":\"Up2Date\"}";
      break;
    case HTTP_UPDATE_OK:
      writeLogFile( F("HTTP_UPDATE_OK"), 1, 3 );
      message = "{\"success\":\"Updating..\"}";
      break;
  }  

  return message;
}

/*
void update_started() {
  serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}
*/

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