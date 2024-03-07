![Header](assets/header.png)

In this repository, we have implemented **BirdNet**, a distributed system for 
bird monitoring. By deploying nodes in the wild, the system captures bird sounds
and classifies the detected sounds using Deep Learning. Each node periodically 
reports identified bird species to a gateway. Detections from these
nodes can then be visualized using a dashboard. 

__Table of Contents__

- [Hardware](#hardware)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Demo](#demo)
- [Training Networks](#training-networks)
  - [Configuration](#configuration)
  - [Deployment](#deployment)
- [Repository Structure](#repository-structure)
- [Further documentation](#further-documentation)

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

Before running code contained in this repository, the following prerequisites need to be fulfilled.

## Prerequisites

 Note that the abbreviations ESP, RPI and DEV correspond to the ones within [Hardware](#hardware). 

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

## Demo

Once you have checked that you fulfill all the prerrequisites, follow these
steps to have a functional demo of BirdNet to detect Water Rails, Cetti's Warblers and Common Blackbirds 🐦.

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

4. For the backend please go to [https://github.com/MarlonMueller/edge-iot/tree/backend](https://github.com/MarlonMueller/edge-iot/tree/backend/backend)

5. For the frontend please go to https://github.com/MarlonMueller/edge-iot/tree/frontend/bird-watcher

# Training Networks

To train your own networks, an end-to-end ML pipeline can be run.

> [!CAUTION]
> The full pipeline can currently only be executed on a Linux-based operating system due to the internals of [ESP-DL](https://github.com/espressif/esp-dl). Please adhere to the suggested versions for Python, dependencies, and other components to reduce the chance of errors.

## Configuration

Setup the Python environment. Example

```bash
# Create conda environment
conda create -n "birdnet" python=3.8.18 pip
conda activate birdnet
# Install requirements
python -m pip install -r requirements.txt
```

We apply the same audio preprocessing to both training and inference. For
this, we have implemented a C++ audio processing module, which can also be invoked from Python. For its setup run

```bash
python src/audio/setup.py build
```
and set the correct path to the C library in [`audio_processing.py`](https://github.com/MarlonMueller/edge-iot/blob/main/src/audio/audio_processing.py#L27). Then run

```bash
gcc -x c -o src/audio/preprocess.so -D RUN_PC -shared -fPIC -I include/ src/audio/preprocess.cpp -lm
```

For more details please refer to [`docs/audio_processing.md`](docs/audio_processing.md) and the source files.

Within [`pipeline.py`](https://github.com/MarlonMueller/edge-iot/blob/main/pipeline.py) first, define a [Xeno Canto query](https://xeno-canto.org/explore/api). 
By default, the network will be trained to detect the top-3 bird species resulting from this query. Data from [ESC50](https://github.com/karolpiczak/ESC-50) is used as a negative class.

Within [`birdnet.py`](https://github.com/MarlonMueller/edge-iot/blob/main/src/model/birdnet.py) you can define your own CNN. Note that our [Jinja template](https://github.com/MarlonMueller/edge-iot/blob/main/src/templates/birdnet.jinja) currently only supports Conv2D, MaxPool2D, Flatten, FullyConnected and Softmax layers. Please refer to [this link](https://github.com/espressif/esp-dl/tree/master/include/layer) before including other layers to ensure that they are supported by ESP-DL. 
Note that although the app partition size is [extended by default](https://github.com/MarlonMueller/edge-iot/blob/main/sdkconfig.defaults#L46), larger networks might necessitate further size adjustments.

## Deployment

On a high-level, the pipeline consists of the following steps

1. Download audio files from Xeno Canto and ESC50 if not already present
2. Convert audio signal to normalized mel-frequency cepstral coefficients
3. Generate TensorFlow datasets from h5 file and annotation file
4. Train model (and optionally visualize training history)
5. Quantize model to int8 and int16 and evaluate performance
6. Render model to C++ code using Jinja template

and can be executed using

```bash
python pipeline.py
```

If successful, six model and weight C++ files will be placed in the [`src/model`](https://github.com/MarlonMueller/edge-iot/tree/main/src/model) directory: three for an **int8** quantized version of the trained model and three for an **int16** version.

Please note that multiple intermediate models will be generated during this process, which are not relevant for the ESP32 code. To run the model on the ESP32, integrate the generated `.cpp coefficient` file in [`src/esp32/CMakeLists.txt`](https://github.com/MarlonMueller/edge-iot/blob/main/src/esp32/CMakeLists.txt), for example

```
set(srcs
    main.cpp
    ../model/birdnet_int8_coefficient.cpp
)
```
 and include the model it [`main.cpp`](https://github.com/MarlonMueller/edge-iot/blob/main/src/esp32/main.cpp), e.g.,

 ```
#include "birdnet_int8.hpp"
```

Furthermore, ensure that the correct input quantization exponent input_exponent is set in [`main.cpp`](https://github.com/MarlonMueller/edge-iot/blob/main/src/esp32/main.cpp). You can find the coefficient as a comment in the model `.hpp` file, for instance: `input_exponent: ['-7']`, or alternatively in the quantization log.

Time and heap logging can be enabled inside the [`Kconfig.projbuild`](https://github.com/MarlonMueller/edge-iot/blob/main/src/esp32/Kconfig.projbuild) file. Ensure that the project is fully cleaned before compiling to ensure that the changes take effect.

Additionally, if modifications are made such as changing the number of species, it is necessary to adjust the ESP32 code accordingly.

For more insights, please consult [`pipeline.py`](https://github.com/MarlonMueller/edge-iot/blob/main/pipeline.py) and the respective Python modules within [`/src`](https://github.com/MarlonMueller/edge-iot/tree/main/src).


# Repository Structure

Please note that this provides only a high-level overview. Additionally, some data is not contained on GitHub but will be generated during processing.

```
./
├── .github/
│   └── workflows/
│       └── continuous_integration.yml
├── assets/
├── components/
│   └── espressif__esp-dsp
├── data/
│   ├── audio/
│   │   ├── ESC-50-master
│   │   ├── annotation.csv
│   │   └── audio_preprocessed.h5
├── docs/
│   ├── reports/
│   ├── slides/
│   └── [docs.md]*
├── include/ <- C++/C header
├── src/
│   ├── audio/ <- Microphone, audio preprocessing
│   ├── dataset/ <- Xeno Canto, ESC-50, TF datasets
│   ├── esp-dl/ <- (Hotfixed) ESP-DL
│   ├── esp-led/
│   ├── esp32/ <- ESP32-S3 main.cpp
│   ├── gps/
│   ├── heap-log/
│   ├── lora/
│   ├── model/ <- Python, C++ models
│   ├── quantization/
│   ├── rpi/ <- RPI main.cpp
│   ├── templates/ <- Jinja template
│   ├── utils/ <- Plots
│   ├── websocket/
│   └── wifi/
├── .gitignore
├── .pre-commit-config.yaml
├── CMakeLists.txt
├── Makefile.RPi
├── README.md
├── pipeline.py
├── requirements.txt
├── requirements_idf.txt
└── sdkconfig.defaults
```

# Further documentation

Please see [`docs/`](docs/) for further interesting documentation. Of special
relevance are:

- [`docs/audio_processing.md`](docs/audio_processing.md). It provides further
  information about how you could run the preprocessing in PC. 

- [`docs/lora_interconnection.md`](docs/lora_interconnection.md). It provides
  further information about the packages sent during in the system. 
