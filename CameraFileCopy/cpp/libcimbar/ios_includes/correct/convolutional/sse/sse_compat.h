/* 
 * iOS兼容层 - SSE指令集替代实现
 * 为ARM架构提供SSE类型和函数的基本替代实现
 */
#pragma once

#ifndef __SSE_COMPAT_H
#define __SSE_COMPAT_H

#ifdef CIMBAR_IOS_PLATFORM

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 模拟完整的SSE基础类型
typedef struct {
    int32_t val[4];
} __m128i;

typedef struct {
    float val[4];
} __m128;

// 提供基本的SSE函数替代实现
static inline __m128i _mm_setzero_si128(void) {
    __m128i result;
    for (int i = 0; i < 4; i++) {
        result.val[i] = 0;
    }
    return result;
}

static inline __m128i _mm_set1_epi32(int a) {
    __m128i result;
    for (int i = 0; i < 4; i++) {
        result.val[i] = a;
    }
    return result;
}

static inline __m128i _mm_set1_epi8(char a) {
    __m128i result;
    char* bytes = (char*)result.val;
    for (int i = 0; i < 16; i++) {
        bytes[i] = a;
    }
    return result;
}

static inline __m128i _mm_setr_epi8(char e15, char e14, char e13, char e12, 
                                   char e11, char e10, char e9, char e8,
                                   char e7, char e6, char e5, char e4,
                                   char e3, char e2, char e1, char e0) {
    __m128i result;
    char* bytes = (char*)result.val;
    bytes[0] = e0;   bytes[1] = e1;   bytes[2] = e2;   bytes[3] = e3;
    bytes[4] = e4;   bytes[5] = e5;   bytes[6] = e6;   bytes[7] = e7;
    bytes[8] = e8;   bytes[9] = e9;   bytes[10] = e10; bytes[11] = e11;
    bytes[12] = e12; bytes[13] = e13; bytes[14] = e14; bytes[15] = e15;
    return result;
}

static inline __m128i _mm_shuffle_epi8(__m128i a, __m128i mask) {
    __m128i result;
    char* res_bytes = (char*)result.val;
    char* a_bytes = (char*)a.val;
    char* mask_bytes = (char*)mask.val;
    
    for (int i = 0; i < 16; i++) {
        uint8_t idx = mask_bytes[i] & 0x0F; // 只使用低4位
        res_bytes[i] = (mask_bytes[i] & 0x80) ? 0 : a_bytes[idx]; // 如果最高位为1，结果为0
    }
    
    return result;
}

static inline __m128i _mm_add_epi8(__m128i a, __m128i b) {
    __m128i result;
    uint8_t* res = (uint8_t*)result.val;
    uint8_t* a_val = (uint8_t*)a.val;
    uint8_t* b_val = (uint8_t*)b.val;
    
    for (int i = 0; i < 16; i++) {
        res[i] = a_val[i] + b_val[i];
    }
    
    return result;
}

static inline __m128i _mm_min_epu8(__m128i a, __m128i b) {
    __m128i result;
    uint8_t* res = (uint8_t*)result.val;
    uint8_t* a_val = (uint8_t*)a.val;
    uint8_t* b_val = (uint8_t*)b.val;
    
    for (int i = 0; i < 16; i++) {
        res[i] = (a_val[i] < b_val[i]) ? a_val[i] : b_val[i];
    }
    
    return result;
}

// 添加缺失的SSE函数声明
static inline __m128i _mm_load_si128(const __m128i*);
static inline void _mm_store_si128(__m128i*, __m128i);

// 添加更多需要的SSE函数...

#ifdef __cplusplus
}
#endif

#endif // CIMBAR_IOS_PLATFORM
#endif // __SSE_COMPAT_H
