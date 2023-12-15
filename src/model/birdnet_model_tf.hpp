#pragma once

#include <stdint.h>

#include "dl_layer_model.hpp"
#include "dl_layer_relu.hpp"
#include "dl_layer_conv2d.hpp"
#include "dl_layer_softmax.hpp"
#include "dl_layer_reshape.hpp"
#include "dl_layer_max_pool2d.hpp"
#include "dl_layer_fullyconnected.hpp"

#include "birdnet_tf_conv_optimized_coefficient.hpp"

using namespace dl;
using namespace layer;
using namespace birdnet_tf_conv_optimized_coefficient;

class BIRDNET : public Model<int16_t>
{
private:
    Conv2D<int16_t> conv1;
    MaxPool2D<int16_t> pool1;
    Conv2D<int16_t> conv2;
    MaxPool2D<int16_t> pool2;
    Conv2D<int16_t> conv3;
    MaxPool2D<int16_t> pool3;
    Reshape<int16_t> flatten;
    FullyConnected<int16_t> fc1;
    FullyConnected<int16_t> fc2;

public:
    Softmax<int16_t> softmax;

    BIRDNET() : conv1(Conv2D<int16_t>(-15, get_statefulpartitionedcall_sequential_conv2d_biasadd_filter(), get_statefulpartitionedcall_sequential_conv2d_biasadd_bias(), get_statefulpartitionedcall_sequential_conv2d_biasadd_activation(), PADDING_NOT_SET, {0, 0, 0, 0}, 3, 1, "conv1")),
                pool1(MaxPool2D<int16_t>({2, 1}, PADDING_NOT_SET, {0, 0, 0, 0}, 2, 1, "pool1")),

                conv2(Conv2D<int16_t>(-14, get_statefulpartitionedcall_sequential_conv2d_1_biasadd_filter(), get_statefulpartitionedcall_sequential_conv2d_1_biasadd_bias(), get_statefulpartitionedcall_sequential_conv2d_1_biasadd_activation(), PADDING_NOT_SET, {0, 0, 0, 0}, 1, 1, "conv2")),
                pool2(MaxPool2D<int16_t>({2, 2}, PADDING_NOT_SET, {0, 0, 0, 0}, 2, 2, "pool2")),

                conv3(Conv2D<int16_t>(-12, get_statefulpartitionedcall_sequential_conv2d_2_biasadd_filter(), get_statefulpartitionedcall_sequential_conv2d_2_biasadd_bias(), get_statefulpartitionedcall_sequential_conv2d_2_biasadd_activation(), PADDING_NOT_SET, {0, 0, 0, 0}, 1, 1, "conv3")),
                pool3(MaxPool2D<int16_t>({2, 2}, PADDING_NOT_SET, {0, 0, 0, 0}, 2, 2, "pool3")),

                flatten(Reshape<int16_t>({1, 1, 288}, "flatten")),

                fc1(FullyConnected<int16_t>(-13, get_fused_gemm_0_filter(), get_fused_gemm_0_bias(), get_fused_gemm_0_activation(), true, "fc1")),

                fc2(FullyConnected<int16_t>(-12, get_fused_gemm_1_filter(), get_fused_gemm_1_bias(), NULL, true, "fc2")),

                softmax(Softmax<int16_t>(-15, "softmax"))
    {
    }

    void build(Tensor<int16_t> &input)
    {

        this->conv1.build(input, true);

        this->pool1.build(this->conv1.get_output(), true);

        this->conv2.build(this->pool1.get_output(), true);

        this->pool2.build(this->conv2.get_output(), true);

        this->conv3.build(this->pool2.get_output(), true);

        this->pool3.build(this->conv3.get_output(), true);

        this->flatten.build(this->pool3.get_output(), true);

        this->fc1.build(this->flatten.get_output(), true);

        this->fc2.build(this->fc1.get_output(), true);

        this->softmax.build(this->fc2.get_output(), true);
    }

    void call(Tensor<int16_t> &input)
    {

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