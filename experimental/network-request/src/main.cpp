#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>

// reference: https://loginov-rocks.medium.com/quick-start-with-nodemcu-v3-esp8266-arduino-ecosystem-and-platformio-ide-b8415bf9a038
void setup()
{
  Serial.begin(9600);

  // Initialize WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("esp");

  // Confirm connection
  Serial.println("Connected!");
}

void loop()
{
  // Wait for WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;

    // Specify the URL for the request
    http.begin(client, "http://jsonplaceholder.typicode.com/posts/1");

    // Send HTTP GET request
    int httpCode = http.GET();

    // Check the returning HTTP status code
    if (httpCode > 0) {
      // HTTP header has been sent and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // File found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response:");
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end(); // Free resources

  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(10000); // Make the request every 10 seconds
}