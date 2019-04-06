cmake_minimum_required(VERSION 3.10)
project(argparseCPP)

set(CMAKE_CXX_STANDARD 11)

add_executable(argparseCPP main.cpp include/argparse.hpp)

include(CTest)
add_executable(unitTest test/test.cpp)
include_directories(unitTest include)

macro(add_arg)
    foreach(argu IN ITEMS ${ARGN})
        message(STATUS "${argu}\n")
    endforeach()
    add_test(UnitTest unitTest ${ARGN})
endmacro()

add_arg(-r --cook rice -e chicken beans -c well -d 1 2 3)

#add_test(NAME UnitTest COMMAND unitTest)