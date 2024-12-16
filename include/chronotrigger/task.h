#ifndef CHRONOTRIGGER_TASK_H
#define CHRONOTRIGGER_TASK_H

#include <functional>

#include "./types.h"

namespace chronotrigger {

enum class TaskStatusE : int {
  Ready,
  Started,
  Finished,
  Scheduled,
};

enum class TaskTypeE : int {
  FixedRate,
  FixedDelay,
};

class Task {
 public:
  Task(TaskID tid,
       TaskTypeE type,
       const std::function<void()>& functor,
       TimeUnit interval);

  ~Task() = default;

  std::function<void()> getFunctor() const;

  TaskStatusE getStatus() const;

  void setStatus(TaskStatusE status, TimePoint time);

  TimePoint getStartedAt() const;

  TimePoint getFinishedAt() const;

  TimePoint getDesiredStartingTime() const;

 private:
  TaskID tid;
  TaskStatusE status;

  TaskTypeE type;
  TimeUnit interval;

  TimePoint startedAt;
  TimePoint finishedAt;
  TimePoint changedStatusAt;

  std::function<void()> functor;
};

}  // namespace chronotrigger

#endif  // CHRONOTRIGGER_TASK_H
