#ifndef AUX_PC_WIND
#define AUX_PC_WIND

void dsps_wind_hann_f32(float *window, int len) {
    float len_mult = 1/(float)(len-1);
    for (int i = 0; i < len; i++) {
        window[i] = 0.5 * (1 - cosf(i * 2 * M_PI * len_mult));
    }
}

#endif