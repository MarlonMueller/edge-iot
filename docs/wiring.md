# Wiring

The different components of the distributed systems must be wired together 
in order to cooperate and act as an interconnected network. By default, we have 
established a default wiring, which you are free to change according to your 
interests. Instructions about that may be found at the end of this section.

In this section, for the different components attached to each node we will
use the actual name shown on the boards. For RPI and ESP, the assigned names
by the manufacturer will be used. You may see a map for their distribution
[here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1-v1.0.html#header-block) 
(for ESP) and [here](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html) 
(for RPI). 

## Default wiring

From the part of RPI, the following table maps each connection to the
corresponding pin of the LoRa module SX1278. 

| SX1278 Pin | RPI Pin |
|------------|---------|
| VCC        | 3V3     |
| GND        | Ground  |
| RST        | GPIO22  |
| DIO0       | GPIO27  |
| MOSI       | GPIO10  |
| MISO       | GPIO9   |
| SCK        | GPIO11  |
| NSS        | GPIO8   |

On the other hand, for ESP we need to wire several components: (i) microphone, 
(ii) LoRa module and (iii) GPS module. 

TODO MICROPHONE

For LoRa module, the corresponding wiring is displayed in the following table.

| SX1278 Pin | ESP Pin |
|------------|---------|
| VCC        | 3V3     |
| GND        | GND     |
| RST        | GPIO2   |
| MOSI       | GPIO11  |
| MISO       | GPIO13  |
| SCK        | GPIO12  |
| NSS        | GPIO10  |

Notice that that the DIO0 pin is not required in this case, unlike in the case
of RPI. 

On the other hand, for GPS module, the following pins were required.

| GPS Pin | ESP Pin |
|---------|---------|
| VCC     | 3V3     |
| RX      | GPIO17  |
| TX      | GPIO18  |
| GND     | GND     |

## Change wiring

> [!CAUTION] 
> Do not change the wiring unless it is strictly necessary. Be specially
> careful with boards pin layouts. There could be some pins that, at
> initialization, are not properly configured and could fry the components.

The change of the default wiring pins for the different components is possible.
Please, make sure that, if you decide to change wiring, that all pins from
the previous table are used. Otherwise, the result may be unexpected (even not
working at all). 

For RPI, the pins for RST and DIO0 may be changed by assigning the
attributes `modem.eth.resetGpioN` and `modem.eth.dio0GpioN` at [this source file](../src/rpi/main.c). 

On the other hand, for ESP, the wiring of the different pins is managed using
ESP-IDF. Instead of directly changing the variables manually, the best option
is to change them using the corresponding framework. If you have successfully
followed the instructions for [Getting Started in README](../README.md#getting-started),
in the root directory of this repository in a DEV machine, write 
the following command in a terminal with `idf.py` available:

```sh
idf.py menuconfig
```

Then navigate to `LoRa Configuration`. An interface shall be displayed with 
the corresponding associations between ESP pins and SX1278 pins, as shown
in the following picture.

![Screenshot of LoRa Configuration layout](../assets/lora_menuconfig.png)