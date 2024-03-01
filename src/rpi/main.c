#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "lora/lora_rpi.h"
#include "lora/lora_package.h"
#include <curl/curl.h>

LoRa_ctl modem;

uint8_t calculate_next_time() {
    // Get the current time
    time_t currentTime;
    time(&currentTime);

    struct tm* currentTimeStruct = localtime(&currentTime);

    // Calculate the minutes until the next half past or hour
    uint8_t minutesUntilNext;
    if (currentTimeStruct->tm_min < 30) {
        // Minutes until the next half past
        minutesUntilNext = 30 - currentTimeStruct->tm_min;
    } else {
        // Minutes until the next hour
        minutesUntilNext = 60 - currentTimeStruct->tm_min;
    }

    return minutesUntilNext;
}

// Function to find the value associated with a key in a JSON string
bool find_json_value(const char* jsonStr, const char* key, uint8_t* value) {
    const char* keyStart = strstr(jsonStr, key);

    if (keyStart == NULL) {
        // Key not found
        return 0;
    }

    // Move to the end of the key
    keyStart += strlen(key);

    // Find the first digit of the value
    while (*keyStart && (*keyStart < '0' || *keyStart > '9'))
        keyStart++;

    // Convert the value to an integer
    sscanf(keyStart, "%d", value);

    return true;
}

// Callback function to handle the response data
size_t write_callback(void* contents, size_t size, size_t nmemb, char* output) {
    size_t total_size = size * nmemb;
    strcat(output, (char*)contents);
    return total_size;
}


void send_initialization(uint8_t *mac, float longitude, float latitude, uint8_t *local_id) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/node/put");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        const char data[256];

        sprintf(data, "{\r\n    \"_id\":\"%d:%d:%d:%d:%d:%d\",\r\n    \"long\":\"%f\",\r\n    \"lat\":\"%f\"\r\n}", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], longitude, latitude);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        char response_data[4096] = "";  // Adjust the buffer size as needed

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);

        res = curl_easy_perform(curl);

        printf("Response data: %s\n", response_data);
        find_json_value(response_data, "\"localId\"", local_id);

        printf("Local ID assigned: %d\n", *local_id);
    }
    curl_easy_cleanup(curl);
}

void send_classification(uint8_t local_id, bool d1, bool d2, bool d3) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/birds/put");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        const char data[256];

        if (d1) {
            sprintf(data, "{\r\n    \"nodeId\":\"%d\",\r\n    \"name\":\"Water Rail\"\r\n}", local_id);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            res = curl_easy_perform(curl);
        }

        if (d2) {
            sprintf(data, "{\r\n    \"nodeId\":\"%d\",\r\n    \"name\":\"Common Sandpiper\"\r\n}", local_id);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            res = curl_easy_perform(curl);
        }

        if (d3) {
            sprintf(data, "{\r\n    \"nodeId\":\"%d\",\r\n    \"name\":\"Cetti's Warbler\"\r\n}", local_id);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            res = curl_easy_perform(curl);
        }
            
        if (!d1 && !d2 && !d3) {
            sprintf(data, "{\r\n    \"nodeId\":\"%d\",\r\n    \"name\":\"No detection\"\r\n}", local_id);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            res = curl_easy_perform(curl);
        }
    }
    curl_easy_cleanup(curl);
}

void tx_f(txData *tx) {
    printf("transmit done.\n");

    LoRa_receive(&modem);
}

void * rx_f(void *p){
    rxData *rx = (rxData *)p;

    if (rx->size == LORA_INIT_PAYLOAD_SIZE) {
        printf("init package received\n");

        uint8_t id[6];
        float latitude, longitude;

        disassemble_init_package(rx->buf, id, &latitude, &longitude);
        printf("ID: %d. GPS coord: %f %f \n", id, latitude, longitude);

        uint8_t send_buf[255];
        int size;

        uint8_t assigned_id = 0;

        send_initialization(id, longitude, latitude, &assigned_id);
        assemble_init_ack_package(id, assigned_id, send_buf, &size);

        // Copy to buffer

        modem.tx.callback = tx_f;
        modem.tx.data.buf = (char *) send_buf;
        modem.tx.data.size = size;

        LoRa_send(&modem);

        printf("init ack sent\n");
    } else if (rx->size == LORA_NN_PAYLOAD_SIZE) {
        printf("data package received\n");

        uint8_t local_id, counter;
        bool d1, d2, d3;
        disassemble_nn_package(rx->buf, &local_id, &counter, &d1, &d2, &d3);

        printf("local_id: %d counter: %d d1: %d d2: %d d3: %d \n", local_id, counter, d1, d2, d3);
        send_classification(local_id, d1, d2, d3);

        // TODO: Do stuff with the data.

        // Send ack

        uint8_t send_buf[255];
        int size;
        uint8_t timer = calculate_next_time();

        assemble_nn_ack_package(local_id, timer, send_buf, &size);

        // Copy to buffer

        modem.tx.callback = tx_f;
        modem.tx.data.buf = (char *) send_buf;
        modem.tx.data.size = size;

        LoRa_send(&modem);

        printf("data ack sent\n");
    }

    free(p);
    return NULL;
}

int main(){

    //See for typedefs, enumerations and there values in LoRa.h header file
    modem.spiCS = 0;//Raspberry SPI CE pin number
    modem.rx.callback = rx_f;
    modem.eth.preambleLen=8;
    modem.eth.bw = BW125;//Bandwidth 125KHz
    modem.eth.sf = SF7;//Spreading Factor 7
    modem.eth.ecr = CR5;//Error coding rate CR4/5
    modem.eth.freq = 868000000;// 868 MHz
    modem.eth.resetGpioN = 22;//GPIO4 on lora RESET pi
    modem.eth.dio0GpioN = 27;//GPIO17 on lora DIO0 pin to control Rxdone and Txdone interrupts
    modem.eth.outPower = OP20;//Output power
    modem.eth.powerOutPin = PA_BOOST;//Power Amplifire pin
    modem.eth.AGC = 1;//Auto Gain Control
    modem.eth.OCP = 240;// 45 to 240 mA. 0 to turn off protection
    modem.eth.implicitHeader = 0;//Explicit header mode
    modem.eth.syncWord = 0x1a;
    modem.eth.CRC = 1; // enable CRC
    //For detail information about SF, Error Coding Rate, Explicit header, Bandwidth, AGC, Over current protection and other features refer to sx127x datasheet https://www.semtech.com/uploads/documents/DS_SX1276-7-8-9_W_APP_V5.pdf

    LoRa_begin(&modem);
    LoRa_receive(&modem);

    while (1);
    
    printf("end\n");
    LoRa_end(&modem);
}