/**
 * Module dependencies.
 */

const path = require('path');
const WebSocket = require('ws');
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


//SET UP WEBSOCKET
const WS_PORT = process.env.WS_PORT || 8888;

const wsServer = new WebSocket.Server({ port: WS_PORT }, () =>
  console.log(`WS server is listening at ws://localhost:${WS_PORT}`)
);
// array of connected websocket clients
let connectedClients = [];

wsServer.on("connection", (ws, req) => {
  console.log("Connected");
  // add new connected client
  connectedClients.push(ws);
  // listen for messages from the streamer, the clients will not send anything so we don't need to filter
  ws.on("message", (data) => {
    connectedClients.forEach((ws, i) => {
      if (ws.readyState === ws.OPEN) {
        ws.send(data);
      } else {
        connectedClients.splice(i, 1);
      }
    });
  });
});
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

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

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
app.use("/image", express.static("image"));
app.use("/js", express.static("js"));
app.get("/audio", (req, res) =>
  res.sendFile(path.resolve(__dirname, "./audio_client.html"))
);
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
  console.log(`Recordings page at http://localhost:${app.get('port')}/audio`);
  console.log('Press CTRL-C to stop');
});
module.exports = app;
