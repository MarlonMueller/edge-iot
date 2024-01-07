#include "esp_err.h"

#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_event.h"

#include "esp_websocket_client.h"
#include "websocket/websocket.h"

#define CHUNCK_SIZE 1024

static const char *TAG = "WEBSOCKET";

#define URI "ws://192.168.178.65"
#define PORT 8888

static void websocket_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Connected to %s:%d", URI, PORT);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected from %s:%d", URI, PORT);
        break;
    default:
        break;
    }
}

esp_websocket_client_handle_t websocket_connect(void)
{
    esp_websocket_client_config_t websocket_cfg = {
        .uri = URI,
        .port = PORT,
    };

    ESP_LOGI(TAG, "Connecting to %s:%d...", URI, PORT);

    // Start websocket session
    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);

    // Register event handler
    ESP_ERROR_CHECK(esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client));

    // Open websocket connection
    ESP_ERROR_CHECK(esp_websocket_client_start(client));

    return client;
}

void websocket_send(esp_websocket_client_handle_t client, char *data, size_t len)
{
    if (esp_websocket_client_is_connected(client))
    {
        while (len > 0)
        {
            size_t chunk = len > CHUNCK_SIZE ? CHUNCK_SIZE : len;
            esp_websocket_client_send_bin(client, data, chunk, portMAX_DELAY);
            data += chunk;
            len -= chunk;
        }
    }
}

void websocket_disconnect(esp_websocket_client_handle_t client)
{
    ESP_LOGI(TAG, "Disconnecting...");
    esp_websocket_client_stop(client);
    esp_websocket_client_destroy(client);
}