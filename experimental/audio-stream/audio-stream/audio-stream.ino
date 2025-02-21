#include <WiFi.h>
#include <WebSocketsServer.h>
#include <driver/i2s.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Fardin's iPhone";
const char* password = "freewifi";

// I2S pins
#define I2S_BCLK_PIN    15
#define I2S_LRC_PIN     16
#define I2S_DOUT_PIN    17
#define SAMPLE_RATE     48000
#define DMA_BUF_COUNT   4 // modify for experiments (try 4) or x*2
#define DMA_BUF_LEN     32 // modify for experiments (try 32) or x*2

#define USE_WIFI_PACKAGE 1

// Buffer to hold received audio data
int16_t audioBuffer[512]; // modify for experiments

// WebSocket server on port 81
WebSocketsServer webSocket = WebSocketsServer(81);

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
    WiFi.setSleep(false);
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
      // Serial.println(F("in payload"));
      if (length % sizeof(int16_t) == 0) {
        if (!payload || length == 0 || length > 1024) {  // Ensure payload is valid
          Serial.println("Invalid payload received!");
          return;
        }
        memcpy(audioBuffer, payload, min(length, sizeof(audioBuffer))); // payload, length
        size_t bytesWritten = 0; // put in directly instead of into auido buffer 
        esp_err_t err = i2s_write(I2S_NUM_0, payload, length, &bytesWritten, 10 / portTICK_PERIOD_MS); // pass into audio buffer instead of payload

        if (err != ESP_OK) {
            Serial.println(F("I2S write failed!"));
        } 
        // free(audioBuffer);
        memset(audioBuffer, 0, sizeof(audioBuffer));
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