#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);  // Start serial communication for debugging
  pinMode(LED_BUILTIN, OUTPUT);  // Initialize LED pin as output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn off LED initially (active LOW)

  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP8266-Config");  // Start AP for WiFi config if no known network is available

  Serial.println("Connected to WiFi");
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/soundlevel", HTTP_GET, []() {
    if (server.hasArg("value")) {
      int soundLevel = server.arg("value").toInt();
      Serial.print("Received sound level: ");
      Serial.println(soundLevel);

      if (soundLevel > 20) {
        digitalWrite(LED_BUILTIN, LOW);  // Turn on LED (active LOW)
        Serial.println("LED ON");
      } else {
        digitalWrite(LED_BUILTIN, HIGH); // Turn off LED
        Serial.println("LED OFF");
      }
      server.send(200, "text/plain", "LED state updated.");
    } else {
      server.send(400, "text/plain", "Missing 'value' parameter");
    }
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();  // Handle incoming client requests
}