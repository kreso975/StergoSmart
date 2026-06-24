#pragma once

//set local UDP port - using it for device 2 device communication (WiFi)
#define LOCAL_UDP_PORT 4210
#define NUMBER_OF_FOUND_UDP 10

struct UDPDevice {
   IPAddress ip;
   unsigned long timestamp; // Use millis() or a time function for the timestamp
};
UDPDevice foundUDPdevices[NUMBER_OF_FOUND_UDP];

int actualUDPdevices;

// Declare function because of default param provided
void sendUDP(const char* payloadUDP, IPAddress udpDeviceIP = (0,0,0,0), int udpPort = LOCAL_UDP_PORT );
char* parseAndExtract(const char* input, const char* key, const char* delimiter, int part = -1);

// Time interval for UDP Discovery M-SEARCH
bool measureDiscoveryFirstRun = true;
const long intervalDiscovery = 1000 * 60;   // 1000 * 60 * 10 - 10min
unsigned long previousDiscovery = intervalDiscovery;  // time of last point added

