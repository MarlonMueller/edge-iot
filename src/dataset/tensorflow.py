from typing import Tuple

import logging
import pathlib
import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras import optimizers, losses, metrics, callbacks

from src.audio import audio_processing

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

def get_dataset(data_dir: pathlib.Path, annotation_path: pathlib.Path, h5file:str, mfcc_shape: Tuple[int, int], train_test_split:float =0.8, batch_size:int =16) -> tf.data.Dataset:
    """Create a TensorFlow dataset from the preprocessed audio data.

    :param data_dir: data directory
    :param annotation_path: annotation file path
    :param h5file: h5 file name 
    :param mfcc_shape: shape of the mfcc features
    :param train_test_split: train_test_split, defaults to 0.8
    :param batch_size: batch_size, defaults to 16
    :return: tuple of train and test dataset
    """
    
    labels = pd.read_csv(annotation_path)
    labels.set_index("idx", inplace=True)
    h5_path = data_dir / h5file

    def _generator_fn():
        for idx in labels.index:
            data = audio_processing.load_data([idx], h5_path)
            data = np.squeeze(data, axis=0)
            data = np.transpose(data, (1, 2, 0))
            label = labels.loc[idx, "class_id"]
            yield data, np.array([label], dtype=np.int32)


    dataset = tf.data.Dataset.from_generator(
        _generator_fn,
        output_signature=(
            tf.TensorSpec(shape=(*mfcc_shape, 1), dtype=tf.float32),
            tf.TensorSpec(shape=(1, ), dtype=tf.int32),
        ),
    )

    
    dataset_size = labels.index.max() + 1
    train_size = int(train_test_split * dataset_size)
    test_size = dataset_size - train_size
    
    train_dataset = dataset.take(train_size).shuffle(buffer_size=1024).batch(batch_size).prefetch(buffer_size=tf.data.experimental.AUTOTUNE)
    test_dataset = dataset.skip(train_size).batch(batch_size).prefetch(buffer_size=tf.data.experimental.AUTOTUNE)
    
    logger.info(f"Train dataset size: {train_size}")
    logger.info(f"Test dataset size: {test_size}")
    
    return train_dataset, test_dataset


def train_model(model, train_dataset: tf.data.Dataset, test_dataset: tf.data.Dataset, checkpoint_dir:pathlib.Path, num_epochs:int=10, batch_size:int=16):
    """Train the model.

    :param model: tensorflow model
    :param train_dataset: train dataset
    :param test_dataset: test dataset
    :param checkpoint_dir: checkpoint directory
    :param num_epochs: num_epochs, defaults to 10
    :param batch_size: batch_size, defaults to 16
    :return: trained model and history
    """
    
    for file in checkpoint_dir.glob("*"):
        file.unlink()
    
    optimizer = optimizers.Adam()
    loss_fn = losses.SparseCategoricalCrossentropy()

    model.compile(
        optimizer=optimizer,
        loss=loss_fn,
        metrics=[metrics.SparseCategoricalAccuracy()]
    )
    
    checkpoint_path = checkpoint_dir / "{epoch}.ckpt"

    tf_callbacks = [
        callbacks.EarlyStopping(
            restore_best_weights=True,
            monitor="val_loss",
            patience=5,
        ),
        callbacks.ModelCheckpoint(
            filepath=checkpoint_path,
            save_weights_only=True,
            save_best_only=True,
            monitor="val_loss",
            mode="min",
            save_freq=1
        )
    ]
    
    history = model.fit(
        train_dataset,
        epochs=num_epochs,
        callbacks=tf_callbacks,
        validation_data=test_dataset,
    )

    return model, history