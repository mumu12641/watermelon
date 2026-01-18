cmake_minimum_required(VERSION 3.16)
project(gc)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)
set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)

set(LLVMIRFLAGS -S -emit-llvm -O0 -std=c++17 -Xclang -disable-O0-optnone -fno-discard-value-names)

find_program(OPT_EXECUTABLE opt REQUIRED)

file(MAKE_DIRECTORY ${BUILD_DIR})
file(MAKE_DIRECTORY ${LIB_DIR})

file(GLOB SOURCES "${SRC_DIR}/*.cpp")
file(GLOB HEADERS "${INCLUDE_DIR}/*.hpp")

foreach(SOURCE_FILE ${SOURCES})
  get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME_WE)
  set(LLVM_IR_FILE ${LIB_DIR}/${SOURCE_NAME}.ll)

  add_custom_command(
    OUTPUT ${LLVM_IR_FILE}
    COMMAND ${CMAKE_CXX_COMPILER} ${LLVMIRFLAGS} ${SOURCE_FILE} -o ${LLVM_IR_FILE}
    DEPENDS ${SOURCE_FILE} ${HEADERS}
    COMMENT "Generating LLVM IR for ${SOURCE_NAME}"
    VERBATIM
  )

  list(APPEND LLVM_IR_FILES ${LLVM_IR_FILE})
endforeach()

add_custom_target(llvm-ir ALL DEPENDS ${LLVM_IR_FILES})

add_custom_target(clean-all
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${BUILD_DIR}
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${LIB_DIR}
  COMMENT "Cleaning build and lib directories"
)

set(LOCAL_INSTALL_DIR "$ENV{HOME}/.watermelon")

install(CODE "file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}/gc\")")

install(DIRECTORY ${LIB_DIR}/
        DESTINATION "${LOCAL_INSTALL_DIR}/gc"
)
