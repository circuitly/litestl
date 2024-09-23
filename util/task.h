#pragma once

#include "platform/cpu.h"
#include "platform/time.h"
#include "util/alloc.h"
#include "util/index_range.h"
#include "util/vector.h"

#include <condition_variable>
#include <functional>
#include <thread>

namespace litestl::task {
/* [&](IndexRange range) {} */
template <typename Callback>
void parallel_for(util::IndexRange range, Callback cb, int grain_size = 1)
{
  using namespace util;

#if 0
  cb(range);
#else
  if (range.size <= grain_size) {
    cb(range);
    return;
  }

  bool have_remain = false;

  int task_count = range.size / grain_size;
  if (range.size % grain_size) {
    task_count++;
    have_remain = true;
  }

  int thread_count = platform::max_thread_count();

  struct ThreadData {
    Vector<IndexRange> tasks;
    bool done = false;
  };

  Vector<ThreadData> thread_datas;
  thread_datas.resize(thread_count);
  int thread_i = 0;

  for (int i = 0; i < task_count; i++) {
    int start = range.start + grain_size * i;
    IndexRange range;

    if (i == task_count - 1 && have_remain) {
      range = IndexRange(start, range.size - start);
    } else {
      range = IndexRange(start, grain_size);
    }

    thread_datas[thread_i].tasks.append(range);
    thread_i = (thread_i + 1) % thread_count;
  }

  Vector<std::thread *> threads;

  for (int i : IndexRange(thread_count)) {
    std::thread *thread =
        alloc::New<std::thread>("std::thread", std::thread([i, &thread_datas, &cb]() {
                                  for (IndexRange &range : thread_datas[i].tasks) {
                                    cb(range);
                                  }

                                  thread_datas[i].done = true;
                                }));

    threads.append(thread);
  }

  while (1) {
    bool ok = true;
    for (ThreadData &data : thread_datas) {
      ok = ok && data.done;
    }

    if (ok) {
      break;
    }

    /* TODO: use a condition variable. */
    time::sleep_ns(100);
  }

  for (std::thread *thread : threads) {
    thread->join();
    alloc::Delete<std::thread>(thread);
  }
#endif
}
} // namespace litestl::task
