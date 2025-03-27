#ifndef CORRECT_CONVOLUTIONAL_SSE_CONVOLUTIONAL_H
#define CORRECT_CONVOLUTIONAL_SSE_CONVOLUTIONAL_H

// iOS平台兼容头文件 - 不包含x86特有的指令集
// 在iOS平台上，我们避免包含x86特定的头文件

#include "../convolutional.h"
#include "lookup.h"

// 定义CIMBAR_IOS_PLATFORM确保所有iOS兼容代码生效
#ifndef CIMBAR_IOS_PLATFORM
#define CIMBAR_IOS_PLATFORM
#endif

// 定义SSE版本的结构体，确保与原始版本兼容
struct correct_convolutional_sse {
    correct_convolutional base_conv;
    oct_lookup_t oct_lookup;
};

typedef struct correct_convolutional_sse correct_convolutional_sse;

// 函数声明 - 为iOS兼容层提供的API
correct_convolutional_sse *correct_convolutional_sse_create(size_t rate,
                                                          size_t order,
                                                          const polynomial_t *poly);

void correct_convolutional_sse_destroy(correct_convolutional_sse *conv);

// 空实现，确保在iOS上编译通过
void convolutional_encode_sse(correct_convolutional *conv,
                              const uint8_t *msg, size_t msg_len,
                              uint8_t *encoded);

// 确保API兼容性的空实现
void convolutional_decode_sse(correct_convolutional *conv, unsigned int sets,
                             const uint8_t *soft);

// 确保API兼容性的空实现
void _convolutional_decode_init_sse(correct_convolutional *conv, unsigned int min_traceback,
                                   unsigned int traceback_length,
                                   unsigned int renormalize_interval);

#endif // CORRECT_CONVOLUTIONAL_SSE_CONVOLUTIONAL_H
