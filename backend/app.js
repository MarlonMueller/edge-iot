/**
 * Module dependencies.
 */
const express = require('express');
const bodyParser = require('body-parser');
const errorHandler = require('errorhandler');
const lusca = require('lusca');
const dotenv = require('dotenv');
const cors = require('cors');
const cookieParser = require('cookie-parser');
const mongoose = require('mongoose');
const { bootstrap } = require('./middleware/bootstrap');
const { explore } = require('./middleware/autoRouter');
const { requestHandler } = require('./helpers/requestHandler');

/**
 * Load environment variables from .env file, where API keys and passwords are configured.
 */
dotenv.config({ path: '.env' });

/**
 * Create Express server.
 */
const app = express();

/**
 * Configure CORS Policies
 */
app.use(cors({
  origin: ['http://localhost:5173'], // Add dev and deployed URLs for frontend here
  credentials: true, // Allow httpOnly cookie for security
}));

/**
 * Connect to DB.
 */
mongoose
  .connect('mongodb+srv://cami:cami@cluster0.curwd.mongodb.net/?retryWrites=true&w=majority')
  .then(() => {
    console.log('Connected to db.');
  })
  .catch((err) => {
    console.error('Unable to connect to db .', err);
  });

/**
 * Express configuration.
 */
app.set('port', process.env.PORT || 8080);

// app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
// A CSRF token is a secure random token (e.g., synchronizer token or challenge token) that is used to prevent CSRF attacks.
// The token needs to be unique per user session and should be of large random value to make it difficult to guess.
// A CSRF secure application assigns a unique CSRF token for every user session

// app.use((req, res, next) => {
//   lusca.csrf()(req, res, next);
// });

app.use(lusca.xframe('SAMEORIGIN'));
app.use(lusca.xssProtection(true));
app.disable('x-powered-by');

/**
 * Set up HTTP Only Cookie for authentication.
 */
app.use(cookieParser());

// /**
//  * Primary app routes.
//  */

// bootstrap does the following:
// 1. Initialize routes for request handler
// 2. Initialize authorization for request handler
bootstrap();
app.get('/', (_, res) => {
  res.send('Welcome to the Bird Watcher backend');
});
// /explore shows all possible routes/endpoints of the backend
app.post('/explore', async (req, res) => {
  const routes = await explore(req);
  return res.json(routes);
});

app.post('*', requestHandler);
/**
 * Error Handler.
 */

if (process.env.NODE_ENV === 'development') {
  // only use in development
  app.use(errorHandler());
} else {
  app.use((err, req, res, next) => {
    console.error(err);
    res.status(500).send('Server Error');
  });
}

/**
 * Start Express server.
 */
app.listen(app.get('port'), () => {
  console.log(`App is running on http://localhost:${app.get('port')} in ${app.get('env')} mode`);
  console.log('Press CTRL-C to stop');
});
module.exports = app;
