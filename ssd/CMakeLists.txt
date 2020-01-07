
cmake_minimum_required (VERSION 2.8)
project(ssd)

set( INSTALL_DIR ${TENGINE_DIR})
set( TENGINE_LIBS tengine hclcpu)

#include
include_directories(${INSTALL_DIR}/include )

#lib
link_directories(${INSTALL_DIR}/lib)

#exe
set( CODE_SRCS ssd.cpp )
add_executable(ssd ${CODE_SRCS})
target_link_libraries(ssd ${TENGINE_LIBS} ${PROTOBUF_LIB}  ${BLAS_LIB} ${ACL_LIB})



