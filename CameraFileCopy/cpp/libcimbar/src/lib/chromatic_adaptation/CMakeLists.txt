cmake_minimum_required(VERSION 3.10...3.31)

# 确保模块能找到OpenCV头文件
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    /Users/jim/Projects/CameraFileCopy/Pods/OpenCV/opencv2.framework/Headers
)

set(SOURCES
	adaptation_transform.h
	color_correction.h
)

add_library(chromatic_adaptation INTERFACE)

# 为接口库设置包含目录，这样所有使用这个库的目标也能看到这些目录
target_include_directories(chromatic_adaptation INTERFACE
    ${OpenCV_INCLUDE_DIRS}
    /Users/jim/Projects/CameraFileCopy/Pods/OpenCV/opencv2.framework/Headers
)

if(NOT DEFINED DISABLE_TESTS)
	add_subdirectory(test)
endif()
