# -*- coding: utf-8 -*-

# preprocess.py
# Description: Preprocess audio files
# This file provides the functions for preprocessing audio files. 

import numpy as np
import matplotlib.pyplot as plt
import librosa
import argparse
import scipy

# CONSTANTS. Need to be adjusted for our application.
N_FFT = 512
HOP_LENGTH = 30
N_MELS = 40

# Functions

def mfcc_from_file(file: str, plot: bool = False) -> np.ndarray:
    """
    Compute the MFCC of an audio file.

    Args:
        file (str): Path to the audio file.

    Returns:
        mfcc (numpy.ndarray): MFCC of the audio file.
    """

    # Load the audio file
    y, sr = librosa.load(file)

    # Compute the MFCC
    mfcc = librosa.feature.mfcc(
        y=y, 
        sr=sr, 
        n_fft=N_FFT, 
        hop_length=HOP_LENGTH, 
        win_length = N_FFT,
        n_mfcc=N_MELS,
        htk = True,
        window = scipy.signal.windows.hann,
    )

    # Plot

    if plot:
        plt.figure(figsize=(10, 4))

        librosa.display.specshow(mfcc)

        plt.colorbar()
        plt.title('MFCC')
        plt.tight_layout()

        plt.show()

    return mfcc

if __name__ == '__main__':

    # Arguments parse

    parser = argparse.ArgumentParser()
    parser.add_argument('--file', type=str, help='Path to the audio file.', required=True)
    args = parser.parse_args()

    mfcc = mfcc_from_file(args.file, plot = True)
