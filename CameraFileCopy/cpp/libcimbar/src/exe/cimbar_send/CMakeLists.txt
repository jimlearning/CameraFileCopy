cmake_minimum_required(VERSION 3.10)

project(cimbar_send)

# 添加额外的包含路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party_lib
)

set (SOURCES
	send.cpp
)

add_executable (
	cimbar_send
	${SOURCES}
)

target_link_libraries(cimbar_send
	cimbar_js
)

add_custom_command(
	TARGET cimbar_send POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cimbar_send> cimbar_send.dbg
	COMMAND ${CMAKE_STRIP} -g $<TARGET_FILE:cimbar_send>
)

install(
	TARGETS cimbar_send
	DESTINATION bin
)
