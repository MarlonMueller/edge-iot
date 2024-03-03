from typing import Dict, List

import os
import csv
import pathlib
import asyncio
import warnings
from PIL import Image
from io import BytesIO
from dataclasses import dataclass

import httpx
import asyncio
import pandas as pd
from tqdm import tqdm
import matplotlib.pyplot as plt

from src import utils

import logging
logging.basicConfig(level=logging.INFO)
logging.getLogger("httpx").setLevel(logging.WARNING)
logger = logging.getLogger(__name__)


API_URL = "https://xeno-canto.org/api/2/recordings"

class Counter:
    """A thread-safe counter."""

    def __init__(self):
        self._value = 0
        self._lock = asyncio.Lock()

    async def increment(self):
        async with self._lock:
            self._value += 1

    def value(self):
        return self._value


def download_xeno_canto_audio(query: Dict[str, str], num_species:int, audio_dir: pathlib.Path, annotation_path: pathlib.Path, assets_dir: pathlib.Path):
    """Use xeno-canto interface to async. download bird audio files and create annotation file

    :param query: xeno-canto query
    :param num_species: top-k species to download
    :param audio_dir: audio directory
    :param annotation_path: annotation file path
    :return: map of species names to numeric values
    """
    
    logger.info("Downloading Xeno-Canto audio files ...")
    
    # Query xeno-canto API
    page = get_composite_page(query)
    # plot_distribution(assets_dir, "bird_species", page, threshold=0.8)
    
    # Filter top-k species
    page = filter_top_k_species(page, k=num_species)
    plot_distribution(assets_dir, "bird_species_top", page , threshold=0)
    
    species_map = asyncio.run(
      get_page_audio(page, audio_dir, annotation_path)
    )
    
    return species_map

@dataclass
class Page:
    
    """Dataclass for a page of recordings from the Xeno-Canto API."""

    numRecordings: int
    numSpecies: int
    page: int
    numPages: int
    recordings: pd.DataFrame

    def __str__(self):
        return f"(Page {self.page}/{self.numPages}) numRecordings: {len(self.recordings)}, totalRecordings: {self.numRecordings}, numSpecies: {self.numSpecies}"


def get_page(query: Dict[str, str], page: int = 1) -> Page:
    """Get a page of recordings from the Xeno-Canto API

    :param query: xeno-canto query
    :param page: page number, defaults to 1
    :return: page of recordings
    """

    # Set query and params
    query = " ".join([f"{k}:{v}" for k, v in query.items()])
    params = {"query": query, "page": page}
    logger.info(f"Query: {params}")

    response = httpx.get(API_URL, timeout=10, params=params).raise_for_status().json()

    # Store 'recordings' as Pandas DataFrame with index: 'id'
    recordings_df = pd.json_normalize(response.pop("recordings"))
    assert len(recordings_df) > 0, "numRecordings on page is zero"
    recordings_df = recordings_df.set_index("id")

    # Replace en field of recordings with lower and _-separated string
    recordings_df["en"] = recordings_df["en"].str.lower().str.replace(" ", "_")

    return Page(**response, recordings=recordings_df)


def get_composite_page(query: Dict[str, str]) -> Page:
    """Get combined pages of recordings from the Xeno-Canto API

    :param query: xenocanto query
    :return: combined page of recordings
    """

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
    
    logger.info(f"Raw page: {composite_page}")
    return composite_page


async def _get_audio(url: str, path: str):
    """Download audio file from url to path

    :param url: URL of audio file
    :param path: path to save audio file
    """

    async with httpx.AsyncClient() as client:
        response = None
        try:
            response = await client.get(url)
            response.raise_for_status()

            with open(path, "wb") as file:
                for chunk in response.iter_bytes():
                    file.write(chunk)

        finally:
            if response is not None:
                await response.aclose()


async def _download_audio(
    id_: str,
    page:Page,
    species_map: Dict[str, int],
    audio_dir: pathlib.Path,
    annotation_path : pathlib.Path,
    columns : List[str],
    semaphore,
    progress_bar,
    num_errors,
    lock,
):
    """Download audio file from xeno-canto and document in annotation file.

    :param id_: xeno-canto recording id
    :param page: page of recordings
    :param species_map: map of species names to numeric values
    :param audio_dir: audio directory
    :param annotation_path: annotation file path
    :param columns: csv columns
    :param semaphore: shared semaphore
    :param progress_bar: progress bar
    :param num_errors: shared counter
    :param lock: shared lock 
    :raises ValueError: if file is not .mp3 or .wav
    """

    async with semaphore:
        try:
            # Extract species name from 'en' field and map to numeric value
            cls_str = page.recordings.loc[id_].en
            cls_id = species_map[cls_str]

            # Get file names
            file_name = page.recordings.loc[id_]["file-name"]

            if ".mp3" in file_name:
                file_name = f"x-{id_}-{cls_id}-0.mp3"
            elif ".wav" in file_name:
                file_name = f"x-{id_}-{cls_id}-0.wav"
            else:
                raise ValueError(f"{file_name} not .mp3 or .wav")

            audio_path = os.path.join(audio_dir, file_name)
            if not os.path.isfile(audio_path):
                await asyncio.create_task(
                    _get_audio(
                        page.recordings.loc[id_]["file"],
                        audio_path,
                    )
                )

                async with lock:
                    # Document audio file in annotation file
                    with open(annotation_path, mode="a", newline="") as file:
                        writer = csv.DictWriter(file, fieldnames=columns)
                        writer.writerow(
                            {
                                "idx": "",
                                "file_name": file_name,
                                "class_id": cls_id,
                                "class": cls_str,
                                "xeno_id": id_,
                                "slice": 0,
                                "quality": page.recordings.loc[id_]["q"],
                                "length": page.recordings.loc[id_]["length"],
                                "sampling_rate": page.recordings.loc[id_]["smp"],
                            }
                        )
                

        except (httpx.HTTPError, ValueError) as e:
            logger.debug(f"Error downloading {id_}: {str(e)}")
            await num_errors.increment()

        finally:
            if progress_bar is not None:
                progress_bar.update(1)


