#include "chronotrigger/task.h"

using namespace chronotrigger;

Task::Task(TaskID tid,
           TaskTypeE type,
           const std::function<void()>& functor,
           std::chrono::milliseconds interval)
    : tid(tid), type(type), interval(interval), functor(std::move(functor)) {
  auto now = TimeClock::now();

  startedAt = now;
  finishedAt = now;
  changedStatusAt = now;
}

std::function<void()> Task::getFunctor() const {
  return this->functor;
}

TimePoint Task::getStartedAt() const {
  return this->startedAt;
}

TimePoint Task::getFinishedAt() const {
  return this->finishedAt;
}

TimePoint Task::getDesiredStartingTime() const {
  const TimePoint* timeReference = nullptr;
  switch (this->type) {
    case TaskTypeE::FixedRate:
      timeReference = &this->startedAt;
      break;
    case TaskTypeE::FixedDelay:
      timeReference = &this->finishedAt;
      break;
  }

  if (nullptr == timeReference) {
    return TimeClock::now();
  }

  return *timeReference + this->interval;
}

TaskStatusE Task::getStatus() const {
  return this->status;
}

void Task::setStatus(TaskStatusE status, TimePoint time) {
  if (time >= this->changedStatusAt) {
    this->status = status;
    this->changedStatusAt = time;

    switch (this->status) {
      case TaskStatusE::Finished:
        this->finishedAt = time;
        break;
      case TaskStatusE::Started:
        this->startedAt = time;
        break;
      default:
        break;
    }
  }
}