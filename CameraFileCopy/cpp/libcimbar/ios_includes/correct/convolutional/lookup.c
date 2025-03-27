#include "correct/convolutional/lookup.h"
#include <stdlib.h>
#include <string.h>

// iOS兼容实现 - 添加popcount函数
static inline unsigned int popcount(unsigned int x) {
    // 计算二进制中1的数量的简单实现
    unsigned int count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

// 表格有numstates行
// 每行包含所有多项式输出位连接在一起
// 例如，对于速率2，每行有2位
// 第一个多项式得到最低有效位，最后一个多项式得到最高有效位
void fill_table(unsigned int rate,
                unsigned int order,
                const polynomial_t *poly,
                unsigned int *table) {
    for (shift_register_t i = 0; i < 1 << order; i++) {
        unsigned int out = 0;
        unsigned int mask = 1;
        for (size_t j = 0; j < rate; j++) {
            out |= (popcount(i & poly[j]) % 2) ? mask : 0;
            mask <<= 1;
        }
        table[i] = out;
    }
}

pair_lookup_t pair_lookup_create(unsigned int rate,
                                 unsigned int order,
                                 const unsigned int *table) {
    pair_lookup_t pairs;

    pairs.keys = malloc(sizeof(unsigned int) * (1 << (order - 1)));
    pairs.outputs = calloc((1 << (rate * 2)), sizeof(unsigned int));
    unsigned int *inv_outputs = calloc((1 << (rate * 2)), sizeof(unsigned int));
    unsigned int output_counter = 1;
    
    // 对于每个（偶数编号）移位寄存器状态，找到该状态和随后跟随它的状态（低位设置）的连接输出
    // 然后，检查这个连接的输出是否已经分配了唯一的键。如果没有，给它一个键。
    // 如果有，检索键。将此键分配给移位寄存器状态。
    for (unsigned int i = 0; i < (1 << (order - 1)); i++) {
        // 首先获取输出对
        unsigned int out = table[i * 2 + 1];
        out <<= rate;
        out |= table[i * 2];

        // 这个连接的输出是否已经存在于输出表中？
        if (!inv_outputs[out]) {
            // 不存在，分配一个新键
            inv_outputs[out] = output_counter;
            pairs.outputs[output_counter] = out;
            output_counter++;
        }
        // 将第i个移位寄存器状态的不透明键设置为连接的输出条目
        pairs.keys[i] = inv_outputs[out];
    }
    pairs.outputs_len = output_counter;
    pairs.output_mask = (1 << (rate)) - 1;
    pairs.output_width = rate;
    pairs.distances = calloc(pairs.outputs_len, sizeof(distance_pair_t));
    free(inv_outputs);
    return pairs;
}

void pair_lookup_destroy(pair_lookup_t pairs) {
    free(pairs.keys);
    free(pairs.outputs);
    free(pairs.distances);
}

void pair_lookup_fill_distance(pair_lookup_t pairs, distance_t *distances) {
    for (unsigned int i = 1; i < pairs.outputs_len; i += 1) {
        output_pair_t concat_out = pairs.outputs[i];
        unsigned int i_0 = concat_out & pairs.output_mask;
        concat_out >>= pairs.output_width;
        unsigned int i_1 = concat_out;

        pairs.distances[i] = (distances[i_1] << 16) | distances[i_0];
    }
}
