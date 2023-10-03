# CMakeList.txt : CMake project for ToobAmp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)


set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)




execute_process(COMMAND
  dpkg-architecture
    -qDEB_HOST_ARCH
  OUTPUT_VARIABLE
    CMAKE_DEB_HOST_ARCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_DEB_HOST_ARCH STREQUAL "arm64")
        set(CXX_ARCH_FLAGS "-mtune=cortex-a72") # raspberry pi 4. cortex-a72
    endif()


    set (CXX_EXTRA_FLAGS "-Wno-psabi -Werror -Wall -pedantic -Wno-reorder")

    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ${CXX_ARCH_FLAGS} ${CXX_EXTRA_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -D_GLIBCXX_DEBUG ${CXX_ARCH_FLAGS} ${CXX_EXTRA_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -DRELWITHDEBINFO -g ${CXX_ARCH_FLAGS} ${CXX_EXTRA_FLAGS}")   #--param=max-vartrack-size=1500000

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    endif()

    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_EXTRA_FLAGS}")
endif()

if (CMAKE_COMPILER_IS_GNUCXX )
  #If the mold linker is installed, use mold instead of ld (spectacularly faster linking)
  if (EXISTS /usr/local/libexec/mold/ld)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -B/usr/local/libexec/mold")
    message(STATUS "Using mold linker in /usr/local.")
  endif()
  if (EXISTS /usr/libexec/mold/ld)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -B/usr/libexec/mold")
    message(STATUS "Using mold linker.")
    #message(STATUS "mold linker temporarily disabled.")  # FIXME!
  endif()
endif()



message(STATUS "lvtk: CMAKE_DEB_HOST_ARCH: ${CMAKE_DEB_HOST_ARCH}")
message(STATUS "lvtk: CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "lvtk: CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(generate_lvtk_plugin_info)
add_subdirectory(lvtk)
add_subdirectory(lvtk_ui)
add_subdirectory(test)
add_subdirectory(test_plugin)
