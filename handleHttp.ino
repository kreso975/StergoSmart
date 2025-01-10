/* ======================================================================
Function: setupHttpServer
Purpose : Setup and Initialize HTTP Server
Input   :
Output  :
Comments: -  */
void setupHttpServer()
{
	if (WiFi.getMode() == WIFI_STA)
	{
		server.on("/", handleIndex);
		server.on("/index.html", handleIndex);
    #if defined(ESP8266)
    server.on( "/description.xml", HTTP_GET, [&]() { SSDP.schema(server.client()); });
    #elif defined(ESP32)
    server.on("/description.xml", HTTP_GET, []() { server.send(200, "text/xml", SSDP.getSchema());});
    #endif
	}
	else
	{ // IF SERVER AP
		// Setup the DNS server redirecting all the domains to the apIP
		dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
		dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

		server.on("/", handleRoot);
		server.on("/generate_204", handleRoot); // Android captive portal. <= Android 6.0.1
		server.on("/fwlink", handleRoot);		 // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.

		server.onNotFound(handleNotFound);
	}

	server.serveStatic("/favicon.ico", LittleFS, "/img/favicon.ico");

	server.on("/css/dashboard.css", []() {
		if (!handleFileRead("/css/dashboard.css"))				// send it if it exists
			server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
		});

	server.serveStatic("/img", LittleFS, "/img");

	server.serveStatic("/config.json", LittleFS, "/config.json");
	server.serveStatic("/log.json", LittleFS, "/log.json");

	#ifdef MODULE_WEATHER //===============================================
	server.serveStatic("/history.json", LittleFS, "/history.json");
	server.on("/measures.json", sendMeasures);
	#endif //===============================================

	#ifdef MODULE_SWITCH //===============================================
	server.on("/POWER", checkSwitchState);
	#endif //===============================================

	// Use to fast modifications and Setup
	#ifdef MODULE_DISPLAY //===============================================
	server.on("/display", displayState);
	#endif //===============================================

	server.on("/gpio", updateConfig);
	server.on("/deviceinfo.json", sendDeviceInfo);
	server.on("/wifiscan.json", wifiScanJSON);

	server.on("/upload", HTTP_POST, []() { // if the client posts to the upload page
			server.send(200, "text/plain", "");
		},
				 handleFileUpload // Send status 200 (OK) to tell the client we are ready to receive
	);									// Receive and save the file

	server.begin();
	#if (DEBUG == 1)
	writeLogFile(F("HTTP server started"), 1, 3);
	#endif
}

/* ===================== updateConfig =================
Function: updateConfig
Purpose : Handle various configuration updates and return JSON response
Input   : None
Output  : HTTP JSON response
Comments: Handles different configuration updates such as restarting,
			 updating firmware, updating SPIFFS, initializing weather,
			 erasing history, erasing log, and updating WiFi, MQTT,
			 device, or configuration settings. */
void updateConfig()
{
	String what = server.arg("id");
	String message;

	if ( what == "restart" )
		handleRestart();
	else if ( what == "updateFirmware" )
		handleUpdateFirmware();
	else if ( what == "updateSpiffs" )
		handleUpdateSpiffs();
	else if ( what == "eraseLog" )
		handleEraseLog();
	else if ( what == "updateWiFi" || what == "updateMQTT" || what == "updateDevice" || what == "updateConfig" )
		handleUpdateConfig();
	#ifdef MODULE_WEATHER //===============   MODULE_WEATHER  =============
	else if ( what == "initWeather" )
		handleInitWeather();
	else if ( what == "eraseHistory" )
		handleEraseHistory();
	#endif //===============================================
}

void handleRestart()
{
	#if (DEBUG == 1)
	writeLogFile(F("Restarting"), 1);
	#endif
	sendJSONheaderReply(1, F("Restarting..."));
	delay(500);
	ESP.restart();
}

void handleUpdateFirmware()
{
	#if (DEBUG == 1)
	writeLogFile(F("Updating Firmware"), 1, 3);
	#endif
	String message = firmwareOnlineUpdate(2);
	sendJSONheaderReply(3, message);
	if (message.indexOf("success") != -1)
	{
		delay(500);
		ESP.restart();
	}
}

void handleUpdateSpiffs()
{
	#if (DEBUG == 1)
	writeLogFile(F("Updating LittleFS"), 1, 3);
	#endif
	String message = firmwareOnlineUpdate(1);
	sendJSONheaderReply(3, message);
}

void handleEraseLog()
{
	#if (DEBUG == 1)
	writeLogFile(F("Erasing Log"), 1, 3);
	#endif
	if (saveLogFile(1))
		sendJSONheaderReply(1, F("Success erased Log file"));
	else
		sendJSONheaderReply(0, F("Error delete Log file"));
}

