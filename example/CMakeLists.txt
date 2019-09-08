message(STATUS "Examples ON")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(example example.cpp ${PROJECT_SOURCE_DIR}/include/argparse.hpp)