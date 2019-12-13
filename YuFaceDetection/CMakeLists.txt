
cmake_minimum_required (VERSION 2.8)
project(YuFace)

set( INSTALL_DIR ${TENGINE_DIR} )
set( TENGINE_LIBS tengine hclcpu)

set( YuFace_SRCS yu_facedetect.cpp )

#include
include_directories(${INSTALL_DIR}/include)

#lib
link_directories(${INSTALL_DIR}/lib)

#exe
add_executable(YuFace ${YuFace_SRCS})
target_link_libraries(YuFace ${TENGINE_LIBS} ${PROTOBUF_LIB}  ${BLAS_LIB} ${ACL_LIB})

