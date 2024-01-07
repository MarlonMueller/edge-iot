#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

#include "esp_wifi.h"
#include "nvs_flash.h"

#define WIFI_OK BIT0
#define WIFI_FAIL BIT1

static EventGroupHandle_t event_group;

static const char *TAG = "WIFI";

#define SSID ""
#define PASS ""

static int num_retries = 0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:

            ESP_LOGI(TAG, "Connecting to %s...", SSID);
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            if (num_retries < 3)
            {
                ESP_LOGI(TAG, "Retrying connecting to %s...", SSID);
                esp_wifi_connect();
                num_retries++;
            }
            else
            {
                ESP_LOGI(TAG, "Disconnected from %s", SSID);
                xEventGroupSetBits(event_group, WIFI_FAIL);
            }
            break;
        default:
            break;
        }
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "IP: %s", ip4addr_ntoa(&((ip_event_got_ip_t *)event_data)->ip_info.ip));
            xEventGroupSetBits(event_group, WIFI_OK);
            break;
        default:
            break;
        }
    }
}

esp_err_t wifi_connect(void)
{

    // Initialize non-volatile storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialze the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create default wifi station
    esp_netif_create_default_wifi_sta();

    // Configure default wifi station
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    // Register event handlers
    event_group = xEventGroupCreate();
    esp_event_handler_instance_t wifi_event_handler_instance_any_id;
    esp_event_handler_instance_t ip_event_handler_instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &wifi_event_handler_instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL, &ip_event_handler_instance_got_ip));

    // Wifi configuration
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }};

    // Set wifi mode to station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Set wifi configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // Start wifi
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for status
    EventBits_t event_bits = xEventGroupWaitBits(event_group, WIFI_OK | WIFI_FAIL, pdFALSE, pdFALSE, portMAX_DELAY);

    // Unregister event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler_instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_handler_instance_got_ip));
    vEventGroupDelete(event_group);

    if (event_bits & WIFI_OK)
    {
        ESP_LOGI(TAG, "Connected to %s", SSID);
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to connect to %s", SSID);
        return ESP_FAIL;
    }
}