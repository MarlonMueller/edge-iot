#ifndef AUX_PC_FFT
#define AUX_PC_FFT

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sim_dsp/aux_err.h"
#include "sim_dsp/aux_fft_tables.h"

#define CONFIG_DSP_MAX_FFT_SIZE 4096

#define memalign(align_, size_) malloc(size_)

float *dsps_fft_w_table_fc32;
int dsps_fft_w_table_size;
uint8_t dsps_fft2r_initialized = 0;
uint8_t dsps_fft2r_mem_allocated = 0;

uint16_t *dsps_fft2r_ram_rev_table = NULL;

bool dsp_is_power_of_two(int x) {
    return (x != 0) && ((x & (x - 1)) == 0);
}

int dsp_power_of_two(int x) {
    for (size_t i = 0; i < 32; i++)
    {
        x = x >> 1;
        if(0 == x) return i;
    }
    return 0;
}

esp_err_t dsps_fft2r_fc32_ansi_(float *data, int N, float *w) {
    if (!dsp_is_power_of_two(N)) {
        return ESP_ERR_DSP_INVALID_LENGTH;
    }
    if (!dsps_fft2r_initialized) {
        return ESP_ERR_DSP_UNINITIALIZED;
    }

    esp_err_t result = ESP_OK;

    int ie, ia, m;
    float re_temp, im_temp;
    float c, s;
    ie = 1;
    for (int N2 = N / 2; N2 > 0; N2 >>= 1) {
        ia = 0;
        for (int j = 0; j < ie; j++) {
            c = w[2 * j];
            s = w[2 * j + 1];
            for (int i = 0; i < N2; i++) {
                m = ia + N2;
                re_temp = c * data[2 * m] + s * data[2 * m + 1];
                im_temp = c * data[2 * m + 1] - s * data[2 * m];
                data[2 * m] = data[2 * ia] - re_temp;
                data[2 * m + 1] = data[2 * ia + 1] - im_temp;
                data[2 * ia] = data[2 * ia] + re_temp;
                data[2 * ia + 1] = data[2 * ia + 1] + im_temp;
                ia++;
            }
            ia += N2;
        }
        ie <<= 1;
    }
    return result;
}

esp_err_t dsps_fft2r_fc32(float *data, int N) {
    return dsps_fft2r_fc32_ansi_(data, N, dsps_fft_w_table_fc32);
}

esp_err_t dsps_bit_rev_fc32_ansi(float *data, int N) {
    if (!dsp_is_power_of_two(N)) {
        return ESP_ERR_DSP_INVALID_LENGTH;
    }

    esp_err_t result = ESP_OK;

    int j, k;
    float r_temp, i_temp;
    j = 0;
    for (int i = 1; i < (N - 1); i++) {
        k = N >> 1;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
        if (i < j) {
            r_temp = data[j * 2];
            data[j * 2] = data[i * 2];
            data[i * 2] = r_temp;
            i_temp = data[j * 2 + 1];
            data[j * 2 + 1] = data[i * 2 + 1];
            data[i * 2 + 1] = i_temp;
        }
    }
    return result;
}

esp_err_t dsps_bit_rev_fc32(float *data, int N) {
    return dsps_bit_rev_fc32_ansi(data, N);
}


esp_err_t dsps_gen_w_r2_fc32(float *w, int N) {
    if (!dsp_is_power_of_two(N)) {
        return ESP_ERR_DSP_INVALID_LENGTH;
    }

    esp_err_t result = ESP_OK;

    int i;
    float e = M_PI * 2.0 / N;

    for (i = 0; i < (N >> 1); i++) {
        w[2 * i] = cosf(i * e);
        w[2 * i + 1] = sinf(i * e);
    }

    return result;
}

esp_err_t dsps_fft2r_init_fc32(float *fft_table_buff, int table_size) {
    esp_err_t result = ESP_OK;
    if (dsps_fft2r_initialized != 0) {
        return result;
    }
    if (table_size > CONFIG_DSP_MAX_FFT_SIZE) {
        return ESP_ERR_DSP_PARAM_OUTOFRANGE;
    }
    if (table_size == 0) {
        return result;
    }
    if (fft_table_buff != NULL) {
        if (dsps_fft2r_mem_allocated) {
            return ESP_ERR_DSP_REINITIALIZED;
        }
        dsps_fft_w_table_fc32 = fft_table_buff;
        dsps_fft_w_table_size = table_size;
    } else {
        if (!dsps_fft2r_mem_allocated) {
            #if CONFIG_IDF_TARGET_ESP32S3
                if (table_size <= 1024)
                {
                    dsps_fft_w_table_fc32 = dsps_fft2r_w_table_fc32_1024;
                } else
                {
                    dsps_fft_w_table_fc32 = (float*)memalign(16, sizeof(float) * table_size);
                }
            #else
            dsps_fft_w_table_fc32 = (float *)malloc(table_size * sizeof(float));
            #endif 
            if (dsps_fft_w_table_fc32 == NULL) {
                return ESP_ERR_DSP_PARAM_OUTOFRANGE;
            }
        }
        dsps_fft_w_table_size = table_size;
        dsps_fft2r_mem_allocated = 1;

    }

    // FFT ram_rev table allocated
    int pow = dsp_power_of_two(table_size);
    if ((pow > 3) && (pow < 13)) {
        dsps_fft2r_ram_rev_table = (uint16_t *)malloc(2 * dsps_fft2r_rev_tables_fc32_size[pow - 4] * sizeof(uint16_t));
        if (dsps_fft2r_ram_rev_table == NULL) {
            return ESP_ERR_DSP_PARAM_OUTOFRANGE;
        }
        memcpy(dsps_fft2r_ram_rev_table, dsps_fft2r_rev_tables_fc32[pow - 4], 2 * dsps_fft2r_rev_tables_fc32_size[pow - 4] * sizeof(uint16_t));
        dsps_fft2r_rev_tables_fc32[pow - 4] = dsps_fft2r_ram_rev_table;
    }

    result = dsps_gen_w_r2_fc32(dsps_fft_w_table_fc32, dsps_fft_w_table_size);
    if (result != ESP_OK) {
        return result;
    }
    result = dsps_bit_rev_fc32_ansi(dsps_fft_w_table_fc32, dsps_fft_w_table_size >> 1);
    if (result != ESP_OK) {
        return result;
    }
    dsps_fft2r_initialized = 1;

    return ESP_OK;
}

void dsps_fft2r_deinit_fc32() {
    if (dsps_fft2r_mem_allocated) {
        #if CONFIG_IDF_TARGET_ESP32S3
            if (dsps_fft_w_table_fc32 != dsps_fft2r_w_table_fc32_1024)
            {
                free(dsps_fft_w_table_fc32);
            }
        #else
            free(dsps_fft_w_table_fc32);
        #endif
    }
    if (dsps_fft2r_ram_rev_table != NULL) {
        free(dsps_fft2r_ram_rev_table);
        dsps_fft2r_ram_rev_table = NULL;
    }
    // Re init bitrev table for next use
    dsps_fft2r_rev_tables_init_fc32();
    dsps_fft2r_mem_allocated = 0;
    dsps_fft2r_initialized = 0;
}

#endif // AUX_PC_FFT