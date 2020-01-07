
cmake_minimum_required (VERSION 2.8)
project(tflite-int8-mobilenet)

set( INSTALL_DIR ${TENGINE_DIR})
set( TENGINE_LIBS tengine hclcpu)

#include
include_directories(${INSTALL_DIR}/include )

#lib
link_directories(${INSTALL_DIR}/lib)

#exe
set( CODE_SRCS mobilenet_tflite.cpp )
add_executable(tflite-int8-mobilenet ${CODE_SRCS})

target_link_libraries(tflite-int8-mobilenet ${TENGINE_LIBS} ${PROTOBUF_LIB}  ${BLAS_LIB} ${ACL_LIB})



