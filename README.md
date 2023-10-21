# edge-iot
Edge Computing and the Internet of Things

![Screenshot](https://github.com/MarlonMueller/edge-iot/assets/43891487/db83ec13-d95a-4153-ad0e-078c2cbca759)

- (multiple) ESP32 Inference, (one) Raspberry Training
- (multiple) Raspberry Inference, (one) Cloud Training
- (multiple) Raspberry Inference (+ Training) + interconnected  
- (multiple) ESP32 send data to Raspberry/Could

## Directory Structure

```markdown

./edge-iot
├── .github/
│   └── workflows/
│       └── continuous_integration.yml
│       # GitHub Actions workflow for continuous integration
├── docs/
│   # Documentation for the project
│   ├── user_guide.md
│   ├── api_reference.md
│   └── ...
├── src/
│   # Source code for your federated learning application
│   ├── main.py
│   ├── federated_learning/
│   │   ├── client/
│   │   │   ├── client.py
│   │   │   └── ...
│   │   ├── server/
│   │   │   ├── server.py
│   │   │   └── ...
│   │   └── ...
├── data/
│   # Data files used in your federated learning model
│   ├── dataset1.csv
│   ├── dataset2.csv
│   └── ...
├── config/
│   # Configuration files for your application
│   ├── config.json
│   └── ...
├── tests/
│   # Unit tests and test data
│   ├── test_client.py
│   ├── test_server.py
│   └── ...
├── bin/
│   # Executable binary files
│   ├── myapp
│   └── ...
├── lib/
│   # Library files
│   ├── mylib.so
│   └── ...
├── Dockerfile
│   # Docker configuration for containerizing your application
├── .gitignore
│   # Files and directories to be ignored by Git
├── README.md
│   # Project documentation and overview
```
