#include "../../include/chronotrigger/workerPool.h"

#include <iostream>

using namespace chronotrigger;

WorkerPool::WorkerPool(int size) : keepRunning(true) {
  for (auto i = 0; i < size; i++) {
    threadPool.emplace_back([=] { executeTasksInThread(i); });
  }
}

WorkerPool::~WorkerPool() {
  keepRunning = false;
  workersQv.notify_all();

  for (auto& t : threadPool) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void WorkerPool::submit(const WorkerTask& task) {
  std::lock_guard<std::mutex> lock(workerQueueMtx);

  workerQueue.push(task);
  workersQv.notify_one();
}

void WorkerPool::executeTasksInThread(int workerID) {
  std::cout << "Worker " << workerID << " BONJOUR " << std::endl;
  while (true) {
    std::unique_ptr<WorkerTask> ptr = nullptr;
    {
      std::unique_lock<std::mutex> lock(workerQueueMtx);
      workersQv.wait(lock,
                     [&] { return !keepRunning || !workerQueue.empty(); });

      if (!keepRunning) {
        break;
      }

      if (!workerQueue.empty()) {
        ptr = std::make_unique<WorkerTask>(workerQueue.front());
        workerQueue.pop();
      } else {
        ptr = nullptr;
      }
    }

    if (ptr) {
      std::cout << "worker_id " << workerID << " - task_id: " << ptr->tid
                << std::endl;
      ptr->task();
    }
  }
}