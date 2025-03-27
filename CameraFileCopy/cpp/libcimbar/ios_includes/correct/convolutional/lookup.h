#ifndef CORRECT_CONVOLUTIONAL_LOOKUP_H
#define CORRECT_CONVOLUTIONAL_LOOKUP_H

#include <stdint.h>
#include <stdlib.h>
#include "../convolutional.h"

// 基本类型定义
typedef struct {
    unsigned int *keys;
    unsigned int *outputs;
    unsigned int output_mask;
    unsigned int output_width;
    size_t outputs_len;
    distance_t **distances;
} pair_lookup_t;

// 函数声明
pair_lookup_t pair_lookup_create(unsigned int rate, unsigned int order, const unsigned int *table);
void pair_lookup_destroy(pair_lookup_t pair_lookup);
void pair_lookup_fill_distance(pair_lookup_t pair_lookup, distance_t *distances);

#endif // CORRECT_CONVOLUTIONAL_LOOKUP_H
