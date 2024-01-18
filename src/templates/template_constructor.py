from keras.layers import Conv2D, MaxPooling2D, Dense

def render_template(template_dir, model_dir, model_name, tags):
    
    template_path = template_dir / f"{model_name}.jinja"
    model_path = model_dir / f"{model_name}.hpp"
    
    with open(template_path, "r") as f:
        template = f.read()
        
    template = Template(template).render(tags)
    
    with open(model_path, "w") as f :
        f.write(template)

def get_jinja_info(layers_info):
    
    layers = []
    
    def add_jinja_layer_info(layer_type, layer_name, previous_layer_name, **kwargs):
        jinja_layer_info = {
            "layer": layer_type,
            "layer_name": layer_name,
            "previous_layer_name": previous_layer_name
        }
        jinja_layer_info.update(kwargs)
        layers.append(layer_info)
        
    previous_layer_name = None
    
    for layer_info in layer_infos:
        
        layer_type = layer_info["layer_type"]
        layer_name = layer_info["layer_name"]
        
        if layer_type == "Conv2D":
        
            stride = layer_info["stride"]
            exponent = layer_info["exponent"]
            
            if layer_info["padding"] == "valid":
                padding_type = "PADDING_VALID"
            elif layer_info["padding"] == "same":
                padding_type = "PADDING_SAME_END"
            
            add_jinja_layer_info(
                "conv2d",
                layer_name,
                previous_layer_name,
                output_exponent=exponent,
                padding_type=padding_type,
                stride_x=stride[1],
                stride_y=stride[0],
                padding="{}",
                activation=True
            )
        
        elif layer_type == "MaxPooling2D":
            
            stride = layer_info["stride"]
            
            if layer_info["padding"] == "valid":
                padding_type = "PADDING_VALID"
            elif layer_info["padding"] == "same":
                padding_type = "PADDING_SAME_END"
            
            filter_shape = map(str, list(layer_info["filter_shape"]))
            filter_shape = "{" + ",".join(filter_shape) + "}"
            
            add_jinja_layer_info(
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
            
            add_jinja_layer_info(
                "flatten",
                layer_name,
                previous_layer_name,
                inplace="false"
            )
        
        elif layer_type == "Dense":
            
            exponent = layer_info["exponent"]
            softmax = layer_info["activation"] == "softmax"
            
            add_jinja_layer_info(
                "fc",
                layer_name,
                previous_layer_name,
                output_exponent=exponent,
                flatten="true",
                activation=not softmax
            )
            
            if softmax:
                exponent = layer_info["softmax_exponent"]
                previous_layer_name = layer_name
                add_jinja_layer_info(
                    "softmax",
                    "softmax",
                    previous_layer_name,
                    output_exponent=exponent,
                    inplace="false"
                )
                previous_layer_name = "softmax"
        
        previous_layer_name = layer_name
        
    return



def get_layer_info(model, exponents):
    
    layers_info = []
    exponent_idx = 0

    for layer in model.layers:
        layer_info = {
            "layer_type": type(layer).__name__,
            "layer_name": layer.name,
            "input_shape": layer.input_shape,
            "output_shape": layer.output_shape,
            "activation": layer.activation.__name__ if hasattr(layer, "activation") else None
        }

        if isinstance(layer, Conv2D):
            idx = layer.name.split("_")[-1]
            idx = int(idx) if idx.isdigit() else None
            layer_info.update({
                "index": idx,
                "stride": layer.strides,
                "padding": layer.padding,
                "filter_shape": layer.kernel_size,
                "exponent": exponents[exponent_idx],
            })
            exponent_idx += 1
        elif isinstance(layer, MaxPooling2D):
            layer_info.update({
                "filter_shape": layer.pool_size,
                "stride": layer.strides,
                "padding": layer.padding,
            })
        elif isinstance(layer, Dense):
            idx = layer.name.split("_")[-1]
            idx = int(idx) if idx.isdigit() else None
            layer_info.update({
                "index": idx,
                "exponent": exponents[exponent_idx],
            })
            exponent_idx += 1
            
            if layer_info["activation"] == "softmax":
                layer_info["softmax_exponent"] = exponents[exponent_idx]
                exponent_idx += 1

        layers_info.append(layer_info)
        
    return layers_info
