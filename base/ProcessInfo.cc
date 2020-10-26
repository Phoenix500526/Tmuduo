#include "base/ProcessInfo.h"
#include "base/CurrentThread.h"
#include "base/FileUtil.h"

#include <assert.h>
#include <dirent.h>  //Unix 类目录操作头文件
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <unistd.h>
#include <algorithm>

namespace tmuduo {

namespace detail {
thread_local int t_numOpenedFiles = 0;
int fdDirFilter(const struct dirent* d) {
  if (::isdigit(d->d_name[0])) {
    ++t_numOpenedFiles;
  }
  return 0;
}

int scanDir(const char* dirpath, int (*filter)(const struct dirent*)) {
  struct dirent** namelist = nullptr;
  // scandir 会对dirpath下的所有元素调用 filter，如果调用结果非0,则将
  //结果加入 namelist 中，若结果为0,则跳过。因此 scanDir 只是对目录下
  //的所有文件执行 filter 函数而已
  int result = ::scandir(dirpath, &namelist, filter, alphasort);
  assert(namelist == nullptr);
  return result;
}

thread_local std::vector<pid_t>* t_pids = nullptr;
int taskDirFilter(const struct dirent* d) {
  if (::isdigit(d->d_name[0])) {
    t_pids->push_back(atoi(d->d_name));
  }
  return 0;
}

Timestamp g_startTime = Timestamp::now();
// assume those won't change during the life time of a process
// POSIX 允许应用程序在编译期或运行期检测某些选项是否支持。在编译期可以直接
//通过判断 <unistd.h> 或 <limits.h> 中的某些宏是否已被定义来判断。在运行
//期则可以通过 sysconf、fpathconf或pathconf、confstr 来判断。
// sysconf：用于获取数值配置项
// fpathconf和pathconf：用于获取和文件系统相关的配置项
// confstr：用于获取字符串类型的配置项
//注意：上述配置项均为常量，它们在程序运行的过程当中是不会发生任何改变的。
int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));
int g_pageSize = static_cast<int>(::sysconf(_SC_PAGE_SIZE));
}  // namespace detail
}  // namespace tmuduo

using namespace tmuduo;
using namespace tmuduo::detail;

pid_t ProcessInfo::pid() { return ::getpid(); }

std::string ProcessInfo::pidString() {
  char buf[32];
  snprintf(buf, sizeof buf, "%d", pid());
  return buf;
}

uid_t ProcessInfo::uid() { return ::getuid(); }

std::string ProcessInfo::username() {
  struct passwd pwd;
  struct passwd* result = nullptr;
  char buf[8192];
  const char* name = "unknownuser";
  getpwuid_r(uid(), &pwd, buf, sizeof buf, &result);
  if (result) {
    name = pwd.pw_name;
  }
  return name;
}

uid_t ProcessInfo::euid() { return ::geteuid(); }

Timestamp ProcessInfo::startTime() { return g_startTime; }

int ProcessInfo::clockTicksPerSecond() { return g_clockTicks; }

int ProcessInfo::pageSize() { return g_pageSize; }

bool ProcessInfo::isDebugBuild() {
#ifdef NDEBUG
  return false;
#else
  return true;
#endif
}

std::string ProcessInfo::hostname() {
  char buf[256];
  if (0 == ::gethostname(buf, sizeof buf)) {
    buf[sizeof(buf) - 1] = '\0';
    return buf;
  } else {
    return "unknownhost";
  }
}

StringPiece ProcessInfo::procname(const std::string& stat) {
  StringPiece name;
  size_t lp = stat.find('(');
  size_t rp = stat.rfind(')');
  if (lp != std::string::npos && rp != std::string::npos && lp < rp) {
    name.set(stat.data() + lp + 1, static_cast<int>(rp - lp - 1));
  }
  return name;
}

std::string ProcessInfo::procname() { return procname(procStat()).as_string(); }

std::string ProcessInfo::procStat() {
  std::string result;
  FileUtil::readFile("/proc/self/stat", 65536, &result);
  return result;
}

int ProcessInfo::openedFiles() {
  t_numOpenedFiles = 0;
  scanDir("/proc/self/fd", fdDirFilter);
  return t_numOpenedFiles;
}

std::string ProcessInfo::threadStat() {
  char buf[64];
  snprintf(buf, sizeof buf, "/proc/self/task/%d/stat", CurrentThread::tid());
  std::string result;
  FileUtil::readFile(buf, 65536, &result);
  return result;
}

std::string ProcessInfo::procStatus() {
  std::string result;
  FileUtil::readFile("/proc/self/status", 65536, &result);
  return result;
}

int ProcessInfo::numThreads() {
  int result = 0;
  std::string status = procStatus();
  size_t pos = status.find("Threads:");
  if (pos != std::string::npos) {
    result = ::atoi(status.c_str() + pos + 8);
  }
  return result;
}

std::vector<pid_t> ProcessInfo::threads() {
  std::vector<pid_t> result;
  t_pids = &result;
  scanDir("/proc/self/task", taskDirFilter);
  t_pids = nullptr;
  std::sort(result.begin(), result.end());
  return result;
}

std::string ProcessInfo::exePath() {
  std::string result;
  char buf[1024];
  // readlink 可以读取符号链接的信息，并将其保存在 buf 当中
  ssize_t n = ::readlink("/proc/self/exe", buf, sizeof buf);
  if (n > 0) {
    result.assign(buf, n);
  }
  return result;
}

// struct rlimit{
//     rlim_t rlim_cur; //soft limit
//     rlim_t rlim_max; //hard limit
// };

int ProcessInfo::maxOpenFiles() {
  struct rlimit rl;
  //判断当前进程所打开的fd 是否超过了 RLIMIT_NOFILE
  if (::getrlimit(RLIMIT_NOFILE, &rl)) {
    //未超限，则返回实际打开的fd数目
    return openedFiles();
  } else {
    //超限则返回进程的软资源限制 rlim.cur
    return static_cast<int>(rl.rlim_cur);
  }
}

ProcessInfo::CpuTime ProcessInfo::cpuTime() {
  ProcessInfo::CpuTime t;
  struct tms tms;
  if (::times(&tms) >= 0) {
    const double hz = static_cast<double>(clockTicksPerSecond());
    t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
    t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
  }
  return t;
}