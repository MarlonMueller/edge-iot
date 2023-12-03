#pragma once

#include <stdint.h>

#include "dl_layer_model.hpp"
#include "dl_layer_relu.hpp"
#include "dl_layer_conv2d.hpp"
#include "dl_layer_softmax.hpp"
#include "dl_layer_flatten.hpp"
#include "dl_layer_max_pool2d.hpp"
#include "dl_layer_fullyconnected.hpp"

#include "bird_coefficient.hpp"

using namespace dl;
using namespace layer;
using namespace bird_coefficient;

class Bird : public Model<int8_t>
{
private:

    Conv2D<int8_t> conv1;
    MaxPool2D<int8_t> pool1;
    Conv2D<int8_t> conv2;
    MaxPool2D<int8_t> pool2;
    Conv2D<int8_t> conv3;
    MaxPool2D<int8_t> pool3;
    Flatten<int8_t> flatten;
    FullyConnected<int8_t> fc1;
    FullyConnected<int8_t> fc2;
    

public:

    Softmax<int8_t> softmax;
    

    Bird() :
        conv1(Conv2D<int8_t>(3, get__conv1_conv_filter(), get__conv1_conv_bias(), get__conv1_conv_activation(), PADDING_VALID, {}, 2, 2, "conv1")),
        
        pool1(MaxPool2D<int8_t>([1, 3], PADDING_VALID, {}, 1, 3, "pool1")),
        
        conv2(Conv2D<int8_t>(1, get__conv2_conv_filter(), get__conv2_conv_bias(), get__conv2_conv_activation(), PADDING_VALID, {}, 1, 1, "conv2")),
        
        pool2(MaxPool2D<int8_t>([1, 5], PADDING_VALID, {}, 1, 5, "pool2")),
        
        conv3(Conv2D<int8_t>(0, get__conv3_conv_filter(), get__conv3_conv_bias(), get__conv3_conv_activation(), PADDING_VALID, {}, 1, 1, "conv3")),
        
        pool3(MaxPool2D<int8_t>([1, 3], PADDING_VALID, {}, 1, 3, "pool3")),
        
        flatten(Flatten<int8_t>("flatten", false)),
        
        fc1(FullyConnected<int8_t>(0, get__fc1_gemm_filter(), get__fc1_gemm_bias(), get__fc1_gemm_activation(), true, "fc1")),
        
        fc2(FullyConnected<int8_t>(-2, get__fc2_gemm_filter(), get__fc2_gemm_bias(), get__fc2_gemm_activation(), true, "fc2")),
        
        softmax(Softmax<int8_t>(-6, "softmax", false)){}
        

    void build(Tensor<int8_t>& input) {
        
        
        this->conv1.build(input);
        
        this->pool1.build(this->conv1.get_output());
        
        this->conv2.build(this->pool1.get_output());
        
        this->pool2.build(this->conv2.get_output());
        
        this->conv3.build(this->pool2.get_output());
        
        this->pool3.build(this->conv3.get_output());
        
        this->flatten.build(this->pool3.get_output());
        
        this->fc1.build(this->flatten.get_output());
        
        this->fc2.build(this->fc1.get_output());
        
        this->softmax.build(this->fc2.get_output());
        
    }

    void call(Tensor<int8_t>& input) {
        
        
        this->conv1.call(input);
        this->.free_element();
        
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