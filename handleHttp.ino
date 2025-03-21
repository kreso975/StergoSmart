#include "Config.h"	// Need it in VS Code :)

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
	server.on("/measures.json", []() { sendMeasures(); });
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
	server.on("/wifiscan.json", []() { wifiManager.startWiFiScan([](String result) { sendJSONheaderReply(3, result); }); });

	server.on("/upload", HTTP_POST, []() { // if the client posts to the upload page
			server.send(200, "text/plain", "");
		},
		handleFileUpload // Send status 200 (OK) to tell the client we are ready to receive
	);						  // Receive and save the file

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
		handleRestart();
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
	snprintf(data, sizeof(data), "{\"result\":[{\"Name\":\"Firmware\",\"Value\":\"%s\"},{\"Name\":\"Chip ID\",\"Value\":\"%s\"},{\"Name\":\"CPU Frequency(MHz)\",\"Value\":\"%u\"},{\"Name\":\"Free Heap\",\"Value\":\"%u\"},{\"Name\":\"DeviceName\",\"Value\":\"%s\"},{\"Name\":\"Uptime\",\"Value\":\"%s\"},{\"Name\":\"DeviceIP\",\"Value\":\"%s\"},{\"Name\":\"MAC address\",\"Value\":\"%s\"},{\"Name\":\"FreeSPIFFS\",\"Value\":\"%ld\"}]}",
							 FIRMWARE, chipID, ESP.getCpuFreqMHz(), ESP.getFreeHeap(), deviceName, showDuration(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), GetMeFSinfo().toInt());

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
void sendWebhook(const char* localURL, const char* data)
{
    HTTPClient http;
    http.begin(espClient, localURL);
    http.addHeader("Content-Type", "application/json"); // Set request as JSON

    // Send POST request
    int httpResponseCode = http.POST(data);

    #if (DEBUG == 1)
    	char logMessage[50];
		snprintf(logMessage, sizeof(logMessage), "HTTP Response code: %d", httpResponseCode);
		writeLogFile(logMessage, 1, 1);
    #endif

    http.end();
}


/* ============================================================================
Function: sendJSONheaderReply
Purpose : Sends a JSON response to the client with the appropriate header and
          message format based on the specified type.
Input   : byte type - Specifies the type of response:
                      0 = Error, 1 = Success, 2 = Info, 3 = Raw JSON message.
          String message - The message content to include in the JSON response.
Output  : None
Comments: - Formats the JSON response according to the type, using the keys 
            "Error", "success", or "Info" for types 0, 1, and 2, respectively.
          - Sends the JSON response with the "Access-Control-Allow-Origin" header 
            for cross-origin resource sharing (CORS).
          - Type 3 sends the message as a raw JSON object without additional formatting.
          - Utilizes the `F()` macro to store string literals in flash memory, 
            optimizing memory usage. */
void sendJSONheaderReply(byte type, String message)
{
	String output;
	switch ( type )
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
String firmwareOnlineUpdate(byte what)
{
	String message;
   String prefix = (what == 1) ? "LittleFS" : "Firmware";
   t_httpUpdate_return ret;

	#if defined(ESP8266)
	ESPhttpUpdate.rebootOnUpdate(false);
	ESPhttpUpdate.closeConnectionsOnUpdate(false);
	#elif defined(ESP32)
	httpUpdate.rebootOnUpdate(false);
	#endif

	if ( what == 1 )
	{
		#if defined(ESP8266)
		ret = ESPhttpUpdate.updateFS(espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", SERIAL_NUMBER);
		#elif defined(ESP32)
		// Use the Update class for ESP32
		WiFiClient client;
		if (client.connect("192.168.1.101", 80))
		{
			client.print(String("GET ") + "/StergoWeather/firmwareCheck.php" + " HTTP/1.1\r\n" +
							 "Host: " + "192.168.1.101" + "\r\n" +
							 "Connection: close\r\n\r\n");
			while (client.connected())
			{
				String line = client.readStringUntil('\n');
				if (line == "\r")
				{
					break;
				}
			}
			if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS))
			{
				Update.printError(Serial);
			}
			while (client.available())
			{
				uint8_t buff[128];
				int len = client.read(buff, sizeof(buff));
				if (len <= 0)
				{
					break;
				}
				Update.write(buff, len);
			}
			if (!Update.end(true))
			{
				Update.printError(Serial);
			}
		}
		ret = HTTP_UPDATE_OK;
		#endif
	}
	else if (what == 2)
	{
		#if defined(ESP8266)
		ret = ESPhttpUpdate.update(espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", FIRMWARE);
		#elif defined(ESP32)
		ret = httpUpdate.update(espClient, "http://192.168.1.101/StergoWeather/firmwareCheck.php", FIRMWARE);
		#endif
	}

	switch (ret)
	{
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
