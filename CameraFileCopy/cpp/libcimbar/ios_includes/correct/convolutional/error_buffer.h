#ifndef CORRECT_CONVOLUTIONAL_ERROR_BUFFER_H
#define CORRECT_CONVOLUTIONAL_ERROR_BUFFER_H

#include <stdlib.h>
#include <string.h>
#include "correct/convolutional.h"

typedef struct {
    unsigned int index;
    distance_t *errors[2];
    unsigned int num_states;

    const distance_t *read_errors;
    distance_t *write_errors;
} error_buffer_t;

error_buffer_t *error_buffer_create(unsigned int num_states);
void error_buffer_destroy(error_buffer_t *buf);
void error_buffer_reset(error_buffer_t *buf);
void error_buffer_swap(error_buffer_t *buf);

#endif // CORRECT_CONVOLUTIONAL_ERROR_BUFFER_H
