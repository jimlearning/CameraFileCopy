// ios_includes/sse_compat.h
#ifndef SSE_COMPAT_H
#define SSE_COMPAT_H

// 添加iOS平台检测
#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE && !defined(CIMBAR_IOS_PLATFORM)
#define CIMBAR_IOS_PLATFORM 1
#endif
#endif

// 仅在非iOS平台启用SSE
#ifndef CIMBAR_IOS_PLATFORM
#include <emmintrin.h>
#include <tmmintrin.h>
#include <correct/convolutional/sse/lookup.h>
#else

// Add header guards to prevent multiple inclusions
#pragma once
#include <stdint.h>
#include <arm_neon.h>  // For ARM NEON types

typedef uint8x16_t __m128i;

#ifdef CIMBAR_IOS_PLATFORM
typedef struct {
    unsigned int rate;
    unsigned int order;
    uint8x16_t *table; // 使用NEON类型
} oct_lookup_t;
#else
#include <correct/convolutional/sse/lookup.h>
#endif

void oct_lookup_fill_distance(oct_lookup_t lookup, int32_t *distances);

static inline __m128i _mm_load_si128(const __m128i *p) {
    return vld1q_u8((const uint8_t *)p);
}

static inline void _mm_store_si128(__m128i *p, __m128i a) {
    vst1q_u8((uint8_t *)p, a);
}

static inline __m128i _mm_set_epi32(int i3, int i2, int i1, int i0) {
    uint32_t data[] = {i0, i1, i2, i3};
    return vld1q_u32(data);
}

static inline __m128i _mm_loadl_epi64(const __m128i *p) {
    return vreinterpretq_u8_u64(vld1q_dup_u64((const uint64_t*)p));
}

static inline __m128i _mm_add_epi16(__m128i a, __m128i b) {
    return vreinterpretq_u8_s16(vaddq_s16(
        vreinterpretq_s16_u8(a),
        vreinterpretq_s16_u8(b)
    ));
}

static inline __m128i _mm_shuffle_epi8(__m128i a, __m128i mask) {
    return vqtbl1q_u8(a, vandq_u8(mask, vdupq_n_u8(0x0F)));
}

static inline __m128i _mm_slli_epi64(__m128i a, int imm) {
    return vreinterpretq_u8_u64(vshlq_u64(vreinterpretq_u64_u8(a), vdupq_n_s64(imm)));
}

#endif

// sse_compat.h
#ifdef __SSE2__
#include <emmintrin.h>
#else
// ARM NEON兼容层
#ifdef __ARM_NEON
#include <arm_neon.h>
#define __m128i uint8x16_t
#else
// iOS通用兼容实现
typedef struct {
    uint64_t u64[2];
} __m128i;
#endif

// 空定义SSE内在函数
#define _mm_shuffle_epi8(a,b) (a)
#define _mm_slli_epi64(a, imm) (a)
// 添加其他必要函数的空定义...
#endif

#ifdef __ARM_NEON__
#include <arm_neon.h>

// Add NEON implementations for SSE intrinsics
#define _mm_set1_epi8(x) vdupq_n_s8(x)
#define _mm_loadu_si128(p) vld1q_s8((const int8_t*)p)
#define _mm_storeu_si128(p, v) vst1q_s8((int8_t*)p, v)

// Implement missing vtbl4q_u8 using NEON
static inline uint8x16_t vtbl4q_u8(uint8x16x4_t a, uint8x16_t b) {
    return vqtbl4q_u8(a, b);
}

// Type compatibility
typedef uint8x16_t __m128i;
#else
#include <emmintrin.h>
#endif

#endif