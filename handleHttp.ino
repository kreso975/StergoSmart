void setupSSDP()
{
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName( String(deviceName) );
  SSDP.setDeviceType("urn:schemas-upnp-org:device:StergoSmart:1");  //In case: put after SSDP.begin
  SSDP.setSerialNumber(SERIAL_NUMBER);                              //This must be adjusted to chipID
  SSDP.setURL("index.html");
  SSDP.setModelName(MODEL_NAME);
  SSDP.setModelNumber(MODEL_NUMBER);                                // This must be SET in main config 
  SSDP.setModelURL( String(COMPANY_URL) + "/" + PRODUCT );          // Product is Model_name + Model_number
  SSDP.setManufacturer( "Stergo" );
  SSDP.setManufacturerURL( COMPANY_URL );
  SSDP.begin();
  writeLogFile( F("SSDP Started"), 1 );
}

void setupHttpServer()
{
  if ( WiFi.getMode() == 1 )
  {
    server.on( "/", handleIndex );
    server.on( "/index.html", handleIndex );
    
    server.on( "/description.xml", HTTP_GET, [&]() {
      SSDP.schema(server.client());
    });
  }
  else
  { // IF SERVER AP
    // Setup the DNS server redirecting all the domains to the apIP 
    dnsServer.setErrorReplyCode( DNSReplyCode::NoError );
    dnsServer.start( DNS_PORT, "*", WiFi.softAPIP() ) ;

    server.on( "/", handleRoot );
    server.on( "/generate_204", handleRoot );  //Android captive portal. <= Android 6.0.1
    server.on( "/fwlink", handleRoot );  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    
    server.onNotFound ( handleNotFound );
  }

  server.serveStatic( "/favicon.ico", SPIFFS, "/img/favicon.ico");

  server.on( "/js/jquery.min.js", []() {
    if ( !handleFileRead("/js/jquery.min.js") )             // send it if it exists
      server.send(404, "text/plain", "404: Not Found");     // otherwise, respond with a 404 (Not Found) error
  });
  server.on( "/css/dashboard.css", []() {
    if ( !handleFileRead("/css/dashboard.css") )            // send it if it exists
      server.send(404, "text/plain", "404: Not Found");     // otherwise, respond with a 404 (Not Found) error
  });
  server.on( "/css/bootstrap-table.min.css", []() {
    if ( !handleFileRead("/css/bootstrap-table.min.css") )  // send it if it exists
      server.send(404, "text/plain", "404: Not Found");     // otherwise, respond with a 404 (Not Found) error
  });
    
  server.serveStatic( "/img", SPIFFS, "/img" );

  server.serveStatic( "/config.json", SPIFFS, "/config.json" );
  server.serveStatic( "/log.json", SPIFFS, "/log.json" );

  #if ( STERGO_PROGRAM == 1 )                                       //===============================================
    server.serveStatic( "/history.json", SPIFFS, "/history.json" );
    server.on( "/mesures.json", sendMeasures );
  #elif ( STERGO_PROGRAM == 0 )                                     //===============================================
    server.on( "/POWER", checkSwitchState );
  #elif ( STERGO_PROGRAM == 3 )                                     //===============================================
    server.serveStatic( "/history.json", SPIFFS, "/history.json" );
    server.on( "/mesures.json", sendMeasures );
    server.on( "/POWER", checkSwitchState );
  #endif                                                            //===============================================
  
  server.on( "/gpio", updateConfig );
  server.on( "/deviceinfo.json", sendDeviceInfo );
  server.on( "/wifiscan.json", wifiScanJSON );
/*
  server.on("/upload", HTTP_GET, []() {                 // if the client requests the upload page
    if (!handleFileRead("/upload.html"))                // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });
  */
  server.on( "/upload", HTTP_POST, []() {    // if the client posts to the upload page
    //Serial.println("In upload POST");
    server.send(200, "text/plain", ""); 
    }, handleFileUpload                                 // Send status 200 (OK) to tell the client we are ready to receive
  );                                                    // Receive and save the file

  server.begin();
  writeLogFile( F("HTTP server started"), 1 );

  if ( WiFi.getMode() == 1 )
    setupSSDP();
}

