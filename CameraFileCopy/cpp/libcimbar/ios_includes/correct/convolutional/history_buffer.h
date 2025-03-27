#ifndef CORRECT_CONVOLUTIONAL_HISTORY_BUFFER_H
#define CORRECT_CONVOLUTIONAL_HISTORY_BUFFER_H

#include <stdint.h>

// 基本类型定义
typedef struct {
    unsigned int *path_metrics;
    uint8_t **histories;
    unsigned int history_length;
    unsigned int *output_ids;
    unsigned int output_count;
    unsigned int output_offset;
    unsigned int output_write_index;
    unsigned int *trunk_metrics;
    unsigned int numstates;
    unsigned int highbit;
    unsigned int n_traceback;
    unsigned int best_state;
    unsigned int best_count;
    unsigned int *best_path;
    unsigned int bits_populated;
    unsigned int bits_populated_highbit;
    unsigned int renormalize_counter;
    unsigned int renormalize_interval;
} history_buffer;

// 函数声明
history_buffer *history_buffer_create(unsigned int numstates, unsigned int traceback_length, unsigned int min_traceback, unsigned int renormalize_interval);
void history_buffer_reset(history_buffer *buffer);
void history_buffer_destroy(history_buffer *buffer);
void history_buffer_process_output(history_buffer *buffer, unsigned int min_traceback);
void history_buffer_traceback(history_buffer *buffer, unsigned int min_traceback);
void history_buffer_normalize(history_buffer *buffer);
void history_buffer_extract_byte(history_buffer *buffer, uint8_t *byte);
void history_buffer_extract_packed(history_buffer *buffer, uint8_t *packed, uint8_t *bit);

#endif // CORRECT_CONVOLUTIONAL_HISTORY_BUFFER_H
