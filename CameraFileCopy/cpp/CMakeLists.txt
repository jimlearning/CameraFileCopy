cmake_minimum_required(VERSION 3.4.1)

# 统一控制测试编译
if(CIMBAR_IOS_PLATFORM)
    set(BUILD_TESTING OFF CACHE BOOL "Disable testing for iOS" FORCE)
endif()

# 检查是否使用ios-cmake工具链
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
else()
    message(STATUS "No toolchain file specified. Using default compiler.")
endif()

# 如果是iOS平台，设置相关变量
if(IOS OR (CMAKE_SYSTEM_NAME STREQUAL "iOS"))
    set(IOS_PLATFORM TRUE)
    message(STATUS "Building for iOS platform with PLATFORM=${PLATFORM}")
    add_definitions(-DCIMBAR_IOS_PLATFORM)
    
    # 设置iOS相关变量
    if(NOT DEFINED DEPLOYMENT_TARGET)
        set(DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum iOS deployment target" FORCE)
    endif()
    
    if(NOT DEFINED PLATFORM)
        set(PLATFORM "OS64" CACHE STRING "iOS platform target" FORCE)
    endif()
else()
    set(IOS_PLATFORM FALSE)
    message(STATUS "Building for non-iOS platform: ${CMAKE_SYSTEM_NAME}")
endif()

project(cfc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置包含路径
if(IOS_PLATFORM)
    # iOS平台特定的包含路径设置
    set(GLOBAL_INCLUDE_DIRS
        ${libcimbar_SOURCE_DIR}/ios_includes
    )
    
    # 设置Xcode特定属性
    set(CMAKE_XCODE_ATTRIBUTE_HEADER_SEARCH_PATHS "${GLOBAL_INCLUDE_DIRS}")
else()
    # 非iOS平台的包含路径保持不变
    set(GLOBAL_INCLUDE_DIRS
        ${libcimbar_SOURCE_DIR}/src/lib
        ${libcimbar_SOURCE_DIR}/src/third_party_lib
    )
endif()

# 对所有子项目使用相同的包含路径
include_directories(
    ${GLOBAL_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# OpenCV配置 - 强化配置以确保所有子项目都能找到OpenCV
set(OpenCV_FOUND TRUE)
set(OpenCV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../Pods/OpenCV/opencv2.framework/Headers")
set(OpenCV_LIBS "-framework opencv2")

# 添加预处理器定义，将opencv2.framework/Headers作为根目录
add_compile_options(-I${OpenCV_INCLUDE_DIRS})
add_compile_definitions(HAVE_OPENCV)

# 项目文件设置s
set(DISABLE_TESTS true)
set(OPENCV_LIBS ${OpenCV_LIBS})

# 向所有子模块传播OpenCV路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -F${CMAKE_CURRENT_SOURCE_DIR}/../Pods/OpenCV")

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
