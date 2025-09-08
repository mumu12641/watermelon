cmake_minimum_required(VERSION 3.16)
project(opt)

set(CMAKE_CXX_COMPILER clang++)

execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE LLVM_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REGEX REPLACE "-std=[^ ]+" "" LLVM_CXXFLAGS_CLEAN "${LLVM_CXXFLAGS}")

set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS_CLEAN} -O3 -fPIC -fno-rtti -std=c++17")
set(CMAKE_SHARED_LINKER_FLAGS "${LLVM_LDFLAGS}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

file(GLOB SOURCES "src/*.cpp")

foreach(SOURCE ${SOURCES})
    get_filename_component(NAME ${SOURCE} NAME_WE)
    add_library(lib_${NAME} SHARED ${SOURCE})
    set_target_properties(lib_${NAME} PROPERTIES PREFIX "")
endforeach()

set(LOCAL_INSTALL_DIR "$ENV{HOME}/.watermelon")
install(CODE "
  file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}/opt\")
")

install(DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        DESTINATION "${LOCAL_INSTALL_DIR}/opt"
)
