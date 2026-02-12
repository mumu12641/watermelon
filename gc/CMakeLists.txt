# 查找 clang++ 程序 (用于生成 IR)
# 优先查找环境变量或路径中的 clang++
find_program(CLANG_CXX_COMPILER NAMES clang++ llvm-g++ REQUIRED)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

# 编译选项
set(GC_FLAGS -S -emit-llvm -O0 -std=c++17 -Xclang -disable-O0-optnone -fno-discard-value-names)

file(GLOB SOURCES "${SRC_DIR}/*.cpp")
file(GLOB HEADERS "${INCLUDE_DIR}/*.hpp")

set(GENERATED_IR_FILES "")

foreach(SOURCE_FILE ${SOURCES})
    get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME_WE)
    set(OUTPUT_LL "${GC_BUILD_DIR}/${SOURCE_NAME}.ll")

    # 添加自定义命令：将 .cpp 编译为 .ll
    add_custom_command(
        OUTPUT ${OUTPUT_LL}
        COMMAND ${CLANG_CXX_COMPILER} ${GC_FLAGS} -I${INCLUDE_DIR} ${SOURCE_FILE} -o ${OUTPUT_LL}
        DEPENDS ${SOURCE_FILE} ${HEADERS}
        COMMENT "Generating LLVM IR for GC: ${SOURCE_NAME}"
        VERBATIM
    )

    list(APPEND GENERATED_IR_FILES ${OUTPUT_LL})
endforeach()

# 创建一个自定义目标来驱动上述命令执行
add_custom_target(gc_ir_compilation ALL DEPENDS ${GENERATED_IR_FILES})

# 安装规则：安装生成的 .ll 文件
install(FILES ${GENERATED_IR_FILES} DESTINATION "${WATERMELON_HOME}/gc")
