import torch
import torch.nn as nn
import numpy as np
import torch.nn.functional as F
from torch.utils.data import DataLoader
from sklearn.metrics import classification_report, accuracy_score


def test(
    model: nn.Module,
    data_loader: DataLoader,
    device,
    create_classification_report=False,
):
    model.eval()

    epoch_loss = 0

    predictions = []
    ground_truth = []

    with torch.no_grad():
        for x_batch, y_batch in data_loader:
            x_batch = x_batch.to(device)
            y_batch = y_batch.to(device)

            out = model(x_batch)

            epoch_loss += F.nll_loss(torch.log(out), y_batch)

            _, idxs = torch.max(out, 1)

            predictions.append(idxs.cpu().numpy())
            ground_truth.append(y_batch.cpu().numpy())

    predictions = np.concatenate(predictions)
    ground_truth = np.concatenate(ground_truth)

    print(
        f"[Testing] Loss: {epoch_loss/len(data_loader):.4f}, Accuracy: {accuracy_score(ground_truth, predictions):.4f}"
    )

    if create_classification_report:
        print(classification_report(ground_truth, predictions))
