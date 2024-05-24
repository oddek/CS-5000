cmake_minimum_required(VERSION 3.16)
project(SUT)

set(CMAKE_CXX_STANDARD 20)

message(CrossCompiling: ${CMAKE_CROSSCOMPILING})
message(SystemName: ${CMAKE_SYSTEM_NAME})
MESSAGE(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

if(NOT CMAKE_CROSSCOMPILING)
add_compile_options(
        -Wall
        -Wno-main
        -Wundef
        -pedantic
        -Wno-variadic-macros
        -Wfatal-errors
        -Wl,--relax,--gc-sections
        -g
        -Wno-psabi 
        -Wno-sign-compare
        -fpermissive
)
endif()

include(FetchContent)
FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.1.1
)
FetchContent_MakeAvailable(fmt)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package ( LibNL REQUIRED )

set(CMAKE_SYSTEM_PROCESSOR arm)
if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

add_subdirectory(Applications)
add_subdirectory(Components)