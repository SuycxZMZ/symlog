file(GLOB SRC_LIST
    # 列出所有需要编译的源文件
    base/*.cc
    log/*.cc
)

add_library(symlog SHARED ${SRC_LIST})