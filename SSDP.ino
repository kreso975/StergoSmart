/* ======================================================================
Function: setupSSDP
Purpose : Initialize SSDP (Simple Service Discovery Protocol) Service
Input   :
Output  :
Comments: - */
void setupSSDP()
{
	SSDP.setSchemaURL("description.xml");
	SSDP.setHTTPPort(80);
	SSDP.setName(String(deviceName));
	SSDP.setDeviceType("urn:schemas-upnp-org:device:StergoSmart:1"); // In case: put after SSDP.begin
	SSDP.setSerialNumber(SERIAL_NUMBER);									  // This must be adjusted to chipID
	SSDP.setURL("index.html");
	SSDP.setModelName(MODEL_NAME);
	SSDP.setModelNumber(MODEL_NUMBER);							 // This must be SET in main config
	SSDP.setModelURL(String(COMPANY_URL) + "/" + PRODUCT); // Product is Model_name + Model_number
	SSDP.setManufacturer("Stergo");
	SSDP.setManufacturerURL(COMPANY_URL);
	SSDP.begin();
	writeLogFile(F("SSDP Started"), 1, 3);

	// Register the TicTacToe handler for the keyword "TicTac"
	// These needs to be moved 
	subscribeUDP("TicTac", playTicTacToe);
	subscribeUDP("Arduino", arduinoHandler);
	subscribeUDP("Stergo", StergoHandler);
}

/* ======================================================================
Function: updateSSDP
Purpose : Main updateSSDP Constructor ( called from Loop )
Input   :
Output  :
Comments:
TODO    : */
void updateSSDP()
{
	// Search for Devices in LAN
	if ((millis() - previousSSDP > intervalSSDP) || measureSSDPFirstRun)
	{
		previousSSDP = millis();
		measureSSDPFirstRun = false;
		discoverySSDP(); // Init M-SEARCH over UDP, Searching for devices compatible to me :)
	}

	// SSDP Discovery, Listen for TicTacToe
	receiveUDP(); // WE DON'T NEED SSDP in WiFi AP mode
}

/* ==========================================================================================
Function: arduinoHandler
Purpose : Manage a list of discovered devices using UDP received message.
           - Add new devices to the list.
           - Update timestamps for existing devices.
           - Remove devices from the list if they are older than 24 hours.
Input   : const char *message - The UDP message containing device message.
Output  : Updates the global `foundSSDPdevices[]` array and `actualSSDPdevices` count.
Comments: Uses `millis()` to track timestamps of devices.
TODO    : 1. Use a real-time clock (RTC) for precise time handling.
          2. Improve logging for better debugging. */
void arduinoHandler(const char *message)
{
	// Check if the message contains "TICTACTOE_ProxyHUB"
	if (strstr(message, "TICTACTOE_ProxyHUB") != NULL)
	{
		#if (DEBUG == 1) 	// -------------------------------------------
		writeLogFile(F("Found TICTACTOE_ProxyHUB in Arduino"), 1, 1);
		#endif 				// -------------------------------------------
		HUBproxy = 1;
		HUBproxy_ip = ntpUDP.remoteIP();
		//HUBproxy_port = ntpUDP.remotePort();
		return;
	}
		
	// Get the device's IP address
	IPAddress ssdpDeviceIP = ntpUDP.remoteIP();
	unsigned long currentTime = millis(); // Current time in milliseconds

	// Loop through the list to check for existing records or an empty slot
	for (int x = 0; x < NUMBER_OF_FOUND_SSDP; x++)
	{
		// If the device already exists, update its timestamp
		if (foundSSDPdevices[x].ip == ssdpDeviceIP)
		{
			#if (DEBUG == 1) 					// -------------------------------------------
			writeLogFile(F("Found Device udating Timestamp: "), 1, 1);
			#endif 								// -------------------------------------------
			foundSSDPdevices[x].timestamp = currentTime; // Update timestamp
			return;													// Exit, as the device is already in the list
		}

		// If the slot is empty, add the new device
		if (foundSSDPdevices[x].ip == IPAddress(0, 0, 0, 0))
		{
			actualSSDPdevices = x + 1;
			foundSSDPdevices[x].ip = ssdpDeviceIP;			// Add device
			foundSSDPdevices[x].timestamp = currentTime; // Add timestamp

			#if (DEBUG == 1) 					// -------------------------------------------
			writeLogFile(F("Found Device: ") + foundSSDPdevices[x].ip.toString(), 1, 1);
			writeLogFile(F("Actual Devices: ") + String(actualSSDPdevices), 1, 1);
			#endif 								// -------------------------------------------

			return;
		}
	}

	// Remove records older than 24 hours (86400000 ms)
	for (int x = 0; x < NUMBER_OF_FOUND_SSDP; x++)
	{
		if (foundSSDPdevices[x].ip != IPAddress(0, 0, 0, 0) && (currentTime - foundSSDPdevices[x].timestamp) > 86400000)
		{
			// Clear the old record
			foundSSDPdevices[x].ip = IPAddress(0, 0, 0, 0);
			foundSSDPdevices[x].timestamp = 0;
			actualSSDPdevices--;
			#if (DEBUG == 1) 					// -------------------------------------------
			writeLogFile(F("Found Device for removal: ") + foundSSDPdevices[x].ip.toString(), 1, 3);
			#endif 								// -------------------------------------------
		}
	}
}

