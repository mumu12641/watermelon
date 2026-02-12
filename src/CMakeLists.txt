# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 收集源文件
# 注意：虽然 GLOB_RECURSE 方便，但手动列出文件是 CMake 最佳实践。
# 这里为了方便你迁移，依然使用 GLOB_RECURSE，但建议未来改为显式列出。
file(GLOB_RECURSE MAIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 定义可执行文件
add_executable(watermelon ${MAIN_SOURCES})

# 映射 LLVM 组件
llvm_map_components_to_libnames(llvm_libs 
  core 
  support 
  analysis 
  transformutils
)

# 链接库
target_link_libraries(watermelon PRIVATE ${llvm_libs})

# 安装规则
# 默认安装到 /usr/local/bin (由 CMAKE_INSTALL_PREFIX 控制)
install(TARGETS watermelon DESTINATION bin)
