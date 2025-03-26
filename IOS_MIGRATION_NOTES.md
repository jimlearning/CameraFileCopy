# libcimbar iOS 移植问题总结

## 项目概述

- 项目名称: CameraFileCopy
- 目标: 将 libcimbar 库从 Linux/Android 平台成功移植到 iOS 平台
- 主要挑战: 解决平台特定代码依赖，确保跨平台兼容性

## 已解决的问题

### 1. CMake 配置与平台检测

#### 问题

- 原始 CMake 配置无法正确识别 iOS 平台，导致编译包含不兼容代码的模块
- 多处 CMake 文件需要调整以适应 iOS 构建系统
- 子项目依赖关系在 iOS 平台上需要重新配置
- 可执行文件的构建规则在 iOS 上需要特殊处理
- 某些库和功能（如 GLFW）在 iOS 上不可用，需要条件编译

#### 解决方案

- 在主 CMakeLists.txt 中添加 iOS 平台检测逻辑，以检测 iOS 平台

```cmake
# 检测 iOS 平台
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(IOS_PLATFORM TRUE)
    message(STATUS "Building for iOS platform")
else()
    set(IOS_PLATFORM FALSE)
    message(STATUS "Building for non-iOS platform")
endif() 
```

- 使用条件编译排除不兼容的功能模块

```cmake
# 添加平台相关模块
if(NOT IOS_PLATFORM)
    # 只在非iOS平台上添加这些模块
    set(PROJECTS
        ${PROJECTS}
        src/exe/cimbar_send
        src/exe/cimbar_scan
    )
endif()
```

- 对于子项目，为项目特定的 CMakeLists.txt 添加额外的 include_directories 指令，以确保 iOS 平台的头文件路径可用

```cmake
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party_lib
)
```

- 在主 CMakeLists.txt 中调整链接选项和编译标志，使其与iOS兼容

```cmake
if(IOS_PLATFORM)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fembed-bitcode")
endif()
```

- 对于可执行文件，为 iOS 平台提供特定的构建和安装规则

```cmake
if(NOT IOS_PLATFORM)
    # 仅在非 iOS 平台上执行这些安装命令
    install(
        TARGETS my_executable
        DESTINATION bin
    )
endif()
```

### 2. OpenCV集成问题

#### 问题

- Linux/Android 版本使用 CMake 的 find_package 寻找 OpenCV
- iOS 平台使用 CocoaPods 管理 OpenCV 依赖（pod 'OpenCV', '~> 4.3.0'）
- OpenCV 包含路径和库路径在 iOS 上与其他平台不同
- 需要确保 C++ 代码能正确找到 iOS 上的 OpenCV 框架

#### 解决方案

- CMake 配置需要适应 CocoaPods 的框架结构
- 在 Podfile 中指定 OpenCV 依赖，并确保 Xcode 项目正确集成
- 修改主 CMakeLists.txt，为 iOS 平台提供 OpenCV 路径

```cmake
if(IOS_PLATFORM)
    # iOS 平台上使用 CocoaPods 提供的 OpenCV
    set(OpenCV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../../Pods/OpenCV/opencv2.framework/Headers")
    set(OpenCV_LIBS "-framework OpenCV")
else()
    # 其他平台使用常规方法
    find_package(OpenCV REQUIRED)
endif()
```

- 确保OpenCV包含路径在所有相关CMake文件中可用

```cmake
include_directories(
    ${libcimbar_SOURCE_DIR}/src/lib
    ${libcimbar_SOURCE_DIR}/src/third_party_lib
    ${OpenCV_INCLUDE_DIRS}
)
```

- 确保 OpenCV 库路径在所有相关 CMake 文件中可用

```cmake
link_directories(
    ${OpenCV_LIBS}
)
```

- 在 cimbar_js 模块中使用 OpenCV 替代 GLFW 功能，特别是在 iOS 平台上

