#ifndef CORRECT_CONVOLUTIONAL_METRIC_H
#define CORRECT_CONVOLUTIONAL_METRIC_H

#include <stdint.h>
#include <stdlib.h>
#include "correct/convolutional.h"

// popcount函数定义 - 所有平台都需要
static inline unsigned int popcount(unsigned int x) {
    // 计算二进制中1的数量的简单实现
    unsigned int count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

// 与原始实现保持一致
// 度量两个比特串的汉明距离
// 实现为x XOR y的置位计数
static inline distance_t metric_distance(unsigned int x, unsigned int y) {
    return popcount(x ^ y);
}

// 线性软距离度量
static inline distance_t metric_soft_distance_linear(unsigned int hard_x, const uint8_t *soft_y, size_t len) {
    distance_t dist = 0;
    for (unsigned int i = 0; i < len; i++) {
        unsigned int soft_x = ((int8_t)(0) - (hard_x & 1)) & 0xff;
        hard_x >>= 1;
        int d = soft_y[i] - soft_x;
        dist += (d < 0) ? -d : d;
    }
    return dist;
}

// 二次方软距离度量
// 这个函数在metric.c中实现，所以这里只声明不实现
distance_t metric_soft_distance_quadratic(unsigned int hard_x, const uint8_t *soft_y, size_t len);

// 原始convolutional.h中定义的枚举
typedef enum {
    CORRECT_SOFT_LINEAR,
    CORRECT_SOFT_QUADRATIC,
} soft_measurement_t;

// 给distance_max提供定义
static const distance_t distance_max = UINT16_MAX;

#endif // CORRECT_CONVOLUTIONAL_METRIC_H
