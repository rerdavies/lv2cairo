# CMakeList.txt : CMake project for ToobAmp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

include(FindPkgConfig)

pkg_check_modules(LILV_0 "lilv-0" REQUIRED)
if(!LILV_0_FOUND)
    message(ERROR "liblilv-dev package not installed.")
else()
    message(STATUS "LILV_0_LIBRARIES: ${LILV_0_LIBRARIES}")
    message(STATUS "LILV_0_INCLUDE_DIRS: ${LILV_0_INCLUDE_DIRS}")
endif()

set (LV2C_SRC_DIR )

add_executable(generate_lv2c_plugin_info  
    LilvPiPedalUI.cpp
    LilvPiPedalUI.hpp
    AutoLilvNode.cpp
    AutoLilvNode.hpp
    CMakeLists.txt
    CommandLineParser.hpp
    LilvPluginInfo.cpp
    LilvPluginInfo.hpp
    ClassFileWriter.cpp
    ClassFileWriter.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../lv2c_ui/Lv2Units.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../lv2c_ui/Lv2PluginType.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../lv2c_ui/Lv2PluginInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../lv2c_ui/PiPedalUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../lv2c_ui/MimeTypes.cpp
    main.cpp
    ss.hpp
)
target_include_directories(generate_lv2c_plugin_info PRIVATE 
    ${LILV_0_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lv2c/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lv2c_ui/include
    ${LV2C_UI_INCLUDE_DIRS}

)
target_link_libraries(generate_lv2c_plugin_info PRIVATE 

    ${LILV_0_LIBRARIES}
)

