// 包含全部用到的头文件用户只需要包含该头文件即可
#ifndef SYMLOG_H
#define SYMLOG_H

#include "Logging.h"
#include "LogStream.h"
#include "LogFile.h"
#include "AsyncLogging.h"
#include "TimeStamp.h"
#include "MuduoThread.h"
#include "CurrentThread.h"

#include <memory>

namespace symlog
{
    
std::shared_ptr<symlog::AsyncLogging> g_asyncLog;

// static symlog::AsyncLogging* g_asyncLog = NULL;

void asyncLog(const char* msg, int len)
{
    if (symlog::g_asyncLog)
    {
        symlog::g_asyncLog->append(msg, len);
    }
}

void initAsyncLogging(const char* filename, const off_t RollSize)
{
    symlog::g_asyncLog.reset(new symlog::AsyncLogging(filename, RollSize));
    symlog::Logger::setOutput(symlog::asyncLog);
}

void AsyncLogStart()
{
    if (g_asyncLog) 
    {
        g_asyncLog->start();
    }
}

}

#endif