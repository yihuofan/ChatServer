# 定义SRC_LIST变量，包含该目录下所有源文件
aux_source_directory(. SRC_LIST)

# 指定生成可执行文件
add_executable(ChatServer ${SRC_LIST})

#链接时的库文件
target_link_libraries(ChartServer muduo_net muduo_base pthread)