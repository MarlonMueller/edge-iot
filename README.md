# edge-iot

## Getting Started

```
conda create -n edge-iot python=3.8.18 pip
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
│   ├── main.py
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
