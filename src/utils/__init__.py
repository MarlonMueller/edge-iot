COLORS = [
    "#86c7b0",
    "#f7b2bd",
    "#a9d18e",
    "#f3d250",
    "#c2a0e6",
    "#f5a09f",
    "#b0d9e6",
    "#d3a8cc",
    "#f6c85d",
    "#b9a2c5",
]

from .counter import Counter
from .config import load_config
from .vis import plot_image, plot_wave, plot_mel
from .model import save_model, load_model, export_onnx, load_onnx
