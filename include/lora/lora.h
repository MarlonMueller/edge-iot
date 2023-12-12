/*
MIT License

Copyright (c) 2022 nopnop2002

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

extern "C" {

void lora_reset(void);
void lora_explicit_header_mode(void);
void lora_implicit_header_mode(int size);
void lora_idle(void);
void lora_sleep(void); 
void lora_receive(void);
int lora_get_irq(void);
void lora_set_tx_power(int level);
void lora_set_frequency(long frequency);
void lora_set_spreading_factor(int sf);
int lora_get_spreading_factor(void);
void lora_set_dio_mapping(int dio, int mode);
int lora_get_dio_mapping(int dio);
void lora_set_bandwidth(int sbw);
int lora_get_bandwidth(void);
void lora_set_coding_rate(int denominator);
int lora_get_coding_rate(void);
void lora_set_preamble_length(long length);
long lora_get_preamble_length(void);
void lora_set_sync_word(int sw);
void lora_enable_crc(void);
void lora_disable_crc(void);
int lora_init(void);
void lora_send_packet(uint8_t *buf, int size);
int lora_receive_packet(uint8_t *buf, int size);
int lora_received(void);
int lora_packet_lost(void);
int lora_packet_rssi(void);
float lora_packet_snr(void);
void lora_close(void);
int lora_initialized(void);
void lora_dump_registers(void);

void setup_lora_comm();
void task_rx(void *pvParameters);
void task_tx(uint8_t *buf, int size);

}