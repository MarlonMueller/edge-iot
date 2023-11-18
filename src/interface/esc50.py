import os
import httpx
import shutil

def _get_zip(url: str, path: str):
    """Download zip file from url to path"""

    with httpx.stream("GET", url) as response:
        with open(path, "wb") as file:
            for chunk in response.iter_bytes():
                file.write(chunk)


def get_esc50_audio(dir: str):
    
    os.makedirs(dir, exist_ok=True)
    url = "https://github.com/karoldvl/ESC-50/archive/master.zip"
    
    zip_path = os.path.join(dir, "ESC-50.zip")
    if not os.path.isfile(zip_path):
        _get_zip(url, zip_path)
    
    shutil.unpack_archive(zip_path, dir)
    os.remove(zip_path)