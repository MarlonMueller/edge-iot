const mongoose = require('mongoose')
const { responseJSON, errorJSON } = require('../helpers/generalHelper')
const { unknown } = require('../constants/errorCodes')

const birdSchema = new mongoose.Schema(
  {
    name: String,
    long: Number,
    lat: Number,
  },
  { timestamps: true }
)
const Bird = mongoose.model('Bird', birdSchema)
const put = async (req, res) => {
  const { name, long, lat } = req.body
  try {
    const bird = new Bird({ name, long, lat })
    const newBird = await bird.save()
    return responseJSON(newBird)
  } catch (exception) {
    return errorJSON(unknown, exception.message)
  }
}

const get = async (req, res) => {
  try {
    const birds = await Bird.find()
    return responseJSON(birds)
  } catch (exception) {
    return errorJSON(unknown, exception.message)
  }
}
module.exports = {
  routes: {
    put,
    get,
  },
}
