cmake_minimum_required(VERSION 3.2)

project(watermelon)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_VERBOSE_MAKEFILEON ON)
set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(watermelon  ${SRC_LIST})

# find_library(ANTLR4_RUNTIME_LIB antlr4-runtime)
# if (ANTLR4_RUNTIME_LIB)
#     target_link_libraries(watermelon ${ANTLR4_RUNTIME_LIB})
# else()
#     message(FATAL_ERROR "antlr4-runtime library not found.")
# endif()


