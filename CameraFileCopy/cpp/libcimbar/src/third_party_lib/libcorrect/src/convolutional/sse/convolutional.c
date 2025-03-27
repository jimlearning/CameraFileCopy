#include "correct/convolutional/sse/convolutional.h"
#include "correct/convolutional/convolutional.h"

// 定义CIMBAR_IOS_PLATFORM确保所有iOS兼容代码生效
#ifndef CIMBAR_IOS_PLATFORM
#define CIMBAR_IOS_PLATFORM
#endif

// 为iOS平台提供的空实现 - 确保API兼容性和编译可通过
correct_convolutional_sse *correct_convolutional_sse_create(size_t rate,
                                                           size_t order,
                                                           const polynomial_t *poly) {
    correct_convolutional_sse *conv = malloc(sizeof(correct_convolutional_sse));
    if (!conv) {
        return NULL;
    }
    
    // 初始化基础convolutional编码器
    correct_convolutional *init_conv = _correct_convolutional_init(&conv->base_conv, rate, order, poly);
    if (!init_conv) {
        free(conv);
        return NULL;
    }
    
    // 在iOS上我们不初始化SSE特定的功能
    // 这里只是保留兼容的API结构
    
    return conv;
}

void correct_convolutional_sse_destroy(correct_convolutional_sse *conv) {
    if (!conv) {
        return;
    }
    
    // 清理资源
    if (conv->base_conv.has_init_decode) {
        // 在iOS上避免访问oct_lookup成员
        // 什么都不做
    }
    
    // 调用基础的清理函数
    _correct_convolutional_teardown(&conv->base_conv);
    free(conv);
}

// 空实现 - 在iOS上不使用SSE
void convolutional_encode_sse(correct_convolutional *conv,
                             const uint8_t *msg, size_t msg_len,
                             uint8_t *encoded) {
    // 空实现 - 在iOS平台上不使用SSE指令
    // 实际应用应该使用非SSE版本
}

// 空实现 - 在iOS上不使用SSE
void convolutional_decode_sse(correct_convolutional *conv, 
                              unsigned int sets,
                              const uint8_t *soft) {
    // 空实现 - 在iOS平台上不使用SSE指令
    // 实际应用应该使用非SSE版本
}

// 空实现 - 在iOS上不使用SSE
void _convolutional_decode_init_sse(correct_convolutional *conv, 
                                   unsigned int min_traceback,
                                   unsigned int traceback_length,
                                   unsigned int renormalize_interval) {
    // 空实现 - 在iOS平台上不使用SSE指令
    // 实际应用应该使用非SSE版本初始化
}
