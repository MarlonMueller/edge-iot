# ESP Software Architecture

The software architecture for the ESP32 has been designed so that it can be
integrated within the 
[ESP-IDF build system](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html). 
Therefore, a modular approach has been followed. Each module can be 
identified with any folder under `src` with a `CMakeLists.txt` file.

The main parts of the software architecture are:

- `src/esp32/`. This folder contains the code for the main program that shall
  be executed on the ESP32. It is responsible for the initialization of the
  different components and the execution of the main loop.

- `src/audio/`. This folder contains the code for the audio processing. It
  includes the implementation of the preprocessing algorithms. Detailed 
  information can be found in `docs/audio_processing.md`.

