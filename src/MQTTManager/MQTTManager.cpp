/*********************************************************************************************************/
/* MQTT Server address
 * TODO : WebClient - In devices - Run Services (on | off )
 *                    Check if MQTT Broker is available, Check if MQTT is set - give alerts (graphic)
 *********************************************************************************************************/

#include "../../settings.h"
#include "MQTTManager.h"

const unsigned long MQTTManager::mqttTempDownInt = 1000 * 60 * 15;  // 15 minutes

std::vector<const char *> subscriptionList;

MQTTManager::MQTTManager() 
    : mqtt_start(0),                     			// Initialize mqtt_start to 0 / stop
	 	client(espClient),								// Initialize PubSubClient with the external WiFiClient
	 	mqtt_connectTry(3),                     	// How many times we will try to connect to MQTT before we put it to sleep
      mqttTempDown(0),                        	// If we couldn't reconnect, mqtt_start is set to 0, and mqttTempDown is set to 1
      lastmqttTempDownMillis(mqttTempDownInt) 	// Time of the last point added
{
    // Constructor body if needed
}


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
   // Use the getter method to access mqtt_start
   if ( getMqttStart() )
   {
      if (!client.connected())
         MQTTreconnect();

      client.loop();
   }

   if ( this->mqttTempDown )
   {
      if ((millis() - this->lastmqttTempDownMillis > MQTTManager::mqttTempDownInt))
      {
         this->lastmqttTempDownMillis = millis();

         setMqttStart(1); 
         this->mqttTempDown = 0;

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
	char tempMessage[128]; // Adjust the size if needed
	const char *msg;

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
					msg = "Success MQTT Start";
				}
				else
				{
					*message = ""; // Handle connection failure here, if needed.
					return false;
				}
			}
			else
			{
				msg = "Success MQTT already running";
			}
		}
		else
		{
			msg = "Success MQTT already running";
		}
	}
	else
	{
		if (client.connected())
		{
			client.disconnect();
		}
		msg = "Success MQTT Stop";
	}

	// Perform snprintf only once at the end
	snprintf(tempMessage, sizeof(tempMessage), "{\"success\":\"%s\"}", msg);
	*message = String(tempMessage);

	writeLogFile(String(msg), 1, 3);
	return true;
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
			if (this->mqtt_connectTry > 0)
			{
				this->mqtt_connectTry--;
				delay(2000);
			}
			else
			{
				setMqttStart(0);
				this->mqttTempDown = 1;
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

void MQTTManager::setMqttServer(const char* server)
{
	strlcpy(this->mqtt_server, server, sizeof(this->mqtt_server));
}

void MQTTManager::setMqttPort(int port)
{
	this->mqtt_port = port;
}

void MQTTManager::setMqttClientName(const char* clientName)
{
	strlcpy(this->mqtt_clientName, clientName, sizeof(this->mqtt_clientName));
}

void MQTTManager::setMqttClientUsername(const char* clientUsername)
{
	strlcpy(this->mqtt_clientUsername, clientUsername, sizeof(this->mqtt_clientUsername));
}

void MQTTManager::setMqttClientPassword(const char* clientPassword)
{
	strlcpy(this->mqtt_clientPassword, clientPassword, sizeof(this->mqtt_clientPassword));
}

void MQTTManager::setMqttMyTopic(const char* myTopic)
{
	strlcpy(this->mqtt_myTopic, myTopic, sizeof(this->mqtt_myTopic));
}

void MQTTManager::setMqttStart(byte mqttStart)
{
	mqtt_start = mqttStart; // Assign value to the global variable
}

byte MQTTManager::getMqttStart() const
{
	return mqtt_start;
}
