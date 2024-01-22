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

#define TAG "LORA_ESP32_LOGIC"

// Local functions

void task_rx(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	uint8_t buf[256]; // Maximum Payload size of SX1276/77/78/79 is 255

	while(1) {
		lora_receive(); // put into receive mode

		if (lora_received()) {
			int rxLen = lora_receive_packet(buf, sizeof(buf));
			ESP_LOGI(pcTaskGetName(NULL), "%d byte packet received:[%.*s]", rxLen, rxLen, buf);
		}

		vTaskDelay(1); // Avoid WatchDog alerts
	} // end while
}


void task_tx(uint8_t *buf, int size)
{
   ESP_LOGI(TAG, "Start");
   lora_send_packet(buf, size);
   ESP_LOGI(TAG, "%d Bytes packets sent...", size);

   int lost = lora_packet_lost();

   if (lost != 0) {
      ESP_LOGW(TAG, "%d packets lost", lost);
   }
}


bool send_nn_data(uint8_t *tx_buffer, int total_input) {
    bool status = false;

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    task_tx((uint8_t *) tx_buffer, total_input);
    ESP_LOGI(TAG, "Data sent. Waiting for ACK...");

    lora_receive(); // put into receive mode

    vTaskDelay(50);

    if (lora_received()) {
        int rxLen = lora_receive_packet(rx_buffer, sizeof(rx_buffer));
        uint8_t local_local_counter;

        disassemble_nn_ack_package(rx_buffer, &local_id, &local_local_counter);

        // Display id and counter

        ESP_LOGI(TAG, "ID: %d - Counter: %d", local_id, local_local_counter);

        status = true;
    } else {
        ESP_LOGI(TAG, "No ACK received. Waiting...");
        vTaskDelay(PETITION_DELAY); // Avoid WatchDog alerts
    }

    return status;
}

void send_data() 
{
    int total_input;
    assemble_nn_package(local_id, counter, detection.d1, detection.d2, 
                        detection.d3, tx_buffer, &total_input);

    bool flag = false;

    for (int i=0; i<NUM_TRIES && !flag; ++i) {
        flag = send_nn_data(tx_buffer, total_input);
    }

    if (!flag) 
        ESP_LOGE(TAG, "Could not send data...");

    // Prepare next iteration

    ++counter;
    detection.d1 = false;
    detection.d2 = false;
    detection.d3 = false;  
}


// End local function

void setup_lora_comm() 
{
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

void initialize_comm(void) 
{
    int total_input = 0;

    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    assemble_init_package(mac, tx_buffer, &total_input);

    // Wait for ACK

    bool flag = false;

    while (!flag) {
        task_tx(tx_buffer, total_input);
        ESP_LOGI(TAG, "Init package sent. Waiting for ACK...");

		lora_receive(); // put into receive mode

        vTaskDelay(50);

		if (lora_received()) {
			int rxLen = lora_receive_packet(rx_buffer, sizeof(rx_buffer));


			ESP_LOGI(TAG, "%d byte packet received:[%.*s]", rxLen, rxLen, rx_buffer);

            uint8_t the_local_mac[6];
            uint8_t the_local_id;

            disassemble_init_ack_package(rx_buffer, the_local_mac, &the_local_id);

            // compare local_mac with mac
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
                flag = true;
            }
		} else {
            ESP_LOGI(TAG, "No packet received. Waiting...");
            vTaskDelay(PETITION_DELAY); // Avoid WatchDog alerts
        }
		
	} // end for

    // reset data

    counter = 0;
    detection.d1 = false;
    detection.d2= false;
    detection.d3 = false;
}

void send_periodic_data(void *pvParameters) {

    while (true) {
        send_data();
        vTaskDelay(NEXT_PERIODIC_DATA);
    }
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