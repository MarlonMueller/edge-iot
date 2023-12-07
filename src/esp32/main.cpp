/**
 * @file main.c
 * @brief Entry point for ESP32 applicatio
 */
#include "esp_log.h"

#include "audio/microphone.h"
#include "audio/preprocess.h"

#include "dl_tool.hpp"
#include "birdnet_model.hpp"

#define MAIN_TAG "MAIN"

// Entry point for ESP32 application
#define TEST_WAV_SIZE 16000

static float s_audio[TEST_WAV_SIZE];

static int input_exponent = -7;

extern "C" void app_main()
{
    ESP_LOGI(MAIN_TAG, "Starting application");

    /**********
    MICROPHONE
    ************/
    init_i2s_mic();
    record_i2s_mic(s_audio, TEST_WAV_SIZE);

    // for (int i = 0; i < TEST_WAV_SIZE; i++)
    // {
    //     ESP_LOGI(MAIN_TAG, "Value %f", s_audio[i]);
    // }

    deinit_i2s_mic();

    /**********
    MFCC
    ************/

    int num_mfcc = 32;

    float **mfcc_output;
    size_t num_frames = 0;

    malloc_mfcc_module();
    mfcc(s_audio, TEST_WAV_SIZE, &mfcc_output, &num_frames);
    free_mfcc_module();

    size_t total_elements = num_frames * num_mfcc;
    ESP_LOGI(MAIN_TAG, "MFCC num frames: %d", num_frames);

    if (mfcc_output == NULL)
    {
        ESP_LOGE(MAIN_TAG, "MFCC output is NULL");
    }

    // for (int i = 0; i < mfcc_size; i++)
    // {
    //     ESP_LOGI(MAIN_TAG, "Frame %d", i);

    //     for (int j = 0; j < 5; j++)
    //     {
    //         ESP_LOGI(MAIN_TAG, "MFCC %d: %f", j, mfcc_output[i][j]);
    //     }
    // }

    /**********
    DL
    ************/

    int8_t *model_input = (int8_t *)dl::tool::malloc_aligned_prefer(total_elements, sizeof(int8_t *));

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
    // ESP_LOGI(MAIN_TAG, "Min value: %f - Max value: %f", min_value, max_value);

    // Normalize and clip values
    int pos = 0;
    for (size_t i = 0; i < num_frames && pos < total_elements; ++i)
    {
        for (size_t j = 0; j < 32 && pos < total_elements; ++j)
        {
            float normalized_input = (mfcc_output[i][j] - min_value) / (max_value - min_value);
            model_input[pos] = (int8_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);
            ++pos;
        }
    }

    Tensor<int8_t> input;
    int num_frames_int = static_cast<int>(num_frames); // TODO: remove this cast
    input.set_element((int8_t *)model_input).set_exponent(input_exponent).set_shape({num_mfcc, num_frames_int, 1}).set_auto_free(false);

    BIRDNET model;
    model.forward(input);

    float *probs = model.softmax.get_output().get_element_ptr();

    for (int i = 0; i < 4; ++i)
    {
        ESP_LOGI(MAIN_TAG, "Prob %d: %f", i, probs[i]);
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