void handleUpdateConfig()
{
	String message;
	#if (DEBUG == 1)
	writeLogFile(F("Updating Config"), 1, 3);
	#endif
	writeToConfig(&message);
	sendJSONheaderReply(3, message);
}

#ifdef MODULE_WEATHER										//===============   MODULE_WEATHER  =============
void handleInitWeather()
{
	#if (DEBUG == 1)
	writeLogFile(F("Init Weather"), 1, 3);
	#endif
	if ( setupWeather() )
		sendJSONheaderReply(1, F("Success init Weather"));
	else
		sendJSONheaderReply(0, F("Error init Weather"));
}

void handleEraseHistory()
{
	#if (DEBUG == 1)
	writeLogFile(F("Erasing History"), 1, 3);
	#endif
	if (updateHistory(1))
		sendJSONheaderReply(1, F("Success erased History file"));
	else
		sendJSONheaderReply(0, F("Error delete History file"));
}
#endif															//===============================================

/* ======================================================================
Function: sendDeviceInfo
Purpose : Return JSON in HTTP server - All Device Data
Input   :
Output  : HTTP JSON
Comments: - */
void sendDeviceInfo()
{
	char data[460];
	int len = snprintf(data, sizeof(data), "{\"result\":[{\"Name\":\"Firmware\",\"Value\":\"%s\"},{\"Name\":\"Chip ID\",\"Value\":\"%u\"},{\"Name\":\"CPU Frequency(MHz)\",\"Value\":\"%u\"},{\"Name\":\"Free Heap\",\"Value\":\"%u\"},{\"Name\":\"DeviceName\",\"Value\":\"%s\"},{\"Name\":\"Uptime\",\"Value\":\"%s\"},{\"Name\":\"DeviceIP\",\"Value\":\"%s\"},{\"Name\":\"MAC address\",\"Value\":\"%s\"},{\"Name\":\"FreeSPIFFS\",\"Value\":\"%ld\"}]}",
							 FIRMWARE, chipID, ESP.getCpuFreqMHz(), ESP.getFreeHeap(), deviceName, showDuration().c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), GetMeFSinfo().toInt());

	// 3 is indicator of JSON already formated reply
	sendJSONheaderReply(3, data);
}

void handleIndex()
{
	if (!handleFileRead("/index.html"))						  // send it if it exists
		server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
}

/** Handle root or redirect to captive portal */
void handleRoot()
{
	if (captivePortal()) // If captive portal redirect instead of displaying the page.
		return;

	if (!handleFileRead("/captive.html"))					  // send it if it exists
		server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
}

/* ======================================================================
Function: captivePortal
Purpose : Redirect to captive portal if we got a request for another domain.
			 Return true in that case so the page handler do not try to handle the request again.
Input   :
Output  :
Comments: - */
bool captivePortal()
{
	IPAddress ip;
	if (!ip.fromString(server.hostHeader()) && server.hostHeader() != (String(wifi_hostname) + ".local"))
	{
		server.sendHeader("Location", String("http://") + server.client().localIP().toString(), true);
		server.send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
		server.client().stop();					// Stop is needed because we sent no content length

		return true;
	}

	return false;
}

void handleNotFound()
{
	if (captivePortal())
		return; // If captive portal redirect instead of displaying the error page.

	server.send(404, "text/plain", "Not Found");
}

/* ======================================================================
Function: sendWebhook
Purpose : Sending data as HTTP POST to selected URL
Input   : localURL = URL where to send | data = JSON payload
Output  :
Comments: - I have local web server for forwarding Discord Webhook */
void sendWebhook(char *localURL, String data)
{
	HTTPClient http;
	http.begin(espClient, localURL);
	http.addHeader("Content-Type", "application/json"); // Set request as JSON

	// Send POST request
	int httpResponseCode = http.POST(data);

#if (DEBUG == 1)
	Serial.print(F("HTTP Response code: "));
	Serial.println(httpResponseCode);
#endif

	http.end();
}

/* ======================================================================
Function: sendJSONheaderReply
Purpose : Display as an response Web Header with JSON reply
Input   : byte type , String message
Output  :
Comments:*/
void sendJSONheaderReply(byte type, String message)
{
	String output;
	switch (type)
	{
	case 0:
		output = F("{\"Error\":\"") + message + F("\"}");
		break;
	case 1:
		output = F("{\"success\":\"") + message + F("\"}");
		break;
	case 2:
		output = F("{\"Info\":\"") + message + F("\"}");
		break;
	case 3:
		output = message;
		break;
	}

	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(200, "application/json", output);
}
