void loop(void) {

  MDNS.update();

  WiFiClient client = server.accept();
  if (!client) { return; }
  Serial.println("New client");

  // Wait for data from client to become available
  while (client.connected() && !client.available()) { delay(1); }

  // Read the HTTP request
  String req = client.readStringUntil('\r');
  Serial.println("Request: " + req);

  // Determine if the request is POST
  if (req.indexOf("POST") >= 0) {
    // Read POST body content
    String body = "";
    while (client.available()) {
      body += (char)client.read();
    }

    Serial.println("POST Body: " + body);

    // Example of processing POST request (adjust according to your use case)
    if (body.indexOf("\"command\":\"connect\"") >= 0) {
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print("Connected successfully");
      Serial.println("Sent: Connected successfully");
    } else {
      client.print("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n");
      client.print("Invalid command");
      Serial.println("Sent: Invalid command");
    }
  } else {
    client.print("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n");
    client.print("Not Found");
    Serial.println("Sent: 404 Not Found");
  }

  client.stop();
  Serial.println("Client disconnected");
}
