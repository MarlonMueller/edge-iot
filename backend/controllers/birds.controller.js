const mongoose = require('mongoose')
const { responseJSON, errorJSON } = require('../helpers/generalHelper')
const { unknown } = require('../constants/errorCodes')
const { Node } = require('./node.controller');

const birdSchema = new mongoose.Schema(
  {
    name: String,
    long: Number,
    lat: Number,
    esp: String,
  },
  { timestamps: true }
)
const Bird = mongoose.model('Bird', birdSchema)
const put = async (req, res) => {
  const { name, nodeId } = req.body
  try {
    
    const node = await Node.findOne({localId:nodeId})

    const bird = new Bird({ name, long: node?.long, lat: node?.lat, esp:nodeId })
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
