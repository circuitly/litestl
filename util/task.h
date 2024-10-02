#pragma once

#include "platform/cpu.h"
#include "platform/time.h"
#include "util/alloc.h"
#include "util/index_range.h"
#include "util/vector.h"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace litestl::task {
using ThreadMain = std::function<void()>;
using litestl::util::Vector;

namespace detail {
struct TaskWorker {
  Vector<ThreadMain> queue;
  std::recursive_mutex mutex;
  std::mutex wait_mutex;
  std::condition_variable cv;
  bool running = false;

  TaskWorker()
  {
  }

  void start()
  {
    {
      std::lock_guard guard(mutex);
      if (running) {
        return;
      }
      running = true;
    }

    std::thread *thread = new std::thread([&]() { this->run(); });
    thread->detach();
    delete thread;
  }

  void run() {
    while (1) {
      while (!this->empty()) {
        ThreadMain main = this->pop();
        main();
      }
      
      {
        std::lock_guard guard(mutex);
        if (queue.size() == 0) {
          std::unique_lock lock(wait_mutex);
          cv.wait(lock);
        }
      }
    }
  }

  void run_old()
  {
    using namespace std::chrono_literals;

    while (!this->empty()) {
      ThreadMain main = this->pop();
      main();
    }

    bool empty;
    {
      std::lock_guard guard(mutex);
      empty = this->empty();
      if (empty) {
        running = false;
      }
    }

    if (!empty) {
      run();
    }
  }

  int size()
  {
    return queue.size();
  }
  void push(ThreadMain main)
  {
    {
      std::lock_guard guard(mutex);
      queue.append(main);
      if (queue.size() == 1) {
        cv.notify_all();
      }
    }

    if (!running) {
      //start();
    }
  }
  ThreadMain pop()
  {
    std::lock_guard guard(mutex);
    ThreadMain ret = queue.pop_back();
    return ret;
  }
  bool empty()
  {
    std::lock_guard guard(mutex);
    return queue.size() == 0;
  }
};

TaskWorker workers[8];
int curWorker = 0;

TaskWorker &getWorker()
{
  int minWorker = 0;
  int minCount = workers[0].size();

  for (int i = 0; i < array_size(workers); i++) {
    if (workers[i].size() < minCount) {
      minWorker = i;
      minCount = workers[i].size();
    }
  }

  printf("using worker %ds\n", minWorker);
  return workers[minWorker];
}

} // namespace detail

template <typename Callback> static void run(Callback cb)
{
  detail::getWorker().push(cb);
}

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
