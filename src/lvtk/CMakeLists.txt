# CMakeList.txt : CMake project for ToobAmp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)

message(STATUS "GLIB_INCLUDE_DIRS: ${GLIB_INCLUDE_DIRS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")


find_package(Cairo REQUIRED)
find_package(X11 REQUIRED)
find_package(Pango REQUIRED)
find_package(ICU COMPONENTS uc i18n)

message(STATUS "ICU_LIBRARIES=${ICU_LIBRARIES}")

find_library(RSVG2_LIB 
        NAMES rsvg-2 librsvg-2.so.2 REQUIRED )

find_path(RSVG2_INCLUDE_DIR
    NAMES librsvg/rsvg.h
    PATH_SUFFIXES librsvg-2.0
    REQUIRED
)



# message(STATUS "RSVG2_LIB=${RSVG2_LIB}")
# message(STATUS "RSVG2_INCLUDE_DIR=${RSVG2_INCLUDE_DIR}")

find_path(GDK_PIXBUF_INCLUDE_DIR
    NAMES gdk-pixbuf/gdk-pixbuf.h
    PATH_SUFFIXES gdk-pixbuf-2.0
    REQUIRED

find_library(GDK_PIXBUF_LIB REQUIRED
    NAMES libgdk_pixbuf_xlib-2.0.so.0 gdk_pixbuf_xlib-2.0 gdk_pixbuf-2.0 gdk_pixbuf-2 REQUIRED )
)

message(STATUS "GDK_PIXBUF_LIB=${GDK_PIXBUF_LIB}")
message(STATUS "GDK_PIXBUF_INCLUDE_DIR=${GDK_PIXBUF_INCLUDE_DIR}")


