#include "../../settings.h"

#include "WiFiManager.h"



char wifi_ssid[20] = "";
char wifi_password[20] = "";
char wifi_StaticIP[16] = "";
char wifi_gateway[16] = "";
char wifi_subnet[16] = "";
char wifi_DNS[16] = "";
char wifi_hostname[20] = "StergoSmart";
byte wifi_static = 0;
byte wifi_runAS = 1;

/******************************************************************************************************
 *  CAPTIVE PORTAL - SETUP
 *
 *  TODO: 
 *  
 *
 * hostname (wifi_hostname) for mDNS. Should work at least on windows. Try http://stergoweather.local *
 * Set these to your desired softAP credentials. They are not configurable at runtime                 *
 ******************************************************************************************************/
char softAP_ssid[20] = "StergoSmart_ap";
char softAP_pass[20] = "123456789";


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
     - void startWiFiScan(ScanCompleteCallback callback): Start the asynchronous scan with a callback */
     
WiFiManager::WiFiManager()
    : ap_previousMillis(0),
    ap_intervalHist(300000),
    scanCompleteCallback(nullptr)
{
    // Constructor body (if needed)
}
     
bool WiFiManager::disconnectSTA()
{
    WiFi.disconnect(true);
    return true;
}

bool WiFiManager::startWPS()
{
    #if defined(ESP32)
    WiFi.beginSmartConfig();
    while (!WiFi.smartConfigDone())
        delay(500);
    #elif defined(ESP8266)
    if (WiFi.beginWPSConfig())
        return true;
    else
        return false;
    #endif
}

bool WiFiManager::startSTA(int STAmode)
{
    WiFi.mode(WIFI_STA);

    if (STAmode == 0)
    {
        WiFi.begin(wifi_ssid, wifi_password); // Accessing globals directly
    }
    else if (STAmode == 1)
    {
        IPAddress _ip, _gw, _sn, _dns;
        _ip.fromString(wifi_StaticIP);
        _gw.fromString(wifi_gateway);
        _sn.fromString(wifi_subnet);
        _dns.fromString(wifi_DNS);

        WiFi.config(_ip, _dns, _gw, _sn);
        WiFi.begin(wifi_ssid, wifi_password);
    }

    int cnt = 1;
    while (WiFi.status() != WL_CONNECTED)
    {
        if (cnt > 60)
            return false;

        delay(500);
        cnt++;
    }
    return true;
}


/**
 * @brief Starts the Access Point (AP) mode on the ESP8266/ESP32 device.
 * 
 * This function sets the WiFi mode to Access Point (WIFI_AP), constructs the
 * Access Point SSID by concatenating the predefined SSID (`softAP_ssid`) with
 * the unique chip ID (`chipID`), and starts the Access Point with the 
 * constructed SSID and password (`softAP_pass`). A delay of 500 milliseconds 
 * is introduced to ensure the AP mode is initiated properly. The current 
 * time is recorded to manage AP restart intervals.
 * 
 * @return true if the AP mode is successfully started.
 */
bool WiFiManager::startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(softAP_ssid, softAP_pass);
    delay(500);
    ap_previousMillis = millis();
    return true;
}

void WiFiManager::manageWiFi()
{
    #if defined(ESP32)
    WiFi.setHostname(wifi_hostname);
    #elif defined(ESP8266)
    wifi_station_set_hostname(wifi_hostname);
    WiFi.hostname(wifi_hostname);
    #endif

    if (strcmp(wifi_ssid, "") && strcmp(wifi_password, ""))
    {
        if (wifi_static == 0)
        {
            if (!startSTA())
            {
                disconnectSTA();
                startAP();
            }
        }
        else
        {
            if (strcmp(wifi_StaticIP, ""))
            {
                if (!startSTA(1))
                {
                    if (!startSTA())
                    {
                        disconnectSTA();
                        startAP();
                    }
                }
            }
        }
    }
    else
    {
        startAP();
    }
}

