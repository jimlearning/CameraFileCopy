#include "correct/convolutional/convolutional.h"
#include "correct/convolutional/sse/lookup.h"
// BIG HEAPING TODO sort out the include mess
#include "correct-sse.h"

// 只有非iOS平台才包含x86内联函数
#ifndef CIMBAR_IOS_PLATFORM
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#endif // CIMBAR_IOS_PLATFORM


struct correct_convolutional_sse {
    correct_convolutional base_conv;
    oct_lookup_t oct_lookup;
};