set(LV2CAIRO_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" )

#print all variables
if (0)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endif()

add_library(lvtk STATIC
    ./include/lvtk/LvtkObject.hpp
    ./include/lvtk/IcuString.hpp
    ./include/lvtk/JsonVariant.hpp
    ./include/lvtk/JsonIo.hpp
    ./include/lvtk/LvtkDialog.hpp
    ./include/lvtk/LvtkStatusTextElement.hpp
    ./include/lvtk/LvtkLampElement.hpp
    ./include/lvtk/LvtkDbVuElement.hpp
    ./include/lvtk/LvtkVuElement.hpp
    ./include/lvtk/LvtkSwitchElement.hpp
    ./include/lvtk/LvtkOnOffSwitchElement.hpp
    ./include/lvtk/LvtkPngStripElement.hpp
    ./include/lvtk/LvtkValueElement.hpp
    ./include/lvtk/LvtkNumericEditBoxElement.hpp
    ./include/lvtk/LvtkDropShadowElement.hpp
    ./include/lvtk/LvtkRootElement.hpp
    ./include/lvtk/LvtkDropdownElement.hpp
    ./include/lvtk/LvtkDropdownItemElement.hpp
    ./include/lvtk/LvtkPangoContext.hpp
    ./include/lvtk/LvtkEditBoxElement.hpp
    ./include/lvtk/LvtkPngDialElement.hpp
    ./include/lvtk/LvtkDialElement.hpp
    ./include/lvtk/LvtkBindingProperty.hpp
    ./include/lvtk/LvtkPngElement.hpp
    ./include/lvtk/LvtkSvgElement.hpp
    ./include/lvtk/LvtkSvg.hpp
    ./include/lvtk/LvtkDrawingContext.hpp
    ./include/lvtk/LvtkFlexGridElement.hpp
    ./include/lvtk/LvtkButtonBaseElement.hpp
    ./include/lvtk/LvtkButtonElement.hpp
    ./include/lvtk/LvtkTheme.hpp
    ./include/lvtk/Lvtk.hpp
    ./include/lvtk/LvtkStyle.hpp
    ./include/lvtk/LvtkElement.hpp
    ./include/lvtk/LvtkVerticalStackElement.hpp
    ./include/lvtk/LvtkHorizontalStackElement.hpp
    ./include/lvtk/LvtkTypes.hpp
    ./include/lvtk/LvtkWindow.hpp
    ./include/lvtk/LvtkTypographyElement.hpp
    ./include/lvtk/LvtkContainerElement.hpp
    ./include/lvtk/LvtkSettingsFile.hpp
    ./include/lvtk/LvtkMessageDialog.hpp
    ./include/lvtk/LvtkScrollBarElement.hpp
    ./include/lvtk/LvtkScrollContainerElement.hpp
    ./include/lvtk/LvtkAnimator.hpp
    ./include/lvtk/LvtkIndefiniteProgressElement.hpp
    ./include/lvtk/LvtkMotionBlurElement.hpp
    ./include/lvtk/LvtkSlideInOutAnimationElement.hpp
    ./include/lvtk/LvtkCieColors.hpp
    ./include/lvtk/LvtkGroupElement.hpp
    ./LvtkGroupElement.cpp
    ./LvtkCieColors.cpp
    ./LvtkSlideInOutAnimationElement.cpp
    ./LvtkMotionBlurElement.cpp
    ./LvtkIndefiniteProgressElement.cpp
    ./LvtkAnimator.cpp
    ./LvtkScrollContainerElement.cpp
    ./LvtkScrollBarElement.cpp
    ./LvtkMessageDialog.cpp
    ./LvtkSettingsFile.cpp
    ./ss.hpp
    ./cleanup.hpp
    ./LvtkDialog.cpp
    ./IcuString.cpp
    ./LvtkStatusTextElement.cpp
    ./LvtkLampElement.cpp
    ./LvtkDbVuElement.cpp
    ./LvtkVuElement.cpp
    ./LvtkDialBaseElement.cpp
    ./LvtkPngStripElement.cpp
    ./LvtkValueElement.cpp
    ./Utf8Utils.cpp ./Utf8Utils.hpp
    ./LvtkNumericEditBoxElement.cpp
    ./LvtkDropShadowElement.cpp
    ./LvtkDropdownElement.cpp
    ./LvtkDropdownItemElement.cpp
    ./keysym_names.cpp ./keysym_names.hpp
    ./LvtkPangoContext.cpp
    ./LvtkEditBoxElement.cpp
    ./LvtkPngDialElement.cpp
    ./LvtkDialElement.cpp
    ./LvtkBindingProperty.cpp
    ./LvtkPngElement.cpp
    ./LvtkVerticalStackElement.cpp
    ./LvtkVerticalStackElement.cpp
    ./LvtkFlexGridElement.cpp
    ./LvtkButtonBaseElement.cpp
    ./LvtkButtonElement.cpp
    ./LvtkStyle.cpp
    ./LvtkTypographyElement.cpp
    ./LvtkVerticalStackElement.cpp
    ./LvtkWindow.cpp
    ./LvtkSvg.cpp
    ./LvtkDrawingContext.cpp
    ./include/lvtk/LvtkDamageList.hpp
    ./LvtkDamageList.cpp
    ./LvtkTypes.cpp
    ./LvtkTheme.cpp
    ./LvtkContainerElement.cpp
    ./LvtkLog.cpp
    ./include/lvtk/LvtkLog.hpp
    ./LvtkX11Window.cpp
    ./LvtkRootElement.cpp
    ./LvtkSvgElement.cpp
    ./JsonVariant.cpp
    ./JsonIo.cpp

    ./LvtkX11Window.hpp
    ./LvtkElement.cpp
    ./LvtkSwitchElement.cpp

)

target_include_directories(
    lvtk PUBLIC
    ${LV2CAIRO_INCLUDE_DIRECTORY}
    ${CAIRO_INCLUDE_DIRS}
    ${PC_PANGOCAIRO_INCLUDE_DIRS}
    ${GOBJECT_INCLUDE_DIR}
    ${X11_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${RSVG2_INCLUDE_DIR}
    ${GDK_PIXBUF_INCLUDE_DIR}
)



target_link_libraries(lvtk PUBLIC 
    ${CAIRO_LIBRARIES} ${X11_LIBRARIES}
    ${PC_PANGOCAIRO_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${RSVG2_LIB}
    ${GDK_PIXBUF_LIB}
    ${ICU_LIBRARIES}
    Xrandr
)
