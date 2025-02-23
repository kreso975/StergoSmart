#ifndef MQTT_H
#define MQTT_H

#include <vector>
#include <functional>
#include <PubSubClient.h>
#include <WiFiClient.h>


extern PubSubClient client;
extern bool writeLogFile( String message, int newLine, int output);

typedef std::function<void(char*, byte*, unsigned int)> CallbackType;

/**
 * @class MQTTmanager
 * @brief This class manages MQTT communication and message handling.
 *
 * The MQTTmanager class provides functionality to set up and manage MQTT connections,
 * register and unregister message callbacks, and send and receive MQTT messages.
 * Each callback is associated with a unique ID. The class also offers methods to 
 * initialize the callback handling with a PubSubClient instance, invoke all registered 
 * callbacks upon receiving messages, and list all registered callbacks.
 *
 * Example usage:
 * @code
 * MQTTmanager mqttManager;
;
 *
 * // Register callbacks
 * mqttManager.registerCallback(exampleCallback1, 1);
 * mqttManager.registerCallback(exampleCallback2, 2);
 *
 * // Initialize the MQTT client with the handler
 * mqttManager.initCallback(client);
 *
 * // Connect to the MQTT broker
 * String message;
 * boolean runState = true;
 * mqttManager.setupMQTT(&message, runState);
 *
 * // Send a message
 * char topic[] = "myTopic";
 * char payload[] = "myPayload";
 * mqttManager.sendMQTT(topic, payload, false);
 *
 * // Update MQTT periodically
 * mqttManager.updateMQTT();
 *
 * // List registered callbacks
 * mqttManager.listCallbacks();
 *
 * // Unregister a callback by ID
 * mqttManager.unregisterCallback(1);
 *
 * // List registered callbacks again
 * mqttManager.listCallbacks();
 * @endcode
 */
class MQTTManager {
public:
    void registerCallback(CallbackType callback, int id);
    void initCallback(PubSubClient& client);
    void updateMQTT();
    bool setupMQTT(String* message, boolean runState);
    bool sendMQTT(char* Topic, char* Payload, bool retain);
    /*
    void unregisterCallback(int id);
    void listCallbacks() const;
    */

private:
    void callbackMQTT(char* topic, byte* payload, unsigned int length);
    bool MQTTreconnect();
    std::vector<std::pair<int, CallbackType>> callbacks;
};

extern std::vector<const char*> subscriptionList;

// Global variables
extern byte mqtt_connectTry; // How many times we will try to connect to MQTT before we put it to sleep
extern byte mqttTempDown; // If we couldn't reconnect, mqtt_start is set to 0, and mqttTempDown is set to 1
#define mqttTempDownInt 1000 * 60 * 15 // 15 minutes
extern unsigned long lastmqttTempDownMillis; // Time of the last point added

extern byte mqtt_start;
extern char mqtt_server[20];
extern int mqtt_port;
extern char mqtt_clientName[23];
extern char mqtt_clientUsername[50];
extern char mqtt_clientPassword[50];
extern char mqtt_myTopic[120];

// Time Interval for sending MQTT data
extern int mqtt_interval;
extern unsigned long mqtt_intervalHist;
extern unsigned long mqtt_previousMillis;


#endif // MQTT_H
