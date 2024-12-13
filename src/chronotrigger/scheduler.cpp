#include "chronotrigger/scheduler.h"

#include <iostream>
#include <thread>

using namespace chronotrigger;

Scheduler::Scheduler(int workerPoolSize) : workerPool(workerPoolSize) {}

TaskID Scheduler::getNewTaskID() {
  static TaskID taskId = 0;
  return taskId++;
}

void Scheduler::processQueuedExecutionStatuses() {
  while (auto ptr = dequeueExecutionStatusEvent()) {
    if (auto it = taskLookupTable.find(ptr->tid); it != taskLookupTable.end()) {
      it->second->setStatus(ptr->taskStatus, ptr->changedAt);
    }
  }
}

void Scheduler::prepareExecutionPlan() {
  for (const auto& [taskID, taskPtr] : taskLookupTable) {
    if (taskPtr->getStatus() != TaskStatusE::Finished) {
      continue;
    }

    enqueueScheduledTask(ScheduledTask(taskID, taskPtr->getFunctor(),
                                       taskPtr->getDesiredStartingTime()));

    taskPtr->setStatus(TaskStatusE::Scheduled, TimeClock::now());
  }
}

void Scheduler::executeScheduledTask(const ScheduledTask&& task) {
  enqueueExecutionStatusEvent(ExecutionStatusEvent(
      task.getTaskID(), TaskStatusE::Started, TimeClock::now()));

  task.Run();

  enqueueExecutionStatusEvent(ExecutionStatusEvent(
      task.getTaskID(), TaskStatusE::Finished, TimeClock::now()));
}

void Scheduler::enqueueScheduledTask(const ScheduledTask& task) {
  std::lock_guard lock(scheduledTasksQueueMtx);

  scheduledTasksQueue.push(task);
}

std::unique_ptr<ScheduledTask> Scheduler::dequeueScheduledTaskIfTime(
    TimePoint time) {
  std::lock_guard lock(scheduledTasksQueueMtx);

  if (scheduledTasksQueue.empty() ||
      scheduledTasksQueue.top().getSheduledTime() > time) {
    return nullptr;
  }

  auto ptr = std::make_unique<ScheduledTask>(scheduledTasksQueue.top());
  scheduledTasksQueue.pop();
  return ptr;
}

void Scheduler::enqueueExecutionStatusEvent(
    const ExecutionStatusEvent&& event) {
  std::lock_guard lock(execuctionStatQueueMtx);

  executionStasQueue.emplace(event);
}

std::unique_ptr<ExecutionStatusEvent> Scheduler::dequeueExecutionStatusEvent() {
  std::lock_guard lock(execuctionStatQueueMtx);

  if (executionStasQueue.empty()) {
    return nullptr;
  }

  auto ptr = std::make_unique<ExecutionStatusEvent>(executionStasQueue.front());
  executionStasQueue.pop();
  return ptr;
}

void Scheduler::execute() {
  processQueuedExecutionStatuses();
  prepareExecutionPlan();

  while (auto ptr = dequeueScheduledTaskIfTime(TimeClock::now())) {
    workerPool.submit(WorkerTask(ptr->getTaskID(), [this, task = *ptr] {
      executeScheduledTask(std::move(task));
    }));
  }
}

[[noreturn]] void Scheduler::executeInLoop(std::chrono::milliseconds interval) {
  while (true) {
    auto executionStartTime = chronotrigger::TimeClock::now();

    processQueuedExecutionStatuses();
    prepareExecutionPlan();

    while (auto ptr = dequeueScheduledTaskIfTime(TimeClock::now())) {
      workerPool.submit(WorkerTask(ptr->getTaskID(), [this, task = *ptr] {
        executeScheduledTask(std::move(task));
      }));
    }

    auto executionEndTime = TimeClock::now();
    auto consumedPartOfTimeInterval =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            executionEndTime - executionStartTime);

    auto timeToSleep = std::max(std::chrono::milliseconds(0),
                                interval - consumedPartOfTimeInterval);

    std::this_thread::sleep_for(timeToSleep);
  }
}

TaskID Scheduler::addFixedRateTask(const std::function<void()>& functor,
                                   std::chrono::milliseconds interval) {
  return addTask(TaskTypeE::FixedRate, functor, interval);
}

TaskID Scheduler::addFixedDelayTask(const std::function<void()>& functor,
                                    std::chrono::milliseconds interval) {
  return addTask(TaskTypeE::FixedDelay, functor, interval);
}

TaskID Scheduler::addTask(TaskTypeE type,
                          const std::function<void()>& functor,
                          std::chrono::milliseconds interval) {
  auto tid = getNewTaskID();

  taskLookupTable[tid] =
      std::make_unique<Task>(Task(tid, type, functor, interval));

  return tid;
}

void Scheduler::addDependency(TaskID target, TaskID dependency) {
  // add circular dependency detection
  taskDependencies[target].insert(dependency);
}
