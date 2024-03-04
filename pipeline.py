import logging

import os
import sys
import pathlib
import pandas as pd

from src.utils import visualization
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

DATA_DIR = PATH / "data"
AUDIO_DIR = DATA_DIR / "audio"
ANNOTATION_PATH = DATA_DIR / "annotation.csv"
MODEL_DIR = PATH / "src" / "model"
TEMPLATE_DIR = PATH / "src" / "templates"

ASSETS_DIR = PATH / "assets"

H5FILE = "audio_preprocessed.h5"


if __name__ == "__main__":

    """
    Main Python script. Note that the pipeline includes multiple steps:
    1. Download audio files from xeno canto and esc50 if not already present
    2. Convert audio signal to mel-frequency cepstral coefficients (MFCC)
    3. Generate TensorFlow datasets from h5 file and annotation file
    4. Train model and optionally visualize training history
    5. Quantize model to int8 and int16 and evaluate performance
    6. Render model to C++ code using Jinja templates
    
    Please refer to the individual modules within /src for more details.
    """

    MODEL_NAME = f"birdnet_default"

    # https://xeno-canto.org/explore/api
    query = {
        "grp": "1",
        "len": "4-6",
    }

    # Download audio files
    if not os.listdir(AUDIO_DIR) == [".gitkeep"]:

        if os.path.isfile(ANNOTATION_PATH):
            logger.info("Removing existing annotation file.")
            os.remove(ANNOTATION_PATH)

        if os.path.isfile(DATA_DIR / H5FILE):
            logger.info("Removing existing h5 file.")
            os.remove(DATA_DIR / H5FILE)

        # Download xeno canto audio
        species_map = xeno_canto.download_xeno_canto_audio(
            query, NUM_SPECIES, AUDIO_DIR, ANNOTATION_PATH, ASSETS_DIR
        )

        # Download esc50 audio
        esc50.download_esc50_audio(NUM_SPECIES, DATA_DIR, AUDIO_DIR, ANNOTATION_PATH)
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

    # Convert audio signal to MFCC
    audio_processing.preprocess_audio(
        DATA_DIR, AUDIO_DIR, ANNOTATION_PATH, SAMPLE_RATE, DURATION, H5FILE
    )

    # Generate datasets
    mfcc_example = audio_processing.load_features([0], DATA_DIR / H5FILE)
    mfcc_shape = mfcc_example.squeeze().shape

    train_dataset, test_dataset, train_size, test_size = tensorflow.get_dataset(
        DATA_DIR, ANNOTATION_PATH, H5FILE, mfcc_shape
    )

    # Initialize model
    birdnet_model = birdnet.birdnet_model(MODEL_NAME, mfcc_shape, NUM_SPECIES + 1)
    birdnet_model.summary()

    # Train model
    _, history = tensorflow.train_model(
        MODEL_DIR, MODEL_NAME, birdnet_model, train_dataset, test_dataset
    )

    visualization.plot_history(ASSETS_DIR, MODEL_NAME, history)

    # Prepare for quantization
    sys.path.append(os.path.join(PATH, "src", "quantization", "linux"))
    from calibrator import *
    from evaluator import *

    calib_method = "minmax"
    target_chip = "esp32s3"
    granularity = "per-tensor"

    # Quantize and evaluate model
    for quantization_bit in ["int8", "int16"]:

        calibrator = Calibrator(quantization_bit, granularity, calib_method)
        evaluator = Evaluator(quantization_bit, granularity, target_chip)

        quantization_log = quantization.quantize_model(
            birdnet_model,
            MODEL_DIR,
            MODEL_NAME,
            quantization_bit,
            calibrator=calibrator,
            calibration_dataset=test_dataset,
        )

        # visualization.plot_quantized_confusion_matrices(
        #     ASSETS_DIR,
        #     MODEL_DIR,
        #     MODEL_NAME,
        #     quantization_bit,
        #     evaluator,
        #     train_dataset,
        #     test_dataset,
        #     train_size,
        #     test_size,
        #     class_names=["WR", "CW", "CB", "Other"],
        # )

        acc_train, acc_train_quant = quantization.evaluate_model(
            MODEL_DIR,
            MODEL_NAME,
            quantization_bit,
            train_dataset,
            train_size,
            evaluator,
        )
        acc_test, acc_test_quant = quantization.evaluate_model(
            MODEL_DIR, MODEL_NAME, quantization_bit, test_dataset, test_size, evaluator
        )

        evaluation = f"acc_train: {acc_train}, acc_train_quant: {acc_train_quant}, acc_test: {acc_test}, acc_test_quant: {acc_test_quant}"

        # Extract NN layer information
        input_exponent, layer_information = template_constructor.get_layer_information(
            birdnet_model, quantization_log
        )

        model_name_quantized = f"{MODEL_NAME}_{quantization_bit}"

        # Aggregate tags for Jinja template
        tags = {
            "model_name": model_name_quantized,
            "input_exponent": input_exponent,
            "quantization_bit": quantization_bit,
            "evaluation": evaluation,
            "layers": template_constructor.get_jinja_information(layer_information),
        }

        # Render model to C++ code using Jinja templates
        template_constructor.render_template(
            TEMPLATE_DIR, MODEL_DIR, model_name_quantized, tags
        )
