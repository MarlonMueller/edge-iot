import os
import httpx
import shutil
import pandas as pd

#NOTE - First iteration; code could be improved

CLASSES = [
    "dog",
    #"rooster",
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

def _get_zip(url: str, path: str):
    """Download zip file from url to path"""

    #REVIEW - Optionally implement progress bar
    with httpx.stream("GET", url, follow_redirects=True) as response:
        print(response.status_code)
        with open(path, "wb") as file:
            for chunk in response.iter_bytes():
                file.write(chunk)


def get_esc50_audio(dir: str):
    
    os.makedirs(dir, exist_ok=True)
    # TODO - Do not download GitHub repo
    url = "https://github.com/karoldvl/ESC-50/archive/master.zip"
    
    zip_path = os.path.join(dir, "ESC-50.zip")
    if not os.path.isfile(zip_path):
        _get_zip(url, zip_path)
    
    shutil.unpack_archive(zip_path, dir)
    os.remove(zip_path)
    
def generate(
    cls_id: int,
    old_audio_dir: str,
    new_audio_dir: str,
    old_annotation_path: str,
    new_annotation_path: str):
    
    df = pd.read_csv(old_annotation_path)
    df = df[df["category"].isin(CLASSES)]
    
    os.makedirs(new_audio_dir, exist_ok=True)
    
    for index, row in df.iterrows():
        
        file_name = row["filename"]
        freesound_id = row["src_file"]
        slice_ = row['take']
        
        new_file_name = f"e-{freesound_id}-{cls_id}-{slice_}.wav"

        # Copy the audio file to the target directory
        source_path = os.path.join(old_audio_dir, file_name)
        target_path = os.path.join(new_audio_dir, new_file_name)
        shutil.copyfile(source_path, target_path)

    df_new_annotations = pd.DataFrame({
        "new_filename": [f"e-{row['src_file']}-{cls_id}-{row['take']}.wav" for _, row in df.iterrows()],
        "class_id": cls_id
    })
    if os.path.exists(new_annotation_path):
        # Append to existing annotation file
        df_new_annotations.to_csv(new_annotation_path, mode="a", header=False, index=False)
    else:
        # Create new annotation file
        df_new_annotations.to_csv(new_annotation_path, index=False)