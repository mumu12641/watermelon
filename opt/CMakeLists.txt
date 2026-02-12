# 查找源文件
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 确保输出目录存在 (CMake 会自动处理，但为了对应你的逻辑)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

foreach(SOURCE ${SOURCES})
    get_filename_component(NAME ${SOURCE} NAME_WE)
    
    # 创建库，使用 MODULE 类型 (代表插件，不用于链接)
    add_library(lib_${NAME} MODULE ${SOURCE})
    
    # 设置输出名称前缀为空 (lib_xxx -> lib_xxx.so，而不是 liblib_xxx.so)
    set_target_properties(lib_${NAME} PROPERTIES PREFIX "")
    
    # 链接 LLVM (Pass 通常不需要显式链接 LLVM 库，因为它们被加载到 LLVM 进程中，
    # 但为了符号解析，最好加上编译选项)
    target_compile_options(lib_${NAME} PRIVATE ${LLVM_CXXFLAGS})
    # 如果需要链接，取消下面注释，但在 Pass 中通常不需要
    # target_link_libraries(lib_${NAME} PRIVATE ${llvm_libs})

    # 安装规则：安装到 ~/.watermelon/opt
    install(TARGETS lib_${NAME} DESTINATION "${WATERMELON_HOME}/opt")
endforeach()
