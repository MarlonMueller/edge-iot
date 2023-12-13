import librosa
import numpy as np
import matplotlib.pyplot as plt

def plot_image(image):
    """Plot an image"""
    plt.figure(figsize=(12, 2))
    plt.imshow(image)
    plt.axis("off")
    plt.show()

def plot_wave(path:str, sample_rate:int):
    """Plot the waveplot of an audio file"""
    plt.figure(figsize=(6, 1))
    audio, sr = librosa.load(path, sr=sample_rate)
    librosa.display.waveshow(audio, sr=sample_rate)
    plt.title('Waveplot')
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.show()
    
def plot_mel(path:str, sample_rate:int, window_size = 2048, hop_size = 512, n_mels = 128):
    """Plot the mel spectrogram of an audio file"""
    audio, sr = librosa.load(path, sr=sample_rate)
    stft = librosa.stft(audio, n_fft=window_size, hop_length=hop_size)
    mel_spec = librosa.feature.melspectrogram(S=librosa.amplitude_to_db(np.abs(stft)), sr=sr, n_mels=n_mels)
    log_mel_spec = librosa.power_to_db(mel_spec)

    plt.figure(figsize=(6, 1))
    librosa.display.specshow(log_mel_spec, sr=sample_rate)
    plt.title('Mel Spectogram')
    plt.xlabel('Time (s)')
    plt.ylabel('Hz')
    plt.show()

def plot_torch_results(num_epochs, training_losses, testing_losses, testing_accuracies):
    """Plot the training and testing losses & testing accuracy"""
    
    # Plotting training and testing losses, testing accuracy
    fig, ax1 = plt.subplots(figsize=(12, 6))

    # Plotting training loss on the left y-axis
    ax1.plot(range(1, num_epochs + 1), training_losses, label='Training Loss', marker='o', color='blue')
    ax1.plot(range(1, num_epochs + 1), testing_losses, label='Testing Loss', marker='o', color='green')
    ax1.set_xlabel('Epoch')
    ax1.set_ylabel('Loss', color='black')
    ax1.tick_params('y', colors='black')
    ax1.legend(loc='upper left')

    # Creating a secondary y-axis for accuracy on the right
    ax2 = ax1.twinx()
    ax2.plot(range(1, num_epochs + 1), testing_accuracies, label='Testing Accuracy', marker='o', color='orange')
    ax2.set_ylabel('Accuracy', color='black')
    ax2.tick_params('y', colors='black')
    ax2.legend(loc='upper right')

    fig.tight_layout()

    plt.savefig('metrics.pdf')
    plt.show()