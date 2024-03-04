# Audio Processing

As a central element of the system, the different ESP32 nodes need to be able to
record information through their microphone and preprocess this information
for the internal ML model. 

## MFCC

Once the audio is recorded, it needs to be processed before feeding it to a 
Machine Learning model. Motivated by the state-of-the-art in speech recognition, 
we decide to use Mel Frequency Cepstral Coefficients (MFCC) as a feature 
extraction method. Therefore, its implementation on the ESP32 is required. 
Several steps are necessary:

- Implement FFT on the ESP32.
- Implement windowing functions (such as Hamming) on the ESP32.
- Implement the Mel filter bank on the ESP32.
- Implement the Discrete Cosine Transform (DCT) on the ESP32.

In order to do so optimally, we have relied on the usage of the
[ESP-DSP](https://www.github.com/espressif/esp-dsp) library. This library 
provides optimized implementations of some of the aforementioned functions. For 
functionalities absent in the library, we've crafted custom implementations,
trying to balance the performance and the memory footprint of the code. By
interconnecting the different functions, we've achieved a complete MFCC
implementation on the ESP32. The code can be found in `src/audio/preprocess.c`.

### Portability to PC

On the other hand, as this preprocessing is beneficial for the training of the
model, we've modified the library to ensure its applicability on a PC platform. 
As for the preprocessing of the code only some functions from 
[ESP-DSP](https://www.github.com/espressif/esp-dsp) are needed, we have 
ported those functions to the PC by maintaning a snapshot of the
original library, but with dependencies on the ESP32 removed. This snapshot
can be found in `include/sim_dsp`. Consequently, we have achieved
the portability of the preprocessing code to PC. This is beneficial for 
using the same code for both training and inferring. In order to create 
an object file for PC, the following command can be used:

```bash
gcc -c -D RUN_PC -I include/ src/audio/preprocess.c -o build/preprocess.o
```

The preprocessor directive `RUN_PC` is used to export all the snapshot code
to PC. An example can be found in `src/audio/pc_main.c`.