```cpp
#ifdef CIMBAR_IOS_PLATFORM
    // 使用 OpenCV 的 Mat 对象处理图像
    cv::Mat _current;
    // 其他iOS特定实现
#else
    // 其他平台使用GLFW
    std::shared_ptr<cimbar::window_glfw> _window;
#endif
```

- 这些修改确保了 libcimbar 能够在 iOS 平台上成功集成 OpenCV，并保持与其他平台的 API 兼容性。

### 3. 头文件包含路径问题

#### 问题

- 多个文件报告找不到头文件，包括:
  - include/wirehair/wirehair.h
  - correct.h
  - wirehair/wirehair.h
  - zstd/zstd.h
  - cimbar_js/cimbar_js.h
  - cimb_translator/Config.h

#### 解决方案

- 修改包含语句，从尖括号改为引号，以确保编译器能找到所需文件。

- 对于wirehair/wirehair.h，修改了包含方式:

```cpp
// 修改前
#include <wirehair/wirehair.h>
// 修改后
#include "include/wirehair/wirehair.h"
```

- 对于fec_shim.h，修改了包含方式:

```cpp
// 修改前
#include <correct.h>
// 修改后
#include "correct.h"
```

- 对于zstd_dstream.h，使用相对路径:

```cpp
// 修改前
#include "zstd/zstd.h"
// 修改后
#include "../../third_party_lib/zstd/zstd.h"
```

- 对于send.cpp，同样使用相对路径:

```cpp
// 修改前
#include "cimbar_js/cimbar_js.h"
// 修改后
#include "../../lib/cimbar_js/cimbar_js.h"
```

- 在cimbar_send的CMakeLists.txt中添加额外的包含路径:

```cmake
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party_lib
)
```

### 4. 平台特定实现替换

#### 问题

- cimbar_js 模块依赖 GLFW 库，这在 iOS 上不可用，导致编译错误

#### 解决方案

- 移除 GLFW 依赖，使用 iOS 平台特定的图形 API 替代
- 添加平台检测宏

```cpp
// 检测平台
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define CIMBAR_IOS_PLATFORM
#endif
#endif
```

- 使用条件编译提供 iOS 特定实现

```cpp
#ifdef CIMBAR_IOS_PLATFORM
    // iOS 平台特定实现
    cv::Mat _current;
#else
    // 其他平台使用 GLFW 实现
    std::shared_ptr<cimbar::window_glfw> _window;
#endif
```

### 5. 进行中的工作

1. 完善 iOS 图形 API 替代实现

- 状态: 进行中
- 计划:
  - 完成 cimbar_js 模块的 iOS 特定实现
  - 使用 Metal 或 OpenGL ES 替代 GLFW 功能
  - 创建 Objective-C++ 桥接层连接 C++ 代码和 Swift/Objective-C 代码

2. 解决剩余的头文件路径问题

- 状态: 进行中
- 计划:
  - 确定是继续使用相对路径方法还是优化为绝对路径
  - CMake 配置
  - 测试现有修改在 iOS 平台的兼容性

3. 测试核心功能

- 状态: 未开始
- 计划:
  - 完成所有必要修改后，在 iOS 设备上测试核心功能
  - 确保验证编码/解码功能是否正常工作
  - 检查性能和内存使用情况

### 6. 移植策略总结

我们采取的核心策略是**"保留所有功能，提供平台特定实现"**，而不是简单地排除不兼容的模块。这包括:

- 条件编译 - 使用平台检测宏和条件语句区分不同平台的代码路径
- 最小干扰 - 尽量保持原有代码结构和API接口不变
- 平台替代 - 为 iOS 平台提供特定的实现，替代不可用的组件
- 路径修正 - 解决头文件包含路径问题，确保编译器能找到所需文件

这种方法确保了代码的可维护性和跨平台兼容性，同时保留了 libcimbar 的完整功能集。

### 7. 后续更新计划

本文档将随着项目进展定期更新，记录新发现的问题和解决方案。
