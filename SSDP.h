// Declare function because of default param provided
void sendUDP( String payloadUDP, IPAddress ssdpDeviceIP = (0,0,0,0), int udpPort = 4210 );

// Time interval for udp SSDP M-SEARCH
bool measureSSDPFirstRun = true;
const long intervalSSDP = 1000 * 60 * 10;   // 1000 * 60 * 10 - 10min
unsigned long previousSSDP = intervalSSDP;  // time of last point added

