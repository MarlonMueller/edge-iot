/**
 * @file lora_esp32_logic.c
 * 
 * @brief Contains the logic for the LoRa communication on the ESP32.
*/

#include "lora/lora_esp32_logic.h"

#include <stdbool.h>

#include "lora/lora_esp32.h"
#include "lora/lora_package.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "gps/gps.h"

#include "esp_attr.h" // For all logic relatd to deep sleep. 
#include "esp_sleep.h"

#define NUM_TRIES 3 // number of tries to send a packet
#define MAX_COUNTER 95 
#define MAX_BUFFER_SIZE 256

#define PETITION_DELAY (100 / portTICK_PERIOD_MS) // ms between petitions
#define NEXT_INIT_RETRY (300 / portTICK_PERIOD_MS) // ms between init retries
#define NEXT_PERIODIC_DATA (500 / portTICK_PERIOD_MS) // ms between periodic data

typedef struct {
    bool d1, d2, d3;
} detection_t;

// Values stores between deep sleep activations. 

static RTC_DATA_ATTR bool lora_is_initialized = false;

static RTC_DATA_ATTR uint8_t local_id;
static RTC_DATA_ATTR uint8_t counter;
static RTC_DATA_ATTR detection_t detection;

static RTC_DATA_ATTR uint8_t rx_buffer[MAX_BUFFER_SIZE];
static RTC_DATA_ATTR uint8_t tx_buffer[MAX_BUFFER_SIZE];

#define TAG "LORA_ESP32_LOGIC"

// Local functions

bool task_rx()
{
	ESP_LOGI(TAG, "Start reception of packages...");

    lora_receive(); // put into receive mode

    vTaskDelay(50);

    if (lora_received()) {
        int rxLen = lora_receive_packet(rx_buffer, sizeof(rx_buffer));
        ESP_LOGI(TAG, "%d byte packet received:[%.*s]", rxLen, rxLen, rx_buffer);

        return true;
    } else {
        ESP_LOGW(TAG, "task_rx No packet received...");
        return false;
    }
}


bool task_tx(uint8_t *buf, int size)
{
   ESP_LOGI(TAG, "Start transmission of packages...");
   lora_send_packet(buf, size);
   ESP_LOGI(TAG, "%d Bytes packets sent...", size);

   int lost = lora_packet_lost();

   if (lost != 0) {
      ESP_LOGW(TAG, "%d packets lost", lost);
      return false;
   } else {
      return true;
   }
}

// End local function

void setup_lora_comm() 
{
    ESP_LOGI(TAG, "Execution of LoRa setup...");

    if (lora_init() == 0) {
        ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the module");
        while (1) {
            vTaskDelay(1);
        }
    }


    int cr = 5;
    int bw = 7;
    int sf = 7;
    int pl = 8;
    int freq = 868000000; // 868 MHz
    int sw = 0x1a;

    lora_set_coding_rate(cr);
    // lora_set_coding_rate(CONFIG_CODING_RATE);
    // cr = lora_get_coding_rate();
    ESP_LOGI(pcTaskGetName(NULL), "coding_rate=%d", cr);

    lora_set_bandwidth(bw);
    // lora_set_bandwidth(CONFIG_BANDWIDTH);
    // int bw = lora_get_bandwidth();
    ESP_LOGI(pcTaskGetName(NULL), "bandwidth=%d", bw);

    lora_set_spreading_factor(sf);
    // lora_set_spreading_factor(CONFIG_SF_RATE);
    // int sf = lora_get_spreading_factor();
    ESP_LOGI(pcTaskGetName(NULL), "spreading_factor=%d", sf);

    lora_set_preamble_length(pl);
    ESP_LOGI(pcTaskGetName(NULL), "preamble_length=%d", pl);

    lora_set_frequency(freq);
    ESP_LOGI(pcTaskGetName(NULL), "frequency=%d", freq);

    lora_set_sync_word(sw);
    ESP_LOGI(pcTaskGetName(NULL), "sync_word=0x%02x", sw);

    lora_enable_crc();
    ESP_LOGI(pcTaskGetName(NULL), "crc_enabled");
}

