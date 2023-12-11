# -*- coding: utf-8 -*-

# preprocess.py
# Description: Preprocess audio files
# This file provides the functions for preprocessing audio files.

import numpy as np
import matplotlib.pyplot as plt
import librosa
import ctypes # for C library usage


# Functions

def mfcc(wav: int):

    # Calculate data from C library

    lib = ctypes.CDLL('./src/audio/preprocess.so') 

    lib.mfcc.argtypes = [ctypes.POINTER(ctypes.c_int8), ctypes.c_size_t, ctypes.POINTER(ctypes.POINTER(ctypes.POINTER(ctypes.c_float))), ctypes.POINTER(ctypes.c_size_t)]
    lib.mfcc.restype = ctypes.c_int

    wav_values = np.array(wav, dtype=np.int8)
    wav_values_ptr = ctypes.cast(wav_values.ctypes.data, ctypes.POINTER(ctypes.c_int8))

    output_frames = ctypes.c_size_t()
    output = ctypes.POINTER(ctypes.POINTER(ctypes.c_float))()

    lib.malloc_mfcc_module()
    lib.mfcc(wav_values_ptr, len(wav_values), ctypes.byref(output), ctypes.byref(output_frames))
    lib.free_mfcc_module()

    num_mfcc = int(lib.get_num_mfcc())

    # Convert data to numpy array

    mfcc = np.zeros((output_frames.value, num_mfcc))

    for i in range(output_frames.value):
        for j in range(num_mfcc):
            mfcc[i][j] = output[i][j]

    return mfcc

if __name__ == "__main__":
    output = mfcc([1 for i in range(16000)])

    plt.figure(figsize=(10, 4))

    librosa.display.specshow(output)

    plt.colorbar()
    plt.title("MFCC")
    plt.tight_layout()

    plt.show()