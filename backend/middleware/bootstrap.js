const { initializeRoutes } = require('./autoRouter')

exports.bootstrap = async () => {
  // Initialize Routes
  await initializeRoutes()
}
