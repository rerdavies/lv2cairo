# CMakeList.txt : CMake project for ToobAmp, include source and define
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


add_library(lvtk_ui STATIC
    include/lvtk_ui/Lv2ControlConstants.hpp
    include/lvtk_ui/Lv2PortViewFactory.hpp
    include/lvtk_ui/Lv2PluginType.hpp
    include/lvtk_ui/Lv2PluginInfo.hpp
    include/lvtk_ui/Lv2Units.hpp
    include/lvtk_ui/Lv2UI.hpp
    include/lvtk_ui/Lv2PortView.hpp
    include/lvtk_ui/Lv2Exception.hpp
    include/lvtk_ui/Lv2PortViewController.hpp
    include/lvtk_ui/Lv2UI_NativeCallbacks.hpp
    include/lvtk_ui/PiPedalUI.hpp
    include/lvtk_ui/MimeTypes.hpp
    include/lvtk_ui/Lv2FileDialog.hpp
    include/lvtk_ui/GlobMatcher.hpp
    Lv2FileDialog.cpp
    GlobMatcher.cpp
    MimeTypes.cpp
    PiPedalUI.cpp
    Lv2PluginInfo.cpp
    Lv2PortViewFactory.cpp
    Lv2PluginType.cpp
    Lv2PortViewController.cpp
    Lv2PortView.cpp
    Lv2UI.cpp
    Lv2UI_glue.cpp
    Lv2Units.cpp
    ss.hpp
)


set(LV2CAIRO_UI_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" )

target_include_directories(lvtk_ui PUBLIC 
    ${LV2CAIRO_UI_INCLUDE_DIRECTORY}
    ${LILV_0_INCLUDE_DIRS}

)

# target_include_directories(LvtkTest PRIVATE
#     ${Lvtk_INCLUDE_DIRS}
# )

target_link_libraries(lvtk_ui 
    lvtk 
    ${LILV_0_LIBRARIES}

)

