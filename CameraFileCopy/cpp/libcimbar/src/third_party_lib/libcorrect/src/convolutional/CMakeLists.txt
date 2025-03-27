set(SRCFILES bit.c metric.c history_buffer.c error_buffer.c lookup.c convolutional.c encode.c decode.c)
add_library(correct-convolutional OBJECT ${SRCFILES})

# 更明确地检查条件，确保在iOS平台上不构建SSE相关代码
if(HAVE_SSE AND NOT CIMBAR_IOS_PLATFORM AND NOT IOS_PLATFORM)
    message(STATUS "Building SSE optimized code")
    add_subdirectory(sse)
else()
    message(STATUS "Skipping SSE code - not supported on this platform")
endif()
