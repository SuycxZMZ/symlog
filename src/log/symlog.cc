#include "symlog.h"

namespace symlog {

std::shared_ptr<symlog::AsyncLogging> g_asyncLog;

void asyncLog(const char* msg, int len) {
    if (symlog::g_asyncLog) {
        symlog::g_asyncLog->append(msg, len);
    }
}

void initAsyncLogging(const char* filename, const off_t RollSize) {
    symlog::g_asyncLog.reset(new symlog::AsyncLogging(filename, RollSize));
    symlog::Logger::setOutput(symlog::asyncLog);
}

void AsyncLogStart() {
    if (g_asyncLog) {
        g_asyncLog->start();
    }
}

}  // namespace symlog