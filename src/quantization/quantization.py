import io
import os
import onnx
import pathlib
import numpy as np
import onnxoptimizer
import onnxruntime
import tensorflow as tf
from tensorflow.keras import models
from contextlib import redirect_stdout


def evaluate_model(
    model_dir: pathlib.Path,
    model_name: str,
    quantization_bit: str,
    dataset: tf.data.Dataset,
    dataset_size: int,
    evaluator,
):
    """Evaluats a onnx model and its quantized version.

    :param model_dir: the directory containing the onnx model
    :param model_name: the name of the optimized onnx model
    :param quantization_bit: quantization_bit
    :param dataset: the dataset to evaluate the model on
    :param dataset_size: size of the dataset
    :param evaluator: initialized ESP-DL evaluator
    :return: accuracy of the floating-point model and the quantized model
    """

    optimized_model_path = str(
        model_dir / "onnx_birdnet" / f"{model_name}_optimized.onnx"
    )
    quantization_params_path = (
        model_dir
        / "quantization_params"
        / f"{model_name}_{quantization_bit}_params.pickle"
    )

    provider = "CPUExecutionProvider"
    model_proto = onnx.load(optimized_model_path)
    session = onnxruntime.InferenceSession(optimized_model_path, providers=[provider])

    evaluator.set_providers([provider])
    evaluator.generate_quantized_model(model_proto, quantization_params_path)

    input_name = session.get_inputs()[0].name
    output_names = [n.name for n in model_proto.graph.output]

    num_correct = 0
    num_correct_quant = 0

    for (data, labels) in dataset.as_numpy_iterator():

        # Floating-point model
        prediction = session.run(output_names, {input_name: data})
        num_correct += sum(np.argmax(prediction[0], axis=1) == labels.flatten())

        # Quantized model
        [prediction, _] = evaluator.evalute_quantized_model(data, False)
        num_correct_quant += sum(np.argmax(prediction[0], axis=1) == labels.flatten())

    return (
        (num_correct / dataset_size) * 100,
        (num_correct_quant / dataset_size) * 100,
    )


def _convert_model_batch_to_dynamic(model_proto):
    """Converts the batch size of the model_proto to dynamic.

    :param model_proto: model proto
    :return: updated model proto
    """
    initializers = [node.name for node in model_proto.graph.initializer]
    inputs = []

    for node in model_proto.graph.input:
        inputs.append(node)

    for node in inputs:
        if node.name in initializers:
            model_proto.graph.input.remove(node)

    shape = inputs[0].type.tensor_type.shape
    inputs[0].type.tensor_type.elem_type = onnx.TensorProto.FLOAT
    model_proto.graph.output[0].type.tensor_type.elem_type = onnx.TensorProto.FLOAT
    dim = shape.dim

    if not dim[0].dim_param:
        dim[0].dim_param = "N"
        if dim[0].dim_value:
            dim[0].dim_value = ""
    model_proto = onnx.shape_inference.infer_shapes(model_proto)

    return model_proto


def _add_fused_gemm_name(model_proto):
    """Add a name to the fused gemm nodes.
    
    :param model_proto: model proto
    :return: updated model proto
    """
    nodes = [n for n in model_proto.graph.node if n.op_type == "Gemm"]
    node_name_counter = 0
    for node in nodes:
        if node.name == "":
            node.name = "fused_gemm_" + str(node_name_counter)
            node_name_counter += 1


def optimize_model(onnx_model_path: pathlib.Path):
    """Optimizes the onnx model.

    :param onnx_model_path: onnx model path
    """

    model_proto = onnx.load(onnx_model_path)
    model_proto = _convert_model_batch_to_dynamic(model_proto)
    model_proto = onnxoptimizer.optimize(
        model_proto,
        [
            "fuse_add_bias_into_conv",
            "fuse_pad_into_conv",
            "fuse_matmul_add_bias_into_gemm",
            "fuse_transpose_into_gemm",
            "eliminate_nop_flatten",
            "eliminate_nop_pad",
            "eliminate_nop_transpose",
            "eliminate_unused_initializer",
            "eliminate_duplicate_initializer",
        ],
    )

    _add_fused_gemm_name(model_proto)
    model_name = onnx_model_path.split(".onnx")
    onnx_model_path = model_name[0] + "_optimized.onnx"
    onnx.save(model_proto, onnx_model_path)


def quantize_model(
    model: models.Sequential,
    model_dir: pathlib.Path,
    model_name: str,
    quantization_bit: str,
    calibrator,
    calibration_dataset: tf.data.Dataset,
):
    """Quantizes a trained model.

    :param model: trained model
    :param model_dir: model directory
    :param model_name: model name
    :param quantization_bit: int8 or int16
    :param calibrator: initialized ESP-DL calibrator
    :param calibration_dataset: calibration dataset
    :return: ESP-DL quantization log
    """

    tf_model_path = str(model_dir / "tf_birdnet" / f"{model_name}")
    onnx_model_path = str(model_dir / "onnx_birdnet" / f"{model_name}.onnx")

    # Load the latest checkpoint
    checkpoint_dir = model_dir / "checkpoints" / model_name
    checkpoints = [f for f in os.listdir(checkpoint_dir) if f != "checkpoint"]
    num_checkpoint = max([int(f.split(".")[0]) for f in checkpoints])

    model.load_weights(checkpoint_dir / f"{num_checkpoint}.ckpt")
    tf.saved_model.save(model, tf_model_path)

    target_chip = "esp32s3"
    provider = "CPUExecutionProvider"

    # Convert the model to onnx
    os.system(
        f"python -m tf2onnx.convert --saved-model {tf_model_path} --output {onnx_model_path} --opset 13"
    )

    # Optimize onnx model
    optimize_model(onnx_model_path)

    c_data = []
    for (data, _) in calibration_dataset.as_numpy_iterator():
        c_data.append(data)

    calibration_dataset = np.concatenate(c_data, axis=0)

    optimized_model_path = model_dir / "onnx_birdnet" / f"{model_name}_optimized.onnx"
    model_proto = onnx.load_model(optimized_model_path)
    # checker.check_graph(model_proto.graph)

    # Quantize the model
    calibrator.set_providers([provider])

    quantization_params_path = (
        model_dir
        / "quantization_params"
        / f"{model_name}_{quantization_bit}_params.pickle"
    )

    calibrator.generate_quantization_table(
        model_proto, calibration_dataset, quantization_params_path
    )

    f = io.StringIO()
    # Capture the quantization log
    with redirect_stdout(f):
        calibrator.export_coefficient_to_cpp(
            model_proto,
            quantization_params_path,
            target_chip,
            str(model_dir),
            f"{model_name}_{quantization_bit}_coefficient",
            True,
        )

    quantization_log = f.getvalue()

    # Save the quantization log
    with open(
        model_dir / "quantization_params" / f"{model_name}_{quantization_bit}.txt", "w"
    ) as of:
        of.write(quantization_log)

    return quantization_log
