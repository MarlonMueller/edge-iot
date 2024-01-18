/**
 * @file main.c
 * @brief Entry point
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef CONFIG_HEAP_LOG
#include "heap-log/heap_log.h"
#endif

#include "audio/microphone.h"
#include "audio/preprocess.h"
#include "esp-led/esp_led.h"

#include "birdnet_model.hpp"
#include "dl_tool.hpp"

#define TAG "MAIN"

static int input_exponent = -15;

#define SAMPLE_RATE 16000
#define AUDIO_BUFFER_SIZE SAMPLE_RATE * 5

extern "C" void app_main()
{

    ESP_LOGI(TAG, "Initiating application...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    init_esp_led();
    init_i2s_mic(SAMPLE_RATE);

    ESP_LOGI(TAG, "Initialized application...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    int16_t *audio_buffer = (int16_t *)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));

    ESP_LOGI(TAG, "Allocated audio buffer...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    // Set led to red
    set_esp_led_rgb(255, 0, 0);

    ESP_ERROR_CHECK(read_i2s_mic(audio_buffer, AUDIO_BUFFER_SIZE));
    clear_esp_led();

    ESP_LOGI(TAG, "Read audio buffer...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    // Set led to green
    set_esp_led_rgb(0, 255, 0);

    // Rescale to int16_t range
    int16_t max_sample = 0;
    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; ++i)
    {
        max_sample = std::max(max_sample, static_cast<int16_t>(std::abs(audio_buffer[i])));
    }

    if (max_sample > 0)
    {
        const double scale = 32767.0 / max_sample;
        for (size_t i = 0; i < AUDIO_BUFFER_SIZE; ++i)
        {
            audio_buffer[i] = static_cast<int16_t>(audio_buffer[i] * scale);
        }
    }

    ESP_LOGI(TAG, "Preprocessed audio buffer...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    float **mfcc_output;
    size_t num_frames = 0;
    int num_mfcc = get_num_mfcc();

    malloc_mfcc_module();
    mfcc(audio_buffer, AUDIO_BUFFER_SIZE, &mfcc_output, &num_frames);
    free_mfcc_module();
    free(audio_buffer);

    ESP_LOGI(TAG, "Computed MFCCs...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    // Set led to blue
    set_esp_led_rgb(0, 0, 255);

    // TODO: num_frames: size_t -> int
    int num_frames_int = static_cast<int>(num_frames);

    int16_t *model_input = (int16_t *)malloc(num_frames_int * num_mfcc * sizeof(int16_t *));
    float min_value = std::numeric_limits<float>::max();
    float max_value = std::numeric_limits<float>::lowest();

    // Find minimum and maximum values
    for (size_t i = 0; i < num_frames; ++i)
    {
        for (size_t j = 0; j < num_mfcc; ++j)
        {
            float value = mfcc_output[i][j];
            min_value = std::min(min_value, value);
            max_value = std::max(max_value, value);
        }
    }

    // Normalize and clip values
    if (max_value != min_value)
    {
        int pos = 0;

        for (size_t i = 0; i < num_frames && pos < num_frames * num_mfcc; ++i)
        {
            for (size_t j = 0; j < num_mfcc && pos < num_frames * num_mfcc; ++j)
            {
                float normalized_input = (mfcc_output[i][j] - min_value) / (max_value - min_value);
                model_input[pos] = (int16_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);
                ++pos;
            }
        }
    }

    ESP_LOGI(TAG, "Postprocessed MFCCs...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    Tensor<int16_t> input;
    input.set_element((int16_t *)model_input).set_exponent(input_exponent).set_shape({num_frames_int, num_mfcc, 1}).set_auto_free(true);

    BIRDNET model;

    ESP_LOGI(TAG, "Initialized model...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    model.forward(input);

    ESP_LOGI(TAG, "Inferenced model...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    float *probs = model.softmax.get_output().get_element_ptr();

    for (size_t i = 0; i < 4; i++)
    {
        char *class_name = NULL;

        switch (i)
        {
        case 0:
            class_name = "Water Rail";
            break;
        case 1:
            class_name = "Cetti's Warbler";
            break;
        case 2:
            class_name = "Common Blackbird";
            break;
        case 3:
            class_name = "Other";
            break;
        }

        ESP_LOGI(TAG, "%s: %f %%", class_name, probs[i] * 100);
    }

    model.softmax.get_output().free_element();

    clear_esp_led();
    deinit_i2s_mic();

    ESP_LOGI(TAG, "Deinitialized application...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif
}