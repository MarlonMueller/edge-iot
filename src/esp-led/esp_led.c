#include "esp_log.h"
#include "led_strip.h"

static const char *TAG = "LED";

led_strip_handle_t led_strip;

#define LED_PIN 48

void init_esp_led()
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_PIN,
        .max_leds = 1,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}

void set_esp_led_rgb(int r, int g, int b)
{
    ESP_LOGI("esp_led", "Setting LED to RGB(%d, %d, %d)", r, g, b);
    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

void clear_esp_led()
{
    ESP_LOGI("esp_led", "Clearing LED");
    led_strip_clear(led_strip);
    ESP_ERROR_CHECK(led_strip_del(led_strip));
}