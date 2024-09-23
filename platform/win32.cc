#include <windows.h>
#include "platform/cpu.h"
#include "platform/time.h"

#include <chrono>
#include <thread>
#include <shellapi.h>

namespace litestl::time {
void sleep_ms(int ms)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void sleep_ns(int ns)
{
  std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
}
} // namespace sculptcore::time

namespace litestl::platform {
int cpu_core_count()
{
  SYSTEM_INFO info;
  GetSystemInfo(&info);

  return info.dwNumberOfProcessors;
}

int max_thread_count()
{
  return cpu_core_count();
}
} // namespace litestl::platform
