# 定义了 SRC_LIST 变量 包含了该目录下所有的源文件
aux_source_directory(. SRC_LIST)
# 指定生成可执行文件
add_executable(ChatClient ${SRC_LIST})
# 指定可执行文件连接时所需要的依赖的文件库
target_link_libraries(ChatClient pthread)