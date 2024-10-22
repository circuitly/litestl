#pragma once

#ifndef LITESTL_WORKERS_COUNT
#define LITESTL_WORKERS_COUNT 6
#endif

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
  bool stop_ = false;

  TaskWorker()
  {
  }

  void stop()
  {
    stop_ = true;
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

  void run()
  {
    using namespace std::chrono_literals;

    while (1) {
      while (!this->empty()) {
        ThreadMain main = this->pop();
        main();
      }

      int size = 0;
      {
        std::lock_guard guard(mutex);
        size = queue.size();
        if (stop_ && !size) {
          break;
        }
      }

      if (size == 0) {
        std::unique_lock lock(wait_mutex);
        const std::chrono::time_point<std::chrono::steady_clock> start =
            std::chrono::steady_clock::now();

        cv.wait_until(lock, start + 2ms);
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
    bool notify = false;

    {
      std::lock_guard guard(mutex);
      queue.append(main);
      if (queue.size() == 1) {
        notify = true;
      }
    }

    if (running && notify) {
      cv.notify_all();
    }

    {
      std::lock_guard guard(mutex);
      if (!running) {
        start();
      }
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

extern TaskWorker workers[LITESTL_WORKERS_COUNT];
extern int curWorker;
extern std::recursive_mutex curWorkerMutex;

static TaskWorker &getWorker()
{
#if 0
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
#else
  std::lock_guard guard(curWorkerMutex);

  int worker = curWorker;
  // printf("using worker %d (with %d outstanding tasks)\n", worker,
  // workers[worker].size());
  curWorker = (curWorker + 1) % array_size(workers);
  return workers[worker];
#endif
}

} // namespace detail

template <typename Callback> static void run(Callback cb)
{
#if 0
   auto *thread = new std::thread(cb);
   thread->detach();
   delete thread;
#else
  detail::getWorker().push(cb);
#endif
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
