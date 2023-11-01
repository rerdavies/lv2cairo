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
#find_package(ICU COMPONENTS uc i18n)
message(STATUS "ICU_LIBRARIES=${ICU_LIBRARIES}")

# incorrect library versions.
#set(ICU_LIBRARIES libicuuc.a libicui18n.a libicudata.a)
set(ICU_LIBRARIES )

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

add_library(lv2c OBJECT
    ./include/lv2c/Lv2cObject.hpp
    ./include/lv2c/IcuString.hpp
    ./include/lv2c/JsonVariant.hpp
    ./include/lv2c/JsonIo.hpp
    ./include/lv2c/Lv2cDialog.hpp
    ./include/lv2c/Lv2cStatusTextElement.hpp
    ./include/lv2c/Lv2cLampElement.hpp
    ./include/lv2c/Lv2cDbVuElement.hpp
    ./include/lv2c/Lv2cVuElement.hpp
    ./include/lv2c/Lv2cSwitchElement.hpp
    ./include/lv2c/Lv2cOnOffSwitchElement.hpp
    ./include/lv2c/Lv2cPngStripElement.hpp
    ./include/lv2c/Lv2cValueElement.hpp
    ./include/lv2c/Lv2cNumericEditBoxElement.hpp
    ./include/lv2c/Lv2cDropShadowElement.hpp
    ./include/lv2c/Lv2cRootElement.hpp
    ./include/lv2c/Lv2cDropdownElement.hpp
    ./include/lv2c/Lv2cDropdownItemElement.hpp
    ./include/lv2c/Lv2cPangoContext.hpp
    ./include/lv2c/Lv2cEditBoxElement.hpp
    ./include/lv2c/Lv2cPngDialElement.hpp
    ./include/lv2c/Lv2cDialElement.hpp
    ./include/lv2c/Lv2cBindingProperty.hpp
    ./include/lv2c/Lv2cPngElement.hpp
    ./include/lv2c/Lv2cSvgElement.hpp
    ./include/lv2c/Lv2cSvg.hpp
    ./include/lv2c/Lv2cDrawingContext.hpp
    ./include/lv2c/Lv2cFlexGridElement.hpp
    ./include/lv2c/Lv2cButtonBaseElement.hpp
    ./include/lv2c/Lv2cButtonElement.hpp
    ./include/lv2c/Lv2cTheme.hpp
    ./include/lv2c/Lv2c.hpp
    ./include/lv2c/Lv2cStyle.hpp
    ./include/lv2c/Lv2cElement.hpp
    ./include/lv2c/Lv2cVerticalStackElement.hpp
    ./include/lv2c/Lv2cHorizontalStackElement.hpp
    ./include/lv2c/Lv2cTypes.hpp
    ./include/lv2c/Lv2cWindow.hpp
    ./include/lv2c/Lv2cTypographyElement.hpp
    ./include/lv2c/Lv2cContainerElement.hpp
    ./include/lv2c/Lv2cSettingsFile.hpp
    ./include/lv2c/Lv2cMessageDialog.hpp
    ./include/lv2c/Lv2cScrollBarElement.hpp
    ./include/lv2c/Lv2cScrollContainerElement.hpp
    ./include/lv2c/Lv2cAnimator.hpp
    ./include/lv2c/Lv2cIndefiniteProgressElement.hpp
    ./include/lv2c/Lv2cMotionBlurElement.hpp
    ./include/lv2c/Lv2cSlideInOutAnimationElement.hpp
    ./include/lv2c/Lv2cCieColors.hpp
    ./include/lv2c/Lv2cGroupElement.hpp
    ./include/lv2c/Lv2cTableElement.hpp
    ./include/lv2c/Lv2cMarkdownElement.hpp
    ./Lv2cMarkdownElement.cpp
    ./Lv2cTableElement.cpp
    ./Lv2cGroupElement.cpp
    ./Lv2cCieColors.cpp
    ./Lv2cSlideInOutAnimationElement.cpp
    ./Lv2cMotionBlurElement.cpp
    ./Lv2cIndefiniteProgressElement.cpp
    ./Lv2cAnimator.cpp
    ./Lv2cScrollContainerElement.cpp
    ./Lv2cScrollBarElement.cpp
    ./Lv2cMessageDialog.cpp
    ./Lv2cSettingsFile.cpp
    ./ss.hpp
    ./cleanup.hpp
    ./Lv2cDialog.cpp
    ./IcuString.cpp
    ./Lv2cStatusTextElement.cpp
    ./Lv2cLampElement.cpp
    ./Lv2cDbVuElement.cpp
    ./Lv2cVuElement.cpp
    ./Lv2cDialBaseElement.cpp
    ./Lv2cPngStripElement.cpp
    ./Lv2cValueElement.cpp
    ./Utf8Utils.cpp ./Utf8Utils.hpp
    ./Lv2cNumericEditBoxElement.cpp
    ./Lv2cDropShadowElement.cpp
    ./Lv2cDropdownElement.cpp
    ./Lv2cDropdownItemElement.cpp
    ./keysym_names.cpp ./keysym_names.hpp
    ./Lv2cPangoContext.cpp
    ./Lv2cEditBoxElement.cpp
    ./Lv2cPngDialElement.cpp
    ./Lv2cDialElement.cpp
    ./Lv2cBindingProperty.cpp
    ./Lv2cPngElement.cpp
    ./Lv2cVerticalStackElement.cpp
    ./Lv2cVerticalStackElement.cpp
    ./Lv2cFlexGridElement.cpp
    ./Lv2cButtonBaseElement.cpp
    ./Lv2cButtonElement.cpp
    ./Lv2cStyle.cpp
    ./Lv2cTypographyElement.cpp
    ./Lv2cVerticalStackElement.cpp
    ./Lv2cWindow.cpp
    ./Lv2cSvg.cpp
    ./Lv2cDrawingContext.cpp
    ./include/lv2c/Lv2cDamageList.hpp
    ./Lv2cDamageList.cpp
    ./Lv2cTypes.cpp
    ./Lv2cTheme.cpp
    ./Lv2cContainerElement.cpp
    ./Lv2cLog.cpp
    ./include/lv2c/Lv2cLog.hpp
    ./Lv2cX11Window.cpp
    ./Lv2cRootElement.cpp
    ./Lv2cSvgElement.cpp
    ./JsonVariant.cpp
    ./JsonIo.cpp

    ./Lv2cX11Window.hpp
    ./Lv2cElement.cpp
    ./Lv2cSwitchElement.cpp

)

target_include_directories(
    lv2c PUBLIC
    ${LV2CAIRO_INCLUDE_DIRECTORY}
    ${CAIRO_INCLUDE_DIRS}
    ${PC_PANGOCAIRO_INCLUDE_DIRS}
    ${GOBJECT_INCLUDE_DIR}
    ${X11_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${RSVG2_INCLUDE_DIR}
    ${GDK_PIXBUF_INCLUDE_DIR}
)


message(STATUS "LIBS: " ${LV2CAIRO_INCLUDE_DIRECTORY}
${CAIRO_INCLUDE_DIRS}
${PC_PANGOCAIRO_INCLUDE_DIRS}
${GOBJECT_INCLUDE_DIR}
${X11_INCLUDE_DIRS}
${GLIB_INCLUDE_DIRS}
${RSVG2_INCLUDE_DIR}
${GDK_PIXBUF_INCLUDE_DIR}
)



target_link_libraries(lv2c PUBLIC 
    ${CAIRO_LIBRARIES} ${X11_LIBRARIES}
    ${PC_PANGOCAIRO_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${RSVG2_LIB}
    ${GDK_PIXBUF_LIB}
    #${ICU_LIBRARIES}
    Xrandr
)
