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
    Serial.print('.');
    cnt ++;
  }
  
  writeLogFile( F("STA connected"), 1 );
  writeLogFile( "WiFi Mode: " + String(WiFi.getMode()), 1 );
  writeLogFile( "IP: " + WiFi.localIP().toString(), 1 );
  
  return true;
}

bool startAP()
{
  //writeLogFile( F("Starting AP"), 1 );
  
  WiFi.mode( WIFI_AP );
  String tmp = String(softAP_ssid) + "_" + String(ESP.getChipId());  
  WiFi.softAP( tmp.c_str(), softAP_pass );
  delay(500);                                                             // Without delay I've seen the IP address blank

  writeLogFile( F("AP connected"), 1 );
  writeLogFile( "WiFi Mode: " + String(WiFi.getMode()), 1 );
  writeLogFile( "IP: " + WiFi.softAPIP().toString(), 1 );
  
  return true;
}

void WiFiManager()
{
  wifi_station_set_hostname( wifi_hostname );         // This line allows access device by it's name - StergoWeather1.local (define in config.json)
  WiFi.hostname( wifi_hostname );                     // DHCP Hostname (useful for finding device for static lease)

  //1st check if wifi_ssid && wifi_password is setup  != ""
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
          writeLogFile( F("Succes connect with Static IP"), 1 );
        }
        else // PROBLEM
        {
          writeLogFile( F("Problem connect with Static IP"), 1 ); // decide what to do 
          if ( startSTA() ) { }                                      // We Can try again with dynamic IP
          else                                                       // and if that does not work - go to AP
          {
            writeLogFile( F("Problem connect to STA"), 1 );
 
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

          FIRMWARE is a construct MODEL_NAME + MODEL_NUMBER + FW_VERSION
====================================================================== */

void firmwareOnlineUpdate()
{
  /* CODE NEEDS TO BE CHECKED - Based on theese callback We can return Value to Browser whether Success or Error | on success
   *  we can start reload browser process, on Error show in Info bar Error
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);
  */
  
  //This WORKS!!! YEAH
  //t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs( espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", SERIAL_NUMBER );

  t_httpUpdate_return ret = ESPhttpUpdate.update( espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", FIRMWARE );

  switch( ret ) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                writeLogFile( F("HTTP_UPDATE_FAIL"), 1 );
                break;

            case HTTP_UPDATE_NO_UPDATES:
                writeLogFile( F("Firmware Up2Date"), 1 );
                break;

            case HTTP_UPDATE_OK:
                writeLogFile( F("HTTP_UPDATE_OK"), 1 );
                break;
        }  
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


/* ======================================================================
Function: wifiScanJSON
Purpose : scan Wifi Access Point and return JSON code
Input   : -
Output  : -
Comments: -
====================================================================== */
void wifiScanJSON()
{
  String response = "";
  bool first = true;
  int scanStatus = WiFi.scanComplete();

  // Json start
  response += JST;

  if ( scanStatus == WIFI_SCAN_FAILED )
  {
    WiFi.scanNetworks( true );
    response += "\"status" + QCQ + "Scan in progess\"";
  }
  else if ( scanStatus >= 0 )
  {
    response += "\"result\": [";
    for ( uint8_t i = 0; i < scanStatus; ++i )
    {
      int8_t rssi = WiFi.RSSI(i);

      //data["bssid"] = WiFi.BSSIDstr(i);
      //data["isHidden"] = WiFi.isHidden(i);
      // dBm to Quality
      
      if (first)
        first = false;
      else
        response += ",";

      response += "{\"ssid" + QCQ + WiFi.SSID(i) + "\",\"encType\":\"" + WiFi.encryptionType(i) + "\",\"chann\":"+ wifi_get_channel() +",\"rssi\":" + rssi + JSE;

    }
    response += "],\"status" + QCQ + "OK\"";
    WiFi.scanDelete();
  }

  // Json end
  response += JSE;

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response );
}


//
// Testing HTTPS
#if ( STERGO_PROGRAM == 9 )
void connectToSecureAndTest()
{
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client2;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client2.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if ( client2.verify(fingerprint, host) )
    Serial.println("certificate matches");
  else
    Serial.println("certificate doesn't match");

  String url = "/StergoWeather/index.php";
  String PostData = "id=332&mi=76.34&ter=45.76";
  
  Serial.print("requesting URL: ");
  Serial.println(url);

  client2.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: StergoWS001V01\r\n" +
               "Connection: close\r\n" +
               "Accept: */*\r\n" +
               "Content-Type: application/x-www-form-urlencoded;" + "\r\n" +
               "Content-Length: " + PostData.length() + "\r\n\r\n" + PostData + "\n" );

  Serial.println("request sent");

  while ( client2.connected() )
  {
    String line = client2.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  while ( client2.available() )
  {
    String line = client2.readStringUntil('\n');
    
    if ( line.startsWith("\"state\":\"status\"") )
    {
      Serial.println(line);
      break;
    }
  }
  client2.stop();
}
#endif
