/**
 * @file main.c
 * @brief Entry point for ESP32 applicatio
 */
#include "esp_log.h"

#include "audio/microphone.h"
#include "audio/preprocess.h"
#include "lora/lora.h"

#include "dl_tool.hpp"
#include "birdnet_model.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAIN_TAG "MAIN"

// Entry point for ESP32 application
#define TEST_WAV_SIZE 44800

static int8_t s_audio[TEST_WAV_SIZE];

static int input_exponent = -7;

void task_tx(void *pvParameters)
{
    ESP_LOGI(pcTaskGetName(NULL), "Start");
    uint8_t buf[256]; // Maximum Payload size of SX1276/77/78/79 is 255
    while (1)
    {
        TickType_t nowTick = xTaskGetTickCount();
        int send_len = sprintf((char *)buf, "Hello World!! %" PRIu32, nowTick);
        lora_send_packet(buf, send_len);
        ESP_LOGI(pcTaskGetName(NULL), "%d byte packet sent...", send_len);
        int lost = lora_packet_lost();
        if (lost != 0)
        {
            ESP_LOGW(pcTaskGetName(NULL), "%d packets lost", lost);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    } // end while
}

extern "C" void app_main()
{

    /**********
    LORA Module
    ************/
/* 
    ESP_LOGI(MAIN_TAG, "Starting application");
    if (lora_init() == 0)
    {
        ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the module");
        while (1)
        {
            vTaskDelay(1);
        }
    }

#if CONFIG_169MHZ
    ESP_LOGI(pcTaskGetName(NULL), "Frequency is 169MHz");
    lora_set_frequency(169e6); // 169MHz
#elif CONFIG_433MHZ
    ESP_LOGI(pcTaskGetName(NULL), "Frequency is 433MHz");
    lora_set_frequency(433e6); // 433MHz
#elif CONFIG_470MHZ
    ESP_LOGI(pcTaskGetName(NULL), "Frequency is 470MHz");
    lora_set_frequency(470e6); // 470MHz
#elif CONFIG_866MHZ
    ESP_LOGI(pcTaskGetName(NULL), "Frequency is 866MHz");
    lora_set_frequency(866e6); // 866MHz
#elif CONFIG_915MHZ
    ESP_LOGI(pcTaskGetName(NULL), "Frequency is 915MHz");
    lora_set_frequency(915e6); // 915MHz
#elif CONFIG_OTHER
    ESP_LOGI(pcTaskGetName(NULL), "Frequency is %dMHz", CONFIG_OTHER_FREQUENCY);
    long frequency = CONFIG_OTHER_FREQUENCY * 1000000;
    lora_set_frequency(frequency);
#endif

    lora_enable_crc();

    int cr = 1;
    int bw = 7;
    int sf = 7;
#if CONFIF_ADVANCED
    cr = CONFIG_CODING_RATE
        bw = CONFIG_BANDWIDTH;
    sf = CONFIG_SF_RATE;
#endif

    lora_set_coding_rate(cr);
    // lora_set_coding_rate(CONFIG_CODING_RATE);
    // cr = lora_get_coding_rate();
    ESP_LOGI(pcTaskGetName(NULL), "coding_rate=%d", cr);

    lora_set_bandwidth(bw);
    // lora_set_bandwidth(CONFIG_BANDWIDTH);
    // int bw = lora_get_bandwidth();
    ESP_LOGI(pcTaskGetName(NULL), "bandwidth=%d", bw);

    lora_set_spreading_factor(sf);
    // lora_set_spreading_factor(CONFIG_SF_RATE);
    // int sf = lora_get_spreading_factor();
    ESP_LOGI(pcTaskGetName(NULL), "spreading_factor=%d", sf);

#if CONFIG_SENDER
    xTaskCreate(&task_tx, "TX", 1024 * 3, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
    xTaskCreate(&task_rx, "RX", 1024 * 3, NULL, 5, NULL);
#endif */

    /**********
    MICROPHONE
    ************/
    init_i2s_mic();
    record_i2s_mic(s_audio, TEST_WAV_SIZE);

    // for (int i = 0; i < TEST_WAV_SIZE; i++)
    // {
    //     ESP_LOGI(MAIN_TAG, "Value %d", s_audio[i]);
    // }

    deinit_i2s_mic();

    /**********
    MFCC
    ************/

    for (size_t i=0; i<TEST_WAV_SIZE; ++i) {
       s_audio[i] = 1.0;
    }

    int num_mfcc = 16;

    float **mfcc_output;
    size_t num_frames = 0;

    malloc_mfcc_module();
    mfcc(s_audio, TEST_WAV_SIZE, &mfcc_output, &num_frames);
    free_mfcc_module();

    // Print values

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            ESP_LOGI(MAIN_TAG, "MFCC output %d %d: %f", i, j, mfcc_output[i][j]);
        }
    }

    size_t total_elements = num_frames * num_mfcc;
    ESP_LOGI(MAIN_TAG, "MFCC num frames: %d", num_frames);

    if (mfcc_output == NULL)
    {
        ESP_LOGE(MAIN_TAG, "MFCC output is NULL");
    }

    /**********
    DL
    ************/

    int8_t *model_input = (int8_t *)malloc(total_elements * sizeof(int8_t *));

    // TODO: cleanr implementation / code convention
    float min_value = std::numeric_limits<float>::max();
    float max_value = std::numeric_limits<float>::lowest();

    // Find min and max values
    for (size_t i = 0; i < num_frames; ++i)
    {
        for (size_t j = 0; j < num_mfcc; ++j)
        {
            float value = mfcc_output[i][j];
            min_value = std::min(min_value, value);
            max_value = std::max(max_value, value);
        }
    }
    ESP_LOGI(MAIN_TAG, "Min value: %f - Max value: %f", min_value, max_value);

    // Normalize and clip values

    if (max_value != min_value)
    {
        int pos = 0;

        for (size_t i = 0; i < num_frames && pos < total_elements; ++i)
        {
            for (size_t j = 0; j < num_mfcc && pos < total_elements; ++j)
            {
                float normalized_input = (mfcc_output[i][j] - min_value) / (max_value - min_value);
                // ESP_LOGI(MAIN_TAG, "Normalized input: %f", normalized_input);
                // model_input[pos] = 0.0; // (int8_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);
                ++pos;
            }
        }
    }

    // for (int i=0; i<total_elements; ++i) {
    //     ESP_LOGI(MAIN_TAG, "Model input %d:", (int) model_input[i]);
    // }

    Tensor<int8_t> input;
    int num_frames_int = static_cast<int>(num_frames); // TODO: remove this cast
    input.set_element((int8_t *)model_input).set_exponent(input_exponent).set_shape({num_mfcc, num_frames_int, 1}).set_auto_free(false);

    BIRDNET model;
    model.forward(input);

    // ESP-DL softmax implementation fix: this->channel = input.shape[0];
    float *probs = model.softmax.get_output().get_element_ptr();

    float max_prob = probs[0];
    int max_index = 0;

    for (size_t i = 1; i < 4; i++)
    {
        ESP_LOGI(MAIN_TAG, "Prob %d: %f", i, probs[i]);
        if (probs[i] > max_prob)
        {
            max_prob = probs[i];
            max_index = i;
        }
    }

    switch (max_index)
    {
    case 0:
        ESP_LOGI(MAIN_TAG, "CLASS 0");
        break;
    case 1:
        ESP_LOGI(MAIN_TAG, "CLASS 1");
        break;
    case 2:
        ESP_LOGI(MAIN_TAG, "CLASS 2");
        break;
    case 3:
        ESP_LOGI(MAIN_TAG, "CLASS 3");
        break;
    default:
        ESP_LOGE(MAIN_TAG, "");
    }

    model.softmax.get_output().free_element();

    /**********
    MFCC
    ************/

    if (mfcc_output != NULL)
    {
        for (int i = 0; i < num_frames; ++i)
        {
            free(mfcc_output[i]);
        }

        free(mfcc_output);
    }
}