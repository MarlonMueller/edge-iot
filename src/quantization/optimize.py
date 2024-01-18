import os
import numpy as np
import collections

import onnx
import onnxoptimizer
from onnx import TensorProto
from onnx import helper, numpy_helper, version_converter
from onnxruntime.transformers.onnx_model import OnnxModel


def add_fused_gemm_name(model_proto):
    nodes = [n for n in model_proto.graph.node if n.op_type == 'Gemm']
    node_name_counter = 0
    for node in nodes:
        if node.name == '':
            node.name = 'fused_gemm_' + str(node_name_counter)
            node_name_counter += 1


def convert_model_batch_to_dynamic(model):
    initializers =  [node.name for node in model.graph.initializer]
    inputs = []

    for node in model.graph.input:
        inputs.append(node)

    for node in inputs:
        if node.name in initializers:
            model.graph.input.remove(node)

    shape = inputs[0].type.tensor_type.shape
    inputs[0].type.tensor_type.elem_type = TensorProto.FLOAT
    model.graph.output[0].type.tensor_type.elem_type = TensorProto.FLOAT
    dim = shape.dim
    
    if not dim[0].dim_param:
        dim[0].dim_param = 'N'
        if dim[0].dim_value:
            dim[0].dim_value = ''
    model = onnx.shape_inference.infer_shapes(model)

    return model


def optimize_fp_model(model_path):
    check_model_extension(model_path)
    model_proto = onnx.load(model_path)
    model_proto = convert_model_batch_to_dynamic(model_proto)
    model_proto = onnxoptimizer.optimize(model_proto,
                                       ['fuse_add_bias_into_conv', 'fuse_pad_into_conv',
                                        'fuse_matmul_add_bias_into_gemm', 'fuse_transpose_into_gemm',
                                        'eliminate_nop_flatten', 'eliminate_nop_pad', 'eliminate_nop_transpose',
                                        'eliminate_unused_initializer', 'eliminate_duplicate_initializer'])

    add_fused_gemm_name(model_proto)
    model_name = model_path.split(".onnx")
    model_path = model_name[0] + "_optimized.onnx"
    onnx.save(model_proto, model_path)

    return model_path

