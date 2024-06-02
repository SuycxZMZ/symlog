#include "symlog.h"

#include <stdio.h>
#include <unistd.h>
#include <thread>


void test_SyncLogging()
{
    LOG_DEBUG << "debug";
    LOG_INFO << "info";
    LOG_WARN << "warn";
    LOG_ERROR << "error";
    // 注意不能轻易使用 LOG_FATAL, LOG_SYSFATAL, 会导致程序abort
    for (int i = 0; i < 10; ++i) 
    {
        LOG_INFO << "SyncLogging test " << i; 
    }
}

void test_AsyncLogging()
{
    const int cnt = 1000 * 1000;
    for (int i = 0; i < cnt; ++i) 
    {
        LOG_INFO << "AsyncLogging test " << i;
    }
}

int main(int argc, char* argv[])
{
    printf("pid = %d\n", getpid());
    test_SyncLogging();

    symlog::initAsyncLogging(::basename(argv[0]), 1024 * 1024 * 4);
    symlog::AsyncLogStart();

    std::thread t1(test_AsyncLogging);
    std::thread t2(test_AsyncLogging);
    t1.join();
    t2.join();
    
    return 0;
}