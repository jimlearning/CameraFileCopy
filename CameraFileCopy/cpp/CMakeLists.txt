cmake_minimum_required(VERSION 3.4.1)

project(cfc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 检测iOS平台
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(IOS_PLATFORM TRUE)
    message(STATUS "Building for iOS platform")
else()
    set(IOS_PLATFORM FALSE)
endif()

# OpenCV配置 - 强化配置以确保所有子项目都能找到OpenCV
set(OpenCV_FOUND TRUE)
set(OpenCV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../../Pods/OpenCV/opencv2.framework/Headers")
set(OpenCV_LIBS "-framework opencv2")

# 添加预处理器定义，将opencv2.framework/Headers作为根目录
add_compile_options(-I${OpenCV_INCLUDE_DIRS})
add_compile_definitions(HAVE_OPENCV)

# 项目文件设置
set(DISABLE_TESTS true)
set(OPENCV_LIBS ${OpenCV_LIBS})

# 向所有子模块传播OpenCV路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -F${CMAKE_CURRENT_SOURCE_DIR}/../../../Pods/OpenCV")

# 导入子项目
set(PROJECTS
   concurrent
   libcimbar
)

# 只在非iOS平台上添加cfc-cpp (因为它依赖JNI，这是Android特有的)
if(NOT IOS_PLATFORM)
    list(APPEND PROJECTS cfc-cpp)
endif()

include_directories(
    .
    libcimbar/src/lib
    libcimbar/src/third_party_lib
    ${OpenCV_INCLUDE_DIRS}
)

foreach(proj ${PROJECTS})
    add_subdirectory(${proj} build/${proj})
endforeach()
