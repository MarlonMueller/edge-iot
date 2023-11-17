import torch
import pandas as pd
from torch.utils.data import Dataset


# NOTE - alternative: torchaduio


class BirdDataset(Dataset):
    
    def __init__(self, annotation_file: str, audio_dir: str, transform=None):
        """Bird sound dataset"""
        self.annotations = pd.read_csv(annotation_file)
        self.audio_dir = audio_dir
        self.transform = transform

    def __len__(self):
        return len(self.annotations)

    def __getitem__(self, index):
        pass