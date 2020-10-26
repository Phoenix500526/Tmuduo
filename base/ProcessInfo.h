#ifndef TMUDUO_BASE_PROCESSINFO_H_
#define TMUDUO_BASE_PROCESSINFO_H_

#include <sys/types.h>
#include <vector>
#include "base/StringPiece.h"
#include "base/Timestamp.h"
#include "base/TypeCast.h"

namespace tmuduo {

namespace ProcessInfo {
pid_t pid();
std::string pidString();
uid_t uid();
std::string username();
uid_t euid();
Timestamp startTime();
int clockTicksPerSecond();
int pageSize();
bool isDebugBuild();  // constexpr
std::string hostname();
std::string procname();
StringPiece procname(const std::string& stat);

// read /proc/self/status
std::string procStatus();

// read /proc/self/stat
std::string procStat();

std::string threadStat();

int openedFiles();
int numThreads();
std::vector<pid_t> threads();
}  // namespace ProcessInfo

}  // namespace tmuduo

#endif  // TMUDUO_BASE_PROCESSINFO_H_