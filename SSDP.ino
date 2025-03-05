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
		requestSSDP(1); // Init M-SEARCH over UDP, Searching for devices compatible to me :)
	}

	// SSDP Discovery, Listen for TicTacToe
	receiveUDP(); // WE DON'T NEED SSDP in WiFi AP mode
}


/* ==========================================================================================
Function: arduinoHandler
Purpose : go through already collected IPs and update the list
Input   : UDP message
Output  : Update foundSSDPdevices[]
Comments: 
TODO    : */
void arduinoHandler(const char *message)
{
	IPAddress ssdpDeviceIP = ntpUDP.remoteIP();
	for (int x = 0; x < NUMBER_OF_FOUND_SSDP; x++)
	{
		if (foundSSDPdevices[x] == ssdpDeviceIP)
			return; // Device SSDP already in the list

		if (foundSSDPdevices[x] == IPAddress(0, 0, 0, 0))
		{
			actualSSDPdevices = x + 1;
			foundSSDPdevices[x] = ssdpDeviceIP; // Add device to the list

			#if (DEBUG == 1) // -------------------------------------------
			writeLogFile(F("Found Device: ") + foundSSDPdevices[x].toString(), 1, 1);
			writeLogFile(F("Actual Devices: ") + String(actualSSDPdevices), 1, 1);
			#endif // -------------------------------------------
			return;
		}
	}
}

void StergoHandler(const char *message)
{
	char modelName[64];
	char message2[128];
	strncpy(modelName, parseUDP(message, 2, " "), sizeof(modelName) - 1);
	modelName[sizeof(modelName) - 1] = 0; // Ensure null-termination

	snprintf(message2, sizeof(message2), "Hi there! %s, I'm %s", modelName, _devicename);

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
Function: setupSSDP
Purpose : Initialize SSDP (Simple Service Discovery Protocol) Service
Input   :
Output  :
Comments: -
====================================================================== */
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
	subscribeUDP("TicTac", ticTacToeUDPHandler);
	subscribeUDP("Arduino", arduinoHandler);
	subscribeUDP("Stergo", StergoHandler);

}

/* ======================================================================
Function: parseUDP
Purpose : Parse a specific part of a UDP message using a specified delimiter
Input   : const char* input - The input UDP message to be parsed
          int part - The part index to retrieve from the parsed message
          const char* delimiter - The delimiter to use for parsing
Output  : char* - The parsed part of the message as a C-style string */
char *parseUDP(const char *input, int part, const char *delimiter)
{
	static char output[64];				  // Allocate a static buffer for the output (adjust size as needed)
	memset(output, 0, sizeof(output)); // Clear the output buffer

	char buf[strlen(input) + 1];
	strncpy(buf, input, sizeof(buf));
	buf[sizeof(buf) - 1] = 0; // Ensure null-termination

	char *values[5];
	byte i = 0;
	char *token = strtok(buf, delimiter); // Use the specified delimiter

	while (token != NULL && i < 5)
	{
		values[i++] = token;
		token = strtok(NULL, delimiter);
	}

	if (part >= 0 && part < 5 && values[part] != NULL)
	{
		strncpy(output, values[part], sizeof(output) - 1);
		output[sizeof(output) - 1] = 0; // Ensure null-termination
	}
	else
	{
		output[0] = 0; // Return an empty string if part is out of bounds
	}

	return output;
}

/* ======================================================================
Function: requestSSDP
Purpose : UDP packets send
Input   : what = 1 (M-SEARCH devices upnp)
Output  : M-SEARCH &
Comments: -
====================================================================== */
void requestSSDP(int what)
{
	if (what == 1)
	{
		char ReplyBuffer[] = "M-SEARCH * HTTP/1.1\r\nHost:239.255.255.250:1900\r\nMan:\"ssdp:discover\"\r\nST:ssdp:all\r\nMX:1\r\n\r\n";

		ntpUDP.beginPacket(IPAddress(SSDPADRESS), SSDPPORT);
		//ntpUDP.write(ReplyBuffer);
   	ntpUDP.write((uint8_t*)ReplyBuffer, strlen(ReplyBuffer));
		ntpUDP.endPacket();
	}
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
TODO    :
============================================================================================= */
void sendUDP(const char* payloadUDP, IPAddress ssdpDeviceIP, int udpPort)
{
	ntpUDP.beginPacket(ssdpDeviceIP, udpPort);
	ntpUDP.write((uint8_t*)payloadUDP, strlen(payloadUDP));
	ntpUDP.endPacket();
	return;
}
