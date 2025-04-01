#!/bin/bash
# iOS构建脚本 - 使用ios-cmake工具链
set -e

# 项目根目录
PROJECT_DIR=$(pwd)
CMAKE_DIR="${PROJECT_DIR}/cpp"
BUILD_DIR="${PROJECT_DIR}/build-ios"

# 创建构建目录
mkdir -p ${BUILD_DIR}

# 使用iOS工具链配置项目
echo "正在配置使用ios-cmake工具链构建iOS项目..."
cmake -B ${BUILD_DIR} -G Xcode \
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_DIR}/ios.toolchain.cmake \
  -DPLATFORM=OS64 \
  -DDEPLOYMENT_TARGET=13.0 \
  -DENABLE_BITCODE=OFF \
  -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
  ${CMAKE_DIR}

echo "配置完成。现在可以使用以下命令进行构建："
echo "cmake --build ${BUILD_DIR} --config Release"

# 可选：自动构建
if [ "$1" == "--build" ]; then
  echo "开始构建项目..."
  cmake --build ${BUILD_DIR} --config Release
fi
