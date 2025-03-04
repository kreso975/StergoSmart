#include "../../settings.h"
#include <Arduino.h> // Include the Arduino core header file                   

#if defined(ESP8266)                // -----------------  ESP8266  -----------------
  #include <ESP8266WiFi.h>
#elif defined(ESP32)                // -----------------  ESP32  -----------------
  #include <WiFi.h>
#endif                              // -------------------------------------------
#include "WiFiManager.h"


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
     
WiFiManager::WiFiManager(char *ssid, char *password, char *staticIP, char *gateway, char *subnet, char *DNS, char *hostname, byte &useStatic, char *ap_ssid, char *ap_pass, String chipID)
    : wifi_ssid(ssid), wifi_password(password), wifi_StaticIP(staticIP), wifi_gateway(gateway), wifi_subnet(subnet), wifi_DNS(DNS), wifi_hostname(hostname), wifi_static(useStatic), softAP_ssid(ap_ssid), softAP_pass(ap_pass), chipID(chipID), ap_previousMillis(0), ap_intervalHist(300000), scanCompleteCallback(nullptr) {}

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
        WiFi.begin(wifi_ssid, wifi_password);
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
    char tmp[40]; // Adjust size as needed
    snprintf(tmp, sizeof(tmp), "%s_%s", softAP_ssid, chipID.c_str());
    WiFi.softAP(tmp, softAP_pass);
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