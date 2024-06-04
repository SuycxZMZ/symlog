#!/bin/bash

set -e # exit on error

# 检查当前文件夹下是否存在 build 目录 和 bin 目录，如果不存在则创建，如果存在 build 目录,就删除里面的全部内容，但不删除build文件夹
if [ ! -d "build" ]; then
  mkdir build
  echo "创建 build 目录"

else
  rm -rf build/*
fi

if [ ! -d "bin" ]; then
mkdir bin
echo "创建 bin 目录"
fi

if [ ! -d "lib" ]; then
mkdir lib
echo "创建 lib 目录"
fi

# 检查 test 文件夹下是否存在 bin 目录，如果不存在则创建
if [ ! -d "test/bin" ]; then
  mkdir test/bin
  echo "创建 test/bin 目录"
fi

cd build
cmake ..
echo "CMake 配置完成"
make -j4
echo "编译完成"
cd ..

# 将所有的头文件移动到 /usr/loacl/include/symlog 目录下
# 将生成的动态库文件移动到 /usr/loacl/lib 目录下
# 检查 /usr/local/include/symlog 目录是否存在，如果不存在则创建
if [ ! -d "/usr/local/include/symlog" ]; then
  sudo mkdir -p /usr/local/include/symlog
  echo "创建 /usr/local/include/symlog 目录"
fi

cp src/base/*.h /usr/local/include/symlog/
cp src/log/*.h /usr/local/include/symlog/
cp lib/*.so /usr/local/lib 

echo "头文件和动态库文件已移动到 /usr/local/include/symlog 和 /usr/local/lib 目录下， \n
symlog库已完成安装！！！"

ldconfig