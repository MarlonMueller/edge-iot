
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

def get_layer_info(model, log_data):

    layers = []
    
    def add_layer_info(layer_type, layer_name, previous_layer_name, **kwargs):
        layer_info = {"layer": layer_type, "layer_name": layer_name, "previous_layer_name": previous_layer_name}
        layer_info.update(kwargs)
        layers.append(layer_info)

    previous_layer_name = ""
    for i, (layer_name, output_exponent) in enumerate(log_data):

        if "conv" in layer_name:
            layer = getattr(model, layer_name)


            if isinstance(layer.padding, int) or len(layer.padding) == 1:
                if isinstance(layer.padding, int):
                    padding = layer.padding
                else:
                    padding = layer.padding[0]
                if padding == 0:
                    padding_type = "PADDING_VALID"
                    padding_vec = "{}"
                else:
                    padding_type = "PADDING_NOT_SET"
                    padding_vec = f"{{{padding},{padding},{padding},{padding}}}"
               
            else:
                padding_height = layer.padding[0]
                padding_width = layer.padding[1]
                padding_type = "PADDING_NOT_SET"
                padding_vec = f"{{{padding_height},{padding_height},{padding_width},{padding_width}}}"

            add_layer_info("conv2d", layer_name, previous_layer_name, output_exponent=output_exponent, padding_type=padding_type, padding=padding_vec, stride_y=layer.stride[0], stride_x=layer.stride[1], activation=True)
        elif "fc" in layer_name:
            add_layer_info("fc", layer_name, previous_layer_name, output_exponent=output_exponent, flatten="true", activation=(i != len(log_data) - 2))
        elif "relu" in layer_name:
            add_layer_info("relu", layer_name, previous_layer_name, inplace="false")
        elif "pool" in layer_name:
            layer = getattr(model, layer_name)
         

            if isinstance(layer.padding, int) or len(layer.padding) == 1:
                if isinstance(layer.padding, int):
                    padding = layer.padding
                else:
                    padding = layer.padding[0]
                if padding == 0:
                    padding_type = "PADDING_VALID"
                    padding_vec = "{}"
                else:
                    padding_type = "PADDING_NOT_SET"
                    padding_vec = f"{{{padding},{padding},{padding},{padding}}}"
               
            else:
                padding_height = layer.padding[0]
                padding_width = layer.padding[1]
                padding_type = "PADDING_NOT_SET"
                padding_vec = f"{{{padding_height},{padding_height},{padding_width},{padding_width}}}"

            filter_shape = list(layer.kernel_size)
            if len(filter_shape) == 1:
                filter_shape *= 2
            filter_shape = "{" + ",".join(map(str, filter_shape)) + "}"
                
            add_layer_info("maxpool2d", layer_name, previous_layer_name, filter_shape=filter_shape, padding_type=padding_type, padding=padding_vec, stride_y=layer.stride[0], stride_x=layer.stride[1])
        elif "softmax" in layer_name:
            add_layer_info("softmax", layer_name, previous_layer_name, output_exponent=output_exponent, inplace="false")
        elif "flatten" in layer_name:
            add_layer_info("flatten", layer_name, previous_layer_name, inplace="false")

        previous_layer_name = layer_name
        
    return layers