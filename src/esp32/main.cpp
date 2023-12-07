/**
 * @file main.c
 * @brief Entry point for ESP32 applicatio
 */
#include "esp_log.h"

#include "audio/microphone.h"
#include "audio/preprocess.h"

#include "dl_tool.hpp"

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

    init_i2s_mic();

    record_i2s_mic(s_audio, TEST_WAV_SIZE);
    
    // for (int i = 0; i < TEST_WAV_SIZE; i++) {
    //     ESP_LOGI(MAIN_TAG, "Value %f", s_audio[i]);
    // }

    deinit_i2s_mic();
    
    float **mfcc_output;
    size_t num_frames = 0;

    malloc_mfcc_module();
    mfcc(s_audio, TEST_WAV_SIZE, &mfcc_output, &num_frames);
    free_mfcc_module();

    size_t total_elements = 32 * num_frames;

    int16_t *model_input = (int16_t *)dl::tool::malloc_aligned_prefer(total_elements, sizeof(int16_t *));

    int pos = 0;

    for (size_t i = 0; i < num_frames && pos < total_elements; ++i) {
        
        // 32 is for the number of MFCC coefficients
        
        for (size_t j = 0; j < 32 && pos < total_elements; ++j) {
            float normalized_input = mfcc_output[i][j] / 255.0; // Fix normalization

            model_input[pos] = (int16_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);

            ++pos;
        }
    }

    Tensor<int16_t> input;
    input.set_element((int16_t *)model_input).set_exponent(input_exponent).set_shape({mfcc_size, mfcc_output, 1}).set_auto_free(false);

    BIRDNET model;

    dl::tool::Latency latency;

    latency.start();
    // model.build(input)
    model.forward(input);
    latency.end();

    latency.print("BIRDNET", "forward");

    int16_t *score = model.softmax.get_output().get_element_ptr();
    int16_t max_score = score[0];
    int max_index = 0;
    printf("%d, ", max_score);

    for (size_t i = 1; i < 10; i++)
    {
        printf("%d, ", score[i]);
        if (score[i] > max_score)
        {
            max_score = score[i];
            max_index = i;
        }
    }
    printf("\nPrediction Result: %d\n", max_index);
}