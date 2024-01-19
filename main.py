import logging

import os
import io
import sys
import librosa
import pathlib
import numpy as np
import pandas as pd
import tensorflow as tf
from onnx import checker
from contextlib import redirect_stdout

from matplotlib import pyplot as plt

from src.templates import template_constructor
from src.quantization import quantization
from src.dataset import xeno_canto, esc50, tensorflow
from src.audio import audio_processing
from src.model import birdnet

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

PATH = pathlib.Path(__file__).parent.resolve()

DURATION = 5
NUM_SPECIES = 3
SAMPLE_RATE = 16000

MODEL_NAME = "birdnet_default"

DATA_DIR = PATH / "data"
AUDIO_DIR = DATA_DIR / "audio"
ANNOTATION_PATH = DATA_DIR / "annotation.csv"

MODEL_DIR = PATH / "src" / "model"
TEMPLATE_DIR = PATH / "src" / "templates"

H5FILE = "audio_preprocessed.h5"


def download_audio():
    """Download audio files and generate annotation files.
    This can take a while, e.g., depending on the query.
    By default, existing files are not downloaded again.
    """

    # https://xeno-canto.org/explore/api

    query = {
        "grp": "1",
        #"area": "europe",
        # "cnt": "brazil",
        "len": "4-6",
        # "q": "A",
        # "lic": "cc",
        # "box": "LAT_MIN,LON_MIN,LAT_MAX,LON_MAX",
        # "smp": "1",
        # "since": "2021-07-01",
    }

    if not os.listdir(AUDIO_DIR):
        
        if os.path.isfile(ANNOTATION_PATH):
            logger.info("Removing existing annotation file.")
            os.remove(ANNOTATION_PATH)
            
        if os.path.isfile(DATA_DIR / H5FILE):
            logger.info("Removing existing h5 file.")
            os.remove(DATA_DIR / H5FILE)
    
        species_map = xeno_canto.download_xeno_canto_audio(
            query, NUM_SPECIES, AUDIO_DIR, ANNOTATION_PATH
        )

        esc50.download_esc50_audio(
            NUM_SPECIES, DATA_DIR, AUDIO_DIR, ANNOTATION_PATH
        )
        species_map["other"] = NUM_SPECIES
        
        # Index annotation file
        df = pd.read_csv(ANNOTATION_PATH)
        df = df.sample(frac=1).reset_index(drop=True)
        for idx, row in df.iterrows():
            df.loc[idx, "idx"] = str(idx)
        df.to_csv(ANNOTATION_PATH, index=False)
        
        logger.info(f"Species map: {species_map}")
        
    else:
        logger.info("Audio files already downloaded.")
    

####

def plot_metrics(history):

    metrics = [metric for metric in history.history.keys() if not metric.startswith('val_')]

    plt.figure(figsize=(15, len(metrics) * 4))

    for i, metric in enumerate(metrics, 1):
        plt.subplot(len(metrics), 2, i)
        plt.plot(history.history[metric], label=f'Train {metric}')
        plt.plot(history.history[f'val_{metric}'], label=f'Validation {metric}')
        plt.title(f'Model {metric}')
        plt.xlabel('Epoch')
        plt.ylabel(metric)
        plt.legend(loc='upper left')

    plt.tight_layout()
    plt.show()


def confusion(model, train_dataset, test_dataset):
    
    from sklearn.metrics import confusion_matrix
    for dataset in (train_dataset, test_dataset):
        y_true = np.concatenate([y.numpy() for _, y in dataset])
        y_pred_probs = model.predict(dataset)
        y_pred = np.argmax(y_pred_probs, axis=1)
        conf_matrix = confusion_matrix(y_true, y_pred)
        print(conf_matrix)

    plt.show()

####


if __name__ == "__main__":
    
    np.random.seed(0)
    tf.random.set_seed(0)
    
    species_map = download_audio()

    audio_processing.preprocess_audio(
        DATA_DIR, AUDIO_DIR, ANNOTATION_PATH, SAMPLE_RATE, DURATION, H5FILE
    )

    # for i in range(5):
    #     mfcc_example = audio_processing.load_data([i], DATA_DIR / H5FILE)
    #     plt.figure(figsize=(10, 4))
    #     librosa.display.specshow(mfcc_example.T)
    #     plt.colorbar()
    #     plt.title("MFCC")
    #     plt.tight_layout()

    #     plt.show()
        

    mfcc_example = audio_processing.load_data([0], DATA_DIR / H5FILE)
    mfcc_shape = mfcc_example.squeeze().shape
    train_dataset, test_dataset, train_size, test_size = tensorflow.get_dataset(DATA_DIR, ANNOTATION_PATH, H5FILE, mfcc_shape)   
    
    print("Size", train_size, test_size)

    birdnet_model = birdnet.birdnet_model(MODEL_NAME, mfcc_shape, NUM_SPECIES + 1)
    #birdnet_model.summary()
    
    _, history = tensorflow.train_model(MODEL_DIR, MODEL_NAME, birdnet_model, train_dataset, test_dataset, num_epochs=10)
    # plot_metrics(history)
    # confusion(birdnet_model, train_dataset, test_dataset)

    sys.path.append(os.path.join(PATH, "src", "quantization", "linux"))
    from calibrator import *
    from evaluator import *

    calib_method="minmax"
    target_chip = "esp32s3"
    granularity="per-tensor"
    quantization_bit="int8"

    calibrator = Calibrator(quantization_bit, granularity, calib_method)
    evaluator = Evaluator(quantization_bit, granularity, target_chip)

    quantization_exponents = quantization.quantize_model(
        birdnet_model,
        MODEL_DIR,
        MODEL_NAME,
        calibrator=calibrator,
        calibration_dataset=test_dataset,
    )

    acc_train, acc_train_quant = quantization.evaluate_model(MODEL_DIR, MODEL_NAME, train_dataset, train_size, evaluator)
    acc_test, acc_test_quant = quantization.evaluate_model(MODEL_DIR, MODEL_NAME, test_dataset, test_size, evaluator)

    print(acc_train, acc_train_quant, acc_test, acc_test_quant)
    
    input_exponent, layer_information = template_constructor.get_layer_information(birdnet_model, quantization_exponents)
    
    tags = {
        "model_name": MODEL_NAME,
        "input_exponent": input_exponent,
        "quantization_bit": quantization_bit,
        "layers": template_constructor.get_jinja_information(layer_information)
    }

    template_constructor.render_template(
        TEMPLATE_DIR,
        MODEL_DIR,
        MODEL_NAME,
        tags
    )