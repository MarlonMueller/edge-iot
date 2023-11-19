import torch
import torch.nn as nn
from torchsummary import summary


class CustomModel(nn.Module):
    
    def __init__(self, input_size, num_classes):
        super(CustomModel, self).__init__()
        
        print(f"Size of input: {input_size}")
        channels, height, width = input_size
        
        self.features = nn.Sequential(
            nn.Conv2d(channels, 16, kernel_size=5, stride=2, padding=3),  # 174, 18
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
            nn.Linear(self._calculate_feature_size(channels, height, width), 128),
            nn.Dropout(0.5),
            nn.Linear(128, num_classes),
        )

    def forward(self, x):
        x = self.features(x)
        #print(x.size())
        x = x.view(x.size(0), -1)
        x = self.classifier(x)
        return x
    
    def _calculate_feature_size(self, channels, height, width):
        x = torch.rand(1, channels, height, width)
        x = self.features(x)
        feature_size = x.view(1, -1).size(1)
        print(f"Feature size: {feature_size}")
        return feature_size
