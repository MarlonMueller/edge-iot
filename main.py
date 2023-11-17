# TODO - no main.py

import os
import pathlib
import asyncio
from src.interface import xeno_canto

PATH = pathlib.Path(__file__).parent.resolve()

if __name__ == "__main__":
   
    dir = os.path.join(PATH, "data")

    query = {
        "area": "europe",
        "grp": "1",  # birds
        "cnt": "germany",
        "loc": "bavaria",
        # box:LAT_MIN,LON_MIN,LAT_MAX,LON_MAX,
        # lic: '',       #license,
        #'q':">C",      #quality
        "len": "5-10",  # length (s)
        #'smp': ''       # sampling rate
        #'since': ''     # upload date
    }

    page = xeno_canto.get_composite_page(query)
    #xeno_canto.plot_distribution(page)
    
    page = xeno_canto.filter_top_k_species(page, k=5)
    #xeno_canto.plot_distribution(page, threshold=0)

    # Download audio files
    asyncio.run(xeno_canto.get_page_audio(page, dir))
