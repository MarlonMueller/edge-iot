#include <stdio.h>
#include <stdlib.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dl_tool.hpp"
#include "birdnet_model.hpp"

extern "C" void app_main(void)
{

    float example_input[32 * 1723];
    // memset(example_element, 0, sizeof(example_element));

    // Input

    int input_height = 32;
    int input_width = 1723;
    int input_channel = 1;
    int input_exponent = -7;

    int16_t *model_input = (int16_t *)dl::tool::malloc_aligned_prefer(input_height * input_width * input_channel, sizeof(int16_t *));

    for (int i = 0; i < input_height * input_width * input_channel; i++)
    {

        float normalized_input = example_element[i] / 255.0; // Fix normalization

        model_input[i] = (int16_t)DL_CLIP(normalized_input * (1 << -input_exponent), -32768, 32767);
    }

    Tensor<int16_t> input;
    input.set_element((int16_t *)model_input).set_exponent(input_exponent).set_shape({32, 1723, 1}).set_auto_free(false);

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