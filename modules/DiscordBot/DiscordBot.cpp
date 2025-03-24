#include "DiscordBot.h"

/*

DiscordBot discord; // Create a Discord_Webhook object
discord.begin(channel_id, token);

// Send a test message
if (discord.send("Hello World!")) {
	Serial.println("[Discord] Message sent successfully.");
} else {
	Serial.println("[Discord] Failed to send message.");
}
*/

// Get webhook URL
void DiscordBot::begin(String channel_id, String token)
{
	DiscordBot::webhook_url = "https://discordapp.com/api/webhooks/" + channel_id + "/" + token;
}

bool DiscordBot::send(String content)
{
	// Customize the payload as needed
	String jsonPayload = "{\"username\":\"Weather-Attic-898235\",\"avatar_url\":\"https://xxxxs.com/StergoSmart/img/number74.png\",\"content\":\"" + content + "\"}";
	return sendRequest(jsonPayload);
}

bool DiscordBot::sendRequest(String jsonPayload)
{
	String discord_tts = tts ? "true" : "false";
	WiFiClientSecure *Botclient = new WiFiClientSecure;
	bool ok = false;

	if (Botclient)
	{
		Botclient->setInsecure(); // Disable SSL certificate verification

		HTTPClient https;
		if (debug)
		{
			Serial.println("[HTTP] Connecting to Discord...");
			Serial.println("[HTTP] Payload: " + jsonPayload);
		}

		// Start HTTPS request
		if (https.begin(*Botclient, webhook_url))
		{
			Serial.println("1");
			https.addHeader("Content-Type", "application/json"); // JSON request
			Serial.println("2");
			int httpCode = https.POST(jsonPayload); // POST request
			Serial.println("3");
			if (httpCode > 0)
			{
				if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_NO_CONTENT)
				{
					if (debug)
						Serial.println("[HTTP] OK");

					ok = true;
				}
				else
				{
					if (debug)
					{
						Serial.print("[HTTP] ERROR: ");
						Serial.println(https.getString());
					}
				}
			}
			else
			{
				if (debug)
					Serial.printf("[HTTP] Connection failed: %s\n", https.errorToString(httpCode).c_str());
			}
			https.end();
		}
		else
		{
			if (debug)
				Serial.println("[HTTP] Unable to connect");
		}
		delete Botclient;
	}
	else
	{
		if (debug)
			Serial.println("[HTTP] Unable to create client");
	}
	return ok;
}
