/*
  ESP8266 mDNS server that handles HTTP GET and POST requests
  POST requests from connect.html are handled to establish connection
  GET requests are handled to return the IP address of the ESP8266 as a html webpage
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>

/*
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <HTTPClient.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <WiFiManager.h> // WiFi package used in use_wifi_package() method
*/

#ifndef STASSID
// TODO define to your WiFi credentials
#define STASSID "" 
#define STAPSK ""
#define USE_WIFI_PACKAGE 0 // toggle to change between using package and native, currently using native
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// methods defined below
void use_wifi_package(); 
void use_native_package();
void handle_post_request(WiFiClient client);
void handle_get_request(WiFiClient client, String req);

// TCP server at port 80 will respond to HTTP requests
// WiFiServer server(80);
WebServer server(80);

void handle_OnConnect();
void handle_NotFound();

void setup(void) {
  Serial.begin(9600);
  // delay(100);

  if (USE_WIFI_PACKAGE) {
    use_wifi_package();
  } else {
    use_native_package();
  }

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("google")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started");
  
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

/* 
  Serial.println("mDNS responder started");

  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  */
}

void handle_OnConnect() {
  server.send(
    200, 
    "text/html",
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"></head><body><h1>Hello Shaq</h1></body></html>"
  ); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void loop(void) {
  server.handleClient();
  
  /* 
  MDNS.update();

  // Check if a client has connected
  WiFiClient client = server.accept();
  if (!client) { return; }
  Serial.println("");
  Serial.println("New client");

  // Wait for data from client to become available
  while (client.connected() && !client.available()) { delay(1); }

  // Read the first line of the HTTP request
  String req = client.readStringUntil('\r');
  Serial.println("Request: " + req);

  // Handle OPTIONS preflight request if POST request is received
  if (req.indexOf("OPTIONS") >= 0) {
    client.readStringUntil('\n'); // Skip the rest of the line
    Serial.println("Handling preflight request");
    String s = "HTTP/1.1 204 No Content\r\n";
    s += "Access-Control-Allow-Origin: *\r\n";
    s += "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n";
    s += "Access-Control-Allow-Headers: content-type\r\n";
    s += "\r\n";
    client.print(s);

    client.stop();
    return; 
  }

  // Handle POST requests
  if (req.indexOf("POST") >= 0) {
    client.readStringUntil('\n'); // Skip the rest of the line
    handle_post_request(client);
  } else { // GET request handling
    handle_get_request(client, req);
  }
  client.stop(); // TODO maybe do not stop client to allow for persistent connections for POST request when it is requesting connection
  // Also technically do not need to "establish" or handshake connection, as long as user can make requests it does not need to be stateful
  Serial.println("Done with client"); */
}

void use_wifi_package() {
  // Initialize WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("esp");

  // Confirm connection
  Serial.println("Connected!");
}

void use_native_package() {
  // Connect to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void handle_post_request(WiFiClient client) {
  Serial.println("Handling POST request");

  // Read the remaining headers
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break; // End of headers
  }

  // Read the POST body content
  String body = "";
  while (client.available()) {
    body += (char)client.read();
  }

  Serial.println("POST Body: " + body);

  String s;
  // Example of processing POST request
  if (body.indexOf("\"command\":\"connect\"") >= 0) {
    s = "HTTP/1.1 200 OK\r\n";
    s += "Access-Control-Allow-Origin: *\r\n";
    s += "Content-Type: text/plain\r\n\r\n";
    s += "Connected successfully";
    client.print(s);
    Serial.println("Sent: Connected successfully");
  } else {
    s = "HTTP/1.1 400 Bad Request\r\n";
    s += "Access-Control-Allow-Origin: *\r\n";
    s += "Content-Type: text/plain\r\n\r\n";
    s += "Invalid command";
    client.print(s);
    Serial.println("Sent: Invalid command");
  }
}

void handle_get_request(WiFiClient client, String req) {
  String s;
    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    if (addr_start == -1 || addr_end == -1) {
      Serial.print("Invalid request: ");
      Serial.println(req);
      return;
    }
    req = req.substring(addr_start + 1, addr_end);
    Serial.print("Request: ");
    Serial.println(req);
    client.flush();

    if (req == "/") {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      s += ipStr;
      s += "</html>\r\n\r\n";
      Serial.println("Sending 200");
    } else {
      s = "HTTP/1.1 404 Not Found\r\n\r\n";
      Serial.println("Sending 404");
    }
    client.print(s);
}