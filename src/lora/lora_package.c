#include "lora/lora_package.h"

typedef union {
   float f;
   uint8_t b[4];
} float_bytes;

void assemble_init_package(uint8_t *id, float latitude, float longitude, 
   uint8_t *lora_package, int *lora_package_size)
{
   for (int i=0; i<6; ++i) {
      lora_package[i] = id[i];
   }

   float_bytes bytes_latitude = {latitude};

   for (int i=6; i<10; ++i) {
      lora_package[i] = bytes_latitude.b[i-6];
   }

   float_bytes bytes_longitude = {longitude};

   for (int i=10; i<14; ++i) {
      lora_package[i] = bytes_longitude.b[i-10];
   }  

   *lora_package_size = LORA_INIT_PAYLOAD_SIZE;
}

void disassemble_init_package(uint8_t *lora_package, uint8_t *id, 
   float *latitude, float *longitude)
{
   for (int i=0; i<6; ++i) {
      id[i] = lora_package[i];
   }

   float_bytes bytes_latitude;

   for (int i=6; i<10; ++i) {
      bytes_latitude.b[i-6] = lora_package[i];
   }

   *latitude = bytes_latitude.f;

   float_bytes bytes_longitude;

   for (int i=10; i<14; ++i) {
      bytes_longitude.b[i-10] = lora_package[i];
   }

   *longitude = bytes_longitude.f;
}

void assemble_init_ack_package(uint8_t *id, uint8_t local_id, 
   uint8_t *lora_package, int *lora_package_size)
{
   for (int i=0; i<6; ++i) {
      lora_package[i] = id[i];
   }

   lora_package[6] = local_id;

   *lora_package_size = LORA_INIT_ACK_PAYLOAD_SIZE;
}

void disassemble_init_ack_package(uint8_t *lora_package, uint8_t *id, 
   uint8_t *local_id)
{
   for (int i=0; i<6; ++i) {
      id[i] = lora_package[i];
   }

   *local_id = lora_package[6];
}

void assemble_nn_package(uint8_t local_id, uint8_t counter, 
   bool detection_1, bool detection_2, bool detection_3, 
   uint8_t *lora_package, int *lora_package_size) 
{

   // Create LoRa package.

   lora_package[0] = local_id;
   lora_package[1] = counter;

   // NN classification.

   uint8_t nn_classification = 0;

   if (detection_1)
      nn_classification |= 1 << 7;

   if (detection_2)
      nn_classification |= 1 << 6;

   if (detection_3)
      nn_classification |= 1 << 5;

   lora_package[2] = nn_classification;

   *lora_package_size = LORA_NN_PAYLOAD_SIZE;
}

void disassemble_nn_package(uint8_t *lora_package, uint8_t *local_id, 
   uint8_t *counter, bool *detection_1, bool *detection_2, bool *detection_3)
{
   uint8_t nn_classification = lora_package[2];

   *local_id = lora_package[0];
   *counter = lora_package[1];
   *detection_1 = nn_classification & (1 << 7);
   *detection_2 = nn_classification & (1 << 6);
   *detection_3 = nn_classification & (1 << 5);   
}

void assemble_nn_ack_package(uint8_t local_id, uint8_t counter, 
   uint8_t *lora_package, int *lora_package_size)
{
   lora_package[0] = local_id;
   lora_package[1] = counter;

   *lora_package_size = LORA_NN_ACK_PAYLOAD_SIZE;
}

void disassemble_nn_ack_package(uint8_t *lora_package, uint8_t *local_id, 
    uint8_t *counter) 
{
   *local_id = lora_package[0];
   *counter = lora_package[1];
}