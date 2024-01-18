/**
 * @file microphone.c
 * @brief Microphone audio recording functions
 *
 * This file contains the implementation of the audio recording functions.
 */
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"

#include "driver/i2s.h"

#include "audio/microphone.h"

#define TAG "MICROPHONE"

#define BUFFER_SIZE 512
i2s_chan_handle_t rx_handle;

void init_i2s_mic(uint32_t sample_rate)
{

    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL));

    i2s_pin_config_t i2s_pin_config = {
        .bck_io_num = SCK_PIN,
        .ws_io_num = WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = SD_PIN,
    };

    // SPH0645(?)
    // REG_SET_BIT(I2S_RX_TIMING_REG(I2S_PORT), BIT(1)); // Delay by falling edge
    // REG_SET_BIT(I2S_RX_CONF1_REG(I2S_PORT), I2S_RX_MSB_SHIFT); // Philips mode

    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &i2s_pin_config));
}

esp_err_t read_i2s_mic(int16_t *buffer, size_t samples)
{

    /*
     * Note: I2S reads 32-bit samples, but microphone is 24-bit.
     * We read the upper 16 bits and discard the rest (noise and padding).
     */

    char *buf_ptr = (char *)buffer;
    int32_t *rx_buff = (int32_t *)malloc(BUFFER_SIZE * sizeof(int32_t));

    while (samples > 0)
    {
        size_t bytes_read = 0;
        size_t chunk = (samples > BUFFER_SIZE) ? BUFFER_SIZE : samples;

        char *rx_buff_iter = (char *)rx_buff;
        if (i2s_read(I2S_NUM_0, rx_buff_iter, chunk * sizeof(int32_t), &bytes_read, portMAX_DELAY) == ESP_OK)
        {

            size_t samples_read = bytes_read / sizeof(int32_t);

            for (int i = 0; i < samples_read; i++)
            {

                buf_ptr[0] = rx_buff_iter[2];
                buf_ptr[1] = rx_buff_iter[3];

                rx_buff_iter += sizeof(int32_t);
                buf_ptr += sizeof(int16_t);
            }
            samples -= samples_read;
        }
    }

    free(rx_buff);
    return ESP_OK;
}

void deinit_i2s_mic(void)
{
    ESP_ERROR_CHECK(i2s_driver_uninstall(I2S_NUM_0));
}