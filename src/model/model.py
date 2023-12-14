import torch
import torch.nn as nn
from torchsummary import summary
import torch.nn.functional as F

"""
((INPUT-KERNEL+2*PADDING)/STRIDE)+1
"""

class CustomModel(nn.Module):
    def __init__(self, input_size, num_classes):
        super(CustomModel, self).__init__()

        print(f"Size of input: {input_size}")
        channels, height, width = input_size

        # Features (note: no support for nn.Sequential)
        self.conv1 = nn.Conv2d(channels, 16, kernel_size=(5,3), stride=(3,1), padding=(1, 0)) # 183,32 -> 61, 30
        self.relu1 = nn.ReLU(inplace=False)
        self.pool1 = nn.MaxPool2d((2, 1), stride=(2, 1), padding=(1, 0)) # 31, 30
        self.conv2 = nn.Conv2d(16, 16, kernel_size=3, stride=(1,1)) # 29, 28
        self.relu2 = nn.ReLU(inplace=False)
        self.pool2 = nn.MaxPool2d((2, 2), stride=(2, 2), padding=(1, 0)) # 15, 14
        self.conv3 = nn.Conv2d(16, 8, kernel_size=3, stride=(1,1)) # 13,12
        self.relu3 = nn.ReLU(inplace=False)
        self.pool3 = nn.MaxPool2d((2, 2), stride=(2, 2), padding=(1, 0)) # 7, 6

        # Classifier
        self.fc1 = nn.Linear(self._calculate_feature_size(channels, height, width), 64)
        self.relu4 = nn.ReLU(inplace=False)
        self.fc2 = nn.Linear(64, num_classes)

    def forward(self, x):
        x = self.pool1(self.relu1(self.conv1(x)))
        x = self.pool2(self.relu2(self.conv2(x)))
        x = self.pool3(self.relu3(self.conv3(x)))
        #x = self.relu3(self.conv3(x))
        x = torch.flatten(x, 1)
        x = self.fc2(self.relu4(self.fc1(x)))
        output = F.softmax(x, -1)
        return output

        # print(x.size())
        # x = x.view(x.size(0), -1)

    def _calculate_feature_size(self, channels, height, width):
        x = torch.rand(1, channels, height, width)
        x = self.pool1(self.relu1(self.conv1(x)))
        print(f"x.size(): {x.size()}")
        x = self.pool2(self.relu2(self.conv2(x)))
        x = self.pool3(self.relu3(self.conv3(x)))
        #x = self.relu3(self.conv3(x))
        print(f"x.size(): {x.size()}")
        feature_size = x.view(1, -1).size(1)
        print(f"Feature size: {feature_size}")
        return feature_size
