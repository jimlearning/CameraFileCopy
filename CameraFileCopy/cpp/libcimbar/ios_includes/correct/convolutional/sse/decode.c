/* 
 * iOS兼容层 - SSE解码器替代实现
 * 提供简化的非SSE实现，保持API兼容性
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "correct/convolutional/sse/convolutional.h"
#include "correct/convolutional/convolutional.h"
#include "correct/convolutional/sse/sse_compat.h"
#include "sse_lookup.h" // 确保包含oct_lookup_fill_distance声明

// 兼容性实现 - 不使用SSE指令
static void convolutional_sse_decode_inner(correct_convolutional_sse *sse_conv, unsigned int sets,
                                          const uint8_t *soft) {
    correct_convolutional *conv = &sse_conv->base_conv;
    
    // 以下是一个简化版实现，不使用SSE指令
    // 直接调用非SSE版本的解码器处理数据
    
    shift_register_t highbit = 1 << (conv->order - 1);
    unsigned int hist_buf_index = conv->history_buffer->index;
    unsigned int hist_buf_cap = conv->history_buffer->cap;
    unsigned int hist_buf_len = conv->history_buffer->len;
    unsigned int hist_buf_rn_int = conv->history_buffer->renormalize_interval;
    unsigned int hist_buf_rn_cnt = conv->history_buffer->renormalize_counter;
    
    // 简化版实现 - 不使用SSE指令
    for (unsigned int i = conv->order - 1; i < (sets - conv->order + 1); i++) {
        distance_t *distances = conv->distances;
        
        // 使用原始方法计算距离
        if (soft) {
            if (conv->soft_measurement == CORRECT_SOFT_LINEAR) {
                for (unsigned int j = 0; j < 1 << (conv->rate); j++) {
                    distances[j] =
                        metric_soft_distance_linear(j, soft + i * conv->rate, conv->rate);
                }
            } else {
                for (unsigned int j = 0; j < 1 << (conv->rate); j++) {
                    distances[j] =
                        metric_soft_distance_quadratic(j, soft + i * conv->rate, conv->rate);
                }
            }
        } else {
            unsigned int out = bit_reader_read(conv->bit_reader, conv->rate);
            for (unsigned int j = 0; j < 1 << (conv->rate); j++) {
                distances[j] = metric_distance(j, out);
            }
        }
        
        // 在iOS平台上使用我们简化的查找函数
        oct_lookup_fill_distance(sse_conv->oct_lookup, distances);
        
        // 以下是处理错误距离的简化版本
        unsigned int num_iter = highbit << 1;
        const distance_t *read_errors = conv->errors->read;
        distance_t *write_errors = conv->errors->write;
        
        // 简化的错误计算逻辑 - 不使用SSE指令
        for (unsigned int j = 0; j < num_iter; j++) {
            shift_register_t register_val = j;
            distance_t min_distance = read_errors[j];
            
            // 对每个可能的输入位，计算最小距离
            for (unsigned int k = 0; k < 2; k++) {
                shift_register_t prev_register = (j >> 1) | (k << (conv->order - 1));
                distance_t prev_error = read_errors[prev_register];
                
                // 根据当前注册值查找距离
                unsigned int output = conv_bit_writer_lookup(conv->bit_writer, prev_register, conv->rate);
                distance_t distance = prev_error + distances[output];
                
                // 更新最小距离
                if (distance < min_distance) {
                    min_distance = distance;
                    register_val = prev_register;
                }
            }
            
            // 写入错误和历史
            write_errors[j] = min_distance;
            push_history(conv->history_buffer, j, register_val);
        }
        
        // 交换错误缓冲区
        swap_error_buffers(conv->errors);
    }
    
    // 更新历史缓冲区状态
    conv->history_buffer->index = hist_buf_index;
    conv->history_buffer->cap = hist_buf_cap;
    conv->history_buffer->len = hist_buf_len;
    conv->history_buffer->renormalize_interval = hist_buf_rn_int;
    conv->history_buffer->renormalize_counter = hist_buf_rn_cnt;
}

// 以下函数保持与原API完全兼容

void _convolutional_sse_decode_init(correct_convolutional_sse *conv,
                                   unsigned int min_traceback,
                                   unsigned int traceback_length,
                                   unsigned int renormalize_interval) {
    // 在iOS平台上，我们只简单地调用基础版本的初始化函数
    _convolutional_decode_init(&conv->base_conv, min_traceback, traceback_length, renormalize_interval);
}

ssize_t _convolutional_sse_decode(correct_convolutional_sse *sse_conv,
                                 size_t num_encoded_bits, size_t num_encoded_bytes,
                                 uint8_t *msg, const soft_t *soft_encoded) {
    
    correct_convolutional *conv = &sse_conv->base_conv;
    
    // 复制原始SSE解码器的基本逻辑，但使用我们简化的内部解码函数
    if (num_encoded_bits % conv->rate) {
        return -1;
    }
    
    size_t num_sets = num_encoded_bits / conv->rate;
    
    if (!soft_encoded && num_encoded_bytes == 0) {
        return -1;
    }
    
    reset_error_buffer(conv->errors);
    convolutional_sse_decode_inner(sse_conv, num_sets, soft_encoded);
    
    distance_t min_error = conv->errors->read[0];
    size_t error_index = 0;
    for (size_t i = 1; i < (1 << conv->order); i++) {
        if (conv->errors->read[i] < min_error) {
            min_error = conv->errors->read[i];
            error_index = i;
        }
    }
    
    // 回溯历史记录中的路径
    size_t history_length = output_traceback(conv->history_buffer, error_index, msg);
    
    return (history_length + 7) / 8;
}

ssize_t correct_convolutional_sse_decode(correct_convolutional_sse *conv, const uint8_t *encoded,
                                        size_t num_encoded_bits, uint8_t *msg) {
    correct_convolutional *base_conv = &conv->base_conv;
    
    bit_reader_init(base_conv->bit_reader, encoded, num_encoded_bits);
    return _convolutional_sse_decode(conv, num_encoded_bits, (num_encoded_bits + 7) / 8, msg, NULL);
}

ssize_t correct_convolutional_sse_decode_soft(correct_convolutional_sse *conv, const soft_t *encoded,
                                             size_t num_encoded_bits, uint8_t *msg) {
    return _convolutional_sse_decode(conv, num_encoded_bits, 0, msg, encoded);
}
