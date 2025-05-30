﻿# CMakeList.txt : CMake project for ToobAmp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

find_package(Cairo)
find_package(X11)


include(FindPkgConfig)

pkg_check_modules(LILV_0 "lilv-0" REQUIRED)
if(!LILV_0_FOUND)
    message(ERROR "liblilv-dev package not installed.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")


add_library(lv2c_ui OBJECT
    include/lv2c_ui/Lv2TextOutputElement.hpp
    include/lv2c_ui/Lv2MomentaryButtonElement.hpp
    include/lv2c_ui/Lv2ControlConstants.hpp
    include/lv2c_ui/Lv2PortViewFactory.hpp
    include/lv2c_ui/Lv2PluginType.hpp
    include/lv2c_ui/Lv2PluginInfo.hpp
    include/lv2c_ui/Lv2Units.hpp
    include/lv2c_ui/Lv2UI.hpp
    include/lv2c_ui/Lv2PortView.hpp
    include/lv2c_ui/Lv2Exception.hpp
    include/lv2c_ui/Lv2PortViewController.hpp
    include/lv2c_ui/Lv2UI_NativeCallbacks.hpp
    include/lv2c_ui/PiPedalUI.hpp
    include/lv2c_ui/MimeTypes.hpp
    include/lv2c_ui/Lv2FileDialog.hpp
    include/lv2c_ui/GlobMatcher.hpp
    include/lv2c_ui/Lv2FrequencyPlotElement.hpp
    include/lv2c_ui/Lv2TunerElement.hpp
    include/lv2c_ui/Lv2FileElement.hpp
    Lv2TextOutputElement.cpp
    UriHelper.cpp UriHelper.hpp
    Uri.cpp Uri.hpp
    Lv2FileElement.cpp
    Lv2TunerElement.cpp
    Lv2FrequencyPlotElement.cpp
    Lv2FileDialog.cpp
GlobMatcher.cpp
    MimeTypes.cpp
    Lv2PluginInfo.cpp
    Lv2PortViewFactory.cpp
    Lv2PluginType.cpp
    Lv2PortViewController.cpp
    Lv2PortView.cpp
    Lv2UI.cpp
    Lv2UI_glue.cpp
    Lv2Units.cpp
    PiPedalUI.cpp

    ss.hpp
)


message(STATUS "lv2c_ui CMAKE_DEB_HOST_ARCH: ${CMAKE_DEB_HOST_ARCH}")
message(STATUS "lv2c_ui CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "lv2c_ui CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")



set(LV2CAIRO_UI_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" )

target_include_directories(lv2c_ui PUBLIC 
    ${LV2CAIRO_UI_INCLUDE_DIRECTORY}
    ${LILV_0_INCLUDE_DIRS}

)

# target_include_directories(Lv2cTest PRIVATE
#     ${Lv2c_INCLUDE_DIRS}
# )

target_link_libraries(lv2c_ui 
    lv2c 
    ${LILV_0_LIBRARIES}

)

