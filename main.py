# TODO - no main.py

import os
import torch
import pathlib
import asyncio
from torchvision import transforms
from torch.utils.data import DataLoader
from src.interface import xeno_canto, esc50
from src.dataset import BirdDataset
from src.model import CustomModel
from src import procedures, model, utils
from src.procedures import optimize
import torch.optim as optim

PATH = pathlib.Path(__file__).parent.resolve()

# FIXME  Other bird class?

if __name__ == "__main__":
    
    
    ########################################################
    #  Dataset
    ########################################################
    
    data_dir = os.path.join(PATH, "data")
    
    config_path = os.path.join(PATH, "config")
    
    query = utils.load_config(config_path, "xeno-canto.yaml")["query"]

    
    # esc50.get_esc50_audio(dir)
    
    
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
    #import sys
    #sys.exit()
    
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
    
    torch_dir = os.path.join(models_dir, "torch")
    utils.save_model(net, torch_dir, model_name)

    ########################################################
    #  ONNX
    ########################################################
    utils.load_model(net, torch_dir, model_name)

    onnx_dir = os.path.join(models_dir, "onnx")

    utils.export_onnx(net, onnx_dir, model_name, input_size)
    utils.load_onnx(onnx_dir, model_name)
    
    optimize.optimize_fp_model(os.path.join(onnx_dir, f"{model_name}.onnx"))
    
    # Calibration
    #TODO - 


    
