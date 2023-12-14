import tensorflow as tf
from tensorflow.keras import layers, models

class CustomModel(tf.keras.Model):
    def __init__(self, input_size, num_classes):
        super(CustomModel, self).__init__()

        print(f"Size of input: {input_size}")
        channels, height, width = input_size

        # Features
        self.conv1 = layers.Conv2D(16, kernel_size=(5, 3), strides=(3, 1), padding="valid", input_shape=(height, width, channels))  # 183,32 -> 61, 30
        self.relu1 = layers.ReLU()
        self.pool1 = layers.MaxPooling2D(pool_size=(2, 1), strides=(2, 1), padding="valid")  # 31, 30
        self.conv2 = layers.Conv2D(16, kernel_size=3, strides=(1, 1), padding="valid")  # 29, 28
        self.relu2 = layers.ReLU()
        self.pool2 = layers.MaxPooling2D(pool_size=(2, 2), strides=(2, 2), padding="valid")  # 15, 14
        self.conv3 = layers.Conv2D(8, kernel_size=3, strides=(1, 1), padding="valid")  # 13, 12
        self.relu3 = layers.ReLU()
        self.pool3 = layers.MaxPooling2D(pool_size=(2, 2), strides=(2, 2), padding="valid")  # 7, 6

        # Classifier
        self.flatten = layers.Flatten()
        self.fc1 = layers.Dense(64)
        self.relu4 = layers.ReLU()
        self.fc2 = layers.Dense(num_classes)

    def call(self, x):
        x = self.pool1(self.relu1(self.conv1(x)))
        x = self.pool2(self.relu2(self.conv2(x)))
        x = self.pool3(self.relu3(self.conv3(x)))
        x = self.flatten(x)
        x = self.fc2(self.relu4(self.fc1(x)))
        output = tf.nn.softmax(x, axis=-1)
        return output


