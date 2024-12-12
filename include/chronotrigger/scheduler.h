#ifndef CHRONOTRIGGER_SCHEDULER_H
#define CHRONOTRIGGER_SCHEDULER_H

#include <forward_list>
#include <functional>
#include <map>
#include <queue>
#include <tuple>
#include <unordered_set>

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

  TaskID addFixedRateTask(std::function<void()> functor,
                          std::chrono::milliseconds interval);

  TaskID addFixedDelayTask(std::function<void()> functor,
                           std::chrono::milliseconds interval);

  void addDependency(TaskID target, TaskID dependency);

  void addDependency(TaskID target, std::vector<TaskID> dependencies);

  void execute();

 private:
  TaskID addTask(Task::TypeT type,
                 std::function<void()> functor,
                 std::chrono::milliseconds interval);

  void calculateExecutionPlan();

  TaskID getNewTaskID();

  int threadPoolSize;
  std::chrono::milliseconds timerInterval;

  std::map<TaskID, std::unique_ptr<Task>> taskLookupTable;
  std::unordered_map<TaskID, std::unordered_set<TaskID>> taskDependencies;

  std::priority_queue<ScheduledTask> plannedTasksQueue;

  using TaskExecutionStatus = std::tuple<TaskID, Task::StatusT, TimePoint>;
  std::queue<TaskExecutionStatus> executionStatusQueue;
};
}  // namespace chronotrigger
#endif  // CHRONOTRIGGER_SCHEDULER_H
