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
 * @class MQTThandler
 * @brief This class manages MQTT communication and message handling.
 *
 * The MQTThandler class provides functionality to set up and manage MQTT connections,
 * register and unregister message callbacks, and send and receive MQTT messages.
 * Each callback is associated with a unique ID. The class also offers methods to 
 * initialize the callback handling with a PubSubClient instance, invoke all registered 
 * callbacks upon receiving messages, and list all registered callbacks.
 *
 * Example usage:
 * @code
 * MQTThandler mqttHandler;
 *
 * // Register callbacks
 * mqttHandler.registerCallback(exampleCallback1, 1);
 * mqttHandler.registerCallback(exampleCallback2, 2);
 *
 * // Initialize the MQTT client with the handler
 * mqttHandler.initCallback(client);
 *
 * // Connect to the MQTT broker
 * String message;
 * boolean runState = true;
 * mqttHandler.setupMQTT(&message, runState);
 *
 * // Send a message
 * char topic[] = "myTopic";
 * char payload[] = "myPayload";
 * mqttHandler.sendMQTT(topic, payload, false);
 *
 * // Update MQTT periodically
 * mqttHandler.updateMQTT();
 *
 * // List registered callbacks
 * mqttHandler.listCallbacks();
 *
 * // Unregister a callback by ID
 * mqttHandler.unregisterCallback(1);
 *
 * // List registered callbacks again
 * mqttHandler.listCallbacks();
 * @endcode
 */
class MQTTmanager {
public:
    void registerCallback(CallbackType callback, int id);
    void unregisterCallback(int id);
    void callbackMQTT(char* topic, byte* payload, unsigned int length);
    void initCallback(PubSubClient& client);
    void listCallbacks() const;
    void updateMQTT();
    bool setupMQTT(String* message, boolean runState);
    bool MQTTreconnect();
    bool sendMQTT(char* Topic, char* Payload, bool retain);

private:
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
