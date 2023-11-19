# TODO - no main.py

import os
import torch
import pathlib
import asyncio
from torchvision import transforms
from matplotlib import pyplot as plt
from torch.utils.data import DataLoader
from src.interface import xeno_canto, esc50
from src.dataset import BirdDataset
from src.model import CustomModel
from src import procedures, model, utils
import torch.optim as optim
from onnx import checker

PATH = pathlib.Path(__file__).parent.resolve()

# FIXME  Other bird class?

if __name__ == "__main__":
    
    
    ########################################################
    #  Dataset
    ########################################################
    
    data_dir = os.path.join(PATH, "data")

    # esc50.get_esc50_audio(dir)

    query = {
        "area": "europe",
        "grp": "1",  # birds
        "cnt": "germany",
        # "loc": "bavaria",
        # box:LAT_MIN,LON_MIN,LAT_MAX,LON_MAX,
        # lic: '',       #license,
        #'q':">C",      #quality
        "len": "5",  # length (s)
        #'smp': ''       # sampling rate
        #'since': ''     # upload date
    }

    num_species = 3
    audio_dir = os.path.join(data_dir, "audio")
    annotation_path = os.path.join(data_dir, "annotations.csv")

    # Query xeno-canto
    # page = xeno_canto.get_composite_page(query)
    # xeno_canto.plot_distribution(page, threshold=3)

    # Filter to top k species
    # page = xeno_canto.filter_top_k_species(page, k=num_species)
    # xeno_canto.plot_distribution(page, threshold=0)

    # Download audio files
    # species_map = asyncio.run(xeno_canto.get_page_audio(page, audio_dir, annotation_path))
    # species_map["other_species"] = num_species
    # species_map["other_sound"] = num_species

    # Exit program
    # import sys
    # sys.exit()
    
    ########################################################
    #  Deep Learning
    ########################################################

    use_cuda = torch.cuda.is_available()
    device = torch.device("cuda" if use_cuda else "cpu")

    transform = transforms.Compose(
        [
            transforms.ToTensor(),
        ]
    )

    dataset = BirdDataset(annotation_path, audio_dir, transform)

    seed = 0
    torch.manual_seed(seed)
    generator = torch.Generator().manual_seed(seed)

    train_data, test_data = torch.utils.data.random_split(
        dataset, [0.5, 0.5], generator=generator
    )
    input_size = train_data[0][0].size()

    batch_size = 2

    train_dataloader = DataLoader(
        train_data, batch_size=2, shuffle=True, generator=generator
    )
    test_dataloader = DataLoader(
        test_data, batch_size=2, shuffle=True, generator=generator
    )

    model_name = "bird_model"
    models_dir = os.path.join(PATH, "models")
    net = CustomModel(input_size, num_classes=num_species).to(device)

    criterion = torch.nn.CrossEntropyLoss()
    optimizer = optim.Adam(net.parameters(), lr=1.0)

    num_epochs = 10
    for epoch in range(1, num_epochs + 1):
        create_classification_report = True if epoch == num_epochs else False
        procedures.train(net, train_dataloader, optimizer, criterion, epoch, device)
        procedures.test(
            net, test_dataloader, criterion, device, create_classification_report
        )
    
    utils.save_model(net, models_dir, model_name)

    ########################################################
    #  ONNX
    ########################################################


    utils.load_model(net, models_dir, model_name)
    utils.export_onnx(net, models_dir, model_name, input_size)



    
