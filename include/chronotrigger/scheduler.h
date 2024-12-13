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
#include "./types.h"

namespace chronotrigger {

class Scheduler {
 public:
  enum class TickIntervalsE {
    Interval_004ms = 4,
    Interval_040ms = 40,
    Interval_100ms = 100,
    Interval_500ms = 500,
  };

  Scheduler(int threadPoolSize, TickIntervalsE tickInterval);

  TaskID addFixedRateTask(const std::function<void()>& functor,
                          std::chrono::milliseconds interval);

  TaskID addFixedDelayTask(const std::function<void()>& functor,
                           std::chrono::milliseconds interval);

  void addDependency(TaskID target, TaskID dependency);

  void addDependency(TaskID target, std::vector<TaskID> dependencies);

  [[noreturn]] void execute();

 private:
  TaskID addTask(TaskTypeE type,
                 const std::function<void()>& functor,
                 std::chrono::milliseconds interval);

  void processQueuedExecutionStatuses();

  void prepareExecutionPlan();

  void executeScheduledTask(std::unique_ptr<ScheduledTask> task);

  void executeScheduledTasks();

  void enqueueScheduledTask(const ScheduledTask& task);
  std::unique_ptr<ScheduledTask> dequeueScheduledTaskIfTime(TimePoint time);

  void enqueueExecutionStatusEvent(const ExecutionStatusEvent& event);
  std::unique_ptr<ExecutionStatusEvent> dequeueExecutionStatusEvent();

  static TaskID getNewTaskID();

  int threadPoolSize;
  std::chrono::milliseconds timerInterval;

  std::map<TaskID, std::unique_ptr<Task>> taskLookupTable;
  std::unordered_map<TaskID, std::unordered_set<TaskID>> taskDependencies;

  // TODO: Encapsulate in separate class together with associated setter/getter
  std::priority_queue<ScheduledTask> scheduledTasksQueue;
  std::mutex scheduledTasksQueueMtx;

  // TODO: Encapsulate in separate class together with associated setter/getter
  std::queue<ExecutionStatusEvent> executionStasQueue;
  std::mutex execuctionStatQueueMtx;
};

}  // namespace chronotrigger
#endif  // CHRONOTRIGGER_SCHEDULER_H
