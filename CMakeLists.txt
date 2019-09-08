cmake_minimum_required(VERSION 3.11)

option(ENABLE_EXAMPLE "Enable Example" ON)
option(ENABLE_TEST "Enable Testing" ON)

project(argparseCPP
        VERSION 1.0.0
        DESCRIPTION "Argument Parser written in C++ 11"
        LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin)

if (ENABLE_EXAMPLE)
    add_subdirectory(example)
endif()

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()


#include(CTest)
#add_executable(unitTest test/test.cpp)
#include_directories(unitTest include)
#
#macro(add_arg)
#    add_test(UnitTest unitTest ${ARGN})
#endmacro()
#
#add_arg(-r --cook rice -e chicken beans -c well -d 1 2 3)
