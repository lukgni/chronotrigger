#include "chronotrigger/scheduler.h"

#include <ostream>
#include <thread>

#include "chronotrigger/exceptions.h"

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

void Scheduler::updateCompletionStatusesInDependenciesStore() {
  for (auto& [tid, task] : taskLookupTable) {
    if (task->getStatus() == TaskStatusE::Finished) {
      tasksDependencies.markTaskAsFinished(tid);
    }
  }
}

void Scheduler::prepareExecutionPlan() {
  for (auto tid : tasksDependencies.getTaskIdsTopollogicalySorted()) {
    if (taskLookupTable[tid]->getStatus() != TaskStatusE::Finished &&
        taskLookupTable[tid]->getStatus() != TaskStatusE::Initialized) {
      continue;
    }

    enqueueScheduledTask(
        ScheduledTask(tid, taskLookupTable[tid]->getFunctor(),
                      taskLookupTable[tid]->getDesiredStartingTime()));

    taskLookupTable[tid]->setStatus(TaskStatusE::Planned, TimeClock::now());
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

void Scheduler::enqueueBlockedScheduledTask(const ScheduledTask& task) {
  std::lock_guard lock(blockedScheduledTasksQueueMtx);

  blockedScheduledTasksQueue.push(task);
}

std::unique_ptr<ScheduledTask> Scheduler::dequeueBlockedScheduledTaskIfTime(
    TimePoint time) {
  std::lock_guard lock(blockedScheduledTasksQueueMtx);

  if (blockedScheduledTasksQueue.empty() ||
      blockedScheduledTasksQueue.top().getSheduledTime() > time) {
    return nullptr;
  }

  auto ptr = std::make_unique<ScheduledTask>(blockedScheduledTasksQueue.top());
  blockedScheduledTasksQueue.pop();
  return ptr;
}

void Scheduler::enqueueExecutionStatusEvent(
    const ExecutionStatusEvent&& event) {
  std::lock_guard lock(execuctionStatQueueMtx);

  executionStatsQueue.emplace(event);
}

std::unique_ptr<ExecutionStatusEvent> Scheduler::dequeueExecutionStatusEvent() {
  std::lock_guard lock(execuctionStatQueueMtx);

  if (executionStatsQueue.empty()) {
    return nullptr;
  }

  auto ptr =
      std::make_unique<ExecutionStatusEvent>(executionStatsQueue.front());
  executionStatsQueue.pop();
  return ptr;
}

void Scheduler::processBlockedPlannedTasks(
    std::chrono::milliseconds postponingInterval) {
  while (auto ptr = dequeueBlockedScheduledTaskIfTime(TimeClock::now())) {
    if (tasksDependencies.isTaskBlocked(ptr->getTaskID())) {
      ptr->setScheduledTime(TimeClock ::now() + postponingInterval);
      enqueueBlockedScheduledTask(*ptr);
      continue;
    }

    taskLookupTable[ptr->getTaskID()]->setStatus(TaskStatusE::Scheduled,
                                                 TimeClock::now());
    if (taskLookupTable[ptr->getTaskID()]->isFirstRun() == false) {
      tasksDependencies.markTaskAsRunning(ptr->getTaskID());
    }
    workerPool.submit(WorkerTask(ptr->getTaskID(), [this, task = *ptr] {
      executeScheduledTask(std::move(task));
    }));
  }
}

void Scheduler::processPlannedTasks() {
  while (auto ptr = dequeueScheduledTaskIfTime(TimeClock::now())) {
    if (tasksDependencies.isTaskBlocked(ptr->getTaskID())) {
      enqueueBlockedScheduledTask(*ptr);
      continue;
    }

    taskLookupTable[ptr->getTaskID()]->setStatus(TaskStatusE::Scheduled,
                                                 TimeClock::now());
    if (taskLookupTable[ptr->getTaskID()]->isFirstRun() == false) {
      tasksDependencies.markTaskAsRunning(ptr->getTaskID());
    }
    workerPool.submit(WorkerTask(ptr->getTaskID(), [this, task = *ptr] {
      executeScheduledTask(std::move(task));
    }));
  }
}

void Scheduler::execute(std::chrono::milliseconds postponingInterval) {
  processQueuedExecutionStatuses();
  updateCompletionStatusesInDependenciesStore();
  prepareExecutionPlan();
  processBlockedPlannedTasks(postponingInterval);
  processPlannedTasks();
}

[[noreturn]] void Scheduler::executeInLoop(std::chrono::milliseconds interval) {
  while (true) {
    auto executionStartTime = TimeClock::now();

    execute(interval);

    auto executionEndTime = TimeClock::now();
    auto consumedPartOfTimeInterval = std::chrono::duration_cast<TimeUnit>(
        executionEndTime - executionStartTime);

    auto timeToSleep =
        std::max(TimeUnit(0), std::chrono::duration_cast<TimeUnit>(interval) -
                                  consumedPartOfTimeInterval);

    std::this_thread::sleep_for(timeToSleep);
  }
}

TaskID Scheduler::addFixedRateTask(const std::function<void()>& functor,
                                   std::chrono::milliseconds interval) {
  return addTask(TaskTypeE::FixedRate, functor,
                 std::chrono::duration_cast<TimeUnit>(interval));
}

TaskID Scheduler::addFixedDelayTask(const std::function<void()>& functor,
                                    std::chrono::milliseconds interval) {
  return addTask(TaskTypeE::FixedDelay, functor,
                 std::chrono::duration_cast<TimeUnit>(interval));
}

TaskID Scheduler::addTask(TaskTypeE type,
                          const std::function<void()>& functor,
                          TimeUnit interval) {
  auto tid = getNewTaskID();

  taskLookupTable[tid] =
      std::make_shared<Task>(Task(tid, type, functor, interval));
  tasksDependencies.registerTask(tid);

  return tid;
}

void Scheduler::addDependency(TaskID dependentTaskID, TaskID dependencyTaskID) {
  addDependencies(dependentTaskID, std::vector<TaskID>{dependencyTaskID});
}

void Scheduler::addDependencies(TaskID dependentTaskID,
                                const std::vector<TaskID>& dependencyTaskIDs) {
  auto taskExistenceCheckOk = true;
  TaskID notFound = -1;
  if (taskLookupTable.find(dependentTaskID) == taskLookupTable.end()) {
    taskExistenceCheckOk = false;
    notFound = dependentTaskID;
  } else {
    for (auto d : dependencyTaskIDs) {
      if (taskLookupTable.find(d) == taskLookupTable.end()) {
        notFound = d;
        taskExistenceCheckOk = false;
        break;
      }
    }
  }

  if (!taskExistenceCheckOk) {
    std::ostringstream oss;
    oss << "Task has been not found: " << notFound;
    throw TaskNotFoundException(oss.str());
  }

  auto opt = tasksDependencies.tryAddDependenciesOrReturnCycle(
      dependentTaskID, dependencyTaskIDs);

  if (opt.has_value()) {
    std::ostringstream oss;
    oss << "Adding dependency result in tasks relation cycle: ";
    for (auto tid : opt.value()) {
      oss << tid << " ";
    }
    throw TaskCycleException(oss.str());
  }
}