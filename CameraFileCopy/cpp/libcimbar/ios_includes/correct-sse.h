#ifndef CORRECT_SSE_H
#define CORRECT_SSE_H
#include <correct.h>

/* 
 * iOS平台兼容性版本 - 不包含任何实际SSE实现
 * 为保持API兼容性而提供空定义
 */

struct correct_convolutional_sse;
typedef struct correct_convolutional_sse correct_convolutional_sse;

/* 以下函数在iOS平台上提供空实现或默认实现 */

static inline correct_convolutional_sse *correct_convolutional_sse_create(
    size_t rate, size_t order, const correct_convolutional_polynomial_t *poly) {
    // 在iOS平台上返回NULL，表示不支持SSE功能
    return NULL;
}

static inline void correct_convolutional_sse_destroy(correct_convolutional_sse *conv) {
    // 空实现
}

static inline size_t correct_convolutional_sse_encode_len(correct_convolutional_sse *conv, size_t msg_len) {
    // 返回默认值
    return 0;
}

static inline size_t correct_convolutional_sse_encode(correct_convolutional_sse *conv, const uint8_t *msg,
                                     size_t msg_len, uint8_t *encoded) {
    // 返回默认值
    return 0;
}

static inline ssize_t correct_convolutional_sse_decode(correct_convolutional_sse *conv, const uint8_t *encoded,
                                    size_t num_encoded, uint8_t *msg) {
    // 返回默认值
    return -1;
}

static inline void correct_convolutional_sse_decode_soft(correct_convolutional_sse *conv, const soft_t *encoded,
                                       size_t num_encoded, uint8_t *msg) {
    // 空实现
}

static inline ssize_t correct_convolutional_sse_decode_soft_len(correct_convolutional_sse *conv, size_t num_encoded) {
    // 返回默认值
    return 0;
}
#endif
