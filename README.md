# 精简版的muduo日志库

## 使用

## 环境
    linux
    编译器支持c++11标准
    cmake版本3.0以上

    编译不过可酌情修改项目根目录下的 CMakeLists.txt

### 编译 && 安装

    git clone https://github.com/SuycxZMZ/symlog.git
    cd symlog
    sudo bash autobuild.sh

    安装完成之后会有打印提示

### 使用

如果仅作为控制台简单打印调试，则只用包含 symlog.h 头文件，在打印的地方直接 **LOG_XXX << ...** 打印即可

如果对性能有要求，则可以使用异步日志，除了包含 symlog.h 头文件之外，要在main函数中，用户主逻辑开始之前初始化异步日志

```C++
// 两步操作
// 初始化 void initAsyncLogging(const char* filename, const off_t RollSize)，
// filename是日志文件名
// RollSize 代表滚动日志大小，当日志文件达到 RollSize 时即 创建下一个日志文件
symlog::initAsyncLogging(::basename(argv[0]), 1024 * 1024 * 4);
// 开启异步日志
symlog::AsyncLogStart();
```


## 特点

- 只包含日志相关的代码，编译速度快
- 移除boost，使用C++11编写，支持C++11特性
- 使用简单

## 遗留问题

异步日志的滚动功能存在bug
滚动大小没有按照预想的固定
待解决...

## [设计](https://github.com/SuycxZMZ/tiny-muduo)

## [例子/测试](test/asynclogtest.cc)
