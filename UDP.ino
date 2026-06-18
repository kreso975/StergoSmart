/* ======================================================================
Function: setupMDNS
Purpose : Initialize mDNS (micro Domain Name System) Service
Input   :
Output  :
Comments: - */
void setupMDNS()
{
    // Start mDNS with device hostname
    MDNS.begin(wifiManager.getHostname());

    // Publish StergoSmart service on TCP port 80
    MDNS.addService("stergosmart", "tcp", 80);

    // Publish metadata (replaces SSDP XML fields)
    MDNS.addServiceTxt("stergosmart", "tcp", "Name", deviceName);
    MDNS.addServiceTxt("stergosmart", "tcp", "Serial", SERIAL_NUMBER);
    MDNS.addServiceTxt("stergosmart", "tcp", "Model", MODEL_NAME);
    MDNS.addServiceTxt("stergosmart", "tcp", "Version", MODEL_NUMBER);
    MDNS.addServiceTxt("stergosmart", "tcp", "URL", "index.html");
    MDNS.addServiceTxt("stergosmart", "tcp", "Manufacturer", "StergoSmart");
    MDNS.addServiceTxt("stergosmart", "tcp", "MfgURL", COMPANY_URL);
    MDNS.addServiceTxt("stergosmart", "tcp", "ProductURL", String(COMPANY_URL) + "/" + PRODUCT);

    // Optional: device type (UPnP equivalent)
    MDNS.addServiceTxt("stergosmart", "tcp", "DeviceType", "StergoSmart");

    writeLogFile(F("mDNS StergoSmart service started"), 1, 3);

	// Register the TicTacToe handler for the keyword "TicTac"
	// These needs to be moved 
	subscribeUDP("TicTac", manageTicTacToeGame); // This should be only if Tic Tac Toe is Part of Core
	subscribeUDP("Stergo", StergoHandler);
}

/* ======================================================================
Function: updateUDP
Purpose : Main updateUDP Constructor ( called from Loop )
Input   :
Output  :
Comments:
TODO    : */
void updateUDP()
{
	// Search for Devices in LAN
	if ((millis() - previousDiscovery > intervalDiscovery) || measureDiscoveryFirstRun)
	{
		previousDiscovery = millis();
		measureDiscoveryFirstRun = false;
		
		discoveryMDNS();
	}

	// UDP Discovery, Listen for TicTacToe
	receiveUDP(); // WE DON'T NEED SSDP in WiFi AP mode
}

