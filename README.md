# 精简版的muduo日志库

## 使用

### 环境
    linux
    编译器支持c++11标准
    cmake版本3.0以上

    编译不过可酌情修改项目根目录下的 CMakeLists.txt

### 编译 && 安装

    git clone https://github.com/SuycxZMZ/symlog.git
    cd symlog
    mkdir build
    cd build
    cmake ..
    make -j4
    sudo make install
    sudo ldconfig

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

### 简单压力测试

    机器配置：
        CPU:Intel(R) Core(TM) i5-6500 CPU @ 3.20GHz
        内存:8G，ddr4
        硬盘：西部数据蓝固态盘 256G
    
    测试结果：
        单独起2个线程，每个线程不间断生成1000000条日志
        每条日志 80 字节左右
        
        Total log size: 168405455 bytes
        Logging 2000000 entries took 0.987152 seconds
        Rate: 162.694227 MB/second

对比常见[webser](https://github.com/qinguoyi/TinyWebServer)中以阻塞队列方式实现的异步日志，性能提升一个数量级。

对其做同样的2000000条日志，花费时间为 14.25s

## 特点

- 只包含日志相关的代码，编译速度快
- 移除boost，使用C++11编写，支持C++11特性
- 使用简单


## [设计](https://github.com/SuycxZMZ/tiny-muduo)

## [例子/测试](test/asynclogtest.cc)

**参考 && 致谢 ：**

https://github.com/chenshuo/muduo

https://blog.csdn.net/T_Solotov/article/details/124044175

https://zhuanlan.zhihu.com/p/636581210

https://github.com/Shangyizhou/A-Tiny-Network-Library

https://www.cnblogs.com/tuilk/p/16793625.html
