cmake_minimum_required (VERSION 2.8)
#exe
project(LFFD_facedetect)

set( CODE_SRCS LFFD_facedetect.cpp)
#set( CODE_SRCS LFFD_facedetect.cpp PARENT_SCOPE)

add_executable(LFFD_facedetect ${CODE_SRCS})

target_link_libraries(LFFD_facedetect ${TENGINE_LIBS})
