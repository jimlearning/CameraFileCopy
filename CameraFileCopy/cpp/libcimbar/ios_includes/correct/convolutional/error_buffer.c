#include "correct/convolutional/error_buffer.h"

// iOS平台专用版本 - 确保包含所有必要的头文件
#include <stdlib.h>
#include <string.h>

error_buffer_t *error_buffer_create(unsigned int num_states) {
    error_buffer_t *buf = calloc(1, sizeof(error_buffer_t));

    // 初始化结构体成员
    buf->num_states = num_states;

    // 为错误缓冲区分配内存
    buf->errors[0] = calloc(num_states, sizeof(distance_t));
    buf->errors[1] = calloc(num_states, sizeof(distance_t));

    // 设置初始状态
    buf->index = 0;
    buf->read_errors = buf->errors[0];
    buf->write_errors = buf->errors[1];

    return buf;
}

void error_buffer_destroy(error_buffer_t *buf) {
    if (!buf) return;
    
    free(buf->errors[0]);
    free(buf->errors[1]);
    free(buf);
}

void error_buffer_reset(error_buffer_t *buf) {
    if (!buf) return;
    
    memset(buf->errors[0], 0, buf->num_states * sizeof(distance_t));
    memset(buf->errors[1], 0, buf->num_states * sizeof(distance_t));
    buf->index = 0;
    buf->read_errors = buf->errors[0];
    buf->write_errors = buf->errors[1];
}

void error_buffer_swap(error_buffer_t *buf) {
    if (!buf) return;
    
    buf->read_errors = buf->errors[buf->index];
    buf->index = (buf->index + 1) % 2;
    buf->write_errors = buf->errors[buf->index];
}
