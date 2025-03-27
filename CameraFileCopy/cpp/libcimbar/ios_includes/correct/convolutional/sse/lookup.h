#ifndef CORRECT_CONVOLUTIONAL_SSE_LOOKUP_H
#define CORRECT_CONVOLUTIONAL_SSE_LOOKUP_H

#include <stdlib.h>
#include "../convolutional.h"

// iOS平台兼容性定义 - 提供足够的类型兼容性

// 定义所有平台都需要的类型
// 不使用条件编译，确保兼容性

// 基本类型定义
typedef uint64_t output_oct_t;
typedef unsigned int distance_oct_key_t;
typedef unsigned int output_quad_t;
typedef uint64_t distance_quad_t;
typedef unsigned int distance_pair_t;

// 完整的结构体定义，包含所有必要成员
typedef struct {
    unsigned int *keys;
    unsigned int *outputs;
    unsigned int output_mask;
    unsigned int output_width;
    size_t outputs_len;
    distance_quad_t **distances;
} quad_lookup_t;

typedef struct {
    distance_oct_key_t *keys;
    output_oct_t *outputs;
    unsigned int output_mask;
    unsigned int output_width;
    size_t outputs_len;
    uint64_t *distances;
} oct_lookup_t;

// 函数声明 - 在iOS兼容层中会提供空实现
quad_lookup_t quad_lookup_create(unsigned int rate, unsigned int order, const unsigned int *table);
void quad_lookup_destroy(quad_lookup_t quads);
void quad_lookup_fill_distance(quad_lookup_t quads, distance_t *distances);
oct_lookup_t oct_lookup_create(unsigned int rate, unsigned int order, const unsigned int *table);
void oct_lookup_destroy(oct_lookup_t octs);
distance_oct_key_t oct_lookup_find_key(unsigned int *outputs, unsigned int out, size_t num_keys);

#endif // CORRECT_CONVOLUTIONAL_SSE_LOOKUP_H
