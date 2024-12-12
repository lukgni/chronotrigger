#ifndef CHRONOTRIGGER_SCHEDULEDTASK_H
#define CHRONOTRIGGER_SCHEDULEDTASK_H

#include <functional>

#include "./types.h"

namespace chronotrigger {

class ScheduledTask {
 public:
  ScheduledTask(TaskID tid,
                std::function<void()> functor,
                TimePoint scheduledTime)
      : tid(tid), functor(std::move(functor)), scheduledTime(scheduledTime) {}

  TimePoint getSheduledTime() const { return this->scheduledTime; }

  TaskID getTaskID() const { return this->tid; }

  void Run() { this->functor(); }

  bool operator<(const ScheduledTask& other) const {
    return this->scheduledTime > other.scheduledTime;
  }

 private:
  TaskID tid;
  std::function<void()> functor;
  TimePoint scheduledTime;
};

}  // namespace chronotrigger
#endif  // CHRONOTRIGGER_SCHEDULEDTASK_H
