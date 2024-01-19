import logging
from typing import Tuple
from tensorflow.keras import models, layers

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

def birdnet_model(model_name:str, input_shape: Tuple[int, int], num_classes: int) -> models.Sequential:
    """Create a BirdNET model.

    :param model_name
    :param input_shape: height and width of the input
    :param num_classes: number of classes
    :return: birdnet model
    """
    
    if model_name == "birdnet_default":
        model = get_birdnet_default(input_shape, num_classes)
    
    return model
    
def get_birdnet_default(input_shape: Tuple[int, int], num_classes: int) -> models.Sequential:
    """Get the default BirdNET model.
    
    :return: default BirdNET model
    """
    
    # 5s -> (308, 32, 1)
    logger.info(f"Size of input: {input_shape}")
    
    model = models.Sequential()
    
    # (308, 32) -> (102, 30)
    model.add(layers.Conv2D(
        16,
        kernel_size=(5, 3),
        strides=(3, 1),
        activation="relu",
        padding="valid",
        input_shape=(*input_shape, 1),
        ))
    
    # (102, 30) -> (34, 30)
    model.add(layers.MaxPooling2D(
        pool_size=(3, 1),
        strides=(3, 1),
        padding="valid",
        ))
    
    # (34, 30) -> (32, 28)
    model.add(layers.Conv2D(
        16,
        kernel_size=3,
        strides=(1, 1),
        activation="relu",
        padding="valid",
        ))
    
    # (32, 28) -> (16, 14)
    model.add(layers.MaxPooling2D(
        pool_size=(2, 2),
        strides=(2, 2),
        padding="valid",
        ))
    
    # (16, 14) -> (14, 12)
    model.add(layers.Conv2D(
        8,
        kernel_size=3,
        strides=(1, 1),
        activation="relu",
        padding="valid",
        ))
    
    # (14, 12) -> (7, 6)
    model.add(layers.MaxPooling2D(
        pool_size=(2, 2),
        strides=(2, 2),
        padding="valid",
        ))
    
    # (8, 7, 6) -> (8 * 7 * 6)
    model.add(layers.Flatten())
    
    # (8 * 7 * 6) -> 64
    model.add(layers.Dense(64, activation="relu"))
    
    model.add(layers.Dense(num_classes, activation='softmax'))
    
    return model
