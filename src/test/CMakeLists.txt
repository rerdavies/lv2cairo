# CMakeList.txt : CMake project for ToobAmp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

find_package(Cairo)
find_package(X11)



add_executable(lvtk_demo  
    $<TARGET_OBJECTS:lvtk> $<TARGET_OBJECTS:lvtk_ui>

    LvtkTestMain.cpp
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
        TARGET lvtk_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/resources
                ${CMAKE_CURRENT_BINARY_DIR}/resources)




target_include_directories(lvtk_demo PRIVATE
    ${Lvtk_INCLUDE_DIRS}
)

target_link_libraries(lvtk_demo 
    lvtk lvtk_ui pthread
)

add_executable(CatchTest 
    $<TARGET_OBJECTS:lvtk> $<TARGET_OBJECTS:lvtk_ui>

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

target_include_directories(CatchTest PRIVATE
    ${Lvtk_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/lvtk_ui
    lvtk_ui lvtk 
)

target_link_libraries(CatchTest lvtk lvtk_ui pthread)


add_test (NAME CatchTest COMMAND CatchTest)



