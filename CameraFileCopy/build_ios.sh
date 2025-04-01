#!/bin/bash
# iOS构建脚本 - 使用ios-cmake工具链
set -e

# 添加常见的路径到PATH环境变量
export PATH="$PATH:/usr/local/bin:/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin"

# 查找cmake的路径
CMAKE_PATH=$(which cmake)
if [ -z "$CMAKE_PATH" ]; then
    # 如果找不到cmake，尝试常见安装位置
    if [ -f "/usr/local/bin/cmake" ]; then
        CMAKE_PATH="/usr/local/bin/cmake"
    elif [ -f "/opt/homebrew/bin/cmake" ]; then
        CMAKE_PATH="/opt/homebrew/bin/cmake"
    else
        echo "Error: Cannot find cmake. Please install it or provide the full path."
        exit 1
    fi
fi

echo "Using cmake at: $CMAKE_PATH"

# 处理目录路径 - 兼容在Xcode中运行和手动运行
if [ -z "$PROJECT_DIR" ]; then
    # 如果不是在Xcode中运行，使用当前目录
    PROJECT_DIR=$(pwd)
    CPP_DIR="$PROJECT_DIR/cpp"
else
    # 在Xcode中运行
    CPP_DIR="$PROJECT_DIR/CameraFileCopy/cpp"
fi

BUILD_DIR="$CPP_DIR/build_ios"
OPENCV_DIR="$PROJECT_DIR/Pods/OpenCV"

echo "Project directory: $PROJECT_DIR"
echo "C++ directory: $CPP_DIR"
echo "Build directory: $BUILD_DIR"
echo "OpenCV directory: $OPENCV_DIR"

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 使用iOS工具链配置项目
echo "正在配置使用ios-cmake工具链构建iOS项目..."
$CMAKE_PATH \
  -DCMAKE_TOOLCHAIN_FILE="$CPP_DIR/ios.toolchain.cmake" \
  -DPLATFORM=OS64 \
  -DDEPLOYMENT_TARGET=13.0 \
  -DENABLE_BITCODE=OFF \
  -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
  -DOpenCV_DIR="$OPENCV_DIR" \
  "$CPP_DIR"

echo "配置完成。"

# 可选：自动构建
if [ "$1" == "--build" ]; then
  echo "开始构建项目..."
  $CMAKE_PATH --build . -- -j8
  echo "构建完成"
fi
