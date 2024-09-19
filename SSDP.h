// Time interval for udp SSDP M-SEARCH
bool measureSSDPFirstRun = true;
const long intervalSSDP = 1000 * 60 * 10;   // 1000 * 60 * 10 - 10min
unsigned long previousSSDP = intervalSSDP;  // time of last point added