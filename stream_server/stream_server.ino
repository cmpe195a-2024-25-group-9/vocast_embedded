#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/i2s.h>
#include <WiFiManager.h>

// WiFi config constants
#define USE_WIFI_PACKAGE 1
#define RESET_WIFI 1

// I2S pins and config
#define I2S_BCLK_PIN    15
#define I2S_LRC_PIN     16
#define I2S_DOUT_PIN    17
#define SAMPLE_RATE     48000
#define DMA_BUF_COUNT   8
#define DMA_BUF_LEN     64  // Keep small buffer size from working test

// UDP config
#define UDP_PORT        12345
#define UDP_BUFFER_SIZE 1024

// Ring buffer for audio samples
#define RING_BUFFER_SIZE 4096
uint8_t ringBuffer[RING_BUFFER_SIZE];
volatile size_t writePos = 0;
volatile size_t readPos = 0;

WiFiUDP udp;
uint8_t udpBuffer[UDP_BUFFER_SIZE];
int packetCount = 0;

// Add to ring buffer
void addToRingBuffer(uint8_t* data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    ringBuffer[writePos] = data[i];
    writePos = (writePos + 1) % RING_BUFFER_SIZE;
    
    // If buffer overrun, move read position
    if (writePos == readPos) {
      readPos = (readPos + 1) % RING_BUFFER_SIZE;
    }
  }
}

// Get available data amount
size_t availableData() {
  if (writePos >= readPos) {
    return writePos - readPos;
  } else {
    return RING_BUFFER_SIZE - readPos + writePos;
  }
}

// Read from ring buffer
size_t readFromRingBuffer(uint8_t* dest, size_t maxLen) {
  size_t available = availableData();
  size_t bytesToRead = min(available, maxLen);
  
  for (size_t i = 0; i < bytesToRead; i++) {
    dest[i] = ringBuffer[readPos];
    readPos = (readPos + 1) % RING_BUFFER_SIZE;
  }
  
  return bytesToRead;
}

void setup() {
  Serial.begin(115200);

  if (USE_WIFI_PACKAGE) {
    WiFiManager wifiManager;
    if (RESET_WIFI) {
      wifiManager.resetSettings();
      delay(1000);
    }
    wifiManager.autoConnect("esp");
    Serial.println(F("Connected to WiFi"));
  } else {
    WiFi.begin("your-ssid", "your-password");
    WiFi.setSleep(false);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(F("Connected to WiFi"));
  }

  Serial.print(F("ESP32 IP Address: "));
  Serial.println(WiFi.localIP());

  // Start UDP listener
  udp.begin(UDP_PORT);
  Serial.printf("UDP listening on port %d\n", UDP_PORT);

  // I2S configuration - use the same settings that worked with the self-test
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = true,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
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
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.printf("Received UDP packet of size %d bytes\n", packetSize);

    int len = udp.read(udpBuffer, UDP_BUFFER_SIZE);

    if (len % 4 != 0) {
      Serial.printf("Misaligned packet size: %d\n", len);
      return; // or trim/pad to nearest 4-byte boundary
    }

    int16_t* samples = (int16_t*)udpBuffer;
    Serial.printf("Sample L: %d, R: %d\n", samples[0], samples[1]);

    if (len > 0) {
      Serial.printf("Read %d bytes from UDP\n", len);
      size_t bytesWritten;
      esp_err_t res = i2s_write(I2S_NUM_0, udpBuffer, len, &bytesWritten, portMAX_DELAY);
      if (res != ESP_OK) {
        Serial.println("I2S Write Error");
      } else {
        Serial.printf("Wrote %d bytes to I2S\n", bytesWritten);
      }
    } 
  }
}