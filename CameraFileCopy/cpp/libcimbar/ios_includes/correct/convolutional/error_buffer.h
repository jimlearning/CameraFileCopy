#ifndef CORRECT_CONVOLUTIONAL_ERROR_BUFFER_H
#define CORRECT_CONVOLUTIONAL_ERROR_BUFFER_H

#include <stdint.h>

// 基本类型定义
typedef struct {
    unsigned int *errors;
    unsigned int win_size;
    unsigned int win_index;
    unsigned long error_sum;
} error_buffer_t;

// 函数声明
error_buffer_t *error_buffer_create(unsigned int win_size);
void error_buffer_destroy(error_buffer_t *err_buf);
void error_buffer_reset(error_buffer_t *err_buf);
void error_buffer_pushing(error_buffer_t *err_buf, unsigned int error);

#endif // CORRECT_CONVOLUTIONAL_ERROR_BUFFER_H
