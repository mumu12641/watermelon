cmake_minimum_required(VERSION 3.16)

# 定义项目名称和版本
project(watermelon VERSION 1.0.0 LANGUAGES C CXX)

# ==========================================
# 全局配置
# ==========================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 生成 compile_commands.json 供编辑器使用

# 定义安装路径变量，允许用户在命令行覆盖: cmake -DWATERMELON_HOME=~/.watermelon ..
if(NOT DEFINED WATERMELON_HOME)
    set(WATERMELON_HOME "$ENV{HOME}/.watermelon" CACHE PATH "Path to install watermelon libraries and config")
endif()

message(STATUS "Watermelon Home (Libs/Config): ${WATERMELON_HOME}")
message(STATUS "Executable Install Path: ${CMAKE_INSTALL_PREFIX}/bin")

# ==========================================
# 查找依赖 (LLVM) - 全局查找一次即可
# ==========================================
find_package(LLVM 14 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# 将 LLVM 的头文件路径添加到全局
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# ==========================================
# 添加子目录
# ==========================================
add_subdirectory(opt)
add_subdirectory(gc)
add_subdirectory(src)
add_subdirectory(std)

# ==========================================
# 生成并安装配置文件
# ==========================================
# 定义配置文件中的变量
set(CONF_STDLIB_PATH "${WATERMELON_HOME}/std")
set(CONF_LIB_PATH "${WATERMELON_HOME}/opt")
set(CONF_VERSION "${PROJECT_VERSION}")

# 生成 watermelon.conf (需要在根目录创建一个 watermelon.conf.in 模板，见下文)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config/watermelon.conf.in"
    "${CMAKE_CURRENT_BINARY_DIR}/watermelon.conf"
    @ONLY
)

# 安装配置文件
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/watermelon.conf"
        DESTINATION "${WATERMELON_HOME}")

# ==========================================
# 安装完成后的提示信息
# ==========================================
install(CODE "
    message(STATUS \"\")
    message(STATUS \"=== Installation Complete ===\")
    message(STATUS \"Executable: ${CMAKE_INSTALL_PREFIX}/bin/watermelon\")
    message(STATUS \"Config & Libs: ${WATERMELON_HOME}\")
    message(STATUS \"\")
")
