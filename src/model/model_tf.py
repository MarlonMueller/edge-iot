import tensorflow as tf
from tensorflow.keras import Sequential
from tensorflow.keras.layers import Conv2D, ReLU, MaxPooling2D, Flatten, Dense

def create_custom_model(input_size, num_classes):
    print(f"Size of input: {input_size}")
    channels, height, width = input_size

    model = Sequential()

    # Features
    model.add(Conv2D(16, kernel_size=(5, 3), strides=(3, 1), activation="relu", padding="valid", input_shape=(height, width, channels)))  # 183,32 -> 61, 30
    model.add(MaxPooling2D(pool_size=(2, 1), strides=(2, 1), padding="valid"))  # 31, 30
    model.add(Conv2D(16, kernel_size=3, strides=(1, 1), activation="relu", padding="valid"))  # 29, 28
    model.add(MaxPooling2D(pool_size=(2, 2), strides=(2, 2), padding="valid"))  # 15, 14
    model.add(Conv2D(8, kernel_size=3, strides=(1, 1), activation="relu", padding="valid"))  # 13, 12
    model.add(MaxPooling2D(pool_size=(2, 2), strides=(2, 2), padding="valid"))  # 7, 6

    # Classifier
    model.add(Flatten())
    model.add(Dense(64, activation="relu"))
    model.add(Dense(num_classes, activation='softmax'))

    return model


"""
model input name: conv2d_input, exponent: -15
Reshape layer name: StatefulPartitionedCall/sequential/conv2d/BiasAdd__6, output_exponent: -15
Conv layer name: StatefulPartitionedCall/sequential/conv2d/BiasAdd, output_exponent: -15
MaxPool layer name: StatefulPartitionedCall/sequential/max_pooling2d/MaxPool, output_exponent: -15
Conv layer name: StatefulPartitionedCall/sequential/conv2d_1/BiasAdd, output_exponent: -14
MaxPool layer name: StatefulPartitionedCall/sequential/max_pooling2d_1/MaxPool, output_exponent: -14
Conv layer name: StatefulPartitionedCall/sequential/conv2d_2/BiasAdd, output_exponent: -12
MaxPool layer name: StatefulPartitionedCall/sequential/max_pooling2d_2/MaxPool, output_exponent: -12
Transpose layer name: StatefulPartitionedCall/sequential/max_pooling2d_2/MaxPool__28, output_exponent: -12
Reshape layer name: StatefulPartitionedCall/sequential/flatten/Reshape, output_exponent: -12
Gemm layer name: fused_gemm_0, output_exponent: -13
Gemm layer name: fused_gemm_1, output_exponent: -12
Softmax layer name: StatefulPartitionedCall/sequential/dense_1/Softmax, output_exponent: -15
"""
