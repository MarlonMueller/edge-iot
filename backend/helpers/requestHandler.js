const { success, unknown } = require('../constants/errorCodes')
const { errorBuilder, responseBuilder } = require('./generalHelper')

const { distributor } = require('../middleware/autoRouter')

/**
 * Passport configuration.
 */

exports.requestHandler = async (req, res) => {
  try {
    const controller = await distributor(req, res, {})
    if (controller.statusCode === success) {
      return responseBuilder(res, controller.data)
    }
    return errorBuilder(res, controller.statusCode, controller.error)
  } catch (exception) {
    return errorBuilder(res, unknown, exception.message)
  }
}
