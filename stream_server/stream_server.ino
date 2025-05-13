#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/i2s.h>
#include <WiFiManager.h>

// WiFi config constants
#define USE_WIFI_PACKAGE 1
#define RESET_WIFI 0

#define WIFI_SSID ""
#define WIFI_PASS ""

// I2S pins and config
#define AMP_ENABLE_PIN  13
#define I2S_BCLK_PIN    15
#define I2S_LRC_PIN     16
#define I2S_DOUT_PIN    17
#define SAMPLE_RATE     48000
#define DMA_BUF_COUNT   4
#define DMA_BUF_LEN     256

// UDP config
#define UDP_PORT        12345
#define PACKET_SIZE     1024

uint8_t packetBuffer[PACKET_SIZE];
WiFiUDP udp;
int packetCount = 0;

void setup() {
  Serial.begin(115200);

  pinMode(AMP_ENABLE_PIN, OUTPUT);
  digitalWrite(AMP_ENABLE_PIN, HIGH);

  if (USE_WIFI_PACKAGE) {
    WiFiManager wifiManager;
    if (RESET_WIFI) {
      wifiManager.resetSettings();
      delay(1000);
    }
    wifiManager.autoConnect("esp");
    Serial.println(F("Connected to WiFi"));
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    WiFi.setSleep(false);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(F("Connected to WiFi"));
  }

  Serial.print(F("ESP32 IP Address: "));
  Serial.println(WiFi.localIP());

  // I2S configuration
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT ,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = DMA_BUF_COUNT,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = false,
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
  // Start UDP listener
  udp.begin(UDP_PORT);
  Serial.printf("UDP listening on port: %d\n", UDP_PORT);
  // broadcast ip
  udp.beginPacket("255.255.255.255", 12345);
  udp.print("ESP IP: " + WiFi.localIP().toString());
  udp.endPacket();
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    packetCount++;
    /* if (packetCount % 500 == 0) { 
      Serial.println(F("packet received"));
    } */
    size_t bytes_written;
    int32_t sample = (int32_t)udp.read(packetBuffer, PACKET_SIZE) << 16;
    i2s_write(I2S_NUM_0, packetBuffer, sample, &bytes_written, portMAX_DELAY);
  }
}