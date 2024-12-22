<span align="center">

 # StergoSmart

</span>
  
Smart Home IOT - Weather and Switches with GUI  
  
[![Donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://paypal.me/kreso975)

**ESP-8266-01S , minimum 1M**  
**BME280**  
tested with Sonoff S26, T4EU1C

- Bootstrap 5.3 | old UI Bootstrap 3.4
- jQuery
- GUI Interface: Dashboard, Configuration
- ApexCharts | old UI - Google Charts
- Captive portal for initial Setup
- OTA update
- MQTT, HTTP, Publish as Webhook
- WiFi manager
- SSDP
- WS2812b matrix 8x32.
- Clock displaying Time, Date, Temperature, Humidity, Message
- UDP Tic Tac Toe
- Tic Tac Toe winners Publish as Webhook (Discord channel)
  
> [!IMPORTANT]
> **Works with SPIFF**  
> Works only in old Arduino IDE 1.8  
> SPIFF UPLOAD ( ESP8266 Scatch Data Upload )  
>	Download https://github.com/esp8266/arduino-esp8266fs-plugin/releases - ESP8266FS  
> Extract  to Home dir Arduino /tools  
>
>   
> **Libraries Needed**
>   
> NTPClient by Fabrice Weinberg  **v3.2.1**  
> Adafruit BME280 Library by Adafruit **v2.2.4**  
> Adafruit Unified Sensor by Adafruit **v1.1.14**  
> DHT sensor library by Adafruit **v1.4.6**  
> Adafruit BusIO by Adafruit **v1.16.1**  
> Arduinojson by Benoit Blanchon **v5.13.5** || v000.5.103 >= **v6.21.5**  
> PubSubClient by Nick O'Leary **v2.8**  
> OneWire by Jim Studt, Tom Pollard.. **v2.3.8**  
> DallasTemperature by Miles Burton **v3.9.0**  
> Time by Michael Margolis **v1.6.1**  
> FastLED by Daniel Garcia **v3.7.8**  

> [!NOTE]
> **Compile setup: ** 
> ESP8266 : TOOLS >>  
>	Board: Generic ESP8266 Module  
>	Biolt in LED: 2  
>	Upload speed: 115000  
>	CPU Frequency: 80 or 160 MHz  
>	Crystal Frequency: 26MHz  
>		// WS: Flash size: 1M(FS: 160KB OTA: ~422KB)  
>		// PS: Flash size: 1M(FS: 192KB OTA: ~406KB)  
>	Flash mode: DOUT (because of sonnof EU)  
>	Flash Frequency: 80MH  
>	Reset method: no dtr (aka ck)  
>	Debug port: Disabled  
>	Debug Level: None  
>	lwIP Variant: v2 Lower Memory (no features)  
>	Vtables: Flash  
>	Exceptions: Legacy (Null can return nullptr)  
>	Erase flash: Only Sketch ( only code will be uploaded )  
>			Sketch + WiFi settings (WiFi settings will be erased)  
>			All Flash contents ( Everything will be erased including SPIFFS  
>	Espresiff FW: nonos SDK 2.2.1 + 119 (191122)  
>	SSL Support: Basic SSL ciphers (lower ROM use)  


## Screenshots of GUI  
  
**Dashboard**  
  
<div style="display: flex; flex-wrap: wrap; gap: 10px;"> <img src="./ScreenShots/Weather-Dashboard.png?raw=true" alt="Weather Dashboard" title="Weather Dashboard" style="width: 30%;"> <img src="./ScreenShots/Weather-Clock-Dashboard.png?raw=true" alt="Weather Clock Dashboard" title="Weather Clock Dashboard" style="width: 30%;"> <img src="./ScreenShots/Switch-Dashboard.png?raw=true" alt="Switch Dashboard" title="Switch Dashboard" style="width: 30%;"> </div>  

**Info & Log**  
  
<div style="display: flex; justify-content: center;"> <img src="./ScreenShots/Info-and-Log.png?raw=true" alt="Info & Log" title="Info & Log" style="width: 50%;"> </div>
  
   
**Publish**  
  
<div style="display: flex; justify-content: center;"> <img src="./ScreenShots/Module-Settings.png?raw=true" alt="Publish" title="Publish" style="width: 50%;"> </div>

  
**WiFi**  
  
<div style="display: flex; flex-wrap: wrap; gap: 10px;"> <img src="./ScreenShots/Wifi-Setup.png?raw=true" alt="WiFi" title="WiFi" style="width: 45%;"> <img src="./ScreenShots/WiFi-available-AP.png?raw=true" alt="WiFi available networks" title="WiFi available networks" style="width: 45%;"> </div>
  

**Settings**  
  

<div style="display: flex; justify-content: center;"> <img src="./ScreenShots/Settings.png?raw=true" alt="Settings" title="Settings" style="width: 50%;"> </div>
  

**Captive portal** 
  
  
<div style="display: flex; justify-content: center;"> <img src="./ScreenShots/Captive-portal.png?raw=true" alt="Captive portal" title="Captive portal" style="width: 50%;"> </div>

