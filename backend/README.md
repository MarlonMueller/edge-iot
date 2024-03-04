# Bird Watcher Node.js Backend

## Installation

To set up the project, follow these steps:

1. Clone the repository
2. Install the required npm packages:

```bash
npm install
```

3. Running the Project:

```bash
npm start
```

## File hierarchy

```
Backend
├─ .eslintrc
├─ .vscode
│  └─ settings.json
├─ constants
│  └─ errorCodes.js
├─ controllers
├─ app.js
├─ helpers
│  ├─ generalHelper.js
│  └─ requestHandler.js
├─ middleware
│  ├─ autoRouter.js
│  └─ bootstrap.js
├─ package-lock.json
├─ package.json
└─ README.md
```

---

## Description of Files and Folders

### `.eslintrc`:

- ESLint configuration settings for code quality control.

### `.vscode`:

- Directory that contains configuration files used by the Visual Studio Code editor.
- `settings.json`:
  - "editor.codeActionsOnSave": Runs ESLint to fix any issues automatically upon saving the file.
  - "eslint.validate": Lints JavaScript files using ESLint.

### `authorization`:

- Contains files related to authorization of the application.

### `constants`:

- `errorCodes.js`: Contains error codes used throughout the application.

### `controllers`:

- Contains files for defining and handling service endpoints.

### `app.js`:

- Main application file, where the server and application are defined.

### `helpers`:

- Contains helper functions for the application.
- `generalHelper.js`: Contains helper functions used throughout the application.
- `requestHandler.js`: Contains functions for handling requests and responses in the application.

### `middleware`:

- Contains middleware functions for the application.
- `autoRouter.js`: Contains middleware functions for automatically routing requests to the correct endpoint.
- `bootstrap.js`: Contains middleware functions for bootstrapping the application.

---
