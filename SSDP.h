
#define SSDPPORT 1900
#define SSDPADRESS 239, 255, 255, 250

//set local UDP port - using it for device 2 device communication (WiFi)
#define LOCAL_UDP_PORT 4210
#define NUMBER_OF_FOUND_SSDP 10
//IPAddress foundSSDPdevices[NUMBER_OF_FOUND_SSDP];
struct SSDPDevice {
   IPAddress ip;
   unsigned long timestamp; // Use millis() or a time function for the timestamp
};
SSDPDevice foundSSDPdevices[NUMBER_OF_FOUND_SSDP];
int actualSSDPdevices = 0;

// Declare function because of default param provided
void sendUDP(const char* payloadUDP, IPAddress ssdpDeviceIP = (0,0,0,0), int udpPort = LOCAL_UDP_PORT );
char* parseAndExtract(const char* input, const char* key, const char* delimiter, int part = -1);

// Time interval for udp SSDP M-SEARCH
bool measureSSDPFirstRun = true;
const long intervalSSDP = 1000 * 60 * 10;   // 1000 * 60 * 10 - 10min
unsigned long previousSSDP = intervalSSDP;  // time of last point added

