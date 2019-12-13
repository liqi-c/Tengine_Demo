cmake_minimum_required (VERSION 2.8)
project(mssd_tflite)

set( INSTALL_DIR ${TENGINE_DIR} )
set( TENGINE_LIBS  tengine hclcpu)

#include
include_directories(${INSTALL_DIR}/include)

#lib
link_directories(${INSTALL_DIR}/lib)

set(CODE_SRCS detect_tflite_mssd.cpp)
set(BIN_EXE mssd_tflite)

add_executable(${BIN_EXE} ${CODE_SRCS})
target_link_libraries(${BIN_EXE} ${TENGINE_LIBS})


