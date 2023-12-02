
import torch
import torch.nn as nn


"""
{"layer": "conv2d", "layer_name": "conv2", "previous_layer_name": "", "output_exponent": "-1", "padding_type": "PADDING_VALID", "padding": "{}", "stride_y": "1", "stride_x": "1"},
{"layer": "reshape", "layer_name": "reshape", "previous_layer_name": "conv2", "shape": "{1,1,784}", "inplace": "false", "padding_type": "PADDING_VALID", "padding": "{}", "stride_y": "1", "stride_x":1},
{"layer": "maxpool2d", "layer_name": "maxpool2d", "previous_layer_name": "reshape", "filter_shape":"[2, 2]", "padding_type": "PADDING_VALID", "padding": "{}", "stride_y": "1", "stride_x": "1"},
{"layer": "softmax", "layer_name": "softmax", "previous_layer_name": "maxpool2d", "output_exponent": -1, "inplace": "false"},
{"layer": "fc", "layer_name": "fc", "previous_layer_name": "softmax", "output_exponent": -1, "flatten": "true", },
{"layer": "relu", "layer_name": "relu", "previous_layer_name": "fc", "inplace": "false"}, 
"""

def get_layer_info(model, output_exponents):

    layers = []
    output_exponent_iter = iter(output_exponents)
    
    def add_layer_info(layer_type, layer_name, previous_layer_name, **kwargs):
        layer_info = {"layer": layer_type, "layer_name": layer_name, "previous_layer_name": previous_layer_name}
        layer_info.update(kwargs)
        layers.append(layer_info)

    previous_layer_name = ""
    for layer_name, layer in model.named_children():
        if isinstance(layer, nn.Conv2d):
            output_exponent = next(output_exponent_iter, None)
            add_layer_info("conv2d", layer_name, previous_layer_name, output_exponent=output_exponent, padding_type="PADDING_VALID", padding="{}", stride_y=layer.stride[0], stride_x=layer.stride[1])
        elif isinstance(layer, nn.Linear):
            output_exponent = next(output_exponent_iter, None)
            add_layer_info("fc", layer_name, previous_layer_name, output_exponent=output_exponent, flatten="true")
        elif isinstance(layer, nn.ReLU):
            add_layer_info("relu", layer_name, previous_layer_name, inplace="false")
        elif isinstance(layer, nn.MaxPool2d):
            add_layer_info("maxpool2d", layer_name, previous_layer_name, filter_shape=list(layer.kernel_size), padding_type="PADDING_VALID", padding="{}", stride_y=layer.stride[0], stride_x=layer.stride[1])
        elif isinstance(layer, nn.Softmax):
            output_exponent = next(output_exponent_iter, None)
            add_layer_info("softmax", layer_name, previous_layer_name, output_exponent=output_exponent, inplace="false")
        elif isinstance(layer, nn.Flatten):
            add_layer_info("flatten", layer_name, previous_layer_name, inplace="false")
        previous_layer_name = layer_name
        
    return layers