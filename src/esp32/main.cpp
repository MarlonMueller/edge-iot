/**
 * @file main.c
 * @brief Entry point
 */


#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <time.h>
#include <sys/time.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

#ifdef CONFIG_HEAP_LOG
#include "heap-log/heap_log.h"
#endif

#include "audio/microphone.h"
#include "audio/preprocess.h"
#include "esp-led/esp_led.h"
#include "lora/lora_esp32_logic.h"

#include "dl_tool.hpp"


#ifdef CONFIG_QUANTIZATION_BITS_16
#include "birdnet_extended_int16.hpp"
#else
#include "birdnet_extended_int8.hpp"
#endif

// #ifdef CONFIG_QUANTIZATION_BITS_16
// #include "birdnet_default_int16.hpp"
// #else
// #include "birdnet_default_int8.hpp"
// #endif

#define TAG "MAIN"

static int input_exponent = -7;


#define SAMPLE_RATE 16000
#define AUDIO_BUFFER_SIZE SAMPLE_RATE * 5

static gpio_num_t wakeup_pin = GPIO_NUM_4;

static RTC_DATA_ATTR struct timeval last_lora_wakeup;
static RTC_DATA_ATTR int wakeup_lora_us = 15 * 60 * 1000000L;

extern "C" void record_and_infer_sound()
{

    ESP_LOGI(TAG, "Initiating record_and_infer_sound...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    init_esp_led();
    init_i2s_mic(SAMPLE_RATE);


    ESP_LOGI(TAG, "Initialized record_and_infer_sound...");
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

    // Set led to green
    set_esp_led_rgb(0, 255, 0);

    #ifdef CONFIG_TIME_LOG
    dl::tool::Latency latency;
    latency.start();
    #endif

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

    float **mfcc_output;
    size_t num_frames = 0;
    int num_mfcc = get_num_mfcc();

    malloc_mfcc_module();

    ESP_LOGI(TAG, "Allocated MFCC module...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif
    
    mfcc(audio_buffer, AUDIO_BUFFER_SIZE, &mfcc_output, &num_frames);

    ESP_LOGI(TAG, "MFCC module...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    free_mfcc_module();
    free(audio_buffer);

    ESP_LOGI(TAG, "Computed MFCCs...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    // Set led to blue
    set_esp_led_rgb(0, 0, 255);

    int num_frames_int = static_cast<int>(num_frames);

    #ifdef CONFIG_QUANTIZATION_BITS_16
    int16_t *model_input = (int16_t *)malloc(num_frames_int * num_mfcc * sizeof(int16_t));
    #else
    int8_t *model_input = (int8_t *)malloc(num_frames_int * num_mfcc * sizeof(int8_t));
    #endif
    
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

                #ifdef CONFIG_QUANTIZATION_BITS_16
                model_input[pos] = (int16_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);
                #else
                model_input[pos] = (int8_t)DL_CLIP(normalized_input * (1 << -input_exponent), -128, 127);
                #endif
                
                ++pos;
            }
        }
    }

    for (size_t i = 0; i < num_frames; ++i)
    {
        free(mfcc_output[i]);
    }
    free(mfcc_output);

    #ifdef CONFIG_TIME_LOG
    latency.end();
    latency.print("MFCC");
    #endif

    ESP_LOGI(TAG, "Postprocessed MFCCs...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    #ifdef CONFIG_QUANTIZATION_BITS_16
    Tensor<int16_t> input;
    input.set_element((int16_t *)model_input).set_exponent(input_exponent).set_shape({num_frames_int, num_mfcc, 1}).set_auto_free(true);
    #else
    Tensor<int8_t> input;
    input.set_element((int8_t *)model_input).set_exponent(input_exponent).set_shape({num_frames_int, num_mfcc, 1}).set_auto_free(true);
    #endif

    // #ifdef CONFIG_QUANTIZATION_BITS_16
    // BIRDNET_DEFAULT_INT16 model;
    // #else
    // BIRDNET_DEFAULT_INT8 model;
    // #endif

    #ifdef CONFIG_QUANTIZATION_BITS_16
    BIRDNET_EXTENDED_INT16 model;
    #else
    BIRDNET_EXTENDED_INT8 model;
    #endif

    ESP_LOGI(TAG, "Initialized model...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    #ifdef CONFIG_TIME_LOG
    latency.start();
    #endif

    model.forward(input);

    #ifdef CONFIG_TIME_LOG
    latency.end();
    latency.print("BirdNET");
    #endif

    ESP_LOGI(TAG, "Inferenced model...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    float *probs = model.softmax.get_output().get_element_ptr();

    for (size_t i = 0; i < 4; i++)
    {
        const char* class_name = "";

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

    // Set LoRa activation

    int best_index = 0;
    float best_value = probs[best_index];

    for (int i=1; i<4; ++i) {
        if (probs[i] > best_value) {
            best_index = i;
            best_value = probs[i];
        }
    }

    set_activation(best_index);

    model.softmax.get_output().free_element();

    ESP_LOGI(TAG, "Freed model...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif

    clear_esp_led();
    deinit_i2s_mic();

    ESP_LOGI(TAG, "Deinitialized application...");
    #ifdef CONFIG_HEAP_LOG
    log_heap();
    #endif
}



extern "C" void app_main(void)
{

    setup_lora_comm();

    // Enabling EXT0 button wakeup
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(wakeup_pin, 1));
    ESP_ERROR_CHECK(rtc_gpio_pullup_dis(wakeup_pin));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(wakeup_pin));

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_EXT0: {
            ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_EXT0");
            
            record_and_infer_sound();

            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
            int64_t time_us = (int64_t)(tv_now.tv_sec - last_lora_wakeup.tv_sec) * 1000000L + (int64_t)(tv_now.tv_usec - last_lora_wakeup.tv_usec);
            time_us = wakeup_lora_us - time_us;
        
            if (time_us > 0) {
                ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(time_us)); 
                break;
            }
            // If time_us is negative, also send the data
        }
        case ESP_SLEEP_WAKEUP_TIMER: {
            ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_TIMER");

            if (!is_initialized_comm()) {
                ESP_LOGW(TAG, "LoRa not initialized, initializing...");
                initialize_comm();
            }

            uint8_t timer = 0;
            
            send_data(&timer);
            ESP_LOGI(TAG, "Next wakeup in %d minutes", timer);

            if (timer == 0) {
                ESP_LOGW(TAG, "No timer received, setting to 15 minutes");
                timer = 15;
            }

            int64_t time_us = timer * 60 * 1000000L;

            ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(time_us));
            gettimeofday(&last_lora_wakeup, NULL);

            
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_UNDEFINED");

            if (!is_initialized_comm()) {
                ESP_LOGW(TAG, "LoRa not initialized, initializing...");
                initialize_comm();
            }
            
            ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_lora_us)); 
            gettimeofday(&last_lora_wakeup, NULL);
    }

    // ESP_LOGI(TAG, "Entering deep sleep in 10 seconds...");
    // vTaskDelay(10000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Entering deep sleep...");
    esp_deep_sleep_start();
}