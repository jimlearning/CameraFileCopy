# 在iOS平台上完全跳过SSE文件的编译
if(NOT DEFINED CIMBAR_IOS_PLATFORM AND NOT DEFINED IOS_PLATFORM)
    set(SRCFILES lookup.c convolutional.c encode.c decode.c)
    add_library(correct-convolutional-sse OBJECT ${SRCFILES})
    message(STATUS "Building SSE optimized components")
else()
    # 在iOS平台上创建一个空库，确保构建系统的一致性
    message(STATUS "Creating empty SSE library for iOS platform")
    add_library(correct-convolutional-sse OBJECT)
    set_target_properties(correct-convolutional-sse PROPERTIES SOURCES "")
endif()
