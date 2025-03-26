cmake_minimum_required(VERSION 3.4.1)

add_library(cfc-cpp
        SHARED

        jni.cpp
        MultiThreadedDecoder.h
)

# 检查平台，log库只在Android上可用
if(ANDROID)
    find_library(log-lib log)
else()
    # 在iOS上，我们不需要log库，设置为空字符串
    set(log-lib "")
endif()

target_link_libraries(cfc-cpp

        cimb_translator
	extractor

        correct_static
        wirehair
        zstd
        ${OPENCV_LIBS}
        ${log-lib}
)
