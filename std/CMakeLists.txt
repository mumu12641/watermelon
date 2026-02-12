# 直接安装整个目录内容
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION "${WATERMELON_HOME}/std"
        FILES_MATCHING PATTERN "*"
        PATTERN "CMakeLists.txt" EXCLUDE # 排除自身
)
