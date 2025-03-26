cmake_minimum_required(VERSION 3.10...3.31)

# 确保模块能找到OpenCV头文件
# 使用项目根目录设置的变量
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    /Users/jim/Projects/CameraFileCopy/Pods/OpenCV/opencv2.framework/Headers
)

set(SOURCES
	Anchor.h
	Corners.h
	Deskewer.cpp
	Deskewer.h
	DistortionParameters.h
	EdgeScanState.h
	Extractor.cpp
	Extractor.h
	Geometry.h
	Midpoints.h
	Point.h
	ScanState.h
	Scanner.cpp
	Scanner.h
	SimpleCameraCalibration.cpp
	SimpleCameraCalibration.h
	Undistort.h
)

add_library(extractor STATIC ${SOURCES})

if(NOT DEFINED DISABLE_TESTS)
	add_subdirectory(test)
endif()
