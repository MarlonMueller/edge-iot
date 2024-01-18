import logging

import io
import sys
import librosa
import pathlib
import numpy as np
import pandas as pd
import tensorflow as tf
import os
from onnx import checker
from contextlib import redirect_stdout

from matplotlib import pyplot as plt

from src.dataset import xeno_canto, esc50, tensorflow
from src.audio import audio_processing
from src.procedures import optimize
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
CHECKPOINT_DIR = MODEL_DIR / "checkpoints"

H5FILE = "audio_preprocessed.h5"

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


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
    train_dataset, test_dataset = tensorflow.get_dataset(DATA_DIR, ANNOTATION_PATH, H5FILE, mfcc_shape)   
    
    birdnet_model = birdnet.birdnet_model(mfcc_shape, NUM_SPECIES + 1, MODEL_NAME)
    birdnet_model.summary()
    
    print(get_layer_info(birdnet_model))
    
    # _, history = tensorflow.train_model(birdnet_model, train_dataset, test_dataset, CHECKPOINT_DIR)
    # plot_metrics(history)
    # confusion(birdnet_model, train_dataset, test_dataset)
    

    """tf_model_path = str(MODEL_DIR / "tf_birdnet" / f"{MODEL_NAME}")
    onnx_model_path = str(MODEL_DIR / "onnx_birdnet" / f"{MODEL_NAME}.onnx")
    
    birdnet_model.load_weights(CHECKPOINT_DIR / "20.ckpt")
    tf.saved_model.save(birdnet_model, tf_model_path)
    
    os.system(f"python -m tf2onnx.convert --saved-model {tf_model_path} --output {onnx_model_path} --opset 13")
    onnx_optimized_path = optimize.optimize_fp_model(onnx_model_path)
    
    sys.path.append(PATH / "src" / "procedures" / "calibrate" / "linux"))
    from calibrator import *
    from evaluator import *

    target_chip = "esp32s3"
    quantization_bit = "int16"
    granularity = "per-tensor"
    calib_method = "minmax"
    provider = "CPUExecutionProvider"

    calib_data = []
    for (data, labels) in test_dataset.as_numpy_iterator():
        calib_data.append(data)

    calib_data = np.concatenate(calib_data, axis=0)
    
    optimized_model_path =  MODEL_DIR / "onnx_birdnet" / f"{MODEL_NAME}_optimized.onnx"
    model_proto = onnx.load_model(optimized_model_path)
    checker.check_graph(model_proto.graph)    
    
    calib = Calibrator(quantization_bit, granularity, calib_method)
    calib.set_providers([provider])
    
    cpp_file_name = "birdnet_coefficient"
    quantization_params_path = MODEL_DIR / f"{MODEL_NAME}_quantization_params.pickle"
    
    calib.generate_quantization_table(model_proto, calib_data, quantization_params_path)

    f = io.StringIO()
    with redirect_stdout(f):
        calib.export_coefficient_to_cpp(
            model_proto, quantization_params_path, target_chip, str(MODEL_DIR), cpp_file_name, True
        )
    log = f.getvalue()
    print(log)"""
    
    
    """
    # Extract layer names and exponents
    pattern = r'name: \/?(?P<name>\w+)(?:\/\w+)?, (?:output_)?exponent: (?P<exponent>-?\d+)'
    matches = re.findall(pattern, log)
    log_data = [(match[0].lower(), int(match[1])) for match in matches]
    
    ########################################################
    #  JINJA
    ########################################################
    
    # Load the pickle file
    #with open(quantization_params_path, 'rb') as f:
    #    data = pickle.load(f)


    tags = {
        "model_name": model_name,
        "quantization_bit": quantization_bit,
        "layers": procedures.get_layer_info(model, log_data)
    }
    
    
    template_dir = os.path.join(PATH, "src", "templates")
    utils.render_template(template_dir, "model", tags, cpp_dir, f"{model_name}_model")
    

    ########################################################
    #  QUANTIZATION EVALUATION
    ########################################################

    evaluator = Evaluator(quantization_bit, granularity, target_chip)
    evaluator.set_providers([provider])
    evaluator.generate_quantized_model(model_proto, quantization_params_path)
   
    m = rt.InferenceSession(onnx_optimized_path, providers=[provider])

    input_name = m.get_inputs()[0].name
    output_names = [n.name for n in model_proto.graph.output]

    correct = 0
    correct_quantized = 0

    for i in range(int(len(test_data)/batch_size)):

        #TODO: Validate
        
        data = [test_data[j] for j in range(i * batch_size, (i+1) * batch_size)]
        x = np.array([s[0].numpy() for s in data])
        y = [s[1] for s in data]

        [out, _] = evaluator.evalute_quantized_model(x, False)
        correct_quantized += sum(np.argmax(out[0], axis=1) == y)

        out = m.run(output_names, {input_name: x.astype(np.float32)})
        correct += sum(np.argmax(out[0], axis=1) == y)

    print(f"Accuracy of fp32 model: {correct / len(test_data):.4f}")
    print(f"Accuracy of {quantization_bit} model: {correct_quantized / len(test_data):.4f}")
    
    """