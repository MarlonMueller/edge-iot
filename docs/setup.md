# Setup

This document shows how to setup the different components of this project.

**Table of content**:

2. [Raspberry Pi](#raspberry-pi)

## Raspberry Pi

In Raspberry Pi, LoRa communication has to be setup. The pins needed for
communication are:

- MISO. GPIO9
- MOSI. GPIO10
- SCLK. GPIO11
- DI0. GPIO27
- RST. GPIO22
- NSS. GPIO8

Additionally, the following configurations need to be made:

- SPI has to be configured appropiately in Raspberry Pi. Further information may 
  be found [here](https://www.raspberrypi-spy.co.uk/2014/08/enabling-the-spi-interface-on-the-raspberry-pi/).
- `pigpio` library needs to be installed. To install this library, please follow
  the instructions to download and install it in Raspberry Pi
  [here](https://abyz.me.uk/rpi/pigpio/download.html). 



The main function to be executed in Raspberry Pi is found in `src/rpi/main.c`.
To compile this file, please execute this command in the terminal of Raspberry
Pi in the root directory of this project:

```bash
make -f Makefile.RPi
```

To execute the built binary file, please use the following command (`sudo` is 
required for SPI):

```bash
sudo ./build/main_rpi
```