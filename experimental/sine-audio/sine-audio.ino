#include <driver/i2s.h>
#include <math.h>

// I2S Pin Configuration
#define I2S_BCLK_PIN    15  // Bit Clock (BCLK)
#define I2S_LRC_PIN     16  // Left/Right Clock (LRC)
#define I2S_DOUT_PIN    17  // Data Out (DIN)

// Audio Settings
#define SAMPLE_RATE     44100  // Sampling rate in Hz
#define WAVE_FREQUENCY  440    // Frequency of the sine wave (440 Hz = A4)

// Buffers
const int AMPLITUDE = 1500;  // Sine wave amplitude (volume)
const int SAMPLE_COUNT = SAMPLE_RATE / WAVE_FREQUENCY; // Samples per sine wave cycle
int16_t samples[SAMPLE_COUNT]; // Buffer to hold sine wave samples

void setup() {
    Serial.begin(9600);
    Serial.println("Starting I2S Sine Wave Generator");

    // I2S Configuration
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX), // Master mode, Transmitter
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // Stereo
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
        .dma_buf_count = 8,
        .dma_buf_len = 64
    };

    // Install and start I2S driver
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK_PIN,
        .ws_io_num = I2S_LRC_PIN,
        .data_out_num = I2S_DOUT_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);

    // Generate a sine wave
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        samples[i] = AMPLITUDE * sin(2.0 * PI * i / SAMPLE_COUNT); // Generate sine wave sample
    }

    Serial.println("I2S Initialized, Playing Tone...");
}

void loop() {
    // Write sine wave samples to the I2S amplifier
    size_t bytes_written;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        i2s_write(I2S_NUM_0, &samples[i], sizeof(int16_t), &bytes_written, portMAX_DELAY);
    }
}
