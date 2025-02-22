# CMakeList.txt : CMake project for ToobAmp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

find_package(PkgConfig REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")


set(LV2_PLUGIN_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" )


add_library(lv2_plugin OBJECT
    ./include/lv2_plugin/Lv2Plugin.hpp
    ./include/lv2_plugin/Lv2Ports.hpp
    ./Lv2Plugin.cpp
)

target_include_directories(
    lv2_plugin PUBLIC 
    ${LV2_PLUGIN_INCLUDE_DIRECTORY}
)

