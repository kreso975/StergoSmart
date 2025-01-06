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
	 - void checkAPRestart(): Check if AP needs auto restart after 5 min */
class WiFiManager
{
public:
	WiFiManager(char *ssid, char *password, char *staticIP, char *gateway, char *subnet, char *DNS, char *hostname, byte &useStatic, char *ap_ssid, char *ap_pass)
		 : wifi_ssid(ssid), wifi_password(password), wifi_StaticIP(staticIP), wifi_gateway(gateway), wifi_subnet(subnet), wifi_DNS(DNS), wifi_hostname(hostname), wifi_static(useStatic), softAP_ssid(ap_ssid), softAP_pass(ap_pass), ap_previousMillis(0), ap_intervalHist(300000) {} // 5 minutes in milliseconds

	bool disconnectSTA()
	{
		WiFi.disconnect(true);
		return true;
	}

	bool startWPS()
	{
		if (WiFi.beginWPSConfig())
			return true;
		else
			return false;
	}

	bool startSTA(int STAmode = 0)
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
			#if (DEBUG == 1)
			writeLogFile(".", 0, 1);
			#endif
			cnt++;
		}
		#if (DEBUG == 1)
		writeLogFile(F("STA connected"), 1, 3);
		writeLogFile(F("WiFi Mode: ") + String(WiFi.getMode()), 1, 3);
		writeLogFile(F("IP: ") + WiFi.localIP().toString(), 1, 3);
		#endif
		return true;
	}

	bool startAP()
	{
		WiFi.mode(WIFI_AP);
		String tmp = String(softAP_ssid) + "_" + String(ESP.getChipId());
		WiFi.softAP(tmp.c_str(), softAP_pass);
		delay(500);

		#if (DEBUG == 1)
		writeLogFile(F("AP connected"), 1, 3);
		writeLogFile(F("WiFi Mode: ") + String(WiFi.getMode()), 1, 3);
		writeLogFile(F("IP: ") + WiFi.softAPIP().toString(), 1, 3);
		#endif
		ap_previousMillis = millis(); // Initialize the timer
		return true;
	}

	void manageWiFi()
	{
		wifi_station_set_hostname(wifi_hostname);
		WiFi.hostname(wifi_hostname);

		if (strcmp(wifi_ssid, "") && strcmp(wifi_password, ""))
		{
			if (wifi_static == 0)
			{
				if (!startSTA())
				{
					#if (DEBUG == 1)
					writeLogFile(F("Problem connect to STA"), 1);
					#endif
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
						#if (DEBUG == 1)
						writeLogFile(F("Problem connect with Static IP"), 1, 3);
						#endif
						if (!startSTA())
						{
							#if (DEBUG == 1)
							writeLogFile(F("Problem connect to STA"), 1, 3);
							#endif
							disconnectSTA();
							startAP();
						}
					}
					else
					{
						#if (DEBUG == 1)
						writeLogFile(F("Succes connect with Static IP"), 1, 3);
						#endif
					}
				}
				else
				{
					// Handle case where static IP configuration is incomplete
				}
			}
		}
		else
		{
			startAP();
		}
	}

	void checkAPRestart()
	{
		if (WiFi.getMode() == WIFI_AP)
		{
			// If AP is running for 5 minutes and we have configured SSID and Password
			// Restart the device and try to connect to WiFi STA again
			if ((millis() - ap_previousMillis > ap_intervalHist) && strlen(wifi_ssid) > 0 && strlen(wifi_password) > 0)
			{
				#if (DEBUG == 1)
				writeLogFile(F("AP 5min restart"), 1, 3);
				#endif
				delay(500);
				ESP.restart();
			}
		}
	}

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
	unsigned long ap_previousMillis;
	const unsigned long ap_intervalHist;
};
