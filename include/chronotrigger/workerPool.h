#ifndef CHRONOTRIGGER_WORKERPOOL_H
#define CHRONOTRIGGER_WORKERPOOL_H

#include <atomic>
#include <functional>
#include <queue>
#include <thread>

#include "./types.h"

namespace chronotrigger {

struct WorkerTask {
  WorkerTask(TaskID tid, std::function<void()>&& task)
      : tid(tid), task(std::move(task)) {}
  TaskID tid;
  std::function<void()> task;
};

class WorkerPool {
 public:
  WorkerPool(int size);
  ~WorkerPool();

  void submit(const WorkerTask& task);

 private:
  TimeDuration getTimeSinceCreated() const;
  void executeTasksInThread(int workerID);

  const TimePoint creationTimestamp;

  std::vector<std::thread> threadPool;

  std::queue<WorkerTask> workerQueue;
  std::mutex workerQueueMtx;

  std::condition_variable workersQv;

  std::atomic<bool> keepRunning;
};

}  // namespace chronotrigger

#endif  // CHRONOTRIGGER_WORKERPOOL_H
