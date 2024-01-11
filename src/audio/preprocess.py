# -*- coding: utf-8 -*-

# preprocess.py
# 
# Description: Preprocess audio files
# This file provides the functions for preprocessing audio files.
# 
# IMPORTANT: 
# Before using this program, follow the instructions in src/audio/setup.py

import numpy as np
import matplotlib.pyplot as plt
import librosa
import ctypes # for C library usage

# Functions

def mfcc(wav: np.array):
    '''
    Description: Calculate MFCC of a wav array of int16.
    Parameters: 
        wav - wav array of int16 (typical wav values are not int16, so reescaling is needed)
    Return: 
        mfcc - MFCC of wav
    '''

    # Calculate data from C library

    lib = ctypes.CDLL('build/lib.linux-x86_64-3.10/preprocess.cpython-310-x86_64-linux-gnu.so') # Change this line to the path of the C library

    lib.mfcc.argtypes = [ctypes.POINTER(ctypes.c_int16), ctypes.c_size_t, ctypes.POINTER(ctypes.POINTER(ctypes.POINTER(ctypes.c_float))), ctypes.POINTER(ctypes.c_size_t)]
    lib.mfcc.restype = ctypes.c_int

    wav = np.array(wav, dtype = np.int16) # sanity check 
    wav_values_ptr = ctypes.cast(wav.ctypes.data, ctypes.POINTER(ctypes.c_int16))

    output_frames = ctypes.c_size_t()
    output = ctypes.POINTER(ctypes.POINTER(ctypes.c_float))()

    lib.malloc_mfcc_module()
    lib.mfcc(wav_values_ptr, len(wav), ctypes.byref(output), ctypes.byref(output_frames))
    lib.free_mfcc_module()

    num_mfcc = int(lib.get_num_mfcc())

    # Convert data to numpy array

    mfcc = np.zeros((output_frames.value, num_mfcc))

    for i in range(output_frames.value):
        for j in range(num_mfcc):
            mfcc[i][j] = output[i][j]

    return mfcc

if __name__ == "__main__":
    input = np.array(np.random.randn(16000) * 2 ** 15, dtype = np.int16)
    output = mfcc(input)

    plt.figure(figsize=(10, 4))

    librosa.display.specshow(output.T)

    plt.colorbar()
    plt.title("MFCC")
    plt.tight_layout()

    plt.show()