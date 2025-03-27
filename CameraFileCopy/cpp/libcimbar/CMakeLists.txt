cmake_minimum_required(VERSION 3.10...3.31)

project ( libcimbar )
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${libcimbar_SOURCE_DIR}/dist" CACHE PATH "..." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)

# 检测iOS平台
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(IOS_PLATFORM TRUE)
    message(STATUS "Building for iOS platform - tests will be disabled")
    # 设置全局属性，以禁用所有测试
    set_property(GLOBAL PROPERTY IOS_DISABLE_TESTS TRUE)
    
    # 添加iOS平台兼容性目录到包含路径
    message(STATUS "Adding iOS-specific compatibility include directory")
    include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/ios_includes)
    
    # 添加iOS平台兼容性源文件
    message(STATUS "Adding iOS platform compatibility implementation files")
    set(IOS_COMPAT_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_includes/fec.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_includes/sse_lookup.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_includes/correct/convolutional/history_buffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_includes/correct/convolutional/error_buffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_includes/correct/convolutional/lookup.c
    )

    # 创建兼容性静态库
    add_library(ios_compat STATIC ${IOS_COMPAT_SRC})
    
    # 确保其他模块会链接到我们的兼容性库
    link_libraries(ios_compat)
    
    # 确保禁用所有SSE相关功能
    message(STATUS "Disabling SSE functionality on iOS platform")
    add_definitions(-DHAVE_SSE=0 -DCIMBAR_IOS_PLATFORM)
    
    # 设置全局编译器标志，显式指示不使用SSE指令集
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCIMBAR_IOS_PLATFORM -DHAVE_SSE=0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCIMBAR_IOS_PLATFORM -DHAVE_SSE=0")
    
    # 创建特定平台的源文件排除列表，避免编译 libcorrect 工具目录下的文件
    message(STATUS "Excluding libcorrect tools and SSE source files from compilation on iOS platform")
    file(GLOB SSE_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party_lib/libcorrect/src/convolutional/sse/*.c")
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party_lib/libcorrect/tools/find_conv_libfec_poly.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party_lib/libcorrect/tools/find_conv_optim_poly.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party_lib/libcorrect/tools/find_conv_optim_poly_annealing.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party_lib/libcorrect/tools/find_rs_primitive_poly.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party_lib/libcorrect/src/convolutional/history_buffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party_lib/libcorrect/src/convolutional/error_buffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party_lib/libcorrect/src/convolutional/lookup.c
        ${SSE_SOURCE_FILES}
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
    
    # 定义平台特定的编译宏
    add_definitions(-DCIMBAR_IOS_PLATFORM)
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

# 自定义函数，用于有条件地添加子目录
function(add_subdirectory_with_options dir binary_dir)
    # 检查目录名是否包含 "test" 或 "tests"
    string(REGEX MATCH "test[s]?$" is_test_dir "${dir}")
    
    # 获取全局属性
    get_property(disable_tests GLOBAL PROPERTY IOS_DISABLE_TESTS)
    
    # 如果是测试目录且在iOS平台上，则跳过
    if(is_test_dir AND disable_tests)
        message(STATUS "Skipping test directory on iOS platform: ${dir}")
    else()
        add_subdirectory(${dir} ${binary_dir})
    endif()
endfunction()

# 使用自定义函数添加子目录
foreach(proj ${PROJECTS})
    # 检查项目本身是否包含 "test" 或 "tests"
    string(REGEX MATCH "test[s]?$" is_test_proj "${proj}")
    get_property(disable_tests GLOBAL PROPERTY IOS_DISABLE_TESTS)
    
    if(is_test_proj AND disable_tests)
        message(STATUS "Skipping test project on iOS platform: ${proj}")
    else()
        # 正常添加非测试项目
        add_subdirectory(${proj} build/${proj})
    endif()
endforeach()
