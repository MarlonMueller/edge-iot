import os
import httpx
import shutil
import pathlib
import pandas as pd

import logging
logging.basicConfig(level=logging.INFO)
logging.getLogger("httpx").setLevel(logging.WARNING)
logger = logging.getLogger(__name__)


CLASSES = [
    "dog",
    "rooster",
    "pig",
    "cow",
    "frog",
    "cat",
    "hen",
    "insects",
    "sheep",
    "crow",
    "rain",
    #"sea_waves",
    "crackling_fire",
    "crickets",
    "water_drops",
    "wind",
    "pouring_water",
    #"toilet_flush",
    "thunderstorm",
    "crying_baby",
    "sneezing",
    "clapping",
    "breathing",
    "coughing",
    "footsteps",
    "laughing",
    #"snoring",
    "drinking_sipping",
    "door_wood_creaks",
    "helicopter",
    "chainsaw",
    "siren",
    "car_horn",
    "engine",
    "train",
    "church_bells",
    "airplane",
    "fireworks",
    "hand_saw",
]

URL = "https://github.com/karoldvl/ESC-50/archive/master.zip"

def _get_zip(url: str, path: str):
    """Download zip file from url to path.

    :param url: url to download from
    :param path: path to save to
    """

    with httpx.stream("GET", url, follow_redirects=True) as response:
        with open(path, "wb") as file:
            for chunk in response.iter_bytes():
                file.write(chunk)

def download_esc50_audio(
    cls_id: int,
    data_dir: pathlib.Path,
    audio_dir: pathlib.Path,
    annotation_path: pathlib.Path):
    """Extract audio files, copy to audio_dir and append to annotation file using a new class id.

    :param cls_id: class id to use for ESC-50   
    :param data_dir: data directory
    :param audio_dir: audio directory
    :param annotation_path: annotation file path
    """
    
    if not os.path.isdir(data_dir / "ESC-50-master"):
    
        logger.info("Downloading ESC-50 audio files ...")
        
        zip_path = data_dir / "ESC-50.zip"
        if not os.path.isfile(zip_path):
            _get_zip(URL, zip_path)
        
        shutil.unpack_archive(zip_path, data_dir)
        os.remove(zip_path)
        
    else:
        logger.info("ESC-50 audio files already downloaded.")
        
    old_audio_dir = data_dir / "ESC-50-master" / "audio"
    old_annotation_path = data_dir / "ESC-50-master" / "meta" / "esc50.csv"
    
    df = pd.read_csv(old_annotation_path)
    df = df[df["category"].isin(CLASSES)]
    
    os.makedirs(audio_dir, exist_ok=True)
    
    for index, row in df.iterrows():
        
        file_name = row["filename"]
        freesound_id = row["src_file"]
        slice_ = row['take']
        
        new_file_name = f"e-{freesound_id}-{cls_id}-{slice_}.wav"

        # Copy the audio file to the target directory
        source_path = os.path.join(old_audio_dir, file_name)
        target_path = os.path.join(audio_dir, new_file_name)
        shutil.copyfile(source_path, target_path)
        
    assert os.path.exists(annotation_path)
    
    df_old_annotations = pd.read_csv(annotation_path)
    last_idx = df_old_annotations.iloc[-1, 0]

    df_new_annotations = pd.DataFrame({
        "idx": "",
        # "idx": [last_idx + i + 1 for i in range(len(df))],
        "file_name": [f"e-{row['src_file']}-{cls_id}-{row['take']}.wav" for _, row in df.iterrows()],
        "class_id": cls_id,
        "class": "other"
    })

    # Append to existing annotation file
    df_new_annotations.to_csv(annotation_path, mode="a", header=False, index=False)

            