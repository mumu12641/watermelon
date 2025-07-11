cmake_minimum_required(VERSION 3.2)

project(watermelon)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
find_package(LLVM REQUIRED CONFIG)
include_directories(include "${LLVM_INCLUDE_DIR}")

llvm_map_components_to_libnames(llvm_libs core)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(watermelon ${SRC_LIST})
target_include_directories(watermelon PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(watermelon LLVM-14)
