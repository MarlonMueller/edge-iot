# edge-iot

## Getting Started

```
conda create -n edge-iot python=3.8.18
conda activate edge-iot
pip install -r requirements.txt
```

Xeno-Canto Interface:

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


## Directory Structure

```markdown

./edge-iot
├── .github/
│   └── workflows/
│       └── continuous_integration.yml
├── docs/
│   # Documentation for the project
│   ├── slides/
│   │   └── 00_idea_presentation.pdf
│   ├── reports/
│   │   └── 00_idea_report.pdf
├── src/
│   ├── _example
│   ├── audio/
│   │   ├── preprocess.py
│   ├── data/
│   │   ├── xeno_canto.py
│   ├── model/
│   │   ├── model.py
│   ├── utils/
│   │   ├── visualisation.py
├── data/
├── config/
├── tests/
├── bin/
├── lib/
├── Dockerfile
├── .gitignore
├── README.md
```

## Architecture

![Architecture Design](assets/architecture_design.png)
