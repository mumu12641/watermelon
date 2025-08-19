cmake_minimum_required(VERSION 3.2)

set(PROJECT_VERSION "1.0.0")
project(watermelon VERSION ${PROJECT_VERSION})

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
find_package(LLVM 14 REQUIRED CONFIG)

include_directories(include "${LLVM_INCLUDE_DIR}")

llvm_map_components_to_libnames(llvm_libs 
  core 
  support 
  analysis 
  transformutils
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE MAIN_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(watermelon ${MAIN_SOURCES})
target_include_directories(watermelon PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(watermelon ${llvm_libs})

set(LOCAL_INSTALL_DIR "$ENV{HOME}/.watermelon")

install(TARGETS watermelon 
  RUNTIME DESTINATION "/usr/local/bin"
)

add_custom_target(opt_libs
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/opt
    COMMENT "Building opt_libs"
    VERBATIM
)
add_dependencies(watermelon opt_libs)

install(CODE "
  file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}\")
  file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}/lib\")
  file(MAKE_DIRECTORY \"${LOCAL_INSTALL_DIR}/std\")
  message(STATUS \"Created directories in: ${LOCAL_INSTALL_DIR}\")
")

file(GLOB STD_LIB_FILES "${PROJECT_SOURCE_DIR}/std/*")
install(FILES ${STD_LIB_FILES} DESTINATION "${LOCAL_INSTALL_DIR}/std")

file(GLOB OPT_LIB_FILES "${PROJECT_SOURCE_DIR}/opt/lib/*")
install(FILES ${OPT_LIB_FILES} DESTINATION "${LOCAL_INSTALL_DIR}/lib")

install(CODE "
  file(WRITE \"${LOCAL_INSTALL_DIR}/watermelon.conf\" \"# Watermelon compiler configuration\\nstdlib_path=${LOCAL_INSTALL_DIR}/std\\nlib_path=${LOCAL_INSTALL_DIR}/lib\\nversion=${PROJECT_VERSION}\\n\")
  message(STATUS \"Created configuration file with version ${PROJECT_VERSION}\")
")

install(CODE "
  message(STATUS \"=== Installation Complete ===\")
  message(STATUS \"Executable installed to: /usr/local/bin/watermelon\")
  message(STATUS \"Libraries installed to: ${LOCAL_INSTALL_DIR}/lib/\")
  message(STATUS \"Standard library: ${LOCAL_INSTALL_DIR}/std/\")
  message(STATUS \"Configuration: ${LOCAL_INSTALL_DIR}/watermelon.conf\")
  message(STATUS \"\")
  message(STATUS \"Then you can use: watermelon your_file.wm\")
")

add_custom_target(info ALL
  COMMAND ${CMAKE_COMMAND} -E echo "Build completed!"
  COMMAND ${CMAKE_COMMAND} -E echo "Run 'sudo make install' to install"
)