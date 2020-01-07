
cmake_minimum_required (VERSION 2.8)
project(yolov2)

set( INSTALL_DIR ${TENGINE_DIR})
set( TENGINE_LIBS tengine hclcpu)

#include
include_directories(${INSTALL_DIR}/include )

#lib
link_directories(${INSTALL_DIR}/lib)

#exe
set( CODE_SRCS yolov2.cpp )
add_executable(yolov2 ${CODE_SRCS})

target_link_libraries(yolov2 ${TENGINE_LIBS} ${PROTOBUF_LIB}  ${BLAS_LIB} ${ACL_LIB})



