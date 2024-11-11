# CMakeList.txt : CMake project for ToobAmp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

find_package(Cairo)
find_package(X11)

# ffs! Breaking changes (a complete incompatible rewrite) between catch2 v3.x and catch2 v2.x, both installed with `apt install catch2`.
# Unbuntu 24.04 installs catch 3.x; Raspberry Pi OS installs catch 2.x.

if (EXISTS /usr/include/catch2/catch_all.hpp)
    message(STATUS "Using catch2 v3.x")
    set(CATCH2_VERSION_ 3)
elseif (EXISTS /usr/include/catch2/catch.hpp)
    message(STATUS "Using catch2 v2.x")
    set(CATCH2_VERSION_ 2)
else()
    message(FATAL_ERROR "catch2 test framework is not installed.")
endif()




add_executable(lv2c_demo  
    $<TARGET_OBJECTS:lv2c> $<TARGET_OBJECTS:lv2c_ui>

    Lv2cTestMain.cpp
    TestPage.hpp
    TunerTestPage.cpp TunerTestPage.hpp
    TableTestPage.cpp TableTestPage.hpp
    PaletteTestPage.cpp PaletteTestPage.hpp
    Lv2UiTestPage.cpp Lv2UiTestPage.hpp SamplePluginInfo.hpp
    MotionBlurTestPage.cpp MotionBlurTestPage.hpp
    DialTestPage.cpp DialTestPage.hpp
    DropShadowTestPage.cpp DropShadowTestPage.hpp
    StandardDialogTestPage.cpp StandardDialogTestPage.hpp

    ScrollBarTestPage.cpp ScrollBarTestPage.hpp
    PngTestPage.cpp PngTestPage.hpp
    DropdownTestPage.hpp DropdownTestPage.cpp 
    EditBoxTestPage.cpp EditBoxTestPage.hpp
    Lv2ControlTestPage.cpp Lv2ControlTestPage.hpp
    SvgTestPage.cpp SvgTestPage.hpp
    ButtonTestPage.hpp ButtonTestPage.cpp 
    TypographyTestPage.hpp TypographyTestPage.cpp 
    FlexGridTestPage.hpp FlexGridTestPage.cpp 
    VerticalStackTest.hpp VerticalStackTest.cpp
    SyntaxTest.cpp
    ss.hpp
)

add_custom_command(
        TARGET lv2c_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/resources
                ${CMAKE_CURRENT_BINARY_DIR}/resources)




target_include_directories(lv2c_demo PRIVATE
    ${Lv2c_INCLUDE_DIRS}
)

target_link_libraries(lv2c_demo 
    lv2c lv2c_ui pthread
)

add_executable(CatchTest 
    $<TARGET_OBJECTS:lv2c> $<TARGET_OBJECTS:lv2c_ui>
    CatchTest.hpp
    UriTest.cpp
    MaterialColorTest.cpp
    TestMain.cpp
    ColorTest.cpp
    JsonTest.cpp
    NiceEditStringTest.cpp
    DamageListTest.cpp
    BindingTest.cpp
    CapitalizationTest.cpp
    ss.hpp
)

target_compile_definitions(CatchTest PRIVATE CATCH2_VERSION=${CATCH2_VERSION_})

target_include_directories(CatchTest PRIVATE
    ${Lv2c_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/lv2c_ui
    lv2c_ui lv2c 
)

if (${CATCH2_VERSION_} EQUAL 3)
   set(CATCH2_MAIN_LIBS Catch2Main.a Catch2.a )
endif()

target_link_libraries(CatchTest lv2c lv2c_ui pthread  ${CATCH2_MAIN_LIBS})


add_test (NAME CatchTest COMMAND CatchTest)



