#ifndef CHRONOTRIGGER_SCHEDULER_H
#define CHRONOTRIGGER_SCHEDULER_H

#include <forward_list>
#include <functional>
#include <map>
#include <queue>
#include <tuple>
#include <unordered_set>

#include "./executionStatusEvent.h"
#include "./scheduledTask.h"
#include "./task.h"
#include "./taskDependenciesStore.h"
#include "./types.h"
#include "./workerPool.h"

namespace chronotrigger {

class Scheduler {
 public:
  Scheduler(int workerPoolSize);

  TaskID addFixedRateTask(const std::function<void()>& functor,
                          std::chrono::milliseconds interval);

  TaskID addFixedDelayTask(const std::function<void()>& functor,
                           std::chrono::milliseconds interval);

  void addDependency(TaskID dependentTaskID, TaskID dependencyTaskID);

  void addDependencies(TaskID dependentTaskId,
                       const std::vector<TaskID>& dependencyTaskIDs);

  void execute(std::chrono::milliseconds postponingInterval);

  [[noreturn]] void executeInLoop(
      std::chrono::milliseconds tickInterval = std::chrono::milliseconds(4));

 private:
  TaskID addTask(TaskTypeE type,
                 const std::function<void()>& functor,
                 TimeUnit interval);

  void processQueuedExecutionStatuses();

  void updateCompletionStatusesInDependenciesStore();

  void prepareExecutionPlan();

  void processPlannedTasks();

  void processBlockedPlannedTasks(std::chrono::milliseconds postponingInterval);

  void executeScheduledTask(const ScheduledTask&& task);

  void enqueueScheduledTask(const ScheduledTask& task);
  std::unique_ptr<ScheduledTask> dequeueScheduledTaskIfTime(TimePoint time);

  void enqueueBlockedScheduledTask(const ScheduledTask& task);
  std::unique_ptr<ScheduledTask> dequeueBlockedScheduledTaskIfTime(
      TimePoint time);

  void enqueueExecutionStatusEvent(const ExecutionStatusEvent&& event);
  std::unique_ptr<ExecutionStatusEvent> dequeueExecutionStatusEvent();

  static TaskID getNewTaskID();

  std::map<TaskID, std::shared_ptr<Task>> taskLookupTable;
  TaskDependenciesStore tasksDependencies;

  // TODO: Encapsulate into separate class
  std::priority_queue<ScheduledTask> scheduledTasksQueue;
  std::mutex scheduledTasksQueueMtx;
  std::priority_queue<ScheduledTask> blockedScheduledTasksQueue;
  std::mutex blockedScheduledTasksQueueMtx;

  // TODO: Encapsulate into separate class
  std::queue<ExecutionStatusEvent> executionStatsQueue;
  std::mutex execuctionStatQueueMtx;

  WorkerPool workerPool;
};

}  // namespace chronotrigger
#endif  // CHRONOTRIGGER_SCHEDULER_H
