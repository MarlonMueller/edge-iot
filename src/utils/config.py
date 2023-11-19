import os
import yaml

def load_config(config_path, file_name):
    with open(os.path.join(config_path, file_name), "r") as file:
        return yaml.safe_load(file)