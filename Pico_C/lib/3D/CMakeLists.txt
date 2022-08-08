# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_Icm20948_SRCS 变量
aux_source_directory(. DIR_3D_SRCS)

# 生成链接库
add_library(3D ${DIR_3D_SRCS})
target_link_libraries(3D PUBLIC Config)