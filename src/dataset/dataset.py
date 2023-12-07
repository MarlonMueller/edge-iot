import os
import h5py
import torch
import numpy as np
import pandas as pd
from torch.utils.data import Dataset


# NOTE - alternative: torchaduio

from src.audio import preprocess as preprocessing


class BirdDataset(Dataset):
    def __init__(
        self,
        h5py_path: str,
        annotation_path: str,
        audio_dir: str = None,
        transform=None,
    ):
        self.labels = pd.read_csv(annotation_path)
        self.hp5y_path = h5py_path
        self.transform = transform

        if not os.path.exists(h5py_path):
            assert audio_dir is not None

            with h5py.File(h5py_path, "w") as f:
                # TODO - more robust
                audio_path = os.path.join(audio_dir, self.labels.iloc[0, 0])
                mfcc = preprocessing.mfcc_from_file(audio_path, False)

                shape = (len(self.labels), *mfcc.shape)
                print("Creating HDF5 shape", shape)
                f.create_dataset("data", shape=shape)

            with h5py.File(self.hp5y_path, "a") as f:
                for idx, row in self.labels.iterrows():
                    audio_path = os.path.join(audio_dir, row["file_name"])
                    data = preprocessing.mfcc_from_file(audio_path)

                    if data.shape != shape[1:]:
                        padding = shape[2] - data.shape[1]  # Time dimension padding
                        print(f"Warning {audio_path} of by {padding} steps")
                        data = np.pad(data, ((0, 0), (0, padding)), mode="edge")

                    f["data"][idx, :, :] = data

    def __len__(self):
        return len(self.labels)

    def __getitem__(self, idx):
        with h5py.File(self.hp5y_path, "r") as f:
            data = f["data"][idx, :, :]

        data_min, data_max = data.min(), data.max()
        data = (data - data_min) / (data_max - data_min)

        label = self.labels.iloc[idx, 1]

        if self.transform:
            data = self.transform(data)

        #print(torch.max(data), torch.min(data), torch.mean(data), torch.std(data))
        # FIXME - uniform mfcc length

        return data, label
