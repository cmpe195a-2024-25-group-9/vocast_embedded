#include <driver/i2s.h>
#include <LittleFS.h>  // Include the LittleFS library

#define I2S_BCLK_PIN    15
#define I2S_LRC_PIN     16
#define I2S_DOUT_PIN    17

#define SAMPLE_RATE     44100

void setup() {
    Serial.begin(115200);
    if (!LittleFS.begin(true)) {  // Mount LittleFS instead of SPIFFS
        Serial.println("Failed to mount LittleFS \n");
        while (true);
    }

    // I2S Configuration
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64
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
    // Open the audio file from LittleFS
    File audioFile = LittleFS.open("/fatiha_1.wav", "r");
    if (!audioFile) {
        Serial.println("Failed to open /fatiha.wav");
        
        File root = LittleFS.open("/");
        File file = root.openNextFile();

        while (file) {
            Serial.print("File: ");
            Serial.print(file.name());
            Serial.print(" | Size: ");
            Serial.println(file.size());
            file = root.openNextFile();
        }
        return;
    }
    File root = LittleFS.open("/");
    File file = root.openNextFile();

    while (file) {
        Serial.print("File: ");
        Serial.print(file.name());
        Serial.print(" | Size: ");
        Serial.println(file.size());
        file = root.openNextFile();
    }

    // Skip the WAV file header (44 bytes), but adjust this for MP3 file handling
    // Since we're working with MP3, you'd typically need an MP3 decoder, 
    // but this part assumes the file format is already ready for I2S.
    audioFile.seek(44, SeekSet);  // Skip the header (if applicable for non-MP3 files)

    size_t bytesRead;
    int16_t audioBuffer[128]; // Buffer for audio data
    size_t bytesWritten;

    while (audioFile.available()) {
        bytesRead = audioFile.read((uint8_t*)audioBuffer, sizeof(audioBuffer));
        
        // Send audio data to I2S
        i2s_write(I2S_NUM_0, audioBuffer, bytesRead, &bytesWritten, portMAX_DELAY);
    }

    audioFile.close();
    delay(1000); // Pause before restarting
}