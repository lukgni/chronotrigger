#include "chronotrigger/task.h"

using namespace chronotrigger;

Task::Task(TaskID tid,
           TaskTypeE type,
           const std::function<void()>& functor,
           TimeUnit interval)
    : tid(tid),
      status(TaskStatusE::Initialized),
      type(type),
      interval(interval),
      functor(functor),
      firstRun(true) {
  auto now = TimeClock::now();

  startedAt = now;
  finishedAt = now;
  changedStatusAt = now;
}

std::function<void()> Task::getFunctor() const {
  return functor;
}

TimePoint Task::getStartedAt() const {
  return startedAt;
}

TimePoint Task::getFinishedAt() const {
  return finishedAt;
}

TimePoint Task::getDesiredStartingTime() const {
  if (status == TaskStatusE::Initialized) {
    return TimeClock ::now();
  }

  const TimePoint* timeReference = nullptr;
  switch (type) {
    case TaskTypeE::FixedRate:
      timeReference = &startedAt;
      break;
    case TaskTypeE::FixedDelay:
      timeReference = &finishedAt;
      break;
  }

  if (nullptr == timeReference) {
    return TimeClock::now();
  }

  return *timeReference + interval;
}

TaskStatusE Task::getStatus() const {
  return status;
}

void Task::setStatus(TaskStatusE newStatus, TimePoint time) {
  if (time >= changedStatusAt) {
    status = newStatus;
    changedStatusAt = time;

    switch (status) {
      case TaskStatusE::Finished:
        finishedAt = time;
        break;
      case TaskStatusE::Started:
        firstRun = false;
        startedAt = time;
        break;
      default:
        break;
    }
  }
}