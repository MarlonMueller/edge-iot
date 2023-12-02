import os
import torch
import onnx
from onnx import checker
from jinja2 import Template


def save_model(model, dir, model_name):
    path = os.path.join(dir, f"{model_name}.pt")
    torch.save(model.state_dict(), path)


def load_model(model, dir, model_name):
    path = os.path.join(dir, f"{model_name}.pt")
    model.load_state_dict(torch.load(path))
    model.eval()
    return model


def export_onnx(model, dir, model_name, input_size):

    # Note: ESP-DL requires opset_version 13
    

    channels, height, width = input_size
    # NOTE - batch/channel dim correct?
    path = os.path.join(dir, f"{model_name}.onnx")
    torch.onnx.export(
        model,
        torch.randn(1, channels, height, width),
        path,
        verbose = True,
        input_names=["input"],
        output_names=["output"],
        opset_version=13, 
        dynamic_axes={
            "input": {0: "batch_size"},
            "output": {0: "batch_size"}
        }
    )
    
def load_onnx(dir, model_name, check_graph:bool = False):
    path = os.path.join(dir, f"{model_name}.onnx")
    model_proto = onnx.load_model(path)
    if check_graph:
        checker.check_graph(model_proto.graph)
    return model_proto


def render_template(template_dir, template_name, tags, model_dir, model_name):
    template_path = os.path.join(template_dir, f"{template_name}.jinja")
    model_path = os.path.join(model_dir, f"{model_name}.hpp")
    
    with open(template_path, "r") as f:
        template = f.read()
        
    template = Template(template).render(tags)
    
    with open(model_path, "w") as f :
        f.write(template)