# iOS-CMAKE 工具链使用指南

## 概述

`ios-cmake` 工具链提供了使用 CMake 为 iOS、tvOS 和 watchOS 平台构建的功能。本项目已经集成了该工具链以支持 iOS 平台编译。

## 使用方法

### 1. 基本命令格式

在使用 CMake 生成构建文件时，需要通过 `-DCMAKE_TOOLCHAIN_FILE` 参数指定 iOS 工具链文件：

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/<项目路径>/CameraFileCopy/cpp/ios.toolchain.cmake [其他选项]
```

### 2. 平台选项

通过 `-DPLATFORM` 参数指定目标平台：

- `OS64` - 针对 ARM64 架构的 iOS 设备 (iPhone 5s 及更新设备)
- `SIMULATOR64` - 针对 x86_64 架构的 iOS 模拟器
- `SIMULATORARM64` - 针对 Apple Silicon 上的 iOS 模拟器
- `OS64COMBINED` - 包含 ARM64 和 x86_64 的通用二进制文件 (实际设备和模拟器)

例如，构建用于 ARM64 iOS 设备的版本：

```bash
cmake -B build-ios -G Xcode \
  -DCMAKE_TOOLCHAIN_FILE=/<项目路径>/CameraFileCopy/cpp/ios.toolchain.cmake \
  -DPLATFORM=OS64 \
  -DDEPLOYMENT_TARGET=14.0
```

### 3. 其他常用选项

- `-DDEPLOYMENT_TARGET=<版本>` - 指定最低 iOS 系统版本要求（例如：14.0）
- `-DENABLE_BITCODE=OFF` - 控制是否启用 Bitcode
- `-DENABLE_ARC=ON` - 控制是否启用自动引用计数（ARC）
- `-DENABLE_VISIBILITY=OFF` - 控制是否启用符号可见性设置

### 4. 通过脚本构建

为简化构建流程，可以创建专用的构建脚本：

```bash
#!/bin/bash
# 项目根目录
PROJECT_DIR=$(pwd)

# 创建构建目录
mkdir -p build-ios

# 使用iOS工具链配置项目
cmake -B build-ios -G Xcode \
  -DCMAKE_TOOLCHAIN_FILE=$PROJECT_DIR/cpp/ios.toolchain.cmake \
  -DPLATFORM=OS64 \
  -DDEPLOYMENT_TARGET=13.0 \
  -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
  -DENABLE_BITCODE=OFF

# 构建
cmake --build build-ios --config Release
```

## 常见问题

### 编译器检测

确保 CMake 使用了正确的工具链：

```bash
cmake -B build-ios -G Xcode \
  -DCMAKE_TOOLCHAIN_FILE=/<项目路径>/CameraFileCopy/cpp/ios.toolchain.cmake \
  -DPLATFORM=OS64 \
  ..

grep -A 5 "CMAKE_C.*COMPILER" build-ios/CMakeCache.txt
```

输出应该显示 iOS SDK 路径。

### Xcode 配置

如果生成 Xcode 项目，还可以在 CMake 构建配置中添加以下选项：

```
-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=<YOUR_TEAM_ID>
```

### 库链接

确保正确处理 iOS 框架和库的链接：

```cmake
target_link_libraries(your_target
  "-framework Foundation"
  "-framework UIKit"
)
```
