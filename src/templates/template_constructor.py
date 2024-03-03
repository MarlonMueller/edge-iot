from typing import List, Dict, Tuple

import re
import pathlib
from jinja2 import Template
from tensorflow.keras import models
from keras.layers import Conv2D, MaxPooling2D, Dense


def render_template(
    template_dir: pathlib.Path, model_dir: pathlib.Path, model_name: str, tags: Dict
) -> None:

    """Render a Jinja template to a C++ model file

    :param template_dir: the directory containing the Jinja template
    :param model_dir: the directory to save the model file
    :param model_name: the name of the model
    :param tags: a dictionary containing the tags to render the template
    """

    template_path = template_dir / "birdnet.jinja"
    model_path = model_dir / f"{model_name}.hpp"

    with open(template_path, "r") as f:
        template = f.read()

    template = Template(template).render(tags)

    with open(model_path, "w") as f:
        f.write(template)


def get_jinja_information(layer_information: List[Dict]) -> List[Dict]:

    """Aggregate information for ESP-DL C++ template

    :param layer_information: a list of dictionaries containing information about each layer
    :return: dictionaries containing information about each layer adhering to the ESP-DL format
    """

    jinja_layer_information = []

    def add_jinja_layer_information(
        layer_type, layer_name, previous_layer_name, **kwargs
    ):
        jinja_layer_info = {
            "layer": layer_type,
            "layer_name": layer_name,
            "previous_layer_name": previous_layer_name,
        }
        jinja_layer_info.update(kwargs)
        jinja_layer_information.append(jinja_layer_info)

    previous_layer_name = None

    for info in layer_information:

        layer_type = info["layer_type"]
        layer_name = info["layer_name"]

        if layer_type == "Conv2D":

            stride = info["stride"]
            exponent = info["exponent"]

            if info["padding"] == "valid":
                padding_type = "PADDING_VALID"
            elif info["padding"] == "same":
                padding_type = "PADDING_SAME_END"

            add_jinja_layer_information(
                "conv2d",
                layer_name,
                previous_layer_name,
                output_exponent=exponent,
                padding_type=padding_type,
                stride_x=stride[1],
                stride_y=stride[0],
                padding="{}",
                activation=True,
            )

        elif layer_type == "MaxPooling2D":

            stride = info["stride"]

            if info["padding"] == "valid":
                padding_type = "PADDING_VALID"
            elif info["padding"] == "same":
                padding_type = "PADDING_SAME_END"

            filter_shape = map(str, list(info["filter_shape"]))
            filter_shape = "{" + ",".join(filter_shape) + "}"

            add_jinja_layer_information(
                "maxpool2d",
                layer_name,
                previous_layer_name,
                filter_shape=filter_shape,
                padding_type=padding_type,
                stride_x=stride[1],
                stride_y=stride[0],
                padding="{}",
            )

        elif layer_type == "Flatten":

            add_jinja_layer_information(
                "flatten", layer_name, previous_layer_name, inplace="false"
            )

        elif layer_type == "Dense":

            exponent = info["exponent"]
            softmax = info["activation"] == "softmax"

            split = layer_name.split("_")
            layer_number = "0" if len(split) == 1 else split[1]

            add_jinja_layer_information(
                "fc",
                layer_name,
                previous_layer_name,
                output_exponent=exponent,
                flatten="true",
                activation=not softmax,
                layer_number=layer_number,
            )

            if softmax:
                exponent = info["softmax_exponent"]
                previous_layer_name = layer_name
                add_jinja_layer_information(
                    "softmax",
                    "softmax",
                    previous_layer_name,
                    output_exponent=exponent,
                    inplace="false",
                )
                previous_layer_name = "softmax"

        previous_layer_name = layer_name

    return jinja_layer_information


def get_layer_information(
    model: models.Sequential, quantization_log: str
) -> Tuple[str, List[Dict]]:

    """Extract information from the model and the quantization log

    :param model: the model to extract information from
    :param quantization_log: the quantization log
    :return: the input exponent and a list of dictionaries containing information about each layer
    """

    input_exponent = re.findall(r"model input.*exponent:\s(-?\d+)", quantization_log)

    out_exponents = re.findall(
        r"(?:Conv layer|Gemm layer|Softmax layer).*output_exponent:\s(-?\d+)",
        quantization_log,
    )

    layer_information = []
    exponent_idx = 0

    for layer in model.layers:
        info = {
            "layer_type": type(layer).__name__,
            "layer_name": layer.name,
            "input_shape": layer.input_shape,
            "output_shape": layer.output_shape,
            "activation": layer.activation.__name__
            if hasattr(layer, "activation")
            else None,
        }

        if isinstance(layer, Conv2D):
            idx = layer.name.split("_")[-1]
            idx = int(idx) if idx.isdigit() else None
            info.update(
                {
                    "index": idx,
                    "stride": layer.strides,
                    "padding": layer.padding,
                    "filter_shape": layer.kernel_size,
                    "exponent": out_exponents[exponent_idx],
                }
            )
            exponent_idx += 1
        elif isinstance(layer, MaxPooling2D):
            info.update(
                {
                    "filter_shape": layer.pool_size,
                    "stride": layer.strides,
                    "padding": layer.padding,
                }
            )
        elif isinstance(layer, Dense):
            idx = layer.name.split("_")[-1]
            idx = int(idx) if idx.isdigit() else None
            info.update(
                {
                    "index": idx,
                    "exponent": out_exponents[exponent_idx],
                }
            )
            exponent_idx += 1

            if info["activation"] == "softmax":
                info["softmax_exponent"] = out_exponents[exponent_idx]
                exponent_idx += 1

        layer_information.append(info)

    return input_exponent, layer_information
