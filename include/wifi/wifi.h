/**
 * @file wifi.h
 *
 * @brief WiFi functions
 *
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Connect to WiFi
     *
     * @return ESP_OK if success, otherwise ESP_FAIL.
     */
    esp_err_t wifi_connect(void);

#ifdef __cplusplus
}
#endif
