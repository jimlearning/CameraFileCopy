cmake_minimum_required(VERSION 3.10...3.31)

# 确保模块能找到OpenCV头文件
# 直接指定OpenCV的头文件路径
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    /Users/jim/Projects/CameraFileCopy/Pods/OpenCV/opencv2.framework/Headers
)

set(SOURCES
	bitmaps.h
	AdjacentCellFinder.cpp
	AdjacentCellFinder.h
	Cell.h
	CellDrift.cpp
	CellDrift.h
	CellPositions.cpp
	CellPositions.h
	CimbDecoder.cpp
	CimbDecoder.h
	CimbEncoder.cpp
	CimbEncoder.h
	CimbReader.cpp
	CimbReader.h
	CimbWriter.cpp
	CimbWriter.h
	Common.cpp
	Common.h
	Config.cpp
	Config.h
	FloodDecodePositions.cpp
	FloodDecodePositions.h
	Interleave.h
	LinearDecodePositions.h
	PositionData.h
)

add_library(cimb_translator STATIC ${SOURCES})

if(NOT DEFINED DISABLE_TESTS)
	add_subdirectory(test)
endif()
