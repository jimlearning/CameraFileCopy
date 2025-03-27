#ifndef CORRECT_CONVOLUTIONAL
#define CORRECT_CONVOLUTIONAL
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

// 不包含correct.h，而是直接定义我们需要的符号
// #include "correct.h"
// #include "correct/portable.h"

typedef unsigned int shift_register_t;
typedef uint16_t polynomial_t;
typedef uint64_t path_t;
typedef uint8_t soft_t;
static const soft_t soft_max = UINT8_MAX;

typedef uint16_t distance_t;
typedef unsigned int hard_t;

// 前向声明所需的结构体和函数
struct correct_convolutional;
typedef struct correct_convolutional correct_convolutional;

correct_convolutional *correct_convolutional_create(size_t rate, size_t order, const polynomial_t *poly);
void correct_convolutional_destroy(correct_convolutional *conv);
size_t correct_convolutional_encode_len(correct_convolutional *conv, size_t msg_len);
ssize_t correct_convolutional_encode(correct_convolutional *conv, const uint8_t *msg, size_t msg_len, uint8_t *encoded);
ssize_t correct_convolutional_decode(correct_convolutional *conv, const uint8_t *encoded, size_t encoded_len, uint8_t *msg);

// 软判决解码函数声明
size_t correct_convolutional_decode_soft_len(correct_convolutional *conv, size_t soft_len);
ssize_t correct_convolutional_decode_soft(correct_convolutional *conv, const uint8_t *soft, size_t soft_len, uint8_t *msg);

#endif // CORRECT_CONVOLUTIONAL
