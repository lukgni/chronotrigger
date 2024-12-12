#ifndef CHRONOTRIGGER_EXECUTIONSTATUSEVENT_H
#define CHRONOTRIGGER_EXECUTIONSTATUSEVENT_H

#include "./types.h"

namespace chronotrigger {

enum class TaskStatusE : int;

struct ExecutionStatusEvent {
  ExecutionStatusEvent(TaskID tid, TaskStatusE status, TimePoint time)
      : tid(tid), taskStatus(status), changedAt(time) {}

  TaskID tid;
  TaskStatusE taskStatus;
  TimePoint changedAt;
};

}  // namespace chronotrigger
#endif  // CHRONOTRIGGER_EXECUTIONSTATUSEVENT_H