void WiFiManager::checkAPRestart()
{
    if (WiFi.getMode() == WIFI_AP)
    {
        if ((millis() - ap_previousMillis > ap_intervalHist) && strlen(wifi_ssid) > 0 && strlen(wifi_password) > 0)
        {
            delay(500);
            ESP.restart();
        }
    }
}

void WiFiManager::wifiScanResult(int networksFound)
{
    char json[2048];
    snprintf(json, sizeof(json), "{\"result\":[");

    for (int i = 0; i < networksFound; i++)
    {
        char entry[128];
        snprintf(entry, sizeof(entry), "{\"ssid\":\"%s\",\"encType\":\"%d\",\"chann\":%d,\"rssi\":%d}",
                 WiFi.SSID(i).c_str(), WiFi.encryptionType(i), WiFi.channel(i), WiFi.RSSI(i));

        strlcat(json, entry, sizeof(json));
        if (i < networksFound - 1)
            strlcat(json, ",", sizeof(json));
    }

    strlcat(json, "],\"status\":\"OK\"}", sizeof(json));

    scanResult = json;
    #if (DEBUG == 1)
    Serial.println(scanResult);
    #endif
    // Trigger the callback to send the result
    if (scanCompleteCallback)
        scanCompleteCallback(scanResult);
}

void WiFiManager::startWiFiScan(ScanCompleteCallback callback)
{
    scanCompleteCallback = callback; // Store the callback function
    #if defined(ESP8266)
    WiFi.scanNetworksAsync([this](int networksFound) {
        scanCompleteHandler(networksFound, this);
    });
    #elif defined(ESP32)
    WiFi.scanNetworks();
    int n = WiFi.scanComplete();
    wifiScanResult(n);
    #endif
}

void WiFiManager::scanCompleteHandler(int networksFound, WiFiManager* instance)
{
    instance->wifiScanResult(networksFound);
}

String WiFiManager::getScanResult()
{
    return scanResult; // Return the stored scan result
}

const char* WiFiManager::getHostname() const
{
    return wifi_hostname;
}

void WiFiManager::setWifiHostname(const char* wifi_hostname)
{
    strlcpy(this->wifi_hostname, wifi_hostname, sizeof(this->wifi_hostname));
}

void WiFiManager::setSoftAPSSID(const char* softAP_ssid)
{
    strlcpy(this->softAP_ssid, softAP_ssid, sizeof(this->softAP_ssid));
}

void WiFiManager::setSoftAPPassword(const char* softAP_pass)
{
    strlcpy(this->softAP_pass, softAP_pass, sizeof(this->softAP_pass));
}

void WiFiManager::setWifiSSID(const char* wifi_ssid)
{
    strlcpy(this->wifi_ssid, wifi_ssid, sizeof(this->wifi_ssid));
}

void WiFiManager::setWifiPassword(const char* wifi_password)
{
    strlcpy(this->wifi_password, wifi_password, sizeof(this->wifi_password));
}

void WiFiManager::setWifiStaticIP(const char* wifi_StaticIP)
{
    strlcpy(this->wifi_StaticIP, wifi_StaticIP, sizeof(this->wifi_StaticIP));
}

void WiFiManager::setWifiGateway(const char* wifi_gateway)
{
    strlcpy(this->wifi_gateway, wifi_gateway, sizeof(this->wifi_gateway));
}

void WiFiManager::setWifiSubnet(const char* wifi_subnet)
{
    strlcpy(this->wifi_subnet, wifi_subnet, sizeof(this->wifi_subnet));
}

void WiFiManager::setWifiDNS(const char* wifi_DNS)
{
    strlcpy(this->wifi_DNS, wifi_DNS, sizeof(this->wifi_DNS));
}

void WiFiManager::setWifiRunAS(byte wifi_runAS)
{
    this->wifi_runAS = wifi_runAS;
}

void WiFiManager::setWifiStatic(byte wifi_static)
{
    this->wifi_static = wifi_static;
}
