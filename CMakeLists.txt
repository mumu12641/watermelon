cmake_minimum_required(VERSION 3.2)

set(PROJECT_VERSION "1.0.0")
project(watermelon VERSION ${PROJECT_VERSION})

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
find_package(LLVM 14 REQUIRED CONFIG)

include_directories(include "${LLVM_INCLUDE_DIR}")

# 使用基本的 LLVM 组件
llvm_map_components_to_libnames(llvm_libs 
  core 
  support 
  analysis 
  transformutils
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)

# 1. 编译优化 Pass 为共享库
file(GLOB OPT_SOURCES ${PROJECT_SOURCE_DIR}/src/opt/*.cpp)
add_library(watermelon_opt SHARED ${OPT_SOURCES})
target_include_directories(watermelon_opt PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(watermelon_opt ${llvm_libs})

# 设置共享库属性
set_target_properties(watermelon_opt PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  PREFIX "lib"
  SUFFIX ".so"
)

# 2. 编译主程序（排除 opt 目录）
file(GLOB_RECURSE MAIN_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(FILTER MAIN_SOURCES EXCLUDE REGEX "${PROJECT_SOURCE_DIR}/src/opt/.*")

add_executable(watermelon ${MAIN_SOURCES})
target_include_directories(watermelon PUBLIC ${PROJECT_SOURCE_DIR}/include)

# 链接 LLVM 库和优化 Pass 共享库
target_link_libraries(watermelon ${llvm_libs})

# 设置安装目录
set(LOCAL_INSTALL_DIR "$ENV{HOME}/.watermelon")

# 安装可执行文件到系统目录
install(TARGETS watermelon 
  RUNTIME DESTINATION "/usr/local/bin"
)

# 安装共享库到本地目录
install(TARGETS watermelon_opt 
  LIBRARY DESTINATION "${LOCAL_INSTALL_DIR}/lib"
)

# 创建本地目录和安装标准库
install(CODE "
  file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}\")
  file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}/lib\")
  file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}/std\")
  message(STATUS \"Created directories in: ${LOCAL_INSTALL_DIR}\")
")

file(GLOB STD_LIB_FILES "${PROJECT_SOURCE_DIR}/std/*")
install(FILES ${STD_LIB_FILES} DESTINATION "${LOCAL_INSTALL_DIR}/std")

# 创建配置文件，包含库路径信息
install(CODE "
  file(WRITE \"${LOCAL_INSTALL_DIR}/watermelon.conf\" \"# Watermelon compiler configuration\\nstdlib_path=${LOCAL_INSTALL_DIR}/std\\nlib_path=${LOCAL_INSTALL_DIR}/lib\\nversion=${PROJECT_VERSION}\\n\")
  message(STATUS \"Created configuration file with version ${PROJECT_VERSION}\")
")

# 安装完成信息
install(CODE "
  message(STATUS \"=== Installation Complete ===\")
  message(STATUS \"Executable installed to: /usr/local/bin/watermelon\")
  message(STATUS \"Libraries installed to: ${LOCAL_INSTALL_DIR}/lib/\")
  message(STATUS \"Standard library: ${LOCAL_INSTALL_DIR}/std/\")
  message(STATUS \"Configuration: ${LOCAL_INSTALL_DIR}/watermelon.conf\")
  message(STATUS \"\")
  message(STATUS \"Then you can use: watermelon your_file.wm\")
")

# 输出构建信息
add_custom_target(info ALL
  COMMAND ${CMAKE_COMMAND} -E echo "Build completed!"
  COMMAND ${CMAKE_COMMAND} -E echo "Shared library: ${LIBRARY_OUTPUT_PATH}/libwatermelon_opt.so"
  COMMAND ${CMAKE_COMMAND} -E echo "Main executable: ${EXECUTABLE_OUTPUT_PATH}/watermelon"
  COMMAND ${CMAKE_COMMAND} -E echo "Run 'sudo make install' to install"
)