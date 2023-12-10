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

class BIRDNET : public Model<int8_t>
{
private:
    Conv2D<int8_t> conv1;
    MaxPool2D<int8_t> pool1;
    Conv2D<int8_t> conv2;
    MaxPool2D<int8_t> pool2;
    Flatten<int8_t> flatten;
    FullyConnected<int8_t> fc1;
    FullyConnected<int8_t> fc2;

public:
    Softmax<int8_t> softmax;

    BIRDNET() : conv1(Conv2D<int8_t>(-7, get__conv1_conv_filter(), get__conv1_conv_bias(), get__conv1_conv_activation(), PADDING_VALID, {}, 1, 2, "conv1")),
                pool1(MaxPool2D<int8_t>({1, 2}, PADDING_VALID, {}, 1, 2, "pool1")),

                conv2(Conv2D<int8_t>(-7, get__conv2_conv_filter(), get__conv2_conv_bias(), get__conv2_conv_activation(), PADDING_VALID, {}, 1, 2, "conv2")),
                pool2(MaxPool2D<int8_t>({2, 4}, PADDING_VALID, {}, 2, 4, "pool2")),

                flatten(Flatten<int8_t>("flatten", false)),

                fc1(FullyConnected<int8_t>(-5, get__fc1_gemm_filter(), get__fc1_gemm_bias(), get__fc1_gemm_activation(), true, "fc1")),

                fc2(FullyConnected<int8_t>(-4, get__fc2_gemm_filter(), get__fc2_gemm_bias(), NULL, true, "fc2")),

                softmax(Softmax<int8_t>(-6, "softmax"))
    {
    }

    void build(Tensor<int8_t> &input)
    {

        this->conv1.build(input, true);

        this->pool1.build(this->conv1.get_output(), true);

        this->conv2.build(this->pool1.get_output(), true);

        this->pool2.build(this->conv2.get_output(), true);

        this->flatten.build(this->pool2.get_output(), true);

        this->fc1.build(this->flatten.get_output(), true);

        this->fc2.build(this->fc1.get_output(), true);

        this->softmax.build(this->fc2.get_output(), true);
    }

    void call(Tensor<int8_t> &input)
    {

        this->conv1.call(input);
        input.free_element();

        this->pool1.call(this->conv1.get_output());
        this->conv1.get_output().free_element();

        this->conv2.call(this->pool1.get_output());
        this->pool1.get_output().free_element();

        this->pool2.call(this->conv2.get_output());
        this->conv2.get_output().free_element();

        this->flatten.call(this->pool2.get_output());
        this->pool2.get_output().free_element();

        this->fc1.call(this->flatten.get_output());
        this->flatten.get_output().free_element();

        this->fc2.call(this->fc1.get_output());
        this->fc1.get_output().free_element();

        this->softmax.call(this->fc2.get_output());
        this->fc2.get_output().free_element();
    }
};