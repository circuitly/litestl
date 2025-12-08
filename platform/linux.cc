#include "platform/cpu.h"
#include "platform/time.h"

#include <chrono>
#include <thread>

namespace litestl::time {
void sleep_ms(int ms)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void sleep_ns(int ns)
{
  std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
}
} // namespace litestl::time

namespace litestl::platform {
int cpu_core_count()
{
  return 4;
}

int max_thread_count()
{
  return cpu_core_count() * 2;
}
} // namespace litestl::platform
