#!/bin/bash

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

CPP_DIR="$PROJECT_DIR/CameraFileCopy/cpp"
BUILD_DIR="$CPP_DIR/build_ios"
OPENCV_DIR="$PROJECT_DIR/Pods/OpenCV"

echo "OpenCV directory: $OPENCV_DIR"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 为iOS构建C++库
$CMAKE_PATH -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake \
      -DPLATFORM=OS64 \
      -DENABLE_BITCODE=OFF \
      -DCMAKE_BUILD_TYPE=Release \
      -DOpenCV_DIR="$OPENCV_DIR" \
      "$CPP_DIR"
      
$CMAKE_PATH --build . -- -j8