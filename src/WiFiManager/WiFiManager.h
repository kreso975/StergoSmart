#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h> // Include the Arduino core header file

/* ======================== WiFiManager ==============================
Class   : WiFiManager
Purpose : Brain of WiFi behaviour
Members :
	 - char* wifi_ssid: SSID of the WiFi network
	 - char* wifi_password: Password of the WiFi network
	 - char* wifi_StaticIP: Static IP address
	 - char* wifi_gateway: Gateway address
	 - char* wifi_subnet: Subnet mask
	 - char* wifi_DNS: DNS server address
	 - char* wifi_hostname: Hostname of the device
	 - bool wifi_static: Flag to indicate if static IP is used
	 - char* softAP_ssid: SSID for the AP mode
	 - char* softAP_pass: Password for the AP mode
Methods :
	 - bool disconnectSTA(): Disconnects from the STA mode
	 - bool startWPS(): Starts WPS configuration
	 - bool startSTA(int STAmode = 0): Starts STA mode with optional static IP configuration
	 - bool startAP(): Starts AP mode
	 - void manageWiFi(): Manages the WiFi connection based on the configuration
	 - void checkAPRestart(): Check if AP needs auto restart after 5 min
	 - void wifiScanJSON(): Scan WiFi networks and return JSON code */
class WiFiManager
{
public:
	typedef void (*ScanCompleteCallback)(String result); // Define a callback type

	WiFiManager(char *ssid, char *password, char *staticIP, char *gateway, char *subnet, char *DNS, char *hostname, byte &useStatic, char *ap_ssid, char *ap_pass, String chipID);
	bool disconnectSTA();
	bool startWPS();
	bool startSTA(int STAmode = 0);
	bool startAP();
	void manageWiFi();
	void checkAPRestart();
	void startWiFiScan(ScanCompleteCallback callback); // Start the asynchronous scan with a callback
	String getScanResult();


private:
	char *wifi_ssid;
	char *wifi_password;
	char *wifi_StaticIP;
	char *wifi_gateway;
	char *wifi_subnet;
	char *wifi_DNS;
	char *wifi_hostname;
	byte &wifi_static;
	char *softAP_ssid;
	char *softAP_pass;
	String chipID;
	unsigned long ap_previousMillis;
	const unsigned long ap_intervalHist;

	String scanResult; // Store the scan result
	ScanCompleteCallback scanCompleteCallback; // Store the callback function
	static void scanCompleteHandler(int networksFound, WiFiManager* instance); // Add a static callback function
	void wifiScanResult(int networksFound); // Handle the scan results 
};

#endif // WIFIMANAGER_H
