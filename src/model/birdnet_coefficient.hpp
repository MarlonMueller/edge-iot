#pragma once

#include <stdint.h>
#include "dl_constant.hpp"

namespace birdnet_coefficient
{
    const dl::Filter<int8_t> *get__conv1_conv_filter();
    const dl::Bias<int8_t> *get__conv1_conv_bias();
    const dl::Activation<int8_t> *get__conv1_conv_activation();
    const dl::Filter<int8_t> *get__conv2_conv_filter();
    const dl::Bias<int8_t> *get__conv2_conv_bias();
    const dl::Activation<int8_t> *get__conv2_conv_activation();
    const dl::Filter<int8_t> *get__fc1_gemm_filter();
    const dl::Bias<int8_t> *get__fc1_gemm_bias();
    const dl::Activation<int8_t> *get__fc1_gemm_activation();
    const dl::Filter<int8_t> *get__fc2_gemm_filter();
    const dl::Bias<int8_t> *get__fc2_gemm_bias();
}
