/**
 * @file led.h
 * @brief LED functions
 */

#pragma once

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize LED
     */
    void init_esp_led();

    /**
     * @brief Set LED state
     *
     * @param r Red value
     * @param g Green value
     * @param b Blue value
     */
    void set_esp_led_rgb(int r, int g, int b);

    /**
     * @brief Clear LED state
     */
    void clear_esp_led();

#ifdef __cplusplus
}
#endif
