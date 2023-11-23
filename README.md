# edge-iot

## Getting Started

```
conda create -n edge-iot python=3.8.18
conda activate edge-iot
pip install -r requirements.txt
```


## Directory Structure

```markdown

./edge-iot
├── .github/
│   └── workflows/
│       └── continuous_integration.yml
├── assets/
├── config/
│   └── xeno-canto.yaml
├── data/
│   ├── audio/
│   └── img/
├── docs/
│   ├── slides/
│   └── reports/
├── include/
│   └── audio/
│       └── preprocess.h
├── models/
│   ├── cpp/
│   ├── onnx/
│   └── torch/
├── src/
│   ├── _example
│   ├── audio/
│   ├── dataset/
│   ├── esp32/
│   ├── interface/
│   ├── model/
│   ├── procedures/
│   ├── utils/
├── tests/
├── .gitignore
├── .pre-commit-config.yaml
├── CMakeLists.txt
├── README.md
├── dependencies.lock
├── main.py
├── requirements.txt
├── requirements_idf.txt
```


## Xeno-Canto Interface

Files are downloaded to data/

File naming convention is as follows:
```x-[x_id]-[cls_id]-[slice].{mp3/wav}```
where:
- x_id is the xeno-canto id
- cls_id is the class id
- slice is the slice of the file (0 for now)

The download also generates a annoation file in data/annotations.csv; columns:

- file_name is the file name as above
- class_id is the class id
- class is the class name
- xeno_id is the xeno-canto id
- slice is the slice of the file (0 for now)
- quality is the quality of the recording
- length is the length of the recording
- sampling_rate is the sampling rate of the recording


## Architecture

![Architecture Design](assets/architecture_design.png)
