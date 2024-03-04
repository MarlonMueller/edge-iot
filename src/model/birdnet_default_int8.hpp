#pragma once

#include <stdint.h>

#include "dl_layer_model.hpp"
#include "dl_layer_conv2d.hpp"
#include "dl_layer_softmax.hpp"
#include "dl_layer_flatten.hpp"
#include "dl_layer_max_pool2d.hpp"
#include "dl_layer_fullyconnected.hpp"

#include "birdnet_default_int8_coefficient.hpp"

#define TAG "BIRDNET"

using namespace dl;
using namespace layer;
using namespace birdnet_default_int8_coefficient;

// input_exponent: ['-7']

// evaluation: acc_train: 85.9918200408998, acc_train_quant: 86.24744376278119, acc_test: 82.44897959183673, acc_test_quant: 82.85714285714286

class BIRDNET_DEFAULT_INT8 : public Model<int8_t>
{
private:

    Conv2D<int8_t> conv2d;
    MaxPool2D<int8_t> max_pooling2d;
    Conv2D<int8_t> conv2d_1;
    MaxPool2D<int8_t> max_pooling2d_1;
    Conv2D<int8_t> conv2d_2;
    MaxPool2D<int8_t> max_pooling2d_2;
    Flatten<int8_t> flatten;
    FullyConnected<int8_t> dense;
    FullyConnected<int8_t> dense_1;
    

public:

    Softmax<int8_t> softmax;
    

    BIRDNET_DEFAULT_INT8() :
        conv2d(Conv2D<int8_t>(
            -7,
            get_statefulpartitionedcall_sequential_conv2d_biasadd_filter(),
            get_statefulpartitionedcall_sequential_conv2d_biasadd_bias(),
            get_statefulpartitionedcall_sequential_conv2d_biasadd_activation(),
            PADDING_VALID, {},
            3, 1,
            "conv2d")
        ),
        
        max_pooling2d(MaxPool2D<int8_t>({3,1}, PADDING_VALID, {}, 3, 1, "max_pooling2d")),
        
        conv2d_1(Conv2D<int8_t>(
            -5,
            get_statefulpartitionedcall_sequential_conv2d_1_biasadd_filter(),
            get_statefulpartitionedcall_sequential_conv2d_1_biasadd_bias(),
            get_statefulpartitionedcall_sequential_conv2d_1_biasadd_activation(),
            PADDING_VALID, {},
            1, 1,
            "conv2d_1")
        ),
        
        max_pooling2d_1(MaxPool2D<int8_t>({2,2}, PADDING_VALID, {}, 2, 2, "max_pooling2d_1")),
        
        conv2d_2(Conv2D<int8_t>(
            -4,
            get_statefulpartitionedcall_sequential_conv2d_2_biasadd_filter(),
            get_statefulpartitionedcall_sequential_conv2d_2_biasadd_bias(),
            get_statefulpartitionedcall_sequential_conv2d_2_biasadd_activation(),
            PADDING_VALID, {},
            1, 1,
            "conv2d_2")
        ),
        
        max_pooling2d_2(MaxPool2D<int8_t>({2,2}, PADDING_VALID, {}, 2, 2, "max_pooling2d_2")),
        
        flatten(Flatten<int8_t>("flatten", false)),
        
        dense(FullyConnected<int8_t>(
            -3,
            get_fused_gemm_0_filter(),
            get_fused_gemm_0_bias(),
            get_fused_gemm_0_activation(),
            true, "dense")
        ),
        
        dense_1(FullyConnected<int8_t>(
            -3,
            get_fused_gemm_1_filter(),
            get_fused_gemm_1_bias(),
            NULL,
            true, "dense_1")
        ),
        
        softmax(Softmax<int8_t>(-6, "softmax")){}
        


    void build(Tensor<int8_t>& input) {
        
        this->conv2d.build(input, false);
        
        this->max_pooling2d.build(this->conv2d.get_output(), false);
        
        this->conv2d_1.build(this->max_pooling2d.get_output(), false);
        
        this->max_pooling2d_1.build(this->conv2d_1.get_output(), false);
        
        this->conv2d_2.build(this->max_pooling2d_1.get_output(), false);
        
        this->max_pooling2d_2.build(this->conv2d_2.get_output(), false);
        
        this->flatten.build(this->max_pooling2d_2.get_output(), false);
        
        this->dense.build(this->flatten.get_output(), false);
        
        this->dense_1.build(this->dense.get_output(), false);
        
        this->softmax.build(this->dense_1.get_output(), false);
        
    }

    void call(Tensor<int8_t>& input) {
        
        
        this->conv2d.call(input);
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called conv2d");
        log_heap();
        #endif
        input.free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed input");
        log_heap();
        #endif
        
        this->max_pooling2d.call(this->conv2d.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called max_pooling2d");
        log_heap();
        #endif
        this->conv2d.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed conv2d");
        log_heap();
        #endif
        
        this->conv2d_1.call(this->max_pooling2d.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called conv2d_1");
        log_heap();
        #endif
        this->max_pooling2d.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed max_pooling2d");
        log_heap();
        #endif
        
        this->max_pooling2d_1.call(this->conv2d_1.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called max_pooling2d_1");
        log_heap();
        #endif
        this->conv2d_1.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed conv2d_1");
        log_heap();
        #endif
        
        this->conv2d_2.call(this->max_pooling2d_1.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called conv2d_2");
        log_heap();
        #endif
        this->max_pooling2d_1.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed max_pooling2d_1");
        log_heap();
        #endif
        
        this->max_pooling2d_2.call(this->conv2d_2.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called max_pooling2d_2");
        log_heap();
        #endif
        this->conv2d_2.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed conv2d_2");
        log_heap();
        #endif
        
        this->flatten.call(this->max_pooling2d_2.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called flatten");
        log_heap();
        #endif
        this->max_pooling2d_2.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed max_pooling2d_2");
        log_heap();
        #endif
        
        this->dense.call(this->flatten.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called dense");
        log_heap();
        #endif
        this->flatten.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed flatten");
        log_heap();
        #endif
        
        this->dense_1.call(this->dense.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called dense_1");
        log_heap();
        #endif
        this->dense.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed dense");
        log_heap();
        #endif
        
        this->softmax.call(this->dense_1.get_output());
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Called softmax");
        log_heap();
        #endif
        this->dense_1.get_output().free_element();
        #ifdef CONFIG_HEAP_LOG
        ESP_LOGI(TAG, " Freed dense_1");
        log_heap();
        #endif
        
    }
};