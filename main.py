# TODO - no main.py

import sys
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
sys.path.append(os.path.join(PATH, "src", "procedures", "calibrate", "linux"))
from calibrator import *
from evaluator import *


if __name__ == "__main__":
    
    
    ########################################################
    #  Generate Datasets
    ########################################################
    
    data_dir = os.path.join(PATH, "data")
    
    config_path = os.path.join(PATH, "config")
    
    query = utils.load_config(config_path, "xeno-canto.yaml")["query"]
    
    num_species = 3
    audio_dir = os.path.join(data_dir, "audio")
    annotation_path = os.path.join(data_dir, "annotations.csv")

    # Query xeno-canto
    #page = xeno_canto.get_composite_page(query)
    #xeno_canto.plot_distribution(page, threshold=3)

    # Filter to top k species
    #page = xeno_canto.filter_top_k_species(page, k=num_species)
    #xeno_canto.plot_distribution(page, threshold=0)

    # Download audio files
    #species_map = asyncio.run(xeno_canto.get_page_audio(page, audio_dir, annotation_path))
    #species_map["other_species"] = num_species
    #species_map["other_sound"] = num_species

    # Exit program
    #import sys
    #sys.exit()
    
    ########################################################
    #  Torch Dataset
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

    model_name = "bird"
    models_dir = os.path.join(PATH, "models")

    ########################################################
    #  Training
    ########################################################

    batch_size = 2

    train_dataloader = DataLoader(
        train_data, batch_size=2, shuffle=True, generator=generator
    )
    test_dataloader = DataLoader(
        test_data, batch_size=2, shuffle=True, generator=generator
    )


    model = CustomModel(input_size, num_classes=num_species).to(device)

    criterion = torch.nn.NLLLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.1)

    num_epochs = 10
    for epoch in range(1, num_epochs + 1):
        create_classification_report = True if epoch == num_epochs else False
        procedures.train(model, train_dataloader, optimizer, criterion, epoch, device)
        procedures.test(
            model, test_dataloader, criterion, device, create_classification_report
        )
    
    torch_dir = os.path.join(models_dir, "torch")
    utils.save_model(model, torch_dir, model_name)

    ########################################################
    #  ONNX
    ########################################################
    onnx_dir = os.path.join(models_dir, "onnx")

    utils.export_onnx(model, onnx_dir, model_name, input_size)
    utils.load_onnx(onnx_dir, model_name, check_graph=True)
    
    optimize.optimize_fp_model(os.path.join(onnx_dir, f"{model_name}.onnx"))
    
    ########################################################
    #  QUANTIZATION
    ########################################################
    cpp_dir = os.path.join(models_dir, "cpp")
    
    target_chip = "esp32"
    quantization_bit = "int8"
    granularity = "per-tensor"
    calib_method = "minmax"
    provider = "CPUExecutionProvider"

    calib_dataloader = DataLoader(test_data, batch_size=len(test_data))
    calib_data = next(iter(calib_dataloader))[0].numpy()
    model_proto = utils.load_onnx(onnx_dir, model_name)
    
    # Calibration dataset
    calib = Calibrator(quantization_bit, granularity, calib_method)
    calib.set_providers([provider])

    cpp_file_name = f"{model_name}_coefficient"
    quantization_params_path = os.path.join(cpp_dir, f"{model_name}_quantization_params.pickle")

    # Generate quantization table
    calib.generate_quantization_table(model_proto, calib_data, quantization_params_path)
    
    # Export model to cpp
    calib.export_coefficient_to_cpp(model_proto, quantization_params_path, target_chip, cpp_dir, cpp_file_name, True)
    
    ########################################################
    #  QUANTIZATION EVALUATION
    ########################################################
    
    # eva = Evaluator(quantization_bit, granularity, target_chip)
    # eva.set_providers([provider])
    # eva.generate_quantized_model(model_proto, quantization_params_path)

    # output_names = [n.name for n in model_proto.graph.output]
    # m = rt.InferenceSession(optimized_model_path, providers=[provider])

    #TODO - 


    
