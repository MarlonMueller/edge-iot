#pragma once

#include "sim_dsp/aux_fft_tables.h"

esp_err_t dsps_dct_f32_ref(float *data, int N, float *result) 
{
    float factor = M_PI / (2*N);
    for (size_t i = 0; i < N; i++) {
        float sum = 0;
        for (size_t j = 0; j < N; j++) {
            sum += 2 * data[j] * cosf(factor * i * (2*j + 1));
        }

        double scaling_factor = ((i == 0) ? sqrt(1.0f / (4*N)) : sqrt(1.0f / (2*N)));

        result[i] = scaling_factor * sum;
    }
    return ESP_OK;
}

esp_err_t dsps_dct_f32(float *data, int N) 
{
    return dsps_dct_f32_ref(data, N, dsps_fft_w_table_fc32);
}