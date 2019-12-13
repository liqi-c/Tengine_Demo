
cmake_minimum_required (VERSION 2.8)
project(LIGHTEN_CNN)

set( INSTALL_DIR ${TENGINE_DIR}/)
set( TENGINE_LIBS tengine hclcpu)

#include
include_directories(${INSTALL_DIR}/include)

#lib
link_directories(${INSTALL_DIR}/lib)
#exe
set( CODE_SRCS lighten_cnn.cpp )
add_executable(LIGHTEN_CNN ${CODE_SRCS})

target_link_libraries(LIGHTEN_CNN ${TENGINE_LIBS} ${PROTOBUF_LIB}  ${BLAS_LIB} ${ACL_LIB})
