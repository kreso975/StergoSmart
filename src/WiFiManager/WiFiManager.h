#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#define DEBUG_WIFI_MANAGER 1

//htaccess - Not used anywhere in the code - maybe one day implement credentials for HTTP
//char htaccess_username[20];
//char htaccess_password[20];

/* ======================== WiFiManager ==============================
Class   : WiFiManager
Purpose : Brain of WiFi behaviour
Methods :
     - bool disconnectSTA(): Disconnects from the STA mode
     - bool startWPS(): Starts WPS configuration
     - bool startSTA(int STAmode = 0): Starts STA mode with optional static IP configuration
     - bool startAP(): Starts AP mode
     - void manageWiFi(): Manages the WiFi connection based on the configuration
     - void checkAPRestart(): Check if AP needs auto restart after 5 min
     - void startWiFiScan(ScanCompleteCallback callback): Start the asynchronous scan with a callback */
class WiFiManager
{
public:
   typedef void (*ScanCompleteCallback)(String result); // Define a callback type

   // Public set methods to configure WiFi settings
   void setWifiHostname(const char* wifi_hostname);
   void setSoftAPSSID(const char* softAP_ssid);
   void setSoftAPPassword(const char* softAP_pass);
   void setWifiSSID(const char* wifi_ssid);
   void setWifiPassword(const char* wifi_password);
   void setWifiStaticIP(const char* wifi_StaticIP);
   void setWifiGateway(const char* wifi_gateway);
   void setWifiSubnet(const char* wifi_subnet);
   void setWifiDNS(const char* wifi_DNS);
   void setWifiRunAS(byte wifi_runAS);
   void setWifiStatic(byte wifi_static);
   const char* getHostname() const; // Method to access the hostname

   WiFiManager();
   void manageWiFi();
   void checkAPRestart();
   void startWiFiScan(ScanCompleteCallback callback);
   String getScanResult();

private:
	bool disconnectSTA();
   bool startWPS();
   bool startSTA(int STAmode = 0);
   bool startAP();

	char wifi_ssid[20];
   char wifi_password[20];
   char wifi_StaticIP[16];
   char wifi_gateway[16];
   char wifi_subnet[16];
   char wifi_DNS[16];
   char wifi_hostname[20];  // Still accessible as extern elsewhere
   byte wifi_static, wifi_runAS;
   char softAP_ssid[20];
   char softAP_pass[20];

   unsigned long ap_previousMillis;
   const unsigned long ap_intervalHist;

   String scanResult; // Store the scan result
   ScanCompleteCallback scanCompleteCallback; // Store the callback function
   static void scanCompleteHandler(int networksFound, WiFiManager* instance); // Add a static callback function
   void wifiScanResult(int networksFound); // Handle the scan results
};

#endif // WIFIMANAGER_H
