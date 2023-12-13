# A boilerplate for Ahmed Kaddah Node.js microservices

## File hierarchy

```
MICROSERVICE_NAME
├─ .eslintrc
├─ .vscode
│  └─ settings.json
├─ authorization
├─ config
│  ├─ database.js
│  └─ keys.js
├─ constants
│  ├─ enums.js
│  └─ errorCodes.js
├─ controllers
├─ MICROSERVICE_NAME.js
├─ helpers
│  ├─ generalHelper.js
│  └─ requestHandler.js
├─ middleware
│  ├─ authentication.js
│  ├─ authorization.js
│  ├─ autoRouter.js
│  └─ bootstrap.js
├─ models
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

### `config`:

- `database.js`: Contains the configuration settings for the database connection.
- `keys.js`: Contains sensitive data such as API keys, secrets, and passwords.

### `constants`:

- `enums.js`: Contains the constant values used in the application.
- `errorCodes.js`: Contains error codes used throughout the application.

### `controllers`:

- Contains files for defining and handling service endpoints.

### `MICROSERVICE_NAME.js`:

- Main application file, where the server and application are defined.

### `helpers`:

- Contains helper functions for the application.
- `generalHelper.js`: Contains helper functions used throughout the application.
- `requestHandler.js`: Contains functions for handling requests and responses in the application.

### `middleware`:

- Contains middleware functions for the application.
- `authentication.js`: Contains middleware functions for authenticating users.
- `authorization.js`: Contains middleware functions for authorizing access to endpoints.
- `autoRouter.js`: Contains middleware functions for automatically routing requests to the correct endpoint.
- `bootstrap.js`: Contains middleware functions for bootstrapping the application.

### `models`:

- Contains files for defining and accessing database models and schemas.

---
