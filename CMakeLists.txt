if(CMAKE_TOOLCHAIN_FILE)
set(LIBRARY_OUTPUT_PATH_ROOT ${CMAKE_BINARY_DIR} CACHE PATH "root for library output, set this to change where android libs are compiled to")
# get absolute path, but get_filename_component ABSOLUTE only refer with source dir, so find_file here :(
get_filename_component(CMAKE_TOOLCHAIN_FILE_NAME ${CMAKE_TOOLCHAIN_FILE} NAME)
find_file(CMAKE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE_NAME} PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)
message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
endif()

cmake_minimum_required(VERSION 2.8)

#for compatible
add_definitions(-DCONFIG_LEGACY_API)

add_definitions(-Wno-unused-command-line-argument)
add_definitions(-Wall)
add_definitions(-fPIC)
add_definitions(-g)
add_definitions(-O3)
add_definitions(-funroll-loops)
add_definitions(-Wno-overloaded-virtual)
add_definitions(-Wno-deprecated-register)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)

#include
include_directories(${Tengine_Dir}/include)
#lib
link_directories(${Tengine_Dir}/lib)
set( TENGINE_LIBS tengine)

#add_subdirectory(Classify               )
#add_subdirectory(YuFaceDetection        )
#add_subdirectory(faster_rcnn            )
#add_subdirectory(lighten_cnn            )
#add_subdirectory(mobileface             )
#add_subdirectory(mssd_tflite            )
#add_subdirectory(mtcnn                  )
#add_subdirectory(ssd                    )
#add_subdirectory(tflite-int8-mobilenet  )
#add_subdirectory(yolov2                 )
add_subdirectory(LFFD_face               )
