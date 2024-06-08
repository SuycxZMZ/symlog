cmake_minimum_required(VERSION 3.10)

project(symlog)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/src/base)
include_directories(${PROJECT_SOURCE_DIR}/src/log)

file(GLOB SRC_LIST
    # 列出所有需要编译的源文件
    src/base/*.cc
    src/log/*.cc
)

add_library(symlog SHARED ${SRC_LIST})

# -------------------------------------- install --------------------------------------
# 设置安装前缀
set(CMAKE_INSTALL_PREFIX /usr/local)

# 递归查找所有头文件并安装到同一个目录中
file(GLOB_RECURSE HEADER_FILES
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
)

# 添加头文件安装规则
install(FILES ${HEADER_FILES}
    DESTINATION include/symlog
)

# 添加库文件安装规则
install(TARGETS symlog
    LIBRARY DESTINATION lib
)

# 测试代码
option(BUILD_TEST "ON for compile test" ON)
if (BUILD_TEST)
    add_subdirectory(test)
endif()