void StergoHandler(const char *message)
{
	char modelName[64];
	char message2[128];

	if (strstr(message, "TICTACTOE_ProxyHUB") != NULL)
	{
		#if (DEBUG == 1) 	// -------------------------------------------
		writeLogFile(F("Found TICTACTOE_ProxyHUB in Stergo"), 1, 1);
		#endif 				// -------------------------------------------
		HUBproxy = 1;
		HUBproxy_ip = ntpUDP.remoteIP();
		//HUBproxy_port = ntpUDP.remotePort();
	}

	strncpy(modelName, parseAndExtract(message, "", " ", 2), sizeof(modelName) - 1);
	modelName[sizeof(modelName) - 1] = '\0'; // Ensure null-termination

	snprintf(message2, sizeof(message2), "SERVER: Stergo Hi there! %s, I'm %s", modelName, _devicename);
	sendUDP(message2, ntpUDP.remoteIP(), ntpUDP.remotePort());
}

struct UDPSubscription
{
	char keyword[16]; // Adjust the size as needed
	void (*callback)(const char *message);
};

const int maxSubscriptions = 10;
UDPSubscription subscriptions[maxSubscriptions];
int subscriptionCount = 0;

/* ======================================================================
Function: subscribeUDP
Purpose : Register a keyword and its callback function for UDP message handling
Input   : const char* keyword - The keyword to match in incoming UDP messages
          void (*callback)(const char* message) - The callback function to invoke when the keyword is matched
Output  : None
TODO    : Add error handling for invalid inputs or potential buffer overflows */
void subscribeUDP(const char *keyword, void (*callback)(const char *message))
{
	if (subscriptionCount < maxSubscriptions)
	{
		strncpy(subscriptions[subscriptionCount].keyword, keyword, sizeof(subscriptions[subscriptionCount].keyword) - 1);
		subscriptions[subscriptionCount].keyword[sizeof(subscriptions[subscriptionCount].keyword) - 1] = 0; // Ensure null-termination
		subscriptions[subscriptionCount].callback = callback;
		subscriptionCount++;
	}
}

/* ======================================================================
Function: parseAndExtract
Purpose : Extract a specific value or parse a part of a UDP message using a specified key or delimiter.
Input   : const char* input    - The input UDP message to be processed.
          const char* key      - The key to search for in the input message (use an empty string for delimiter-based parsing).
          const char* delimiter - The delimiter to use for splitting the input (ignored if key-based extraction is used).
          int part             - The index of the part to retrieve (used only for delimiter-based parsing; ignored otherwise).
Output  : char* - The extracted value or parsed part of the message as a C-style string.
Usage   : Use this function to extract values like "WePlay", "Player", and "Move" from a UDP message. Examples:
            receivedMove = atoi(parseAndExtract(input, "Move=", " "));
            wantToPlay = atoi(parseAndExtract(input, "WePlay=", " "));
            selectPlayer = atoi(parseAndExtract(input, "Player=", " ")); */
