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