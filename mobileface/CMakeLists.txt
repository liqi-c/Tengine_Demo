
cmake_minimum_required (VERSION 2.8)
project(MobileFace)

set( INSTALL_DIR ${TENGINE_DIR}/)
set( TENGINE_LIBS tengine hclcpu)

#include
include_directories(${INSTALL_DIR}/include)

#lib
link_directories(${INSTALL_DIR}/lib)
#exe
set( CODE_SRCS mobilefacenet.cpp )
add_executable(MobileFace ${CODE_SRCS})

target_link_libraries(MobileFace ${TENGINE_LIBS} ${PROTOBUF_LIB}  ${BLAS_LIB} ${ACL_LIB})
