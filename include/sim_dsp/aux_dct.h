#ifndef AUX_PC_DCT
#define AUX_PC_DCT

#include "sim_dsp/aux_fft_tables.h"

esp_err_t dsps_dct_f32_ref(float *data, int N, float *result) {
    float factor = M_PI / N;
    for (size_t i = 0; i < N; i++) {
        float sum = 0;
        for (size_t j = 0; j < N; j++) {
            sum += data[j] * cosf((j + 0.5) * i * factor);
        }
        result[i] = sum;
    }
    return ESP_OK;
}

esp_err_t dsps_dct_f32(float *data, int N) {
    return dsps_dct_f32_ref(data, N, dsps_fft_w_table_fc32);
}

#endif // AUX_PC_DCT