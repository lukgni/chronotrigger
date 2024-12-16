#include "chronotrigger/scheduledTask.h"

using namespace chronotrigger;

ScheduledTask::ScheduledTask(TaskID tid,
                             std::function<void()> functor,
                             TimePoint scheduledTime)
    : tid(tid), functor(std::move(functor)), scheduledTime(scheduledTime) {}

TimePoint ScheduledTask::getSheduledTime() const {
  return scheduledTime;
}

TaskID ScheduledTask::getTaskID() const {
  return tid;
}

std::function<void()> ScheduledTask::getTask() {
  return std::move(functor);
}

void ScheduledTask::Run() const {
  functor();
}

bool ScheduledTask::operator<(const ScheduledTask& other) const {
  return scheduledTime > other.scheduledTime;
}