#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// 检测平台
#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        // iOS平台: 不包含fec.h
        #define CIMBAR_IOS_PLATFORM
    #else
        // macOS平台: 可能有fec.h
        #include <fec.h>
    #endif
#else
    // 其他平台: 包含fec.h
    #include <fec.h>
    #include <fec.h>
    #include <fec.h>
#endif

#ifdef CIMBAR_IOS_PLATFORM
// iOS平台上的替代声明和实现
void rs_fec_encode(void *encoder, uint8_t *msg, size_t msg_length,
                   uint8_t *msg_out)
{
    // iOS上的空实现或替代实现
}

void rs_fec_decode(void *decoder, uint8_t *encoded, size_t encoded_length,
                   uint8_t *erasure_locations, size_t erasure_length,
                   uint8_t *msg, size_t pad_length, size_t num_roots)
{
    // iOS上的空实现或替代实现
}

// 添加init_rs_char和free_rs_char函数的iOS实现
void *init_rs_char(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad)
{
    // 返回一个非空指针以避免空指针错误
    static int dummy;
    return &dummy;
}

void free_rs_char(void *rs)
{
    // 空实现
}
#else
// 其他平台上的原始声明
void rs_fec_encode(void *encoder, uint8_t *msg, size_t msg_length,
                   uint8_t *msg_out);
void rs_fec_decode(void *decoder, uint8_t *encoded, size_t encoded_length,
                   uint8_t *erasure_locations, size_t erasure_length,
                   uint8_t *msg, size_t pad_length, size_t num_roots);

// 确保这些函数在非iOS平台上也被声明
void *init_rs_char(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad);
void free_rs_char(void *rs);
#endif
