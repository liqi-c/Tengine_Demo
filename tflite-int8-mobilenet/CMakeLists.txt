
cmake_minimum_required (VERSION 2.8)
project(Mobilenet_TFLITE)

set( INSTALL_DIR ${TENGINE_DIR})
set( TENGINE_LIBS tengine hclcpu)

#include
include_directories(${INSTALL_DIR}/include )

#lib
link_directories(${INSTALL_DIR}/lib)

#exe
set( CODE_SRCS mobilenet_tflite.cpp )
add_executable(Mobilenet_TFLITE ${CODE_SRCS})

target_link_libraries(Mobilenet_TFLITE ${TENGINE_LIBS} ${PROTOBUF_LIB}  ${BLAS_LIB} ${ACL_LIB})



