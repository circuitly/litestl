#include "task.h"

namespace litestl::task::detail {
TaskWorker workers[LITESTL_WORKERS_COUNT];
int curWorker = 0;
std::recursive_mutex curWorkerMutex = {};
} // namespace litestl::task::detail
