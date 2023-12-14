#pragma once

#include <stdint.h>

#include "dl_layer_model.hpp"
#include "dl_layer_relu.hpp"
#include "dl_layer_conv2d.hpp"
#include "dl_layer_softmax.hpp"
#include "dl_layer_flatten.hpp"
#include "dl_layer_max_pool2d.hpp"
#include "dl_layer_fullyconnected.hpp"

#include "birdnet_coefficient.hpp"

using namespace dl;
using namespace layer;
using namespace birdnet_coefficient;

class BIRDNET : public Model<int16_t>
{
private:

    Conv2D<int16_t> conv1;
    MaxPool2D<int16_t> pool1;
    Conv2D<int16_t> conv2;
    MaxPool2D<int16_t> pool2;
    Conv2D<int16_t> conv3;
    MaxPool2D<int16_t> pool3;
    Flatten<int16_t> flatten;
    FullyConnected<int16_t> fc1;
    FullyConnected<int16_t> fc2;
    

public:

    Softmax<int16_t> softmax;
    

    BIRDNET() :
        conv1(Conv2D<int16_t>(-14, get__conv1_conv_filter(), get__conv1_conv_bias(), get__conv1_conv_activation(), PADDING_NOT_SET, {1,1,0,0}, 3, 1, "conv1")),
        pool1(MaxPool2D<int16_t>({2,1}, PADDING_NOT_SET, {1,1,0,0}, 2, 1, "pool1")),
        
        conv2(Conv2D<int16_t>(-12, get__conv2_conv_filter(), get__conv2_conv_bias(), get__conv2_conv_activation(), PADDING_NOT_SET, {0,0,0,0}, 1, 1, "conv2")),
        pool2(MaxPool2D<int16_t>({2,2}, PADDING_NOT_SET, {1,1,0,0}, 2, 2, "pool2")),
        
        conv3(Conv2D<int16_t>(-11, get__conv3_conv_filter(), get__conv3_conv_bias(), get__conv3_conv_activation(), PADDING_NOT_SET, {0,0,0,0}, 1, 1, "conv3")),
        pool3(MaxPool2D<int16_t>({2,2}, PADDING_NOT_SET, {1,1,0,0}, 2, 2, "pool3")),
        
        flatten(Flatten<int16_t>("flatten", false)),
        
        fc1(FullyConnected<int16_t>(-10, get__fc1_gemm_filter(), get__fc1_gemm_bias(), get__fc1_gemm_activation(), true, "fc1")),
        
        fc2(FullyConnected<int16_t>(-11, get__fc2_gemm_filter(), get__fc2_gemm_bias(), NULL, true, "fc2")),
        
        softmax(Softmax<int16_t>(-15, "softmax")){}
        

    void build(Tensor<int16_t>& input) {
        
        
        this->conv1.build(input, false);
        
        this->pool1.build(this->conv1.get_output(), false);
        
        this->conv2.build(this->pool1.get_output(), false);
        
        this->pool2.build(this->conv2.get_output(), false);
        
        this->conv3.build(this->pool2.get_output(), false);
        
        this->pool3.build(this->conv3.get_output(), false);
        
        this->flatten.build(this->pool3.get_output(), false);
        
        this->fc1.build(this->flatten.get_output(), false);
        
        this->fc2.build(this->fc1.get_output(), false);
        
        this->softmax.build(this->fc2.get_output(), false);
        
    }

    void call(Tensor<int16_t>& input) {
        
        
        this->conv1.call(input);
        input.free_element();
        
        this->pool1.call(this->conv1.get_output());
        this->conv1.get_output().free_element();
        
        this->conv2.call(this->pool1.get_output());
        this->pool1.get_output().free_element();
        
        this->pool2.call(this->conv2.get_output());
        this->conv2.get_output().free_element();
        
        this->conv3.call(this->pool2.get_output());
        this->pool2.get_output().free_element();
        
        this->pool3.call(this->conv3.get_output());
        this->conv3.get_output().free_element();
        
        this->flatten.call(this->pool3.get_output());
        this->pool3.get_output().free_element();
        
        this->fc1.call(this->flatten.get_output());
        this->flatten.get_output().free_element();
        
        this->fc2.call(this->fc1.get_output());
        this->fc1.get_output().free_element();
        
        this->softmax.call(this->fc2.get_output());
        this->fc2.get_output().free_element();
        
    }
};