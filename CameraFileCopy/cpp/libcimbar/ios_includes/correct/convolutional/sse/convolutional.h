#ifndef CORRECT_CONVOLUTIONAL_SSE_CONVOLUTIONAL_H
#define CORRECT_CONVOLUTIONAL_SSE_CONVOLUTIONAL_H

// iOS平台兼容头文件 - 不包含x86特有的指令集
// 在iOS平台上，我们避免包含x86特定的头文件
// #ifdef _MSC_VER
// #include <intrin.h>
// #else
// #include <x86intrin.h>
// #endif

#include "../convolutional.h"

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
