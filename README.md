![Header](assets/header.png)


# Table of Contents

- [Hardware](#hardware)
- [Getting Started](#getting-started)

# Hardware

The following equipment is required:

- **ESP32-S3-DevKitC-1 (ESP)**. Serve as IoT node to capture data. 
More than one ESP32 can be used (in fact, that is the expected setup). For each
ESP32 device, there must be:
    - A LoRa module model **SX1278**. 
    - A microphone module model **INMP441**.
    - A GPS module model **GY-NEO6MV2**.

- **Raspberry Pi 3 Model B+ (RPI)**. Used as a gateway node. The ESP32s periodically sends classification 
information to it. For the RPI, there must be:
    - A LoRa module model **SX1278**. 
    - A sufficiently sized microSD (e.g., 32 GB)

- **Development device (DEV)**. Used to develop, build and flash code. For complete functionality, choose a Linux-based OS, such as 
Ubuntu.

For ESP and RPI, make sure to have wired the different components. Please 
refer to [docs/wiring.md](docs/wiring.md). If you have made changes to code 
regarding the wiring of some component, wire the physical cables accordingly. 

# Getting Started

Before running the code contained in this repository, the following 
__prerequisites__ need to be fulfilled. Note that the abbreviations ESP, RPI and DEV correspond to the ones within [Hardware](#hardware). 

- For ESP and RPI, ensure that you have the necessary hardware for this project and that it is properly set up.

- For DEV, install [ESP-IDF](https://github.com/espressif/esp-idf). The code contained in this 
repository was succesfully built and developed using the version `v5.1.2`. Make 
sure to be able to use `idf.py`. To do so, open a terminal and run the 
following code (if you can see your version of ESP-IDF, then you can use it):
```sh
idf.py --version
``` 

- For DEV, make sure that you can use the tool `idf.py` to flash
one of your ESP32 via USB. A
[Hello World project in ESP-IDF official documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html#get-started-windows-first-steps)
may be useful for starters. Read it carefully and remember that the connection
names in your DEV may be different from those listed in the tutorial. 

- For RPI, install the [Raspberry Pi OS](https://www.raspberrypi.com/software/)
and flash it into the microSD card. Make sure you have `sudo` privileges. 

- For RPI, make sure that SPI is configured appropiately. Further information
may be found [here](https://www.raspberrypi-spy.co.uk/2014/08/enabling-the-spi-interface-on-the-raspberry-pi/).

- For RPI, install the library 
[pigpio](https://abyz.me.uk/rpi/pigpio/), as it will be required to interact 
with the LoRa module, and make sure to have installed the library 
[curl for C++](https://raspberry-projects.com/pi/programming-in-c/networking/curl/adding-curl-to-your-project),
as it will be required to connect with backend and dashboard. 

- For DEV and RPI, install the [git utility](https://git-scm.com/downloads). 

Once you have checked that you fulfill all the prerrequisites, follow these
steps to have a functional demo of BirdNet:

1. Clone this repository in both DEV and RPI:

```sh
git clone https://github.com/MarlonMueller/edge-iot.git
```

2. In DEV device, move to the cloned repo directory. Flash the ESP code into
each of your edge nodes by executing the command below. To test whether the
code was correctly flashed into the devices, see the output opbtained in
the terminal. If it has failed, a message error should appear.

```sh
idf.py build flash monitor
```

3. For the RPI, move to the cloned repo directory. A `Makefile` is specifically
designed to automate the compilation of the file for Raspberry Pi.

```sh
make -f Makefile.RPi
sudo ./build/main_rpi
```

4. TODO (WEB)
