/* eslint-disable global-require */
/* eslint-disable import/no-dynamic-require */
/* eslint-disable import/no-extraneous-dependencies */
const fs = require('fs-extra');

const {
  controllerNotFound, serviceNotFound, unknown
} = require('../constants/errorCodes');

const controllers = {};

// initializeRoutes does the following:
// 1. Requires all the controller files from the controllers folder
// 2. Stores their routes in the controllers dictionary

const initializeRoutes = async () => {
  const files = await fs.readdir('./controllers');

  files.forEach((file) => {
    const fileControllers = require(`../controllers/${file}`);
    const { routes } = fileControllers;

    const appName = file.split('.')[0];
    controllers[appName] = routes;
  });

  console.log('\x1b[32m%s\x1b[0m', 'Routes Initialized'); // green console log
  return (controllers);
};

// distributor maps the upcoming request URL to the appropriate
// service in the appropriate controller
const distributor = async (req, user) => {
  try {
    const url = req.originalUrl;
    const array = url.split('/');
    const controllerName = array[1];
    const serviceName = array[2];
    const controller = controllers[controllerName];
    if (!controller) {
      return { statusCode: controllerNotFound, error: 'Controller not found' };
    }
    const service = controller[serviceName];
    if (!service) {
      return { statusCode: serviceNotFound, error: 'Service not found' };
    }
    return await service(req, user);
  } catch (exception) {
    return { statusCode: unknown, error: exception.message };
  }
};

// explore returns all exported services in all containers
const explore = async (req) => {
  const result = {};
  Object.keys(controllers).forEach((controller) => {
    result[`Routes for ${controller}`] = [];
    Object.keys(controllers[controller]).forEach((route) => {
      result[`Routes for ${controller}`]
        .push(`${req.protocol}://${req.get('host')}/${controller}/${route}`);
    });
  });
  return result;
};

module.exports = {
  controllers, distributor, explore, initializeRoutes
};
