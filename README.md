<span align="center">

 # StergoSmart

</span>
  
Smart Home IOT - Weather and Switches with GUI  
  
[![Donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://paypal.me/kreso975)

ESP-8266-01S , minimum 1M

> [!IMPORTANT]
> Works with SPIFF  
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
> Adafruit BusIO by Adafruit **v1.16.1**  
> Arduinojson by Benoit Blanchon **v5.13.5**  
> PubSubClient by Nick O'Leary **v2.7**  
> Time by Michael Margolis **v1.6.1**

> [!INFO]
> Compile setup:  
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


