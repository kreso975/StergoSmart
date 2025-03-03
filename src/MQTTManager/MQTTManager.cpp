/*********************************************************************************************************/
/* MQTT Server address
 * TODO : WebClient - In devices - Run Services (on | off )
 *                    Check if MQTT Broker is available, Check if MQTT is set - give alerts (graphic)
 *********************************************************************************************************/

#include "../../settings.h"

#include "MQTTManager.h"
#include <Arduino.h>

// Global variables
byte mqtt_connectTry = 3;                               // How many times we will try to connect to MQTT before we put it to sleep
byte mqttTempDown = 0;                                  // If we couldn't reconnect, mqtt_start is set to 0, and mqttTempDown is set to 1
#define mqttTempDownInt 1000 * 60 * 15                  // 15 minutes
unsigned long lastmqttTempDownMillis = mqttTempDownInt; // Time of the last point added

byte mqtt_start = 0;
char mqtt_server[20];
int mqtt_port;
char mqtt_clientName[23];
char mqtt_clientUsername[50];
char mqtt_clientPassword[50];
char mqtt_myTopic[120];

// Time Interval for sending MQTT data
int mqtt_interval = 120;
unsigned long mqtt_intervalHist;
unsigned long mqtt_previousMillis; // Time of the last point added

std::vector<const char *> subscriptionList;

void MQTTManager::registerCallback(CallbackType callback, int id)
{
  callbacks.emplace_back(id, callback);
}

void MQTTManager::callbackMQTT(char *topic, byte *payload, unsigned int length)
{
  for (auto &callback : callbacks)
  {
    callback.second(topic, payload, length);
  }
}

void MQTTManager::initCallback(PubSubClient &client)
{
  client.setCallback([this](char *topic, byte *payload, unsigned int length)
                     { this->callbackMQTT(topic, payload, length); });
}

/* // NOT IN USE 
  // IF NEEDED JUST UNCOMMENT - saving KBs
void MQTTManager::unregisterCallback(int id)
{
  callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), [id](const std::pair<int, CallbackType> &cb) {
    return cb.first == id;
  }), callbacks.end());
}

void MQTTManager::listCallbacks() const
{
  #if ( DEBUG == 1 )
  writeLogFile(F("Registered Callbacks:"), 1, 1);
  for (size_t i = 0; i < callbacks.size(); ++i)
  {
    writeLogFile(F("Callback "), 0, 1);
    writeLogFile(String(i + 1), 0, 1);
    writeLogFile(F(" (ID: "), 0, 1);
    writeLogFile(String(callbacks[i].first), 0, 1);
    writeLogFile(F("): registered"), 1, 1);
  }
  #endif
}
*/

/* ======================================================================
Function: updateMQTT
Purpose : Main Loop handler for MQTT
Input   : 
Output  : logic execution and listener
Comments: 
TODO    : */
void MQTTManager::updateMQTT()
{
  if (mqtt_start == 1)
  {
    if (!client.connected())
    {
      MQTTreconnect();
    }
    client.loop();
  }
  if (mqttTempDown == 1)
  {
    if ((millis() - lastmqttTempDownMillis > mqttTempDownInt))
    {
      lastmqttTempDownMillis = millis();
      mqtt_start = 1;
      mqttTempDown = 0;
      #if (DEBUG == 1)
      writeLogFile(F("Resetting MQTT"), 1, 1);
      #endif
    }
  }
}

/* ======================================================================
Function: setupMQTT
Input: runState == false | true = stop | start MQTT
TODO: 
Comments:*/
bool MQTTManager::setupMQTT(String *message, boolean runState)
{
  String msg;
  if (runState)
  {
    if (!client.connected())
    {
      client.setServer(mqtt_server, mqtt_port);
      this->initCallback(client);
      if (!client.connected())
      {
        if (MQTTreconnect())
        {
          msg = F("Success MQTT Start");
          String tempMessage = F("{\"success\":\"") + msg + F("\"}");
          *message = tempMessage;
          writeLogFile(msg, 1, 3);
          return true;
        }
      }
    }
    else
    {
      msg = F("Success MQTT already running");
      String tempMessage = F("{\"success\":\"") + msg + F("\"}");
      *message = tempMessage;
      writeLogFile(msg, 1, 3);
      return true;
    }
  }
  else
  {
    if (client.connected())
      client.disconnect();
    msg = F("Success MQTT Stop");
    String tempMessage = F("{\"success\":\"") + msg + F("\"}");
    *message = tempMessage;
    writeLogFile(msg, 1, 3);
    return true;
  }
  return false;
}

bool MQTTManager::MQTTreconnect()
{
  while (!client.connected())
  {
    if (client.connect(mqtt_clientName, mqtt_clientUsername, mqtt_clientPassword))
    {
      client.publish(mqtt_myTopic, "connected");
      for (auto topic : subscriptionList)
        client.subscribe(topic);
    }
    else
    {
      if (mqtt_connectTry > 0)
      {
        mqtt_connectTry--;
        delay(2000);
      }
      else
      {
        mqtt_start = 0;
        mqttTempDown = 1;
        writeLogFile(F("MQTT Not Reconnected"), 1, 3);
        return false;
      }
    }
  }
  return true;
}

/* ==========================================================================================
Function: sendMQTT
Purpose : send MQTT payload 
Input   : Topic, Payload, retain
Output  : Success = True | Fail = False
Comments: 
TODO    :  */
bool MQTTManager::sendMQTT(char *Topic, char *Payload, bool retain)
{
  if (client.connected())
  {
    if (!client.publish(Topic, Payload, retain))
    {
      return false;
    }
    return true;
  }
  else
  {
    #if (DEBUG == 1)
    writeLogFile(F("MQTT Pub Not Connected"), 1, 3);
    #endif
    return false;
  }
}
