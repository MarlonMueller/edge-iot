import numpy as np
import seaborn as sns
import onnx
import onnxoptimizer
import onnxruntime
import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix
from sklearn.utils.multiclass import unique_labels

# Define COLORS
COLORS = [
    "#1f78b4",
    "#33a02c",
    "#6a3d9a",
    "#e31a1c",
]

def plot_history(assets_dir, model_name, history):
    """Plot the training and testing losses & testing accuracy"""

    # plt.rcParams.update({
    #     "font.size": 12,
    #     "font.family": "serif",
    #     "text.usetex": True,
    # })

    metrics = [metric for metric in history.history.keys()]

    training_losses = history.history["loss"]
    testing_losses = history.history["val_loss"]
    training_accuracies = history.history["sparse_categorical_accuracy"]
    testing_accuracies = history.history["val_sparse_categorical_accuracy"]

    num_epochs = len(training_losses)

    fig, ax1 = plt.subplots(figsize=(6, 3)) 

    ax1.plot(range(1, num_epochs + 1), training_losses, label="Training loss", marker="o", color=COLORS[0], zorder=1)
    ax1.plot(range(1, num_epochs + 1), testing_losses, label="Testing loss", marker="o", color=COLORS[1], zorder=1)
    ax1.set_xlabel("Epoch")
    ax1.set_ylabel("Loss", color="black")
    ax1.tick_params("y", colors="black")

   
    ax2 = ax1.twinx()
    ax2.plot(range(1, num_epochs + 1), np.round(np.array(training_accuracies) * 100),
     label="Training accuracy", marker="o", color=COLORS[2], zorder=1)
    ax2.plot(range(1, num_epochs + 1), np.round(np.array(testing_accuracies) * 100),
     label="Testing accuracy", marker="o", color=COLORS[3], zorder=1)
    ax2.set_ylabel("Accuracy (%)", color="black")
    ax2.tick_params("y", colors="black")

    lines, labels = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines + lines2, labels + labels2, loc="lower left", facecolor='white', framealpha=1)

    fig.tight_layout()

    plt.savefig(assets_dir / f"history_{model_name}.pdf")


def plot_confusion_matrices(assets_dir, model_name, model, train_dataset, test_dataset, class_names):
    """
    Plot side-by-side confusion matrices for training and testing datasets.
    """

    # plt.rcParams.update({
    #     "font.size": 21,
    #     "font.family": "serif",
    #     "text.usetex": True,
    # })

    datasets = [train_dataset, test_dataset]
    dataset_names = ["Training", "Testing"]

    fig, axes = plt.subplots(1, 2, figsize=(11, 4))

    for i, (dataset, dataset_name) in enumerate(zip(datasets, dataset_names)):

        y_true = np.concatenate([y.numpy() for _, y in dataset])
        y_pred_probs = model.predict(dataset)
        y_pred = np.argmax(y_pred_probs, axis=1)

        num_classes = len(class_names)
        cm = confusion_matrix(y_true, y_pred, labels=range(num_classes), normalize="all")

        sns.heatmap(
            cm, annot=True, fmt=".0%", cmap="Blues", linewidths=0.5,
            xticklabels=class_names, yticklabels=class_names,
            cbar=False, ax=axes[i], linecolor="black",
        )

        axes[i].tick_params(axis="both", labelsize=19)

        axes[i].set_xlabel("Predicted", fontsize=21)
        axes[i].set_ylabel("True", fontsize=21)
        axes[i].set_title(f"{dataset_name} Dataset", fontsize=21)

    fig.tight_layout()

    plt.savefig(assets_dir / f"cm_{model_name}.pdf")


def plot_quantized_confusion_matrices(assets_dir, model_dir, model_name,quantization_bit,  evaluator, train_dataset, test_dataset, train_size, test_size, class_names):
    """
    Plot side-by-side confusion matrices for training and testing datasets.
    """

    optimized_model_path =  str(model_dir / "onnx_birdnet" / f"{model_name}_optimized.onnx")
    quantization_params_path = model_dir / "quantization_params" / f"{model_name}_{quantization_bit}_params.pickle"
 
    provider = "CPUExecutionProvider"
    model_proto = onnx.load(optimized_model_path)
    session = onnxruntime.InferenceSession(optimized_model_path, providers=[provider])

    evaluator.set_providers([provider])
    evaluator.generate_quantized_model(model_proto, quantization_params_path)

    input_name = session.get_inputs()[0].name
    output_names = [n.name for n in model_proto.graph.output]

    datasets = [train_dataset, test_dataset]
    dataset_names = ["Train", "Test"]
    dataset_sizes = [train_size, test_size]

    fig, axes = plt.subplots(1, 4, figsize=(16, 4))

    for i, (dataset, dataset_name, dataset_size) in enumerate(zip(datasets, dataset_names, dataset_sizes)):
        
        predictions_original = []
        predictions_quantized = []

        for (data, labels) in dataset.as_numpy_iterator():
            # Original Model
            prediction_original = session.run(output_names, {input_name: data})
            predictions_original.append((labels.flatten(), np.argmax(prediction_original[0], axis=1)))

            # Quantized Model
            [prediction_quantized, _] = evaluator.evalute_quantized_model(data, False)
            predictions_quantized.append((labels.flatten(), np.argmax(prediction_quantized[0], axis=1)))

        # Concatenate predictions
        labels_original, preds_original = zip(*predictions_original)
        labels_quantized, preds_quantized = zip(*predictions_quantized)

        # Calculate confusion matrices
        cm_original = confusion_matrix(np.concatenate(labels_original), np.concatenate(preds_original),
                                    labels=range(len(class_names)), normalize="all")

        cm_quantized = confusion_matrix(np.concatenate(labels_quantized), np.concatenate(preds_quantized),
                                        labels=range(len(class_names)), normalize="all")


        # Plot Original Model Confusion Matrix
        sns.heatmap(
            cm_original, annot=True,  fmt=".0%", cmap="Blues", linewidths=0.5,
            xticklabels=class_names, yticklabels=class_names,
            cbar=False, ax=axes[i], linecolor="black", annot_kws={"fontsize": 14}
        )

        axes[i].tick_params(axis="both", labelsize=14)

        axes[i].set_xlabel("Predicted", fontsize=16)
        axes[i].set_ylabel("True", fontsize=16)
        axes[i].set_title(f"{dataset_name} data floating-point", fontsize=16)

        # Plot Quantized Model Confusion Matrix #fmt=".0%",
        sns.heatmap(
            cm_quantized, annot=True,  fmt=".0%", cmap="Blues", linewidths=0.5,
            xticklabels=class_names, yticklabels=class_names,
            cbar=False, ax=axes[i + 2], linecolor="black", annot_kws={"fontsize": 14}
        )

        axes[i + 2].tick_params(axis="both", labelsize=14)

        axes[i + 2].set_xlabel("Predicted", fontsize=16)
        axes[i + 2].set_ylabel("True", fontsize=16)
        axes[i + 2].set_title(f"{dataset_name} data fixed-point ({quantization_bit})", fontsize=16)

    fig.tight_layout()

    plt.savefig(assets_dir / f"cm_{model_name}_{quantization_bit}_original_vs_quantized.pdf")
