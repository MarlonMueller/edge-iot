import torch
import torch.nn as nn
from torchsummary import summary


class Model(nn.Module):
    def __init__(self):
        super(Model, self).__init__()
        self.features = nn.Sequential(
            nn.Conv2d(1, 16, kernel_size=5, stride=2, padding=3),  # 174, 18
            nn.ReLU(),
            nn.MaxPool2d((2, 1), stride=(2, 1)),  # 87, 18
            nn.Conv2d(16, 16, kernel_size=3, stride=1, padding=1),  # 174, 18
            nn.ReLU(),
            nn.MaxPool2d((3, 2), stride=(3, 2)),  # 29, 9
            nn.Conv2d(16, 8, kernel_size=3, stride=1, padding=1),  # 29, 9
            nn.ReLU(),
            nn.MaxPool2d((3, 1), stride=(3, 1)),  # 9, 9
        )

        self.classifier = nn.Sequential(
            nn.Dropout(0.5),
            nn.Linear(8 * 9 * 9, 128),
            nn.Dropout(0.5),
            nn.Linear(128, 5),
            nn.Sigmoid(),
        )

    def forward(self, x):
        x = self.features(x)
        print(x.size())
        x = x.view(x.size(0), -1)
        x = self.classifier(x)
        return x


if __name__ == "__main__":
    model = Model()
    print(summary(model, input_size=(1, 345, 33)))
