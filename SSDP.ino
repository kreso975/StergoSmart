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
}

SSDPcomType getSSDPcomType(String input)
{
	// Maybe Shorter
	int i = input.indexOf("Arduino"); // From M-SEARCH
	int a = input.indexOf("Stergo");	 // From D2D (device2device) communication
	int b = input.indexOf("TicTac");	 // From D2D (device2device) communication

	if (i > 0)
		return Arduino;
	if (a > 0)
		return Stergo;
	if (b > 0)
		return TicTac;
	return NotDeclared;
}

/* ======================================================================
String  : parseUDP()
Purpose : to Optimize code. Easyer to read playTicTacToe
Input   : input - received raw ASCII from UDP | SERVER: TicTac deviceNAME+chipID Move=8
			 part  - requested element of array to be returned
Output  : String of input[part]
TODO    :
====================================================================== */
String parseUDP(String input, int part)
{
	char buf[input.length() + 1];
	input.toCharArray(buf, sizeof(buf));
	char *values[5];
	byte i = 0;
	char *token = strtok(buf, " "); // Delimiter is " ", we get 5 Strings

	while (token != NULL && i < 5)
	{ // Added boundary check for i
		values[i++] = token;
		token = strtok(NULL, " ");
	}

	if (part >= 0 && part < 5)
	{ // Added boundary check for part
		return String(values[part]);
	}
	else
	{
		return String(""); // Return an empty string if part is out of bounds
	}
}

/* ======================================================================
Function: requestSSDP
Purpose : UDP packets send
Input   : what = 1 (M-SEARCH devisec upnp)
			 what = 2 (Send string for communication with Stergo device)
Output  : M-SEARCH &
Comments: -
====================================================================== */
void requestSSDP(int what)
{
	if (what == 1)
	{
		char ReplyBuffer[] = "M-SEARCH * HTTP/1.1\r\nHost:239.255.255.250:1900\r\nMan:\"ssdp:discover\"\r\nST:ssdp:all\r\nMX:1\r\n\r\n";

		ntpUDP.beginPacket(IPAddress(SSDPADRESS), SSDPPORT);
		ntpUDP.write(ReplyBuffer);
		ntpUDP.endPacket();
	}
}

/* ======================================================================
Function: receiveUDP
Purpose : UDP packets receive
Input   :
Output  :
TODO    : Add save to file , check for duplicate entry / JSON format
			 fields: Device name(name from settings), modelName, modelNumber, IP
			 Load existing list and check if servers responds - something simple */
void receiveUDP()
{
	String input, modelNumber, modelName, message;

	char packetBuffer[512];

	int packetSize = ntpUDP.parsePacket();
	// If there is something inside
	if (packetSize)
	{
		int len = ntpUDP.read(packetBuffer, 512);
		if (len > 0)
			packetBuffer[len] = 0;

		input = packetBuffer;

		switch (getSSDPcomType(input))
		{
		case Arduino:
			isSSDPfoundBefore(ntpUDP.remoteIP());
#if (DEBUG == 1) // -------------------------------------------
			writeLogFile(F("Actual Devices: ") + String(actualSSDPdevices), 1, 1);
#endif // -------------------------------------------
			break;

		case Stergo:
			modelName = parseUDP(input, 2);
			message = F("Hi there! ");
			message += modelName;
			message += F(", I'm ");
			message += String(_devicename);
			sendUDP(message, ntpUDP.remoteIP(), ntpUDP.remotePort());
			break;

		case TicTac:
#ifdef MODULE_TICTACTOE // -------------------------------------------
			playTicTacToe(input);
#endif // -------------------------------------------
			break;

		case NotDeclared:
			break;
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
void sendUDP(String payloadUDP, IPAddress ssdpDeviceIP, int udpPort)
{
	ntpUDP.beginPacket(ssdpDeviceIP, udpPort);
	ntpUDP.write(payloadUDP.c_str());
	ntpUDP.endPacket();
	return;
}

/* ==========================================================================================
Function: isSSDPfoundBefore
Purpose : go through already collected IPs and update the list
Input   : ssdpDeviceIP
Output  : no output.
Comments:
TODO    :
============================================================================================= */
void isSSDPfoundBefore(IPAddress ssdpDeviceIP)
{
	for (int x = 0; x < NUMBER_OF_FOUND_SSDP; x++)
	{
		if (foundSSDPdevices[x] != IPAddress(0, 0, 0, 0))
		{
			if (foundSSDPdevices[x] == ssdpDeviceIP)
				return; // Device SSDP Already in my list
		}
		else
		{
			actualSSDPdevices = x + 1;
			foundSSDPdevices[x] = ssdpDeviceIP; // ADD Device to my list

#if (DEBUG == 1) // -------------------------------------------
			writeLogFile(F("Found Device: ") + foundSSDPdevices[x].toString(), 1, 1);
#endif // -------------------------------------------

			return;
		}
	}
}