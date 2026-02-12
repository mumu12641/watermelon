cmake_minimum_required(VERSION 3.16)

# ==========================================
# 1. 查找工具
# ==========================================
# 查找 clang++ (用于生成原始 IR)
find_program(CLANG_CXX_COMPILER NAMES clang++ llvm-g++ REQUIRED)
# 查找 opt (用于加载 Pass 进行优化)
find_program(LLVM_OPT_TOOL opt REQUIRED)

# ==========================================
# 2. 路径设置
# ==========================================
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Clang 编译选项 (生成原始 IR)
set(GC_FLAGS -S -emit-llvm -O0 -std=c++17 -Xclang -disable-O0-optnone -fno-discard-value-names)

# ==========================================
# 3. 定义需要使用的 Pass
# ==========================================
# 这里对应你在 opt/CMakeLists.txt 中定义的库名称
set(PASS_TARGETS 
    lib_mem2reg_pass 
    lib_constant_prop_pass 
    lib_dce_pass 
    lib_cse_pass
)

# 这里对应你在 C++ Pass 代码中注册的 Pass 名称 (参考你之前的 install.sh)
set(PASS_PIPELINE "mem2reg-pass,constant-prop-pass,dce-pass,cse-pass")

# ==========================================
# 4. 生成与优化逻辑
# ==========================================
file(GLOB SOURCES "${SRC_DIR}/*.cpp")
file(GLOB HEADERS "${INCLUDE_DIR}/*.hpp")

set(GENERATED_IR_FILES "")

foreach(SOURCE_FILE ${SOURCES})
    get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME_WE)
    
    # 定义中间文件 (Raw IR) 和 最终文件 (Optimized IR)
    set(RAW_LL "${GC_BUILD_DIR}/${SOURCE_NAME}_raw.ll")
    set(FINAL_LL "${GC_BUILD_DIR}/${SOURCE_NAME}.ll")

    # 构建 opt 加载插件的参数
    # 使用生成器表达式 $<TARGET_FILE:xxx> 自动获取 .so 的绝对路径
    set(OPT_LOAD_FLAGS 
        -load-pass-plugin=$<TARGET_FILE:lib_mem2reg_pass>
        -load-pass-plugin=$<TARGET_FILE:lib_constant_prop_pass>
        -load-pass-plugin=$<TARGET_FILE:lib_dce_pass>
        -load-pass-plugin=$<TARGET_FILE:lib_cse_pass>
    )

    add_custom_command(
        OUTPUT ${FINAL_LL}
        
        # 步骤 1: 使用 Clang 生成原始 IR
        COMMAND ${CLANG_CXX_COMPILER} ${GC_FLAGS} -I${INCLUDE_DIR} ${SOURCE_FILE} -o ${RAW_LL}
        
        # 步骤 2: 使用 Opt 加载插件进行优化
        # 【修改点】去掉 -passes= 后面的双引号
        COMMAND ${LLVM_OPT_TOOL} -S 
                ${OPT_LOAD_FLAGS}
                -passes=${PASS_PIPELINE} 
                ${RAW_LL} -o ${FINAL_LL}
        
        DEPENDS ${SOURCE_FILE} ${HEADERS} ${PASS_TARGETS}
        
        COMMENT "Generating and Optimizing LLVM IR for GC: ${SOURCE_NAME}"
        VERBATIM
    )


    list(APPEND GENERATED_IR_FILES ${FINAL_LL})
endforeach()

# ==========================================
# 5. 目标与安装
# ==========================================
add_custom_target(gc_ir_compilation ALL DEPENDS ${GENERATED_IR_FILES})

# 安装最终优化过的 .ll 文件
install(FILES ${GENERATED_IR_FILES} DESTINATION "${WATERMELON_HOME}/gc")