bool is_initialized_comm() 
{
    return lora_is_initialized;

}

void initialize_comm(void) 
{
    ESP_LOGI(TAG, "LoRa initialization execution...");

    int total_input = 0;

    // Get information

    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);

    float latitude = 0;
    float longitude = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    bool valid_gps = false; 

    get_gps_data(&latitude, &longitude, &hour, &minute, &second, &valid_gps);

    if (valid_gps)
        assemble_init_package(mac, latitude, longitude, tx_buffer, &total_input);
    else 
        assemble_init_package(mac, -1.0, -1.0, tx_buffer, &total_input);

    for (int i=0; i<NUM_TRIES && !lora_is_initialized; ++i) {

        // Tranmit data. 

        task_tx(tx_buffer, total_input);
        ESP_LOGI(TAG, "Trying to initiliaze LoRa. Waiting for ACK...");

        // Wait for ACK.      

        bool received = task_rx();

        if (received) {
            uint8_t the_local_mac[6];
            uint8_t the_local_id = 0;

            disassemble_init_ack_package(rx_buffer, the_local_mac, &the_local_id);

            // Confirm correct ACK. 

            bool mac_match = true;

            for (int i=0; i<6; ++i) {
                if (mac[i] != the_local_mac[i]) {
                    mac_match = false;
                    break;
                }
            }

            if (mac_match) {
                ESP_LOGI(TAG, "Local ID: %d", the_local_id);
                local_id = the_local_id;
                lora_is_initialized = true;
            } else {
                ESP_LOGW(TAG, "MAC does not match. Waiting...");
            }

        } else {
            ESP_LOGW(TAG, "No ACK received. Waiting...");
            vTaskDelay(NEXT_INIT_RETRY); // Avoid WatchDog alerts
        
        }
	} // end for

    // reset data

    if (lora_is_initialized) {
        counter = 0;
        detection.d1 = false;
        detection.d2 = false;
        detection.d3 = false;
    }
}

void send_data() 
{
    ESP_LOGI(TAG, "LoRa send_data execution...");

    // Exceptions

    if (!lora_is_initialized) {
        ESP_LOGW(TAG, "LoRa sending package was requested, but LoRa was not initiated...");
        return;
    }

    // Formal execution

    int total_input;
    assemble_nn_package(local_id, counter, detection.d1, detection.d2, 
                        detection.d3, tx_buffer, &total_input);

    bool flag = false;

    for (int i=0; i<NUM_TRIES && !flag; ++i) {
        
        // Send data

        task_tx((uint8_t *) tx_buffer, total_input);
        ESP_LOGI(TAG, "NN Data sent. Waiting for ACK...");

        // Wait for ACK

        uint8_t local_counter = 0;

        flag = task_rx();

        if (flag) {
            disassemble_nn_ack_package(rx_buffer, &local_id, &local_counter);
            ESP_LOGI(TAG, "ID: %d - Counter: %d", local_id, local_counter);
        } else {
            ESP_LOGW(TAG, "No ACK received. Waiting...");
            vTaskDelay(PETITION_DELAY); // Avoid WatchDog alerts
        }
    }

    if (!flag) 
        ESP_LOGW(TAG, "Could not send data...");

    // Prepare next iteration

    ++counter;
    detection.d1 = false;
    detection.d2 = false;
    detection.d3 = false;  

    if (counter >= MAX_COUNTER)
        lora_is_initialized = false; // needs to resync
}

void set_activation(int idx) {
    switch (idx) {
        case 0:
            detection.d1 = true;
            break;
        case 1:
            detection.d2 = true;
            break;
        case 2:
            detection.d3 = true;
            break;
        default:
            break;
    }
}