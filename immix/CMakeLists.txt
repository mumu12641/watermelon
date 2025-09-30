cmake_minimum_required(VERSION 3.16)
project(immix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER clang++)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE MAIN_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(immix ${MAIN_SOURCES})

# set(SRC_DIR src)
# set(INCLUDE_DIR include)
# set(BUILD_DIR build)
# set(LIB_DIR lib)

# set(LLVMIRFLAGS -S -emit-llvm -O0 -std=c++17 -Xclang -disable-O0-optnone -fno-discard-value-names)

# find_program(OPT_EXECUTABLE opt REQUIRED)

# file(GLOB SOURCES "${SRC_DIR}/*.cpp")
# file(GLOB HEADERS "${INCLUDE_DIR}/*.hpp")

# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUILD_DIR})
# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${LIB_DIR})

# foreach(SOURCE_FILE ${SOURCES})
# get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME_WE)

# set(LLVM_IR_FILE ${CMAKE_BINARY_DIR}/${LIB_DIR}/${SOURCE_NAME}.ll)

# add_custom_command(
#     OUTPUT ${LLVM_IR_FILE}
#     COMMAND ${CMAKE_CXX_COMPILER} ${LLVMIRFLAGS} ${SOURCE_FILE} -o ${LLVM_IR_FILE}
#     DEPENDS ${SOURCE_FILE} ${HEADERS}
#     COMMENT "Generating LLVM IR for ${SOURCE_NAME}"
#     VERBATIM
# )

# list(APPEND LLVM_IR_FILES ${LLVM_IR_FILE})
# endforeach()

# add_custom_target(llvm-ir ALL
# DEPENDS ${LLVM_IR_FILES}
# )

# add_custom_target(clean-all
# COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${BUILD_DIR}
# COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${LIB_DIR}
# COMMENT "Cleaning build and lib directories"
# )

# set(LOCAL_INSTALL_DIR "$ENV{HOME}/.watermelon")
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# install(CODE "
#   file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}/gc\")
# ")
# install(DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
#         DESTINATION "${LOCAL_INSTALL_DIR}/gc"
# )

