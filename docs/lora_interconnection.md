# LoRa interconnection

The different devices shall be interconnected using 
[LoRa protocol](https://lora-developers.semtech.com/documentation/tech-papers-and-guides/lora-and-lorawan/).
[LoRa best practices](https://lora-developers.semtech.com/documentation/tech-papers-and-guides/the-book/packet-size-considerations/)
have been followed. 

## Packages Format

As both ESP32 and Raspberry Pi use C++ code, the same file is used for LoRa
packages. An unique header is used for both cases. The packages format as well
as its interface for both devices may be found at the beginning of
[`include/lora/lora_package.h`](../include/lora/lora_package.h). 