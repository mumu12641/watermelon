cmake_minimum_required(VERSION 3.2)
set(PROJECT_VERSION "1.0.0")
project(watermelon)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
find_package(LLVM 14 REQUIRED CONFIG)

include_directories(include "${LLVM_INCLUDE_DIR}")
llvm_map_components_to_libnames(llvm_libs core)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(watermelon ${SRC_LIST})
target_include_directories(watermelon PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(watermelon LLVM-14)

install(TARGETS watermelon DESTINATION bin)

set(STDLIB_INSTALL_DIR "$ENV{HOME}/.watermelon")

install(CODE "
    file(MAKE_DIRECTORY \"${STDLIB_INSTALL_DIR}\")
    file(MAKE_DIRECTORY \"${STDLIB_INSTALL_DIR}/std\")
    message(STATUS \"Created directory: ${STDLIB_INSTALL_DIR}\")
")

file(GLOB STD_LIB_FILES "${PROJECT_SOURCE_DIR}/std/*")
install(FILES ${STD_LIB_FILES} DESTINATION "${STDLIB_INSTALL_DIR}/std")
configure_file(
    "${PROJECT_SOURCE_DIR}/config/watermelon.conf.in"
    "${PROJECT_BINARY_DIR}/config/watermelon.conf"
    @ONLY
)
install(FILES "${PROJECT_BINARY_DIR}/config/watermelon.conf" DESTINATION "${STDLIB_INSTALL_DIR}")

install(CODE "message(STATUS \"Watermelon compiler installed successfully!\")")
install(CODE "message(STATUS \"Standard library installed to: ${STDLIB_INSTALL_DIR}/std\")")