char *parseAndExtract(const char *input, const char *key, const char *delimiter, int part)
{
	static char output[64];				  // Allocate a static buffer for output (adjust size as needed)
	memset(output, 0, sizeof(output)); // Clear the output buffer

	// Key-based extraction (similar to extractValue)
	if (key && strlen(key) > 0)
	{
		const char *startPtr = strstr(input, key); // Find the key
		if (!startPtr)
			return output;									  // Key not found, return empty string
		startPtr += strlen(key);						  // Move to the value after the key
		const char *endPtr = strchr(startPtr, ' '); // Find the next space or end of string
		if (!endPtr)
			endPtr = input + strlen(input);				 // If no space, set endPtr to end of input
		strncpy(output, startPtr, endPtr - startPtr); // Copy the value into output
		output[endPtr - startPtr] = '\0';				 // Ensure null-termination
		return output;
	}

	// Delimiter-based parsing (similar to parseUDP)
	char buf[strlen(input) + 1];
	strncpy(buf, input, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0'; // Ensure null-termination

	char *token = strtok(buf, delimiter);
	char *values[5];
	int i = 0;

	while (token != NULL && i < 5)
	{
		values[i++] = token;
		token = strtok(NULL, delimiter);
	}

	if (part >= 0 && part < 5 && values[part])
	{
		strncpy(output, values[part], sizeof(output) - 1);
		output[sizeof(output) - 1] = '\0'; // Ensure null-termination
	}
	return output; // Return the parsed part or an empty string
}

/* ======================================================================
Function: discoverySSDP
Purpose : UDP packets send
Input   : 
Output  : M-SEARCH &
Comments: - */
void discoverySSDP()
{
	char payloadUDP[] = "M-SEARCH * HTTP/1.1\r\nHost:239.255.255.250:1900\r\nMan:\"ssdp:discover\"\r\nST:ssdp:all\r\nMX:1\r\n\r\n";
   sendUDP( payloadUDP, IPAddress(SSDPADRESS), SSDPPORT );
}

/* ======================================================================
Function: receiveUDP
Purpose : UDP packets receive
Input   : None
Output  : None
TODO    : Add error handling and optimize buffer size if needed */
void receiveUDP()
{
	char packetBuffer[512];
	int packetSize = ntpUDP.parsePacket();

	if (packetSize)
	{
		int len = ntpUDP.read(packetBuffer, sizeof(packetBuffer) - 1);
		if (len > 0)
			packetBuffer[len] = 0;

		#if (DEBUG == 1)
		writeLogFile(String("Received UDP packet: ") + packetBuffer, 1, 1);
		#endif

		for (int i = 0; i < subscriptionCount; i++)
		{
			// Check if the keyword is found within the message
			if (strstr(packetBuffer, subscriptions[i].keyword) != NULL)
			{ 
				subscriptions[i].callback(packetBuffer);
				break;
			}
		}
	}
}

/* ==========================================================================================
Function: sendUDP
Purpose : send UDP
Input   : payloadUDP - Prepared message to be sent
Output  : no output.
Comments:
TODO    : */
void sendUDP( const char* payloadUDP, IPAddress udpDeviceIP, int udpPort )
{
	// Log the details of the payload, destination IP, and port
	#if (DEBUG == 1)
	char logMessage[200];
	snprintf(logMessage, sizeof(logMessage), "Sending UDP packet to %s:%d with payload: %s", udpDeviceIP.toString().c_str(), udpPort, payloadUDP);
	writeLogFile(String(logMessage), 1, 1);
	#endif

	// Begin sending the packet
	ntpUDP.beginPacket(udpDeviceIP, udpPort);
	ntpUDP.write((uint8_t*)payloadUDP, strlen(payloadUDP));
	ntpUDP.endPacket();
	
	// Log the result of the packet sending
	#if (DEBUG == 1)
	snprintf(logMessage, sizeof(logMessage), "Packet sent to %s:%d", udpDeviceIP.toString().c_str(), udpPort);
	writeLogFile(String(logMessage), 1, 1);
	#endif

	return;
 }
 
