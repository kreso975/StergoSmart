#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h> // Include the Arduino core header file

#define configFile "/config.json"
#define LOG_FILE "/log.json"

bool initConfig( String* message );
bool writeToConfig( String* message );

extern String fOpen;
extern String fWrite;
#define nLOG "New Log file"
#define fsLarge " file size is too large"  //Used to be String
#define faParse "Fail to parse json "      //Used to be String

#endif