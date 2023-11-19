import os
import torch
import onnx
from onnx import checker


def save_model(model, dir, model_name):
    path = os.path.join(dir, f"{model_name}.pt")
    torch.save(model.state_dict(), path)


def load_model(model, dir, model_name):
    path = os.path.join(dir, f"{model_name}.pt")
    model.load_state_dict(torch.load(path))
    model.eval()
    return model


def export_onnx(model, dir, model_name, input_size):
    channels, height, width = input_size
    # NOTE - batch/channel dim correct?
    path = os.path.join(dir, f"{model_name}.onnx")
    torch.onnx.export(
        model,
        torch.randn(1, channels, height, width),
        path,
        input_names=["input"],
        output_names=["output"],
    )
    
def load_onnx(dir, model_name):
    path = os.path.join(dir, f"{model_name}.onnx")
    model_proto = onnx.load_model(path)
    checker.check_graph(model_proto.graph)
