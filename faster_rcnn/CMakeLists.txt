
cmake_minimum_required (VERSION 2.8)
project(FASTER_RCNN)
link_directories( ${PROTOBUF_DIR}/lib )
add_definitions(-std=c++11)
set( INSTALL_DIR ${TENGINE_DIR} )
set( TENGINE_LIBS tengine hclcpu)

#include
include_directories(${INSTALL_DIR}/include ) 
#lib
link_directories( ${INSTALL_DIR}/lib )

#exe
set( RESNET_SRCS faster_rcnn.cpp)
add_executable(FASTER_RCNN ${RESNET_SRCS})
target_link_libraries(FASTER_RCNN ${TENGINE_LIBS} ${PROTOBUF_LIB} ${BLAS_LIB} ${ACL_LIB})
