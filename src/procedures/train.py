import torch
import torch.nn as nn
from torch.utils.data import DataLoader


def train(model: nn.Module, data_loader: DataLoader, optimizer, criterion, epoch, device):
    model.train()

    epoch_loss = 0

    for batch_idx, (x_batch, y_batch) in enumerate(data_loader):
        # NOTE - Keep batch_idx for now

        x_batch = x_batch.to(device)
        y_batch = y_batch.to(device)

        optimizer.zero_grad()

        out = model(x_batch)

        loss = criterion(out, y_batch)
        loss.backward()

        epoch_loss += loss.item()

        optimizer.step()

    print(f"[Training] Epoch: {epoch}, Loss: {epoch_loss/len(data_loader)}")
