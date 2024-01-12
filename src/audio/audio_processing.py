# -*- coding: utf-8 -*-

# preprocess.py
#
# Description: Preprocess audio files
# This file provides the functions for preprocessing audio files.
#
# IMPORTANT:
# Follow the instructions in src/audio/setup.py and set the correct path to the C library.

import os
import h5py
import ctypes
import librosa
import pathlib
import numpy as np
import pandas as pd
from typing import List
import matplotlib.pyplot as plt

import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Path to C library
CDLL = "build/lib.macosx-10.9-x86_64-cpython-38/preprocess.cpython-38-darwin.so"
# CDLL = "build/lib.linux-x86_64-3.10/preprocess.cpython-310-x86_64-linux-gnu.so"


def mfcc(audio_data: np.array):
    """Compute MFCC features via C interface.

    :param audio_data: raw audio data
    :return: Mel-frequency cepstral coefficients
    """

    lib = ctypes.CDLL(CDLL)

    lib.mfcc.argtypes = [ctypes.POINTER(ctypes.c_int16),
                         ctypes.c_size_t,
                         ctypes.POINTER(ctypes.POINTER(ctypes.POINTER(ctypes.c_float))),
                         ctypes.POINTER(ctypes.c_size_t)]
    
    lib.mfcc.restype = ctypes.c_int

    audio_data = np.array(audio_data, dtype = np.int16)
    audio_values_ptr = ctypes.cast(audio_data.ctypes.data, ctypes.POINTER(ctypes.c_int16))

    output_frames = ctypes.c_size_t()
    output = ctypes.POINTER(ctypes.POINTER(ctypes.c_float))()

    lib.malloc_mfcc_module()
    lib.mfcc(audio_values_ptr, len(audio_data), ctypes.byref(output), ctypes.byref(output_frames))
    lib.free_mfcc_module()

    num_mfcc = int(lib.get_num_mfcc())

    mfcc = np.zeros((output_frames.value, num_mfcc))

    for i in range(output_frames.value):
        for j in range(num_mfcc):
            mfcc[i][j] = output[i][j]

    return mfcc


def _load_audio(audio_path: pathlib.Path, sampling_rate, duration):
    """Load audio file, normalize and convert to int16.

    :param audio_path: audio file path
    :param sampling_rate: sampling_rate
    :param duration: duration in seconds
    :return: audio data
    """
    y, sr = librosa.load(
        audio_path, sr=sampling_rate, duration=duration, mono=True
    )
    y = librosa.util.fix_length(y, size=int(sampling_rate * duration))
    
    # [-1, +1]
    y = y / np.max(np.abs(y))
    
    # float -> int16
    y = np.round(y * 32767).astype(np.int16)
    
    return y


def load_audio(
    idxs: List[int],
    audio_dir: pathlib.Path,
    annotation_path: pathlib.Path,
    sampling_rate,
    duration,
):
    """Load audio files.

    :param idxs: idxs of files in annotation file
    :param audio_dir: audio directory
    :param annotation_path: annotation file path
    :param sampling_rate: sampling_rate
    :param duration: duration in seconds
    :return: audio data
    """

    df = pd.read_csv(annotation_path)
    df = df[df["idx"].isin(idxs)]

    y_all = []
    for idx, row in df.iterrows():
        file_ = os.path.join(audio_dir, row["file_name"])
        y = _load_audio(file_, sampling_rate, duration)
        y_all.append(y)

    y_all = np.vstack(y_all)
    return y_all


def load_mfcc(
    idxs: List[int],
    audio_dir: pathlib.Path,
    annotation_path: pathlib.Path,
    sampling_rate,
    duration,
):
    """Load mfcc features.

    :param idxs: idxs of files in annotation file
    :param audio_dir: audio directory
    :param annotation_path: annotation file path
    :param sampling_rate: sampling_rate
    :param duration: duration in seconds
    """
    y_in = load_audio(idxs, audio_dir, annotation_path, sampling_rate, duration)
    y_out = []
    for y in y_in:
        out = mfcc(y)
        # -> [0, 1]
        out = (out - np.min(out)) / (np.max(out) - np.min(out))
        y_out.append(out)
    y_out = np.array(y_out)
    return y_out


def statistics(y: np.array):
    """Compute statistics.

    :param y: audio data
    :return: dictionary with mean, std, min, max, shape
    """
    stats = {
        "mean": np.mean(y),
        "std": np.std(y),
        "min": np.min(y),
        "max": np.max(y),
        "shape": y.shape,
    }
    logger.info(stats)
    return stats


def preprocess_audio(
    data_dir: pathlib.Path,
    audio_dir: pathlib.Path,
    annotation_path: pathlib.Path,
    sampling_rate,
    duration,
    h5file,
):
    """Preprocess audio files.

    :param data_dir: data directory
    :param audio_dir: audio directory
    :param annotation_path: annotation file path
    :param sampling_rate: sampling_rate
    :param duration: duration in seconds
    :param h5file: HDF5 file name
    """

    h5path = data_dir / h5file
    if not os.path.exists(h5path):
        df = pd.read_csv(annotation_path)

        with h5py.File(h5path, "w") as f:
            shape_mfcc = load_mfcc(
                [0], audio_dir, annotation_path, sampling_rate, duration
            ).shape
            f.create_dataset("data", shape=(len(df), *shape_mfcc))
            logger.info(f"Creating HDF5 with shape {(len(df), *shape_mfcc)}")

            for _, row in df.iterrows():
                idx = row["idx"]
                mfcc = load_mfcc(
                    [idx], audio_dir, annotation_path, sampling_rate, duration
                ) 
                
                f["data"][idx, :, :, :] = mfcc

    else:
        logger.info("HDF5 file already exists. Skipping preprocessing.")


def load_data(idxs: List[int], h5_path: pathlib.Path):
    """Load processed audio data.

    :param idx: idxs of files in annotation file
    :param data_dir: data directory
    :param h5file: HDF5 file name
    """

    with h5py.File(h5_path, "r") as f:
        data = f["data"][idxs, :, :, :]
        data = np.array(data, dtype=np.float32)
        
    return data
