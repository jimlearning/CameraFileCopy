cmake_minimum_required(VERSION 2.8)
project(Correct C)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
include(CMakePushCheckState)
include(CheckCCompilerFlag)

# 检测iOS平台
set(CIMBAR_IOS_PLATFORM FALSE)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR DEFINED IOS_PLATFORM)
    set(CIMBAR_IOS_PLATFORM TRUE)
    message(STATUS "Building for iOS platform - tests will be disabled")
endif()

find_library(FEC fec)
CHECK_LIBRARY_EXISTS(FEC dotprod "" HAVE_LIBFEC)

# 检测iOS平台
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(IOS_PLATFORM TRUE)
  message(STATUS "Building for iOS platform - SSE disabled")
  set(HAVE_SSE FALSE)
else()
  set(IOS_PLATFORM FALSE)
  if(NOT CMAKE_CROSSCOMPILING)
    # Check if host machine can compile with SSE 4.1 intrinsic
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_DEFINITIONS -march=native)
    check_c_source_compiles("
      #include <x86intrin.h>
      int main() {
        __m128i a;
        __m128i b;
        __m128i c = _mm_min_epu16(a, b);
        return 0;
      }" HAVE_SSE)
    cmake_pop_check_state()
  endif()

  if(HAVE_SSE)
    message(STATUS "SSE 4.1 support enabled")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1")
  endif()
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(src)

# 仅在非iOS平台上构建测试和工具
if(NOT IOS_PLATFORM)
    add_subdirectory(tests)
    add_subdirectory(tools)
else()
    message(STATUS "Skipping libcorrect tests and tools on iOS platform")
endif()

set(INSTALL_HEADERS "${PROJECT_BINARY_DIR}/include/correct.h")

add_custom_target(correct-h ALL COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/include/correct.h ${PROJECT_BINARY_DIR}/include/correct.h)

if(HAVE_SSE)
  set(correct_obj_files $<TARGET_OBJECTS:correct-reed-solomon> $<TARGET_OBJECTS:correct-convolutional> $<TARGET_OBJECTS:correct-convolutional-sse>)
  set(INSTALL_HEADERS ${INSTALL_HEADERS} ${PROJECT_BINARY_DIR}/include/correct-sse.h)
  add_custom_target(correct-sse-h ALL COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/include/correct-sse.h ${PROJECT_BINARY_DIR}/include/correct-sse.h)
else()
  set(correct_obj_files $<TARGET_OBJECTS:correct-reed-solomon> $<TARGET_OBJECTS:correct-convolutional>)
endif()
add_library(correct_static ${correct_obj_files})
set_target_properties(correct_static PROPERTIES OUTPUT_NAME "correct")
if(HAVE_SSE)
  target_compile_definitions(correct_static PUBLIC HAVE_SSE=1)
endif()

#add_subdirectory(util)
#add_subdirectory(tests)
#add_subdirectory(tools)
# add_subdirectory(benchmarks)


