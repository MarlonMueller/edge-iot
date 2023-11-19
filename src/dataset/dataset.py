import os
import torch
import pandas as pd
from torch.utils.data import Dataset


# NOTE - alternative: torchaduio

from src.audio import preprocess


class BirdDataset(Dataset):
    def __init__(self, annotation_path: str, audio_dir: str, transform=None):

        self.labels = pd.read_csv(annotation_path)
        self.audio_dir = audio_dir
        self.transform = transform

    def __len__(self):
        return len(self.labels)

    def __getitem__(self, idx):
        audio_path = os.path.join(self.audio_dir, self.labels.iloc[idx, 0])
        mfcc = preprocess.mfcc_from_file(audio_path, False)
        
        #FIXME - uniform mfcc length
        #print(audio_path, mfcc.shape)
        mfcc = mfcc[:, :3000]
        
        label = self.labels.iloc[idx, 1]
        
        if self.transform:
            mfcc = self.transform(mfcc)
        
        return mfcc, label
