# TODO - no main.py

import os
import onnx
import torch
import pathlib
import asyncio
from torchvision import transforms
from matplotlib import pyplot as plt
from torch.utils.data import DataLoader
from src.interface import xeno_canto, esc50
from src.dataset import BirdDataset
from src.model import CustomModel
import torch.optim as optim
from onnx import checker

PATH = pathlib.Path(__file__).parent.resolve()

# FIXME  Other bird class?

if __name__ == "__main__":
    
    data_dir = os.path.join(PATH, "data")
    models_dir = os.path.join(PATH, "models")

    # esc50.get_esc50_audio(dir)

    query = {
        "area": "europe",
        "grp": "1",  # birds
        "cnt": "germany",
        "loc": "bavaria",
        # box:LAT_MIN,LON_MIN,LAT_MAX,LON_MAX,
        # lic: '',       #license,
        #'q':">C",      #quality
        "len": "5",  # length (s)
        #'smp': ''       # sampling rate
        #'since': ''     # upload date
    }

    num_species = 2
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

    use_cuda = torch.cuda.is_available()
    device = torch.device("cuda" if use_cuda else "cpu")

    transform = transforms.Compose(
        [
            transforms.ToTensor(),
        ]
    )

    dataset = BirdDataset(annotation_path, audio_dir, transform)

    # Use generator to split data deterministically
    generator = torch.Generator().manual_seed(0)

    # Split data into train and test
    train_data, test_data = torch.utils.data.random_split(
        dataset, [0.5, 0.5], generator=generator
    )
    input_size = train_data[0][0].size()

    # Use dataloader to load data in batches
    train_dataloader = DataLoader(
        train_data, batch_size=64, shuffle=True, generator=generator
    )
    test_dataloader = DataLoader(
        test_data, batch_size=64, shuffle=True, generator=generator
    )

    model = CustomModel(input_size).to(device)
    optimizer = optim.Adam(model.parameters(), lr=1.0)

    for epoch in range(0, 10):
        # train(model, device, train_loader, optimizer, epoch)
        # test(model, device, test_loader)
        pass
    
    model_name = "bird_model"
    pt_path = os.path.join(models_dir, "torch", f"{model_name}.pt")
    torch.save(model.state_dict(), pt_path)


    # Load model and set eval mode
    model.load_state_dict(torch.load(pt_path))
    model.eval()

    #NOTE - batch/channel dim correct?
    channels, height, width = input_size
    onnx_path = os.path.join(models_dir, "onnx",f"{model_name}.onnx")
    torch.onnx.export(
        model,
        torch.randn(1, channels, height, width), 
        onnx_path,
        verbose=True,
        input_names=["input"],
        output_names=["output"],
    )

    model_proto = onnx.load_model(onnx_path)
    #checker.check_graph(model_proto.graph)
