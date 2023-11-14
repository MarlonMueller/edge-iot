#TODO - no main.py

import os
import pathlib
import asyncio
from src.dataset import xeno_canto

if __name__ == "__main__":
    
    PATH = pathlib.Path(__file__).parent.resolve()
    DATA_PATH = os.path.join(PATH, "data")
   
    QUERY = {
        "area": "europe",
        "grp": "1",  # birds
        "cnt": "germany",
        "loc": "bavaria",
        # box:LAT_MIN,LON_MIN,LAT_MAX,LON_MAX,
        # lic: '',       #license,
        #'q':">C",      #quality
        #"len": "10-15",  # length (s)
        #'smp': ''       # sampling rate
        #'since': ''     # upload date
    }
    
    
    page = xeno_canto.get_composite_page(QUERY)
    xeno_canto.plot_distribution(page)
    
    #Download audio files
    #asyncio.run(xeno_canto.get_page_audio(page, DATA_PATH, ["312226", "310470", "308927"]))
