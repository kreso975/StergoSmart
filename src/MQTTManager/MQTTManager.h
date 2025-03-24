#ifndef MQTT_H
#define MQTT_H

#include <vector>
#include <functional>
#include <PubSubClient.h>
#include <WiFiClient.h>

extern WiFiClient espClient;
extern bool writeLogFile( String message, int newLine, int output);

typedef std::function<void(char*, byte*, unsigned int)> CallbackType;

extern std::vector<const char*> subscriptionList;


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
class MQTTManager
{
public:
    MQTTManager(); // Declare the constructor
    void registerCallback(CallbackType callback, int id);
    void initCallback(PubSubClient& client);
    void updateMQTT();
    bool setupMQTT(String* message, boolean runState);
    bool sendMQTT(char* Topic, char* Payload, bool retain);

    // public setter methods for these private variables
    void setMqttStart(byte mqttStart);
    void setMqttServer(const char* server);
    void setMqttPort(int port);
    void setMqttClientName(const char* clientName);
    void setMqttClientUsername(const char* clientUsername);
    void setMqttClientPassword(const char* clientPassword);
    void setMqttMyTopic(const char* myTopic);

    byte getMqttStart() const; 
    /*
    void unregisterCallback(int id);
    void listCallbacks() const;
    */

private:
    byte mqtt_start;
    char mqtt_server[20];
    int mqtt_port;
    char mqtt_clientName[23];
    char mqtt_clientUsername[50];
    char mqtt_clientPassword[50];
    char mqtt_myTopic[120];
    static const unsigned long mqttTempDownInt;                     // 15 minutes
    byte mqtt_connectTry;                                           // How many times we will try to connect to MQTT before we put it to sleep
    byte mqttTempDown;                                              // If we couldn't reconnect, mqtt_start is set to 0, and mqttTempDown is set to 1
    unsigned long lastmqttTempDownMillis;                           // Time of the last point added

    PubSubClient client; // Encapsulated PubSubClient within MQTTManager

    void callbackMQTT(char* topic, byte* payload, unsigned int length);
    bool MQTTreconnect();
    std::vector<std::pair<int, CallbackType>> callbacks;
};


#endif // MQTT_H