/* ==========================================================================================
Function: discoveryMDNS
Purpose : Discover StergoSmart devices using mDNS query.
           - Query LAN for devices advertising the "stergosmart" TCP service.
           - Register new devices into the global device list.
           - Update timestamps for already known devices.
           - Remove devices older than 24 hours (same logic as ArduinoHandler).
Input   : None (mDNS query is internal)
Output  : Updates the global `foundSSDPdevices[]` array and `actualSSDPdevices` count.
Comments: Uses `millis()` for timestamping.
          Called periodically from updateUDP() instead of SSDP M-SEARCH.
          Logging follows the same format as ArduinoHandler for consistency.
TODO    : 1. Parse TXT records for model/name if needed.
          2. Improve hostname/IP correlation for multi-interface networks.
*/
void discoveryMDNS()
{
    unsigned long currentTime = millis();

    // Query mDNS for StergoSmart devices
    int n = MDNS.queryService("stergosmart", "tcp");

    #if (DEBUG == 1)
    writeLogFile(F("mDNS Discovery started"), 1, 1);
    writeLogFile(F("mDNS Devices found: ") + String(n), 1, 1);
    #endif

    if (n <= 0) return;

    for (int i = 0; i < n; i++)
    {
        IPAddress foundIP = MDNS.IP(i);
		  String host = MDNS.hostname(i);

        #if (DEBUG == 1)
        writeLogFile(F("mDNS Device IP: ") + foundIP.toString(), 1, 1);
        writeLogFile(F("mDNS Hostname: ") + host, 1, 1);
        #endif

		  // --- CHECK FOR TICTACTOE_ProxyHUB DEVICE ---
        if (host.indexOf("TICTACTOE_ProxyHUB") != -1)
        {
            HUBproxy = 1;
            HUBproxy_ip = foundIP;

            #if (DEBUG == 1)
            writeLogFile(F("mDNS: Found TICTACTOE_ProxyHUB"), 1, 1);
            writeLogFile(F("mDNS: HUBproxy IP: ") + foundIP.toString(), 1, 1);
            #endif
        }

        // --- DEVICE REGISTRATION LOGIC (from ArduinoHandler) ---
        bool updated = false;

        for (int x = 0; x < NUMBER_OF_FOUND_SSDP; x++)
        {
            // Update timestamp if device already exists
            if (foundSSDPdevices[x].ip == foundIP)
            {
                foundSSDPdevices[x].timestamp = currentTime;

                #if (DEBUG == 1)
                writeLogFile(F("mDNS: Updating timestamp for ") + foundIP.toString(), 1, 1);
                #endif

                updated = true;
                break;
            }

            // Insert new device into empty slot
            if (foundSSDPdevices[x].ip == IPAddress(0, 0, 0, 0))
            {
                foundSSDPdevices[x].ip = foundIP;
                foundSSDPdevices[x].timestamp = currentTime;
                actualSSDPdevices = x + 1;

                #if (DEBUG == 1)
                writeLogFile(F("mDNS: New Device Registered: ") + foundIP.toString(), 1, 1);
                writeLogFile(F("Actual Devices: ") + String(actualSSDPdevices), 1, 1);
                #endif

                updated = true;
                break;
            }
        }

        // If no slot was updated, ignore (list full)
        if (!updated)
        {
            #if (DEBUG == 1)
            writeLogFile(F("mDNS: Device ignored (list full): ") + foundIP.toString(), 1, 3);
            #endif
        }
    }

    // --- REMOVE OLD DEVICES (same as ArduinoHandler) ---
    for (int x = 0; x < NUMBER_OF_FOUND_SSDP; x++)
    {
        if (foundSSDPdevices[x].ip != IPAddress(0, 0, 0, 0) &&
            (currentTime - foundSSDPdevices[x].timestamp) > 86400000)
        {
            #if (DEBUG == 1)
            writeLogFile(F("mDNS: Removing stale device: ") + foundSSDPdevices[x].ip.toString(), 1, 3);
            #endif

            foundSSDPdevices[x].ip = IPAddress(0, 0, 0, 0);
            foundSSDPdevices[x].timestamp = 0;
            actualSSDPdevices--;
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
		HUBproxy_ip = udpSocket.remoteIP();
		//HUBproxy_port = udpSocket.remotePort();
	}

	strncpy(modelName, parseAndExtract(message, "", " ", 2), sizeof(modelName) - 1);
	modelName[sizeof(modelName) - 1] = '\0'; // Ensure null-termination

	snprintf(message2, sizeof(message2), "SERVER: Stergo Hi there! %s, I'm %s", modelName, _devicename);
	sendUDP(message2, udpSocket.remoteIP(), udpSocket.remotePort());
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
	if ( key && strlen(key) > 0 )
	{
		const char *startPtr = strstr(input, key);	// Find the key
		if (!startPtr)
			return output;									 	// Key not found, return empty string

		startPtr += strlen(key);						 	// Move to the value after the key
		const char *endPtr = strchr(startPtr, ' ');	// Find the next space or end of string
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

	while ( token != NULL && i < 5 )
	{
		values[i++] = token;
		token = strtok(NULL, delimiter);
	}

	if ( part >= 0 && part < 5 && values[part] )
	{
		strncpy(output, values[part], sizeof(output) - 1);
		output[sizeof(output) - 1] = '\0'; // Ensure null-termination
	}

	return output; // Return the parsed part or an empty string
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
	int packetSize = udpSocket.parsePacket();

	if (packetSize)
	{
		int len = udpSocket.read(packetBuffer, sizeof(packetBuffer) - 1);
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
	udpSocket.beginPacket(udpDeviceIP, udpPort);
	udpSocket.write((uint8_t*)payloadUDP, strlen(payloadUDP));
	udpSocket.endPacket();
	
	// Log the result of the packet sending
	#if (DEBUG == 1)
	snprintf(logMessage, sizeof(logMessage), "Packet sent to %s:%d", udpDeviceIP.toString().c_str(), udpPort);
	writeLogFile(String(logMessage), 1, 1);
	#endif

	return;
 }
