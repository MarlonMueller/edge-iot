const { unknown, success } = require('../constants/errorCodes');

// Builds a res.json from a JSON
exports.responseBuilder = (res, body) => {
  const result = body
    ? { statusCode: success, data: body }
    : { statusCode: success };
  return res.json(result);
};

// Builds a res.json from a JSON and an error code
exports.errorBuilder = (res,
  statusCode = unknown,
  error = 'Something went wrong') => {
  const result = { statusCode, error };
  return res.json(result);
};

// Returns a json to be built by response builder
exports.responseJSON = (body) => {
  const result = body
    ? { statusCode: success, data: body }
    : { statusCode: success };
  return result;
};

// Returns a json to be built by error builder
exports.errorJSON = (errorCode, body) => {
  if (errorCode) return { statusCode: errorCode, error: body };
  return { statusCode: unknown, error: body || 'Something went wrong' };
};
