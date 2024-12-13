#include "chronotrigger/scheduler.h"

#include <iostream>
#include <thread>

using namespace chronotrigger;

Scheduler::Scheduler(
    int threadPoolSize,
    TickIntervalsE tickInterval = TickIntervalsE::Interval_040ms)
    : timerInterval(std::chrono::milliseconds(static_cast<int>(tickInterval))) {
}

TaskID Scheduler::getNewTaskID() {
  static TaskID taskId = 0;
  return taskId++;
}

// add mutex here
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

    this->enqueueScheduledTask(ScheduledTask(
        taskID, taskPtr->getFunctor(), taskPtr->getDesiredStartingTime()));

    taskPtr->setStatus(TaskStatusE::Scheduled, TimeClock::now());
  }
}

void Scheduler::executeScheduledTask(std::unique_ptr<ScheduledTask> task) {
  this->enqueueExecutionStatusEvent(ExecutionStatusEvent(
      task->getTaskID(), TaskStatusE::Started, TimeClock::now()));

  task->Run();

  this->enqueueExecutionStatusEvent(ExecutionStatusEvent(
      task->getTaskID(), TaskStatusE::Finished, TimeClock::now()));
}

void Scheduler::enqueueScheduledTask(const ScheduledTask& task) {
  std::lock_guard lock(this->scheduledTasksQueueMtx);

  this->scheduledTasksQueue.push(task);
}

std::unique_ptr<ScheduledTask> Scheduler::dequeueScheduledTaskIfTime(
    TimePoint time) {
  std::lock_guard lock(this->scheduledTasksQueueMtx);

  if (this->scheduledTasksQueue.empty() ||
      this->scheduledTasksQueue.top().getSheduledTime() > time) {
    return nullptr;
  }

  auto ptr = std::make_unique<ScheduledTask>(this->scheduledTasksQueue.top());
  this->scheduledTasksQueue.pop();
  return ptr;
}

void Scheduler::enqueueExecutionStatusEvent(const ExecutionStatusEvent& event) {
  std::lock_guard lock(this->execuctionStatQueueMtx);

  this->executionStasQueue.emplace(event);
}

std::unique_ptr<ExecutionStatusEvent> Scheduler::dequeueExecutionStatusEvent() {
  std::lock_guard lock(this->execuctionStatQueueMtx);

  if (this->executionStasQueue.empty()) {
    return nullptr;
  }

  auto ptr =
      std::make_unique<ExecutionStatusEvent>(this->executionStasQueue.front());
  this->executionStasQueue.pop();
  return ptr;
}

[[noreturn]] void Scheduler::execute() {
  while (true) {
    auto executionStartTime = chronotrigger::TimeClock::now();

    this->processQueuedExecutionStatuses();
    this->prepareExecutionPlan();

    while (auto ptr = this->dequeueScheduledTaskIfTime(TimeClock::now())) {
      std::cout << "TaskID: " << ptr->getTaskID() << std::endl;
      this->executeScheduledTask(std::move(ptr));
    }

    auto executionEndTime = TimeClock::now();
    auto consumedSlotTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            executionEndTime - executionStartTime);

    auto timeToSleep = std::max(std::chrono::milliseconds(0),
                                this->timerInterval - consumedSlotTime);

    std::this_thread::sleep_for(timeToSleep);
  }
}

TaskID Scheduler::addFixedRateTask(const std::function<void()>& functor,
                                   std::chrono::milliseconds interval) {
  return this->addTask(TaskTypeE::FixedRate, functor, interval);
}

TaskID Scheduler::addFixedDelayTask(const std::function<void()>& functor,
                                    std::chrono::milliseconds interval) {
  return this->addTask(TaskTypeE::FixedDelay, functor, interval);
}

TaskID Scheduler::addTask(TaskTypeE type,
                          const std::function<void()>& functor,
                          std::chrono::milliseconds interval) {
  auto tid = this->getNewTaskID();

  this->taskLookupTable[tid] =
      std::make_unique<Task>(Task(tid, type, functor, interval));

  return tid;
}

void Scheduler::addDependency(TaskID target, TaskID dependency) {
  // add circular dependency detection
  this->taskDependencies[target].insert(dependency);
}
