/*
LORA PAYLOAD FORMAT
-------------------

1. LoRa initialization package:

 0                   1                   2                   3                   4                   5                   6                   7                   8                   9                   1                   1
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                             ESP ID                                            |                                                              GPS                                                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
2. LoRa initialization package ACK:

 0                   1                   2                   3                   4                   5
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                             ESP ID                                            |    Local ID   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

3. LoRa NN output package:

 0                   1
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Local ID   |    Counter    |NN o.|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Example: 00000000 00000100 110 means:
- ID: 0.
- Counter: 4.
- NN classification: true for class 1, true for class 2, false for class 3.

4. LoRa NN output package ACK:

 0                   1
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Local ID   |    Counter    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define LORA_INIT_PAYLOAD_SIZE 112
#define LORA_INIT_ACK_PAYLOAD_SIZE 32

#define LORA_NN_PAYLOAD_SIZE 19
#define LORA_NN_ACK_PAYLOAD_SIZE 8


/**
 * @brief Creates a LoRa initliazation package. Called by ESP32S3. 
 * 
 * @param id Pointer to ID of the original device (expected ESP MAC). 
 * @param latitude Latitude of the original device.
 * @param longitude Longitude of the original device.
 * @param lora_package Pointer to the LoRa package to be sent.
 * @param lora_package_size Pointer to the LoRa package size.
*/
void assemble_init_package(uint8_t *id, float latitude, float longitude, 
    uint8_t *lora_package, int *lora_package_size);

/**
 * @brief Disassembles a LoRa initialization package. Called by RPi.
 * 
 * @param lora_package Pointer to the LoRa package.
 * @param id Pointer to ID of original device (expected ESP MAC).
*/
void disassemble_init_package(uint8_t *lora_package, uint8_t *id, 
    float *latitude, float *longitude);

/**
 * @brief Creates a LoRa initialization package ACK. Called by RPi. 
 * 
 * @param id Pointer to ID of original device (expected ESP MAC).
 * @param local_id Assigned local ID to the original device.
 * @param lora_package Pointer to the LoRa package to be sent.
 * @param lora_package_size Pointer to the LoRa package size.
*/
void assemble_init_ack_package(uint8_t *id, uint8_t local_id, 
    uint8_t *lora_package, int *lora_package_size);

/**
 * @brief Disassembles a LoRa initialization package ACK. Called by ESP32S3.
 * 
 * @param lora_package Pointer to the LoRa package.
 * @param id Pointer to ID of original device (expected ESP MAC).
 * @param local_id Pointer to assigned local ID to the original device.
*/
void disassemble_init_ack_package(uint8_t *lora_package, uint8_t *id, 
    uint8_t *local_id);

/**
 * @brief Creates a LoRa NN output package. Called by ESP32S3. 
 * 
 * @param id local ID of the original device.
 * @param counter Counter of package.
 * @param detection_1 Detection for class 1.
 * @param detection_2 Detection for class 2.
 * @param detection_3 Detection for class 3.
 * 
 * @param lora_package Pointer to the LoRa package.
 * @param lora_package_size Pointer to the LoRa package size.
*/
void assemble_nn_package(uint8_t local_id, uint8_t counter, 
    bool detection_1, bool detection_2, bool detection_3, 
    uint8_t *lora_package, int *lora_package_size);

/**
 * @brief Disassembles a LoRa package received from Raspberry Pi for NN output.
 * 
 * @param lora_package Pointer to the LoRa package.
 * @param local_id Pointer to local ID of the original device.
 * @param counter Pointer to counter of the original device.
 * @param detection_1 Pointer to detection for class 1.
 * @param detection_2 Pointer to detection for class 2.
 * @param detection_3 Pointer to detection for class 3.
*/
void disassemble_nn_package(uint8_t *lora_package, uint8_t *local_id, 
    uint8_t *counter, bool *detection_1, bool *detection_2, bool *detection_3);

/**
 * @brief Creates a LoRa NN output package ACK.
 * 
 * @param local_id Local ID of the original device.
 * @param counter Counter of package.
 * 
 * @param lora_package Pointer to the LoRa package.
 * @param lora_package_size Pointer to the LoRa package size.
*/
void assemble_nn_ack_package(uint8_t local_id, uint8_t counter, 
    uint8_t *lora_package, int *lora_package_size);


/**
 * @brief Disassembles a LoRa NN output package ACK.
 * 
 * @param lora_package Pointer to the LoRa package.
 * @param local_id Pointer to local ID of the original device.
 * @param counter Pointer to counter of the original device.    
*/
void disassemble_nn_ack_package(uint8_t *lora_package, uint8_t *local_id, 
    uint8_t *counter);

#ifdef __cplusplus
}
#endif
