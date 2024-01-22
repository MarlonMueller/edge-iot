#include "lora/lora_package.h"

void assemble_init_package(uint8_t *id, uint8_t *lora_package, 
    int *lora_package_size)
{
   for (int i=0; i<6; ++i) {
      lora_package[i] = id[i];
   }

   *lora_package_size = LORA_INIT_PAYLOAD_SIZE;
}

void disassemble_init_package(uint8_t *lora_package, uint8_t *id)
{
   for (int i=0; i<6; ++i) {
      id[i] = lora_package[i];
   }
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