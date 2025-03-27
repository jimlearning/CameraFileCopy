cmake_minimum_required(VERSION 3.10...3.31)

project ( libcimbar )
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${libcimbar_SOURCE_DIR}/dist" CACHE PATH "..." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)

# 检测iOS平台
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(IOS_PLATFORM TRUE)
    message(STATUS "Building for iOS platform")
else()
    set(IOS_PLATFORM FALSE)
endif()

# 定义项目根目录宏，用于测试代码和样本文件定位
add_definitions(-DLIBCIMBAR_PROJECT_ROOT="${libcimbar_SOURCE_DIR}")
message(STATUS "Setting LIBCIMBAR_PROJECT_ROOT to: ${libcimbar_SOURCE_DIR}")

# 确保我们接收父项目传递的OpenCV_INCLUDE_DIRS变量
if(DEFINED OpenCV_INCLUDE_DIRS)
	message(STATUS "Using OpenCV include dirs from parent: ${OpenCV_INCLUDE_DIRS}")
else()
	message(WARNING "OpenCV_INCLUDE_DIRS not defined by parent project")
endif()

if(NOT DEFINED OPENCV_LIBS)
	set(OPENCV_LIBS "opencv_imgcodecs" "opencv_imgproc" "opencv_photo" "opencv_core")
endif()

if(NOT DEFINED CPPFILESYSTEM)
	set(CPPFILESYSTEM "stdc++fs")
endif()

# 基础模块 - 所有平台共用
set (PROJECTS
	src/lib/bit_file
	src/lib/chromatic_adaptation
	src/lib/cimb_translator
	src/lib/compression
	src/lib/encoder
	src/lib/fountain
	src/lib/image_hash
	src/lib/serialize
	src/lib/util

	src/third_party_lib/base91
	src/third_party_lib/cxxopts
	src/third_party_lib/intx
	src/third_party_lib/libcorrect
	src/third_party_lib/libpopcnt
	src/third_party_lib/wirehair
	src/third_party_lib/zstd
)

# 添加平台相关模块
if(NOT IOS_PLATFORM)
    # 只在非iOS平台上添加这些模块
    set(PROJECTS
        ${PROJECTS}
        src/lib/cimbar_js
        src/lib/gui
    )
endif()

if(NOT DEFINED USE_WASM)
    set(PROJECTS
        ${PROJECTS}
        src/lib/extractor
    )
endif()

include_directories(
	${libcimbar_SOURCE_DIR}/src/lib
	${libcimbar_SOURCE_DIR}/src/third_party_lib
	${OpenCV_INCLUDE_DIRS}
)

foreach(proj ${PROJECTS})
	add_subdirectory(${proj} build/${proj})
endforeach()
