from typing import Dict, List

import os
import asyncio
import warnings
from PIL import Image
from io import BytesIO
from dataclasses import dataclass

import httpx
import pandas as pd
from tqdm import tqdm
import matplotlib.pyplot as plt


from src.utils import COLORS
from src.utils.visualisation import plot_image


"""
API documentation:
https://xeno-canto.org/explore/api
https://xeno-canto.org/help/search
"""

QUERY = {
    "area": "europe",
    "grp": "1",  # birds
    "cnt": "germany",
    "loc": "bavaria",
    # box:LAT_MIN,LON_MIN,LAT_MAX,LON_MAX,
    # lic: '',       #license,
    #'q':">C",      #quality
    "len": "10-15",  # length (s)
    #'smp': ''       # sampling rate
    #'since': ''     # upload date
}

API_URL = "https://xeno-canto.org/api/2/recordings"


@dataclass
class Page:

    """Dataclass for a page of recordings from the Xeno-Canto API"""

    numRecordings: int
    numSpecies: int
    page: int
    numPages: int
    recordings: pd.DataFrame

    def __str__(self):
        return f"(Page {self.page}/{self.numPages}) numRecordings: {len(self.recordings)}, totalRecordings: {self.numRecordings}, numSpecies: {self.numSpecies}"


def get_page(query: Dict[str, str], page: int = 1) -> Page:
    """Get a page of recordings from the Xeno-Canto API"""

    # Set query and params
    query = " ".join([f"{k}:{v}" for k, v in query.items()])
    params = {"query": query, "page": page}
    print("Query:", params)

    response = httpx.get(API_URL, params=params).raise_for_status().json()

    # Store 'recordings' as Pandas DataFrame with index: 'id'
    recordings_df = pd.json_normalize(response.pop("recordings"))
    assert len(recordings_df) > 0, "numRecordings on page is zero"
    recordings_df = recordings_df.set_index("id")
    return Page(**response, recordings=recordings_df)


def get_composite_page(query: Dict[str, str]) -> Page:
    """Get a composite page of recordings from the Xeno-Canto API"""

    # Fetch first page
    composite_page = get_page(query, page=1)
    composite_recordings = len(composite_page.recordings)

    # Fetch and aggregate remaining pages
    for num_page in range(2, composite_page.numPages + 1):
        page = get_page(query, num_page)

        composite_recordings += len(page.recordings)
        assert page.numRecordings == composite_page.numRecordings
        assert page.numSpecies == composite_page.numSpecies
        assert page.numPages == composite_page.numPages
        assert page.page == num_page

        composite_page.recordings = pd.concat(
            [composite_page.recordings, page.recordings], verify_integrity=True
        )

    assert composite_recordings == int(
        composite_page.numRecordings
    ), "totalRecordings do not add up"
    composite_page.numPages = 1

    print(page)
    return composite_page


async def _get_audio(url: str, path: str):
    """Download audio file from url to path"""

    async with httpx.AsyncClient() as client:
        response = await client.get(url)
        response.raise_for_status()

        with open(path, "wb") as file:
            for chunk in response.iter_bytes():
                file.write(chunk)


async def get_page_audio(page: Page, dir: str, ids: List[str] = None):
    """Download audio files from page to dir"""
    os.makedirs(dir, exist_ok=True)

    if ids is None:
        ids = page.recordings.index

    total_downloads = len(ids)
    progress_bar = tqdm(total=total_downloads, unit="download", dynamic_ncols=True)

    async def download_and_track(url, path, progress_bar):
        await _get_audio(url, path)
        progress_bar.update(1)

    tasks = []
    for id in ids:
        en = (page.recordings.loc[id].en).lower().replace(" ", "_")
        file_name = page.recordings.loc[id]["file-name"]
        if ".mp3" in file_name:
            path = f"{dir}/{en}_{id}.mp3"
        elif ".wav" in file_name:
            path = f"{dir}/{en}_{id}.wav"
        else:
            warnings.warn("no .mp3 or .wav suffix")

        if not os.path.exists(path):
            task = asyncio.create_task(
                download_and_track(page.recordings.loc[id]["file"], path, progress_bar)
            )
            tasks.append(task)

    await asyncio.gather(*tasks)

    progress_bar.close()


def get_field(page: Page, id: str, field: str, prefix=True):
    url = page.recordings.loc[id][field]
    url = f"https:{url}" if prefix else url
    return httpx.get(url).raise_for_status()


def plot_sono(page: Page, id: str, version: str = "small"):
    field = "sono." + version
    response = get_field(page, id, field)
    plot_image(Image.open(BytesIO(response.content)))


def plot_osci(page: Page, id: str, version: str = "small"):
    field = "osci." + version
    response = get_field(page, id, field)
    plot_image(Image.open(BytesIO(response.content)))


def plot_distribution(page: Page, threshold: int = 5):
    """Plot the distribution of bird species in a page of recordings"""

    df = page.recordings
    total_bird_observations = len(df)
    species_counts = df["en"].value_counts()
    species_percentages = species_counts / total_bird_observations * 100
    top_species = species_percentages[species_percentages >= threshold]
    other_species_count = species_percentages[species_percentages < threshold].sum()
    top_species["Other"] = other_species_count

    plt.figure(figsize=(8, 8))
    plt.pie(
        top_species,
        labels=top_species.index,
        autopct="%1.1f%%",
        startangle=140,
        colors=COLORS,
        explode=[0.05] * len(top_species),
    )
    plt.title("Distribution of Bird Species")
    plt.show()
