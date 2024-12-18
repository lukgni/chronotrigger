#include "chronotrigger/workerPool.h"

#include <iomanip>
#include <ostream>

#include "chronotrigger/logger.h"

using namespace chronotrigger;

WorkerPool::WorkerPool(int size)
    : creationTimestamp(TimeClock::now()), keepRunning(true) {
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

TimeDuration WorkerPool::getTimeSinceCreated() const {
  TimePoint timeNow = TimeClock::now();

  return std::chrono::duration_cast<TimeDuration>(timeNow - creationTimestamp);
}

void WorkerPool::submit(const WorkerTask& task) {
  std::lock_guard<std::mutex> lock(workerQueueMtx);

  workerQueue.push(task);
  workersQv.notify_one();
}

void WorkerPool::executeTasksInThread(int workerID) {
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

    auto timeSinceCreated = getTimeSinceCreated();
    auto minutes =
        std::chrono::duration_cast<std::chrono::minutes>(timeSinceCreated);
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(timeSinceCreated) -
        minutes;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                            timeSinceCreated) -
                        seconds - minutes;

    if (ptr) {
      std::ostringstream oss;
      oss << "task_id:" << ptr->tid << " STARTED [worker_id:" << workerID
          << "]";
      logMessage(oss.str());

      ptr->task();

      oss.str("");
      oss.clear();

      oss << "task_id:" << ptr->tid << " FINISHED [worker_id:" << workerID
          << "]";
      logMessage(oss.str());
    }
  }
}

void WorkerPool::logMessage(const std::string& message) const {
  auto timeSinceCreated = getTimeSinceCreated();
  auto minutes =
      std::chrono::duration_cast<std::chrono::minutes>(timeSinceCreated);
  auto seconds =
      std::chrono::duration_cast<std::chrono::seconds>(timeSinceCreated) -
      minutes;
  auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceCreated) -
      seconds - minutes;

  std::ostringstream oss;
  oss << minutes.count() << ":" << std::setw(2) << std::setfill('0')
      << seconds.count() << "." << std::setw(3) << std::setfill('0')
      << milliseconds.count() % 1000 << " * " << message << std::endl;

  Logger::print(oss.str());
}