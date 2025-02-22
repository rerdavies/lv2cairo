# CMakeList.txt : CMake project for ToobAmp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

set(RECORD_PLUGIN_URI https://rerdavies.github.io/lv2c/download_test_plugin)
set(RECORD_LV2_INSTALL_PATH lv2tk_download_test.lv2)  # relative to /usr/lib/lv2, or /usr/local/lib/lv2.
set(RECORD_PLUGIN_UI_URI ${RECORD_PLUGIN_URI}-ui )
set(RECORD_LV2_SO_NAME lv2tk_download_test)
set(RECORD_LV2_UI_SO_NAME ${RECORD_LV2_SO_NAME}_ui)


configure_file(ttl/manifest.ttl.in manifest.ttl)
configure_file(ttl/plugin.ttl.in lv2tk_download_test.ttl)

set (RECORD_PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/lv2/${RECORD_LV2_INSTALL_PATH}")
set (RECORD_PLUGIN_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")



# Pseudo target for installing the test plugin in /usr/local/lib/lv2
add_custom_target(INSTALL_DOWNLOAD_TEST_PLUGIN
    COMMAND  cmake -E echo "Installing to ${RECORD_PLUGIN_INSTALL_DIR}"
    COMMAND  cmake -E make_directory "${RECORD_PLUGIN_INSTALL_DIR}"
    COMMAND  cmake -E make_directory "${RECORD_PLUGIN_INSTALL_DIR}/resources"
    COMMAND  cmake -E copy "${RECORD_PLUGIN_BINARY_DIR}/${RECORD_LV2_SO_NAME}.so" "${RECORD_PLUGIN_INSTALL_DIR}"
    COMMAND  cmake -E copy "${RECORD_PLUGIN_BINARY_DIR}/${RECORD_LV2_SO_NAME}_ui.so" "${RECORD_PLUGIN_INSTALL_DIR}"
    COMMAND  cmake -E copy "${RECORD_PLUGIN_BINARY_DIR}/manifest.ttl" "${RECORD_PLUGIN_INSTALL_DIR}"
    COMMAND  cmake -E copy "${RECORD_PLUGIN_BINARY_DIR}/${RECORD_LV2_SO_NAME}.ttl" "${RECORD_PLUGIN_INSTALL_DIR}"
    COMMAND  cmake -E copy_directory "${PROJECT_SOURCE_DIR}/resources" "${RECORD_PLUGIN_INSTALL_DIR}/resources"
)

add_custom_command(
    OUTPUT  RecordPluginInfo.hpp          # Treated as relative to CMAKE_CURRENT_BINARY_DIR
    COMMAND generate_lv2c_plugin_info 
            ${RECORD_PLUGIN_URI}
              --ttl ${CMAKE_CURRENT_BINARY_DIR}/
              --out ${CMAKE_CURRENT_BINARY_DIR}/RecordPluginInfo.hpp 
              --namespace record_plugin
              --ui-base-class RecordPluginUiInfo
              --plugin-base-class RecordPluginBase
    DEPENDS 
        ${CMAKE_CURRENT_BINARY_DIR}/manifest.ttl 
        ${CMAKE_CURRENT_BINARY_DIR}/${RECORD_LV2_SO_NAME}.ttl    
        generate_lv2c_plugin_info
)

add_library(lv2tk_download_test SHARED

    RecordPlugin.cpp RecordPlugin.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/RecordPluginInfo.hpp
)
set_target_properties(lv2tk_download_test PROPERTIES OUTPUT_NAME "${RECORD_LV2_SO_NAME}")
set_target_properties(lv2tk_download_test PROPERTIES PREFIX "")

target_compile_definitions(
    lv2tk_download_test PRIVATE
    PLUGIN_URI="${RECORD_PLUGIN_URI}"
)

target_include_directories(
    lv2tk_download_test PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/    
    lv2c
)
target_link_libraries(lv2tk_download_test PRIVATE
   lv2_plugin
   )

target_link_directories(
    lv2tk_download_test PRIVATE
)

add_library(lv2tk_download_test_ui SHARED
    $<TARGET_OBJECTS:lv2c> $<TARGET_OBJECTS:lv2c_ui>
    RecordPluginUi.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/RecordPluginInfo.hpp 
)
set_target_properties(lv2tk_download_test_ui PROPERTIES OUTPUT_NAME "${RECORD_LV2_UI_SO_NAME}")
set_target_properties(lv2tk_download_test_ui PROPERTIES PREFIX "")



target_link_options(lv2tk_download_test_ui PRIVATE
    # "-Wl,-z,nodelete" 
    # -Wl,-uiLinkTarget
)


target_compile_definitions(
    lv2tk_download_test_ui PRIVATE
    PLUGIN_URI="${RECORD_PLUGIN_URI}"
    PLUGIN_UI_URI="${RECORD_PLUGIN_UI_URI}"
)

target_include_directories(lv2tk_download_test_ui PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(lv2tk_download_test_ui lv2c lv2c_ui)


option(INSTALL_LV2CAIRO_TEST_PLUGIN "Install LV2Cairo test Plugins" ON)

if (INSTALL_LV2CAIRO_TEST_PLUGIN)

    set(RECORD_INSTALL_DIR /usr/lib/lv2/${RECORD_LV2_INSTALL_PATH})

    

    install(TARGETS lv2tk_download_test lv2tk_download_test_ui
        LIBRARY DESTINATION ${RECORD_INSTALL_DIR}
        )


    # Copy all assets to resources file
    install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/manifest.ttl 
    ${CMAKE_CURRENT_BINARY_DIR}/${RECORD_LV2_SO_NAME}.ttl
        DESTINATION ${RECORD_INSTALL_DIR}
    )
    # copy all resourece files to install resource directory.

    install(DIRECTORY ${PROJECT_SOURCE_DIR}/resources DESTINATION ${RECORD_INSTALL_DIR} )
endif()
