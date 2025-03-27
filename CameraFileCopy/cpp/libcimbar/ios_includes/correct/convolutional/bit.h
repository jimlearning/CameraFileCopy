#ifndef CORRECT_CONVOLUTIONAL_BIT_H
#define CORRECT_CONVOLUTIONAL_BIT_H

#include <stdlib.h>
#include <stdint.h>

// 定义基本类型
typedef struct {
    uint8_t *bytes;
    size_t byte_index;
    size_t bit_index;
    size_t write_index;
    size_t bytes_len;
} bit_writer_t;

typedef struct {
    const uint8_t *bytes;
    size_t byte_index;
    size_t bit_index;
    size_t bytes_len;
} bit_reader_t;

// 函数声明
bit_writer_t *bit_writer_create(size_t len);
void bit_writer_destroy(bit_writer_t *writer);
void bit_writer_write(bit_writer_t *writer, unsigned int val, size_t len);
void bit_writer_pad_to_byte(bit_writer_t *writer);
void bit_writer_flush(bit_writer_t *writer);
size_t bit_writer_length(bit_writer_t *writer);

bit_reader_t *bit_reader_create(const uint8_t *bytes, size_t len);
void bit_reader_destroy(bit_reader_t *reader);
unsigned int bit_reader_read(bit_reader_t *reader, size_t len);
void bit_reader_seek_byte(bit_reader_t *reader);

#endif // CORRECT_CONVOLUTIONAL_BIT_H
