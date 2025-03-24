#ifndef DISCORD_BOT_H
#define DISCORD_BOT_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

class DiscordBot {
public:
    // Send a message to Discord
    void begin(String channel_id, String token);
    bool send(String content);

private:
    // Make the HTTPS request
    bool sendRequest(String jsonPayload);

    String webhook_url;  // Discord webhook URL
    bool tts = false;    // Text-to-speech flag
    bool debug = true;   // Debug flag
};

#endif // DISCORD_BOT_H
