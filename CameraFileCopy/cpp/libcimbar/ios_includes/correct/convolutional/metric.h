#ifndef CORRECT_CONVOLUTIONAL_METRIC_H
#define CORRECT_CONVOLUTIONAL_METRIC_H

#include <stdint.h>
#include "../convolutional.h"

// 基本类型定义
typedef struct {
    unsigned int metric_soft_width;
    unsigned int max_soft_dist;
    distance_t *soft_distance;
} soft_measurement_t;

// 函数声明
distance_t metric_distance_soft(soft_measurement_t soft_measurement, soft_t *received, unsigned int received_len, const hard_t *soft_table, unsigned int table_distance);
distance_t metric_distance_hard(const hard_t *a, const hard_t *b, unsigned int len);
soft_measurement_t soft_measurement_create(unsigned int precision);
void soft_measurement_fill_distance(soft_measurement_t s);
void soft_measurement_destroy(soft_measurement_t s);

#endif // CORRECT_CONVOLUTIONAL_METRIC_H
