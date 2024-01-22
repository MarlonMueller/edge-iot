#include "lora/lora_rpi.h"
#include "lora/lora_package.h"
#include <stdbool.h>

LoRa_ctl modem;

void tx_f(txData *tx) {
    printf("transmit done\n");

    LoRa_receive(&modem);
}

void * rx_f(void *p){
    rxData *rx = (rxData *)p;

    if (rx->size == LORA_INIT_PAYLOAD_SIZE) {
        printf("init package received\n");

        uint8_t id[6];
        disassemble_init_package(rx->buf, id);
        printf("ID: %d\n", id);

        uint8_t send_buf[255];
        int size;

        uint8_t local_id = 1;

        assemble_init_ack_package(id, local_id, send_buf, &size);

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

        // TODO: Do stuff with the data.

        // Send ack

        uint8_t send_buf[255];
        int size;

        assemble_nn_ack_package(local_id, counter, send_buf, &size);

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

    sleep(60);
    printf("end\n");
    LoRa_end(&modem);
}