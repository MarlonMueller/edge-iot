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
