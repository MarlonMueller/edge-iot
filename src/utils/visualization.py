import numpy as np
import seaborn as sns
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

    plt.rcParams.update({
        "font.size": 12,
        "font.family": "serif",
        "text.usetex": True,
    })

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
    ax2.set_ylabel("Accuracy (\%)", color="black")
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

    plt.rcParams.update({
        "font.size": 21,
        "font.family": "serif",
        "text.usetex": True,
    })

    datasets = [train_dataset, test_dataset]
    dataset_names = ["Training", "Testing"]

    fig, axes = plt.subplots(1, 2, figsize=(11, 4))

    for i, (dataset, dataset_name) in enumerate(zip(datasets, dataset_names)):

        y_true = np.concatenate([y.numpy() for _, y in dataset])
        y_pred_probs = model.predict(dataset)
        y_pred = np.argmax(y_pred_probs, axis=1)

        num_classes = len(class_names)
        cm = confusion_matrix(y_true, y_pred, labels=range(num_classes))

        sns.heatmap(
            cm, annot=True, fmt="d", cmap="Blues", linewidths=0.5,
            xticklabels=class_names, yticklabels=class_names,
            cbar=False, ax=axes[i], linecolor="black",
        )

        axes[i].tick_params(axis="both", labelsize=19)

        axes[i].set_xlabel("Predicted", fontsize=21)
        axes[i].set_ylabel("True", fontsize=21)
        axes[i].set_title(f"{dataset_name} Dataset", fontsize=21)

    fig.tight_layout()

    plt.savefig(assets_dir / f"cm_{model_name}.pdf")
