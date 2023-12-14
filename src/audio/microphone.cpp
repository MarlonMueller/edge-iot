/**
 * @file microphone.c
 * @brief Microphone audio recording functions
 * 
 * This file contains the implementation of the audio recording functions.
*/

#include "audio/microphone.h"

#include "driver/i2s.h"
#include "soc/i2s_reg.h"

#include "esp_log.h"

#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE (16000)
#define I2S_READ_LEN (16 * 1024)

#define IGNORE_FIRST_N_VALUES 3

#define SOUND_INITIALIZE_THRESHOLD 50

#define TAG "MICROPHONE"

void init_i2s_mic() 
{
    // Parameters definitions. 
    // Constants are defined in audio/microphone.h, drivers/i2s.h 
    // and soc/i2s_reg.h 

    const i2s_config_t micCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 64,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t micPins = {
        .bck_io_num = BCLK_PIN,
        .ws_io_num = WS_PIN,
        .data_out_num = -1,
        .data_in_num = DATA_PIN,
    };

    // Setup

    i2s_driver_install(I2S_PORT, &micCfg, 0, NULL);

    REG_SET_BIT(I2S_RX_TIMING_REG(I2S_PORT), BIT(1)); // Delay by falling edge
    REG_SET_BIT(I2S_RX_CONF1_REG(I2S_PORT), I2S_RX_MSB_SHIFT); // Philips mode

    i2s_set_pin(I2S_PORT, &micPins);
}

void record_i2s_mic(int8_t *buffer, int num_values) 
{
    int pos = 0;
    char *i2s_read_buff = (char *)calloc(I2S_READ_LEN, sizeof(char));
    size_t bytes_read = 0;

    // Ignore first instants

    for (int i=0; i<IGNORE_FIRST_N_VALUES; ++i) {
        i2s_read(I2S_PORT, (void *)i2s_read_buff, I2S_READ_LEN, &bytes_read, portMAX_DELAY);
    }

    // Trigger only when there is strong enough signal

    ESP_LOGI(TAG, "Waiting for recording.");

    int flag = 0;

    while (flag != 1) {
        i2s_read(I2S_PORT, (void *)i2s_read_buff, I2S_READ_LEN, &bytes_read, portMAX_DELAY);

        for (int i=0; i<bytes_read && pos < num_values; ++i) {
            int8_t value = abs((int8_t) i2s_read_buff[i]);
            
            if (value > SOUND_INITIALIZE_THRESHOLD) {
                flag = 1;
            }
        }
    }

    // Read values

    ESP_LOGI(TAG, "Recording audio");
    
    while (pos < num_values) {

        i2s_read(I2S_PORT, (void *)i2s_read_buff, I2S_READ_LEN, &bytes_read, portMAX_DELAY);

        ESP_LOGI(TAG, "Bytes read: %d", bytes_read);

        for (int i=0; i<bytes_read && pos < num_values; ++i) {
            uint8_t value = (uint8_t) i2s_read_buff[i];
            
            buffer[pos] = value;
            ++pos;
        }
    }

    free(i2s_read_buff);
}

void deinit_i2s_mic()
{
    
}