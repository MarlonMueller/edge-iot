/**
 * @file main.c
 * @brief Entry point for ESP32 applicatio
 */
#include "esp_log.h"

#include "audio/microphone.h"
#include "audio/preprocess.h"
#include "lora/lora.h"

#include "dl_tool.hpp"
#include "birdnet_model_tf.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "audio.c"

#define MAIN_TAG "MAIN"

// Entry point for ESP32 application
#define TEST_WAV_SIZE 48000 // 16000 * 3 seconds #FIXME -
#define MAX_TX_SIZE 255     // bytes. Max size of a LoRa packet

// static int8_t s_audio[TEST_WAV_SIZE];
static uint8_t s_tx_data[MAX_TX_SIZE];

static int input_exponent = -15;

static const int num_frames = 183;
static const int num_mfcc = 32;
static int16_t model_input[num_frames * num_mfcc];

extern "C" void app_main()
{
    ESP_LOGI(MAIN_TAG, "Starting application");

    // /**********
    // LORA Module
    // ************/

    // setup_lora_comm();
    // task_tx(s_tx_data, MAX_TX_SIZE);

    /**********
    MICROPHONE
    ************/
    // init_i2s_mic();

    // record_i2s_mic(s_audio, TEST_WAV_SIZE);

    // ESP_LOGI(MAIN_TAG, "Finished recording audio");

    // for (int i = 50; i < 70; i++) {
    //     ESP_LOGI(MAIN_TAG, "Value %d", s_audio[i]);
    // }

    // deinit_i2s_mic();

    /**********
    MFCC
    ************/

    // for (size_t i = 0; i < TEST_WAV_SIZE; ++i)
    // {
    //     s_audio[i] = 1.0;
    // }

    // int num_mfcc = get_num_mfcc();

    // float **mfcc_output;
    // size_t num_frames = 0;

    // malloc_mfcc_module();
    // mfcc(s_audio, TEST_WAV_SIZE, &mfcc_output, &num_frames);
    // free_mfcc_module();

    // // Print values

    // for (int i = 0; i < 5; ++i)
    // {
    //     for (int j = 0; j < 5; ++j)
    //     {
    //         ESP_LOGI(MAIN_TAG, "MFCC output %d %d: %f", i, j, mfcc_output[i][j]);
    //     }
    // }

    size_t total_elements = num_frames * num_mfcc;
    // ESP_LOGI(MAIN_TAG, "MFCC num frames: %d", num_frames);

    // if (mfcc_output == NULL)
    // {
    //     ESP_LOGE(MAIN_TAG, "MFCC output is NULL");
    // }

    /**********
    DL
    ************/

    for (int i = 0; i < num_frames * num_mfcc; ++i)
    {
        model_input[i] = (int16_t)DL_CLIP(s_audio[i] * (1 << -input_exponent), -32768, 32767);
    }

    // Expected input size [1, 32, 188]

    // ESP_LOGI(MAIN_TAG, "Total elements: %d", num_frames);

    // int16_t *model_input = (int16_t *)malloc(total_elements * sizeof(int16_t *));

    // // TODO: cleanr implementation / code convention
    // float min_value = std::numeric_limits<float>::max();
    // float max_value = std::numeric_limits<float>::lowest();

    // Find min and max values
    // for (size_t i = 0; i < num_frames; ++i)
    // {
    //     for (size_t j = 0; j < num_mfcc; ++j)
    //     {
    //         float value = mfcc_output[i][j];
    //         min_value = std::min(min_value, value);
    //         max_value = std::max(max_value, value);
    //     }
    // }
    // ESP_LOGI(MAIN_TAG, "Min value: %f - Max value: %f", min_value, max_value);

    // Normalize and clip values

    // if (max_value != min_value)
    // {
    //     int pos = 0;

    //     for (size_t i = 0; i < num_frames && pos < total_elements; ++i)
    //     {
    //         for (size_t j = 0; j < num_mfcc && pos < total_elements; ++j)
    //         {
    //             float normalized_input = (mfcc_output[i][j] - min_value) / (max_value - min_value);
    //             // ESP_LOGI(MAIN_TAG, "Normalized input: %f", normalized_input);
    //             model_input[pos] = (int16_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);
    //             ++pos;
    //         }
    //     }
    // }

    // for (int i=0; i<total_elements; ++i) {
    //     ESP_LOGI(MAIN_TAG, "Model input %d:", (int) model_input[i]);
    // }

    Tensor<int16_t> input;
    int num_frames_int = static_cast<int>(num_frames); // TODO: remove this cast
    input.set_element((int16_t *)model_input).set_exponent(input_exponent).set_shape({num_frames_int, num_mfcc, 1}).set_auto_free(false);

    BIRDNET model;

    // dl::tool::Latency latency;
    // latency.start();

    model.forward(input);

    // latency.end();

    // ESP-DL softmax implementation fix: this->channel = input.shape[0];
    float *probs = model.softmax.get_output().get_element_ptr();

    float max_prob = probs[0];
    int max_index = 0;

    for (size_t i = 0; i < 4; i++)
    {
        ESP_LOGI(MAIN_TAG, "Prob %d: %f %%", i, probs[i] * 100);
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

    // if (mfcc_output != NULL)
    // {
    //     for (int i = 0; i < num_frames; ++i)
    //     {
    //         free(mfcc_output[i]);
    //     }

    //     free(mfcc_output);
    // }
}