def filter_top_k_species(page: Page, k: int = 5, exclude_unknown: bool = True) -> Page:
    """Filter page to top-k species

    :param page: page of recordings
    :param k: defaults to 5
    :param exclude_unknown: exclude 'identity_unknown' from top-k species, defaults to True
    :return: filtered page of recordings
    """

    df = page.recordings

    species_counts = df["en"].value_counts()

    if exclude_unknown and "identity_unknown" in species_counts[:k]:
        species_counts = species_counts.drop("identity_unknown")

    top_species = species_counts[:k]
    top_species = top_species.index

    page.recordings = page.recordings[page.recordings["en"].isin(top_species)]
    page.numRecordings = len(page.recordings)
    page.numSpecies = len(top_species)
    logger.info(f"Filtered page: {page}")
    return page


def get_numeric_species_map(page: Page):
    """Get map of species names to numeric values

    :param page: page of recordings
    :return: map of species names to numeric values
    """
    species_names = page.recordings["en"].unique()
    species_map = {name: i for i, name in enumerate(species_names)}
    return species_map


async def get_page_audio(
    page: Page,
    audio_dir: str,
    annotation_path: str,
    ids: List[str] = None,
) -> Dict[str, int]:
    """Download audio files from page and document in annotation file

    :param page: page of recordings
    :param audio_dir: audio directory
    :param annotation_path: annotation file path
    :param ids: xeno-canto recording ids, defaults to None
    :return: map of species names to numeric values
    """
    os.makedirs(audio_dir, exist_ok=True)

    if ids is None:
        ids = page.recordings.index

    # Get species map that maps species names to numeric values
    species_map = get_numeric_species_map(page)

    # Create annotation file
    columns = [
        "idx",
        "file_name",
        "class_id",
        "class",
        "xeno_id",
        "slice",
        "quality",
        "length",
        "sampling_rate",
    ]

    if not os.path.isfile(annotation_path):
        pd.DataFrame(columns=columns).to_csv(annotation_path, index=False)

    tasks = []
    lock = asyncio.Lock()
    num_errors = Counter()
    semaphore = asyncio.Semaphore(5)

    total_downloads = len(ids)
    
    progress_bar = None
    # progress_bar = tqdm(total=total_downloads, unit="download", dynamic_ncols=True)

    for id_ in ids:
        task = asyncio.create_task(
            _download_audio(
                id_,
                page,
                species_map,
                audio_dir,
                annotation_path,
                columns,
                semaphore,
                None, # progress_bar,
                num_errors,
                lock,
            )
        )
        tasks.append(task)

    await asyncio.gather(*tasks)
    
    if progress_bar is not None:
        progress_bar.close()

    logger.debug(f"Attempted {total_downloads} downloads, {num_errors.value()} failed")
    return species_map


def get_field(page: Page, id: str, field: str, prefix=True):
    """Get a field from a page of recordings

    :param page: page of recordings
    :param id: xeno-canto recording id
    :param field: field name
    :param prefix: add https prefix to url, defaults to True
    :return: httpx response
    """
    url = page.recordings.loc[id][field]
    url = f"https:{url}" if prefix else url
    return httpx.get(url, timeout=30).raise_for_status()


def plot_sono(page: Page, id: str, version: str = "small"):
    """Plot the sonogram of a recording

    :param page: page of recordings
    :param id: xenocanto recording id
    :param version: sonogram version, defaults to "small"
    """
    field = "sono." + version
    response = get_field(page, id, field)
    utils.plot_image(Image.open(BytesIO(response.content)))


def plot_osci(page: Page, id: str, version: str = "small"):
    """Plot the oscillogram of a recording

    :param page: page of recordings
    :param id: xenocanto recording id
    :param version: oscillogram version, defaults to "small"
    """
    field = "osci." + version
    response = get_field(page, id, field)
    utils.plot_image(Image.open(BytesIO(response.content)))


def plot_distribution(assets_dir, file_name, page: Page, threshold: int = 2):
    """Plot the distribution of bird species

    :param page: page of recordings
    :param threshold: minimum percentage of species to plot, defaults to 2
    """

    df = page.recordings
    total_bird_observations = len(df)
    species_counts = df["en"].value_counts()
    species_percentages = species_counts / total_bird_observations * 100
    top_species = species_percentages[species_percentages >= threshold]
    other_species_count = species_percentages[species_percentages < threshold].sum()
    if other_species_count > 0:
        top_species["Other"] = other_species_count

    plt.figure(figsize=(8, 8))
    plt.pie(
        top_species,
        labels=top_species.index,
        autopct="%1.1f%%",
        startangle=140,
        colors=utils.COLORS,
        explode=[0.05] * len(top_species),
        textprops={"fontsize": 18}
    )
    plt.title("Distribution of bird species", fontsize=20)
    
    plt.tight_layout()
    plt.savefig(assets_dir / f"{file_name}.pdf")

    plt.show()
