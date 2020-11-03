#include "net/inspect/SystemInspector.h"
#include "base/FileUtil.h"

#include <sys/utsname.h>

using namespace tmuduo;
using namespace tmuduo::net;

namespace tmuduo {
namespace inspect {

std::string uptime(Timestamp now, Timestamp start, bool showMicroseconds);
long getLong(const std::string& content, const char* key);
// __attribute__ format属性可以给被声明的函数加上类似printf或者scanf的特征，
// 它可以使编译器检查函数声明和函数实际调用参数之间的格式化字符串是否匹配。
int stringPrintf(std::string* out, const char* fmt, ...)
    __attribute__((format(printf, 2, 3)));

}  // namespace inspect
}  // namespace tmuduo

using namespace tmuduo::inspect;

void SystemInspector::registerCommands(Inspector* ins) {
  ins->add("sys", "overview", SystemInspector::overview,
           "print system overview");
  ins->add("sys", "loadavg", SystemInspector::loadavg, "print /proc/loadavg");
  ins->add("sys", "version", SystemInspector::version, "print /proc/version");
  ins->add("sys", "cpuinfo", SystemInspector::cpuinfo, "print /proc/cpuinfo");
  ins->add("sys", "meminfo", SystemInspector::meminfo, "print /proc/meminfo");
  ins->add("sys", "stat", SystemInspector::stat, "print /proc/stat");
}

std::string SystemInspector::loadavg(HttpRequest::Method,
                                     const Inspector::ArgList&) {
  std::string loadavg;
  FileUtil::readFile("/proc/loadavg", 65535, &loadavg);
  return loadavg;
}

std::string SystemInspector::version(HttpRequest::Method,
                                     const Inspector::ArgList&) {
  std::string version;
  FileUtil::readFile("/proc/version", 65535, &version);
  return version;
}

std::string SystemInspector::cpuinfo(HttpRequest::Method,
                                     const Inspector::ArgList&) {
  std::string cpuinfo;
  FileUtil::readFile("/proc/cpuinfo", 65535, &cpuinfo);
  return cpuinfo;
}

std::string SystemInspector::meminfo(HttpRequest::Method,
                                     const Inspector::ArgList&) {
  std::string meminfo;
  FileUtil::readFile("/proc/meminfo", 65535, &meminfo);
  return meminfo;
}

std::string SystemInspector::stat(HttpRequest::Method,
                                  const Inspector::ArgList&) {
  std::string stat;
  FileUtil::readFile("/proc/stat", 65535, &stat);
  return stat;
}

std::string SystemInspector::overview(HttpRequest::Method,
                                      const Inspector::ArgList&) {
  std::string result;
  result.reserve(1024);
  Timestamp now = Timestamp::now();
  result += "Page generated at ";
  result += now.toFormattedString();
  result += " (UTC)\n";
  // Hardware and OS info
  {
    struct utsname un;
    if (::uname(&un) == 0) {
      stringPrintf(&result, "Hostname: %s\n", un.nodename);
      stringPrintf(&result, "Machine: %s\n", un.machine);
      stringPrintf(&result, "OS: %s %s %s\n", un.sysname, un.release,
                   un.version);
    }
  }

  std::string stat;
  FileUtil::readFile("/proc/stat", 65536, &stat);
  Timestamp bootTime(Timestamp::kMicroSecondsPerSecond *
                     getLong(stat, "processes "));
  result += "Boot time: ";
  result += bootTime.toFormattedString(false);
  result += " (UTC)\n";
  result += "Up time: ";
  result += uptime(now, bootTime, false);
  result += "\n";

  // CPU load
  {
    std::string loadavg;
    FileUtil::readFile("/proc/loadavg", 65536, &loadavg);
    stringPrintf(&result, "processes created: %ld\n",
                 getLong(stat, "processes "));
    stringPrintf(&result, "Loadavg: %s\n", loadavg.c_str());
  }

  // Memory
  {
    std::string meminfo;
    FileUtil::readFile("/proc/meminfo", 65536, &meminfo);
    long total_kb = getLong(meminfo, "MemTotal:");
    long free_kb = getLong(meminfo, "MemFree:");
    long buffers_kb = getLong(meminfo, "Buffers:");
    long cached_kb = getLong(meminfo, "Cached:");

    stringPrintf(&result, "Total Memory:    %6ld MiB\n", total_kb / 1024);
    stringPrintf(&result, "Free Memory:     %6ld MiB\n", free_kb / 1024);
    stringPrintf(&result, "Buffers:         %6ld MiB\n", buffers_kb / 1024);
    stringPrintf(&result, "Cached:          %6ld MiB\n", cached_kb / 1024);
    stringPrintf(&result, "Real Used:       %6ld MiB\n",
                 (total_kb - free_kb - buffers_kb - cached_kb) / 1024);
    stringPrintf(&result, "Real Free:       %6ld MiB\n",
                 (free_kb + buffers_kb + cached_kb) / 1024);
  }
  return result;
}