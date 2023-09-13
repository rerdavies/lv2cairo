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


add_executable(generate_lvtk_plugin_info  
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
    main.cpp
    ss.hpp
)
target_include_directories(generate_lvtk_plugin_info PRIVATE 
    ${LILV_0_INCLUDE_DIRS}
)
target_link_libraries(generate_lvtk_plugin_info PRIVATE 

    ${LILV_0_LIBRARIES}
    lvtk_ui
)