/**********************************************************/
void updateConfig()
{
  String what = server.arg("id");
  //int att = server.arg("what").toInt();

  String message;
  
  if ( what == "restart" )
  {
    // should return if successs
    writeLogFile( F("Restarting"), 1 );
    server.send(200, "application/json", "{\"success\":\"Restarting...\"}");
    delay(500);
    ESP.restart();
    // should return if successs
  }
  else if ( what == "updateFirmware" )
  {
    // should return if successs
    writeLogFile( F("updateing Firmware"), 1 );
    firmwareOnlineUpdate();
    // should return if successs
  }
  #if ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 3 )               //===============================================
  else if ( what == "initBME280" )
  {
    // should return if successs
    writeLogFile( F("init BME280"), 1 );
    if ( setupBME280() )
      server.send(200, "application/json", "{\"success\":\"Success init BME280\"}");
    else
      server.send(200, "application/json", "{\"Error\":\"Error initBME280\"}");
    // should return if successs
  }
  else if ( what == "eraseHistory" )
  {
      if ( updateHistory( 1 ) )
        server.send(200, "application/json", "{\"success\":\"Success erased History file\"}");
      else
        server.send(200, "application/json", "{\"Error\":\"Error delete History file\"}"); 
  }
  #endif                                    //===============================================
  else if ( what == "eraseLog" )
  {
    if ( saveLogFile( 1 ) )
      server.send(200, "application/json", "{\"success\":\"Success erased Log file\"}");
    else
      server.send(200, "application/json", "{\"Error\":\"Error delete Log file\"}");  
  }
  else if ( what == "updateWiFi" || what == "updateMQTT" || what == "updateDevice" || what == "initialSetup" ) //Need to Fix HTML, not so many args - just updateConfig
  {
    if ( writeToConfig( &message ) )
      server.send( 200, "application/json", "{\"success\":\"" + message + "\"}" );
    else
      server.send( 200, "application/json", "{\"Error\":\"" + message + "\"}" );
  }
  //Serial.println( F"Config updated") );
}


void sendDeviceInfo()
{
  upTime = now();

  /* Less Memory more Program 
  String json = JST +  "\"result\":[{\"Name" + QCQ + "Firmware\",\"Value" + QCQ + String(FW_VERSION) + "\"" + JSE2;
         json += JST + "\"Name" + QCQ + "Chip ID\",\"Value" + QCQ + String(ESP.getChipId()) + "\"" + JSE2;
         json += JST + "\"Name" + QCQ + "Free Heap\",\"Value" + QCQ + String(ESP.getFreeHeap()) + "\"" + JSE2;
         json += JST + "\"Name" + QCQ + "DeviceName\",\"Value" + QCQ + String(deviceName) + "\"" + JSE2;
         json += JST + "\"Name" + QCQ + "Uptime\",\"Value" + QCQ + showDuration(upTime) + "\"" + JSE2;
         json += JST + "\"Name" + QCQ + "DeviceIP\",\"Value" + QCQ + WiFi.localIP().toString() + "\"" + JSE2;
         json += JST + "\"Name" + QCQ + "MAC address\",\"Value" + QCQ + String(WiFi.macAddress()) + "\"" + JSE2;
         json += JST + "\"Name" + QCQ + "FreeSPIFFS\",\"Value" + QCQ + String(GetMeFSinfo()) + "\"}]" + JSE;
  */ 
  
  char data[360];
  sprintf( data, "{\"result\":[{\"Name\":\"Firmware\",\"Value\":\"%s\"},{\"Name\":\"Chip ID\",\"Value\":\"%u\"},{\"Name\":\"Free Heap\",\"Value\":\"%u\"},{\"Name\":\"DeviceName\",\"Value\":\"%s\"},{\"Name\":\"Uptime\",\"Value\":\"%s\"},{\"Name\":\"DeviceIP\",\"Value\":\"%s\"},{\"Name\":\"MAC address\",\"Value\":\"%s\"},{\"Name\":\"FreeSPIFFS\",\"Value\":\"%ld\"}]}",
          FIRMWARE.c_str(), ESP.getChipId(), ESP.getFreeHeap(), deviceName, showDuration(upTime).c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(),GetMeFSinfo().toInt() );

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", data );
}

void handleIndex()
{
  if ( !handleFileRead("/index.html") )                   // send it if it exists
  server.send(404, "text/plain", "404: Not Found");     // otherwise, respond with a 404 (Not Found) error
}
/** Handle root or redirect to captive portal */
void handleRoot()
{
  if ( captivePortal() ) // If captive portal redirect instead of displaying the page.
    return;
  
  if ( !handleFileRead("/captive_1.html") )                // send it if it exists
      server.send(404, "text/plain", "404: Not Found");    // otherwise, respond with a 404 (Not Found) error
}

/** Redirect to captive portal if we got a request for another domain.
 ** Return true in that case so the page handler do not try to handle the request again. */
bool captivePortal()
{  
  IPAddress ip;
  if ( !ip.fromString(server.hostHeader()) && server.hostHeader() != (String(wifi_hostname)+".local") )
  {
    //Serial.println( F("I'm in captivePortal() - true") );
    //Serial.print("Request redirected to captive portal");
    //Serial.println( "HostHeader: " + server.hostHeader() );
      
    server.sendHeader("Location", String("http://") + server.client().localIP().toString(), true);
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }

  //Serial.println("I'm in captivePorta() - false");
  return false;
  
}

void handleNotFound()
{
  if ( captivePortal() )
  { // If captive portal redirect instead of displaying the error page.
    return;
  }
  
  server.send ( 404, "text/plain", "Not Found" );
}
