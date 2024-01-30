const mongoose = require('mongoose')
const { responseJSON, errorJSON } = require('../helpers/generalHelper')
const { unknown } = require('../constants/errorCodes')

const nodeSchema = new mongoose.Schema(
  {
    _id: String,
    long: Number,
    lat: Number,
    localId: Number
  },
  { timestamps: true }
)
const Node = mongoose.model('Node', nodeSchema)
const put = async (req, res) => {
  const { _id, long, lat } = req.body
  try {
    
    let dataToUpdateOrInsert = {long,lat}
    const nodes = await Node.find()
    const nodeExists = await Node.find({_id})
    if(nodeExists.length===0){
      dataToUpdateOrInsert.localId =nodes.length
    }
    console.log(dataToUpdateOrInsert);
    const newNode = await Node.findOneAndUpdate({_id},dataToUpdateOrInsert, {
      new: true,
      upsert: true // Make this update into an upsert
    });
    return responseJSON(newNode)
  } catch (exception) {
    return errorJSON(unknown, exception.message)
  }
}

const get = async (req, res) => {
  try {
    const nodes = await Node.find()
    return responseJSON(nodes)
  } catch (exception) {
    return errorJSON(unknown, exception.message)
  }
}
module.exports = {
  Node,
  routes: {
    put,
    get,
  },
}
