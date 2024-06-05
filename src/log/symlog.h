// 包含全部用到的头文件用户只需要包含该头文件即可
#ifndef SYMLOG_H
#define SYMLOG_H

#include "AsyncLogging.h"
#include "CurrentThread.h"
#include "LogFile.h"
#include "LogStream.h"
#include "Logging.h"
#include "MuduoThread.h"
#include "TimeStamp.h"

#include <memory>

namespace symlog {

void asyncLog(const char* msg, int len);

void initAsyncLogging(const char* filename, const off_t RollSize) ;

void AsyncLogStart() ;

}  // namespace symlog

#endif