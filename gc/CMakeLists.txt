cmake_minimum_required(VERSION 3.16)
project(gc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER clang++)

set(SRC_DIR src)
set(INCLUDE_DIR include)
set(BUILD_DIR build)
set(LIB_DIR lib)

set(LLVMIRFLAGS -S -emit-llvm -O0 -std=c++17 -Xclang -disable-O0-optnone)

# 查找opt工具
find_program(OPT_EXECUTABLE opt REQUIRED)

file(GLOB SOURCES "${SRC_DIR}/*.cpp")
file(GLOB HEADERS "${INCLUDE_DIR}/*.hpp")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUILD_DIR})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${LIB_DIR})

foreach(SOURCE_FILE ${SOURCES})
get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME_WE)

# 原始IR文件
set(RAW_LLVM_IR_FILE ${CMAKE_BINARY_DIR}/${LIB_DIR}/${SOURCE_NAME}_raw.ll)
# 优化后的IR文件
set(LLVM_IR_FILE ${CMAKE_BINARY_DIR}/${LIB_DIR}/${SOURCE_NAME}.ll)

# 生成原始LLVM IR
add_custom_command(
    OUTPUT ${RAW_LLVM_IR_FILE}
    COMMAND ${CMAKE_CXX_COMPILER} ${LLVMIRFLAGS} ${SOURCE_FILE} -o ${RAW_LLVM_IR_FILE}
    DEPENDS ${SOURCE_FILE} ${HEADERS}
    COMMENT "Generating raw LLVM IR for ${SOURCE_NAME}"
    VERBATIM
)

set(LOCAL_INSTALL_DIR "$ENV{HOME}/.watermelon")

# 使用opt优化IR
add_custom_command(
    OUTPUT ${LLVM_IR_FILE}
    COMMAND opt -S 
            -load-pass-plugin="${LOCAL_INSTALL_DIR}/opt/lib_mem2reg_pass.so"
            -load-pass-plugin="${LOCAL_INSTALL_DIR}/opt/lib_constant_prop_pass.so"
            -load-pass-plugin="${LOCAL_INSTALL_DIR}/opt/lib_dce_pass.so"
            -passes="mem2reg-pass,constant-prop-pass,dce-pass"
            ${RAW_LLVM_IR_FILE} -o ${LLVM_IR_FILE}
)

list(APPEND LLVM_IR_FILES ${LLVM_IR_FILE})
endforeach()

add_custom_target(llvm-ir ALL
DEPENDS ${LLVM_IR_FILES}
COMMENT "Building all optimized LLVM IR files"
)

add_custom_target(clean-all
COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${BUILD_DIR}
COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${LIB_DIR}
COMMENT "Cleaning build and lib directories"
)

set(LOCAL_INSTALL_DIR "$ENV{HOME}/.watermelon")
install(CODE "
file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}/gc\")
")

# 安装优化后的IR文件
install(FILES ${LLVM_IR_FILES} DESTINATION "${LOCAL_INSTALL_DIR}/gc")