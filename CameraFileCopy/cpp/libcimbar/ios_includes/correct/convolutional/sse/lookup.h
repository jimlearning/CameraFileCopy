#ifndef CORRECT_CONVOLUTIONAL_SSE_LOOKUP_H
#define CORRECT_CONVOLUTIONAL_SSE_LOOKUP_H

#include <stdlib.h>
#include "../convolutional.h"

// iOS平台兼容性定义 - 提供足够的类型兼容性

// 前向声明
#ifdef CIMBAR_IOS_PLATFORM
// 为iOS平台定义必要的类型
typedef struct {
    unsigned int *keys;
    unsigned int *outputs;
} quad_lookup_t;

typedef struct {
    unsigned int *keys;
    unsigned int *outputs;
} oct_lookup_t;

// 函数声明 - iOS上这些将是空实现
quad_lookup_t quad_lookup_create(unsigned int rate, unsigned int order, const unsigned int *table);
void quad_lookup_destroy(quad_lookup_t quads);
oct_lookup_t oct_lookup_create(unsigned int rate, unsigned int order, const unsigned int *table);
void oct_lookup_destroy(oct_lookup_t octs);
#else
// 不在iOS平台上，使用原始声明
typedef struct {
    unsigned int *keys;
    unsigned int *outputs;
} quad_lookup_t;

typedef struct {
    unsigned int *keys;
    unsigned int *outputs;
} oct_lookup_t;
#endif

#endif // CORRECT_CONVOLUTIONAL_SSE_LOOKUP_H
