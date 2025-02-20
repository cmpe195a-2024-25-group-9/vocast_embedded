#include <WiFi.h>
#include <WebSocketsServer.h>
#include <driver/i2s.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "HaqueWiFi";
const char* password = "!Curtis215";

// I2S pins
#define I2S_BCLK_PIN    15
#define I2S_LRC_PIN     16
#define I2S_DOUT_PIN    17
#define SAMPLE_RATE     48000
#define DMA_BUF_COUNT   8 // modify for experiments
#define DMA_BUF_LEN     64 // modify for experiments

#define USE_WIFI_PACKAGE 1

// Buffer to hold received audio data
int16_t audioBuffer[512]; // modify for experiments

// WebSocket server on port 81
WebSocketsServer webSocket = WebSocketsServer(81);

/*
i2s configuration experiments:
audioBuffer size = 512
dma buf count = 8
dma buf len = 64
latency = ?

audioBuffer size = 256
dma buf count = 6
dma buf len = 256
latency = ?

audioBuffer size = 
dma buf count = 
dma buf len = 
latency = ?

*/

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);
  
  if (USE_WIFI_PACKAGE) {
    WiFiManager wifiManager;
    wifiManager.autoConnect("esp");  // Start AP for WiFi config if no known network is available

    Serial.println(F("Connected to WiFi"));
  }
  else {
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(F("Connected to WiFi"));
  }

  // Print the ESP32 IP address
  Serial.print(F("ESP32 IP Address: "));
  Serial.println(WiFi.localIP());

  // Initialize WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // I2S Configuration
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = DMA_BUF_COUNT,
    .dma_buf_len = DMA_BUF_LEN
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num = I2S_LRC_PIN,
    .data_out_num = I2S_DOUT_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  Serial.println("I2S Initialized.");
}

void loop() {
  webSocket.loop();  // Keep the WebSocket server running
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case WStype_TEXT:
      // Handle text messages (optional)
      break;

    case WStype_BIN: {
      Serial.println("in payload");
      /* Declare variables at the beginning
      DynamicJsonDocument jsonDoc(1024);
      StaticJsonDocument<256> responseDoc;
      */
      // char response[256];

      /* Parse the received JSON
      deserializeJson(jsonDoc, payload, length);

      // Prepare the response
      responseDoc["timestamp"] = jsonDoc["timestamp"]; // Echo the timestamp
      serializeJson(responseDoc, response, sizeof(response));

      // Send response as binary data
      Serial.println("sent data");
      */
      // response to client
      // webSocket.sendBIN(num, (uint8_t*)response, strlen(response));

      if (length % sizeof(int16_t) == 0) {
        memcpy(audioBuffer, payload, length); // payload, length
        size_t bytesWritten = 0;
        // i2s write START timestamp
        // Serial.println("start: ");
        // Serial.println(millis());
        i2s_write(I2S_NUM_0, audioBuffer, length, &bytesWritten, portMAX_DELAY);
        // Serial.println("end: ");
        // Serial.println(millis());
        // i2s write END timestamp (i2s_write time delay)
        // transmission_time + i2s_write = latency

        // print END TIME this includes overhead from inside black box of esp
      } 
      break;
    }

    case WStype_CONNECTED:
      Serial.printf("Client connected: %s\n", payload);
      break;

    case WStype_DISCONNECTED:
      Serial.printf("Client disconnected: %s\n", payload);
      break;
  }
}