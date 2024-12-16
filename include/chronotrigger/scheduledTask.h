#ifndef CHRONOTRIGGER_SCHEDULEDTASK_H
#define CHRONOTRIGGER_SCHEDULEDTASK_H

#include <functional>

#include "./types.h"

namespace chronotrigger {

class ScheduledTask {
 public:
  ScheduledTask(TaskID tid,
                std::function<void()> functor,
                TimePoint scheduledTime);

  TimePoint getSheduledTime() const;
  void setScheduledTime(TimePoint time);
  
  TaskID getTaskID() const;

  std::function<void()> getTask();

  void Run() const;

  bool operator<(const ScheduledTask& other) const;

 private:
  TaskID tid;
  std::function<void()> functor;
  TimePoint scheduledTime;
};

}  // namespace chronotrigger

#endif  // CHRONOTRIGGER_SCHEDULEDTASK_H
