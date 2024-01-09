/**
 * @file main.c
 * @brief Entry point for ESP32 applicatio
 */
#include "esp_log.h"
#include "audio/microphone.h"

// #include "audio/preprocess.h"
// #include "lora/lora.h"

#include "esp_led/esp_led.h"

#include "wifi/wifi.h"
#include "websocket/websocket.h"

// #include "dl_tool.hpp"
// #include "birdnet_model_tf.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "audio.c"

#define TAG "MAIN"

// Entry point for ESP32 application
// #define TEST_WAV_SIZE 16000 * 3// 16000 * 3 seconds #FIXME -
// #define MAX_TX_SIZE 255     // bytes. Max size of a LoRa packet

static int input_exponent = -15;

// static const int num_frames = 183;
// static const int num_mfcc = 32;
// static int16_t model_input[num_frames * num_mfcc];

#define SAMPLE_RATE 16000
#define AUDIO_BUFFER_SIZE SAMPLE_RATE * 3

extern "C" void app_main()
{

    // Larger app

    ESP_LOGI(TAG, "Startup...");
    // ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
    // ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    // esp_log_level_set("*", ESP_LOG_INFO);

    // i2s_main();

    // /**********
    // LORA Module
    // ************/

    // setup_lora_comm();
    // task_tx(s_tx_data, MAX_TX_SIZE);

    /**********
    MICROPHONE
    ************/

    init_esp_led();
    // set_esp_led_rgb(0,135,68);

    // set_esp_led_rgb(60, 35, 0);
    ESP_ERROR_CHECK(wifi_connect());

    esp_websocket_client_handle_t client;
    client = websocket_connect();

    init_i2s_mic(SAMPLE_RATE);

    int16_t *audio_buffer = (int16_t *)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));

    set_esp_led_rgb(100, 100, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(read_i2s_mic(audio_buffer, AUDIO_BUFFER_SIZE));
    clear_esp_led();

    websocket_send(client, (char *)audio_buffer, AUDIO_BUFFER_SIZE * sizeof(int16_t));

    /***/

    // set_esp_led_rgb(0, 0, 100);

    // int num_mfcc = get_num_mfcc();

    // float **mfcc_output;
    // size_t num_frames = 0;

    // // FInd max value in audio buffer

    // malloc_mfcc_module();
    // mfcc(audio_buffer, AUDIO_BUFFER_SIZE, &mfcc_output, &num_frames);
    // free_mfcc_module();

    // size_t total_elements = num_frames * num_mfcc;
    // ESP_LOGI(TAG, "MFCC num frames: %d", num_frames);

    // if (mfcc_output == NULL)
    // {
    //     ESP_LOGE(TAG, "MFCC output is NULL");
    // }

    /**********
    DL
    ************/

    // for (int i = 0; i < num_frames * num_mfcc; ++i)
    // {
    //     model_input[i] = (int16_t)DL_CLIP(s_audio[i] * (1 << -input_exponent), -32768, 32767);
    // }

    // Expected input size [1, 32, 188]

    // ESP_LOGI(TAG, "Total elements: %d", num_frames);

    // int16_t *model_input = (int16_t *)malloc(total_elements * sizeof(int16_t *));

    // // TODO: cleanr implementation / code convention
    // float min_value = std::numeric_limits<float>::max();
    // float max_value = std::numeric_limits<float>::lowest();

    // // Find min and max values
    // for (size_t i = 0; i < num_frames; ++i)
    // {
    //     for (size_t j = 0; j < num_mfcc; ++j)
    //     {
    //         float value = mfcc_output[i][j];
    //         min_value = std::min(min_value, value);
    //         max_value = std::max(max_value, value);
    //     }
    // }
    // ESP_LOGI(TAG, "Min value: %f - Max value: %f", min_value, max_value);

    // // Normalize and clip values

    // if (max_value != min_value)
    // {
    //     int pos = 0;

    //     for (size_t i = 0; i < num_frames && pos < total_elements; ++i)
    //     {
    //         for (size_t j = 0; j < num_mfcc && pos < total_elements; ++j)
    //         {
    //             float normalized_input = (mfcc_output[i][j] - min_value) / (max_value - min_value);
    //             // ESP_LOGI(TAG, "Normalized input: %f", normalized_input);
    //             model_input[pos] = (int16_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);
    //             ++pos;
    //         }
    //     }
    // }

    // // for (int i=0; i<total_elements; ++i) {
    // //     ESP_LOGI(TAG, "Model input %d:", (int) model_input[i]);
    // // }

    // Tensor<int16_t> input;
    // int num_frames_int = static_cast<int>(num_frames); // TODO: remove this cast
    // input.set_element((int16_t *)model_input).set_exponent(input_exponent).set_shape({num_frames_int, num_mfcc, 1}).set_auto_free(false);

    // BIRDNET model;

    // // dl::tool::Latency latency;
    // // latency.start();

    // model.forward(input);

    // // latency.end();

    // // // ESP-DL softmax implementation fix: this->channel = input.shape[0];
    // float *probs = model.softmax.get_output().get_element_ptr();

    // float max_prob = probs[0];
    // int max_index = 0;

    // for (size_t i = 0; i < 4; i++)
    // {
    //     ESP_LOGI(TAG, "Prob %d: %f %%", i, probs[i] * 100);
    //     if (probs[i] > max_prob)
    //     {
    //         max_prob = probs[i];
    //         max_index = i;
    //     }
    // }

    // switch (max_index)
    // {
    // case 0:
    //     ESP_LOGI(TAG, "CLASS 0");
    //     break;
    // case 1:
    //     ESP_LOGI(TAG, "CLASS 1");
    //     break;
    // case 2:
    //     ESP_LOGI(TAG, "CLASS 2");
    //     break;
    // case 3:
    //     ESP_LOGI(TAG, "CLASS 3");
    //     break;
    // default:
    //     ESP_LOGE(TAG, "");
    // }

    // model.softmax.get_output().free_element();

    // clear_esp_led();
    // /***/

    // set_esp_led_rgb(0, 60, 35);
    // set_esp_led_rgb(0, 40, 110);
    free(audio_buffer);

    deinit_i2s_mic();
    websocket_disconnect(client);

    // int i = 0;

    // int n_samples = 1;
    // int16_t *bufferf = (int16_t *)malloc(n_samples * sizeof(int16_t));

    // bufferf[0] = 0x1234;

    // vTaskDelay(1000 / portTICK_PERIOD_MS);

    // while (i < 10)
    // {
    //     send_buffer(client, bufferf, n_samples);
    //     i++;
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }

    // ESP_LOGI(TAG, "Recording ...");

    // int n_samples = read_i2s_mic(buffer, TEST_WAV_SIZE);
    // ESP_LOGI(TAG, "Read %d samples", n_samples);
    // // send_buffer(client, buffer, n_samples);

    // free(buffer);

    // ESP_LOGI(TAG, "Value 0 %d", buffer[0]);
    // ESP_LOGI(TAG, "Value 8000 %d", buffer[8000]);
    // ESP_LOGI(TAG, "Value 16000 %d", buffer[16000]);
    // ESP_LOGI(TAG, "Value 24000 %d", buffer[24000]);
    // ESP_LOGI(TAG, "Value 32000 %d", buffer[32000]);

    // ESP_LOGI(TAG, "Read %d samples", n_samples);

    // // Print the buffer

    // free(buffer);
    // deinit_i2s_mic();

    // ESP_LOGI(TAG, "Finished recording audio");

    // for (int i = 50; i < 70; i++) {
    //     ESP_LOGI(TAG, "Value %d", s_audio[i]);
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
    //         ESP_LOGI(TAG, "MFCC output %d %d: %f", i, j, mfcc_output[i][j]);
    //     }
    // }

    // size_t total_elements = num_frames * num_mfcc;
    // ESP_LOGI(TAG, "MFCC num frames: %d", num_frames);

    // if (mfcc_output == NULL)
    // {
    //     ESP_LOGE(TAG, "MFCC output is NULL");
    // }

    /**********
    DL
    ************/

    // for (int i = 0; i < num_frames * num_mfcc; ++i)
    // {
    //     model_input[i] = (int16_t)DL_CLIP(s_audio[i] * (1 << -input_exponent), -32768, 32767);
    // }

    // Expected input size [1, 32, 188]

    // ESP_LOGI(TAG, "Total elements: %d", num_frames);

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
    // ESP_LOGI(TAG, "Min value: %f - Max value: %f", min_value, max_value);

    // Normalize and clip values

    // if (max_value != min_value)
    // {
    //     int pos = 0;

    //     for (size_t i = 0; i < num_frames && pos < total_elements; ++i)
    //     {
    //         for (size_t j = 0; j < num_mfcc && pos < total_elements; ++j)
    //         {
    //             float normalized_input = (mfcc_output[i][j] - min_value) / (max_value - min_value);
    //             // ESP_LOGI(TAG, "Normalized input: %f", normalized_input);
    //             model_input[pos] = (int16_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);
    //             ++pos;
    //         }
    //     }
    // }

    // for (int i=0; i<total_elements; ++i) {
    //     ESP_LOGI(TAG, "Model input %d:", (int) model_input[i]);
    // }

    // Tensor<int16_t> input;
    // int num_frames_int = static_cast<int>(num_frames); // TODO: remove this cast
    // input.set_element((int16_t *)model_input).set_exponent(input_exponent).set_shape({num_frames_int, num_mfcc, 1}).set_auto_free(false);

    // BIRDNET model;

    // // dl::tool::Latency latency;
    // // latency.start();

    // model.forward(input);

    // // latency.end();

    // // ESP-DL softmax implementation fix: this->channel = input.shape[0];
    // float *probs = model.softmax.get_output().get_element_ptr();

    // float max_prob = probs[0];
    // int max_index = 0;

    // for (size_t i = 0; i < 4; i++)
    // {
    //     ESP_LOGI(TAG, "Prob %d: %f %%", i, probs[i] * 100);
    //     if (probs[i] > max_prob)
    //     {
    //         max_prob = probs[i];
    //         max_index = i;
    //     }
    // }

    // switch (max_index)
    // {
    // case 0:
    //     ESP_LOGI(TAG, "CLASS 0");
    //     break;
    // case 1:
    //     ESP_LOGI(TAG, "CLASS 1");
    //     break;
    // case 2:
    //     ESP_LOGI(TAG, "CLASS 2");
    //     break;
    // case 3:
    //     ESP_LOGI(TAG, "CLASS 3");
    //     break;
    // default:
    //     ESP_LOGE(TAG, "");
    // }

    // model.softmax.get_output().free_element();

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