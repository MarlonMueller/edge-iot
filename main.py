# TODO - no main.py

import sys
import os
import torch
import pathlib
import asyncio
import pickle
import io
import re
from contextlib import redirect_stdout
from torchvision import transforms
from torch.utils.data import DataLoader
from src.interface import xeno_canto, esc50
from src.dataset import BirdDataset
from src.model import CustomModel
from src import procedures, model, utils
from src.procedures import optimize
import torch.optim as optim

PATH = pathlib.Path(__file__).parent.resolve()


if __name__ == "__main__":

    """

    DATASET SIZE: approx. 800 bird, 1400 non-bird
    BIRDS: water_rail, common_moorhen, s_warbler

Size of input: torch.Size([1, 32, 188])
x.size(): torch.Size([1, 8, 6, 7])
Feature size: 336

precision    recall  f1-score   support

           0       0.88      0.56      0.68        68
           1       0.79      0.62      0.70        80
           2       0.92      0.78      0.85        60
           3       0.86      0.99      0.92       346

    accuracy                           0.86       554
   macro avg       0.87      0.74      0.79       554
weighted avg       0.86      0.86      0.85       554

    return torch._C._cuda_getDeviceCount() > 0
Size of input: torch.Size([1, 16, 173])
x.size(): torch.Size([1, 8, 6, 5])
Feature size: 240
Quantized model info:
model input name: input, exponent: -15
Conv layer name: /conv1/Conv, output_exponent: -14
MaxPool layer name: /pool1/MaxPool, output_exponent: -14
Conv layer name: /conv2/Conv, output_exponent: -13
MaxPool layer name: /pool2/MaxPool, output_exponent: -13
Conv layer name: /conv3/Conv, output_exponent: -12
MaxPool layer name: /pool3/MaxPool, output_exponent: -12
Flatten layer name: /Flatten, output_exponent: -12
Gemm layer name: /fc1/Gemm, output_exponent: -11
Gemm layer name: /fc2/Gemm, output_exponent: -10
Softmax layer name: /Softmax, output_exponent: -14



Accuracy of fp32 model: 0.8448
Accuracy of int16 model: 0.8448



Accuracy of fp32 model: 0.7545
Accuracy of int8 model: 0.7635
    """
    
    
   
    
    
    #CONFIG_APPTRACE_DEST_TRAX and CONFIG_APPTRACE_ENABLE with the ESP-IDF: SDK Configuration editor command.
    # Data destination jtag
    
    # Open IDF Terminal
    # Install tools 
    #    cd ~/esp/esp-idf
    #./install.sh esp32s3
    # openocd -f board/esp32s3-builtin.cfg
    # Start monitor and connect via USB (right)
    # https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/heap_debug.html#heap-tracing
    # Use host free mode and configure menu#
    # Enable PSRAM malloc
    # Panic behaviour
    # DL Fix
    # Exlude 0th offset?!!
    # coefficient dimension has zero mean and unit variance:
    
    
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
    # page = xeno_canto.get_composite_page(query)
    # xeno_canto.plot_distribution(page, threshold=1)

    # Filter to top k species
    #page = xeno_canto.filter_top_k_species(page, k=num_species)
    #xeno_canto.plot_distribution(page, threshold=0)

    # Download audio files
    #species_map = asyncio.run(
    #   xeno_canto.get_page_audio(page, audio_dir, annotation_path)
    #)
    # species_map["other_species"] = num_species
    # species_map["other_sound"] = num_species

    #esc50.get_esc50_audio(data_dir)

    old_audio_dir = os.path.join(data_dir, "ESC-50-master", "audio")
    old_annotation_path = os.path.join(data_dir, "ESC-50-master", "meta", "esc50.csv")
    #esc50.generate(num_species, old_audio_dir, audio_dir, old_annotation_path, annotation_path)

    # Exit program
    #sys.exit(0)

    ########################################################
    #  Torch Dataset
    ########################################################

    use_cuda = torch.cuda.is_available()
    device = torch.device("cuda" if use_cuda else "cpu")

    transform = transforms.Compose(
        [transforms.ToTensor()]
    )

    h5py_path = os.path.join(data_dir, "data.h5")
    dataset = BirdDataset(h5py_path, annotation_path, audio_dir, transform)

    seed = 0
    torch.manual_seed(seed)
    generator = torch.Generator().manual_seed(seed)

    train_data, test_data = torch.utils.data.random_split(
        dataset, [0.75, 0.25], generator=generator
    )
    input_size = train_data[0][0].size()

    model_name = "birdnet"
    models_dir = os.path.join(PATH, "models")

    # import librosa
    # import numpy as np
    # import pandas as pd
    # from matplotlib import pyplot as plt
    # from src.audio import preprocess as preprocessing

    # for i in range(5):
    #     mfcc = dataset[i][0].numpy()[0, :, :]
    #     labels = pd.read_csv(annotation_path)
    #     audio_path = os.path.join(audio_dir, labels.iloc[i, 0])
    #     mfcc2 = preprocessing.mfcc_from_file(audio_path, False)
    #     print(np.array_equiv(mfcc, mfcc2))
    #     fig, (ax1, ax2) = plt.subplots(1, 2)
    #     img1 = librosa.display.specshow(mfcc, ax=ax1)
    #     img2 = librosa.display.specshow(mfcc2, ax=ax2)
    #     plt.show()

    # sys.exit(0)

    ########################################################
    #  Training
    ########################################################

    batch_size = 32

    train_dataloader = DataLoader(
        train_data, batch_size=batch_size, shuffle=True, generator=generator
    )
    test_dataloader = DataLoader(
        test_data, batch_size=batch_size, shuffle=True, generator=generator
    )

    model = CustomModel(input_size, num_classes=num_species+1).to(device)

    optimizer = optim.Adam(model.parameters())

    num_epochs = 75
    training_losses = []
    testing_losses = []
    testing_accuracies = []
    for epoch in range(1, num_epochs + 1):
        create_classification_report = True if epoch == num_epochs else False
        training_losses.append(procedures.train(model, train_dataloader, optimizer, epoch, device))
        testing_loss, accuracy = procedures.test(
            model, test_dataloader, device, create_classification_report
        )
        testing_losses.append(testing_loss)
        testing_accuracies.append(accuracy)

    utils.plot_torch_results(num_epochs, training_losses, testing_losses, testing_accuracies)
    torch_dir = os.path.join(models_dir, "torch")

    utils.save_model(model, torch_dir, model_name)
    

    ########################################################
    #  ONNX
    ########################################################

    model = utils.load_model(model, torch_dir, model_name)

    onnx_dir = os.path.join(models_dir, "onnx")

    utils.export_onnx(model, onnx_dir, model_name, input_size)
    utils.load_onnx(onnx_dir, model_name, check_graph=True)

    onnx_optimized_path = optimize.optimize_fp_model(os.path.join(onnx_dir, f"{model_name}.onnx"))

    ########################################################
    #  QUANTIZATION
    ########################################################

    sys.path.append(os.path.join(PATH, "src", "procedures", "calibrate", "linux"))
    from calibrator import *
    from evaluator import *

    #cpp_dir = os.path.join(models_dir, "cpp")
    cpp_dir = os.path.join(PATH, "src", "model")

    target_chip = "esp32s3"
    quantization_bit = "int16"
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
    quantization_params_path = os.path.join(
        cpp_dir, f"{model_name}_quantization_params.pickle"
    )

    # # Generate quantization table
    calib.generate_quantization_table(model_proto, calib_data, quantization_params_path)

    # Export model to cpp
    f = io.StringIO()
    with redirect_stdout(f):
        calib.export_coefficient_to_cpp(
            model_proto, quantization_params_path, target_chip, cpp_dir, cpp_file_name, True
        )
    log = f.getvalue()
    print(log)

    # Extract layer names and exponents
    pattern = r'name: \/?(?P<name>\w+)(?:\/\w+)?, (?:output_)?exponent: (?P<exponent>-?\d+)'
    matches = re.findall(pattern, log)
    log_data = [(match[0].lower(), int(match[1])) for match in matches]
    
    ########################################################
    #  JINJA
    ########################################################
    
    # Load the pickle file
    #with open(quantization_params_path, 'rb') as f:
    #    data = pickle.load(f)


    tags = {
        "model_name": model_name,
        "quantization_bit": quantization_bit,
        "layers": procedures.get_layer_info(model, log_data)
    }
    
    
    template_dir = os.path.join(PATH, "src", "templates")
    utils.render_template(template_dir, "model", tags, cpp_dir, f"{model_name}_model")
    

    ########################################################
    #  QUANTIZATION EVALUATION
    ########################################################

    evaluator = Evaluator(quantization_bit, granularity, target_chip)
    evaluator.set_providers([provider])
    evaluator.generate_quantized_model(model_proto, quantization_params_path)
   
    m = rt.InferenceSession(onnx_optimized_path, providers=[provider])

    input_name = m.get_inputs()[0].name
    output_names = [n.name for n in model_proto.graph.output]

    correct = 0
    correct_quantized = 0

    for i in range(int(len(test_data)/batch_size)):

        #TODO: Validate
        
        data = [test_data[j] for j in range(i * batch_size, (i+1) * batch_size)]
        x = np.array([s[0].numpy() for s in data])
        y = [s[1] for s in data]

        [out, _] = evaluator.evalute_quantized_model(x, False)
        correct_quantized += sum(np.argmax(out[0], axis=1) == y)

        out = m.run(output_names, {input_name: x.astype(np.float32)})
        correct += sum(np.argmax(out[0], axis=1) == y)

    print(f"Accuracy of fp32 model: {correct / len(test_data):.4f}")
    print(f"Accuracy of {quantization_bit} model: {correct_quantized / len(test_data):.4f}")
    