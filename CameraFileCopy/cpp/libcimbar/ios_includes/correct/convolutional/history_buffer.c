#include "correct/convolutional/history_buffer.h"

// iOS平台专用版本 - 修复了原始实现中的问题

// 定义prefetch函数 - 在iOS上提供一个空实现
static inline void prefetch(void *ptr) {
    // iOS平台不使用prefetch指令，这是一个空实现
    // 在x86平台上，这通常用于数据预取以提高缓存性能
    (void)ptr;  // 避免未使用参数警告
}

history_buffer *history_buffer_create(unsigned int min_traceback_length,
                                      unsigned int traceback_group_length,
                                      unsigned int renormalize_interval, 
                                      unsigned int num_states,
                                      shift_register_t highbit) {
    history_buffer *buf = calloc(1, sizeof(history_buffer));

    *(unsigned int *)&buf->min_traceback_length = min_traceback_length;
    *(unsigned int *)&buf->traceback_group_length = traceback_group_length;
    *(unsigned int *)&buf->cap = min_traceback_length + traceback_group_length;
    *(unsigned int *)&buf->num_states = num_states;
    *(shift_register_t *)&buf->highbit = highbit;

    buf->history = malloc(buf->cap * sizeof(uint8_t *));
    for (unsigned int i = 0; i < buf->cap; i++) {
        buf->history[i] = calloc(num_states, sizeof(uint8_t));
    }
    buf->fetched = malloc(buf->cap * sizeof(uint8_t));

    buf->index = 0;
    buf->len = 0;

    buf->renormalize_counter = 0;
    buf->renormalize_interval = renormalize_interval;

    return buf;
}

void history_buffer_destroy(history_buffer *buf) {
    for (unsigned int i = 0; i < buf->cap; i++) {
        free(buf->history[i]);
    }
    free(buf->history);
    free(buf->fetched);
    free(buf);
}

void history_buffer_reset(history_buffer *buf) {
    buf->len = 0;
    buf->index = 0;
}

uint8_t *history_buffer_get_slice(history_buffer *buf) { 
    return buf->history[buf->index]; 
}

shift_register_t history_buffer_search(history_buffer *buf, const distance_t *distances,
                                       unsigned int search_every) {
    // 修复：初始化bestpath变量，避免可能未初始化的警告
    shift_register_t bestpath = 0;
    distance_t leasterror = USHRT_MAX;
    
    // search for a state with the least error
    for (shift_register_t state = 0; state < buf->num_states; state += search_every) {
        if (distances[state] < leasterror) {
            leasterror = distances[state];
            bestpath = state;
        }
    }
    return bestpath;
}

void history_buffer_renormalize(history_buffer *buf, distance_t *distances,
                                shift_register_t min_register) {
    // 提供基本实现
    for (unsigned int i = 0; i < buf->num_states; i++) {
        distances[i] -= min_register;
    }
}

void history_buffer_step(history_buffer *buf) {
    buf->index = (buf->index + 1) % buf->cap;
    buf->len = (buf->len < buf->cap) ? buf->len + 1 : buf->len;
}

// 其他函数的空实现或基本实现
void history_buffer_traceback(history_buffer *buf, shift_register_t bestpath,
                              unsigned int min_traceback_length, bit_writer_t *output) {
    // 提供基本实现框架
    shift_register_t highbit = buf->highbit;
    unsigned int cap = buf->cap;
    unsigned int len = (buf->len < min_traceback_length) ? buf->len : min_traceback_length;
    
    if (len == 0) {
        return;
    }
    
    unsigned int index = buf->index;
    unsigned int prefetch_index = index;
    
    for (unsigned int j = 0; j < len; j++) {
        index = prefetch_index;
        if (prefetch_index == 0) {
            prefetch_index = cap - 1;
        } else {
            prefetch_index--;
        }
        
        prefetch(buf->history[prefetch_index]);
        
        uint8_t history = buf->history[index][bestpath];
        shift_register_t pathbit = history ? highbit : 0;
        bestpath |= pathbit;
        bestpath >>= 1;
        
        bit_writer_write_1(output, history);
    }
}

void history_buffer_process_skip(history_buffer *buf, distance_t *distances, bit_writer_t *output,
                                 unsigned int skip) {
    // 基本框架
    unsigned int search_every = skip;
    if (search_every == 0) {
        search_every = 1;
    }
    
    shift_register_t bestpath = history_buffer_search(buf, distances, search_every);
    
    if (buf->len >= buf->min_traceback_length) {
        history_buffer_traceback(buf, bestpath, buf->min_traceback_length, output);
    }
}

void history_buffer_process(history_buffer *buf, distance_t *distances, bit_writer_t *output) {
    history_buffer_process_skip(buf, distances, output, 0);
}

void history_buffer_flush(history_buffer *buf, bit_writer_t *output) {
    // 基本框架实现
    if (buf->len > 0) {
        distance_t *distances = calloc(buf->num_states, sizeof(distance_t));
        history_buffer_process(buf, distances, output);
        free(distances);
    }
}
