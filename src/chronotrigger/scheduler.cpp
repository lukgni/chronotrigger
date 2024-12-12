#include "chronotrigger/scheduler.h"

#include <iostream>
#include <thread>

using namespace chronotrigger;

Scheduler::Scheduler(
    int threadPoolSize,
    TickIntervalsE tickInterval = TickIntervalsE::Interval_040ms)
    : timerInterval(std::chrono::milliseconds(static_cast<int>(tickInterval))) {
}

TaskID Scheduler::getNewTaskID() {
  static TaskID taskId = 0;
  return taskId++;
}

void Scheduler::execute() {
  while (true) {
    auto processingStartTime = chronotrigger::TimeClock::now();

    // process all pending statuses
    while (this->executionStatusQueue.empty() == false) {
      auto statusTuple = this->executionStatusQueue.front();
      this->executionStatusQueue.pop();

      if (auto it = taskLookupTable.find(std::get<0>(statusTuple));
          it != taskLookupTable.end()) {
        it->second->setStatus(std::get<1>(statusTuple),
                              std::get<2>(statusTuple));
      }
    }

    // prepare execution plan
    for (const auto& [taskID, taskPtr] : taskLookupTable) {
      if (taskPtr->getStatus() != Task::StatusT::Finished) {
        continue;
      }

      this->plannedTasksQueue.push(ScheduledTask(
          taskID, taskPtr->getFunctor(), taskPtr->getDesiredStartingTime()));
      taskPtr->setStatus(Task::StatusT::Scheduled, TimeClock::now());
    }

    // Execution phase
    while (this->plannedTasksQueue.empty() == false) {
      auto task = this->plannedTasksQueue.top();
      if (task.getSheduledTime() > TimeClock::now()) {
        break;
      }
      std::cout << "TaskID: " << task.getTaskID() << std::endl;

      this->plannedTasksQueue.pop();

      this->executionStatusQueue.push(TaskExecutionStatus(
          task.getTaskID(), Task::StatusT::Started, TimeClock::now()));
      task.Run();
      this->executionStatusQueue.push(TaskExecutionStatus(
          task.getTaskID(), Task::StatusT::Finished, TimeClock::now()));
    }

    auto processingEndTime = TimeClock::now();
    auto consumedSlotTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            processingEndTime - processingStartTime);

    auto timeToSleep = std::max(std::chrono::milliseconds(0),
                                this->timerInterval - consumedSlotTime);
    std::this_thread::sleep_for(timeToSleep);
  }
}

TaskID Scheduler::addFixedRateTask(std::function<void()> functor,
                                   std::chrono::milliseconds interval) {
  return this->addTask(Task::TypeT::FixedRate, std::move(functor), interval);
}

TaskID Scheduler::addFixedDelayTask(std::function<void()> functor,
                                    std::chrono::milliseconds interval) {
  return this->addTask(Task::TypeT::FixedDelay, std::move(functor), interval);
}

TaskID Scheduler::addTask(Task::TypeT type,
                          std::function<void()> functor,
                          std::chrono::milliseconds interval) {
  auto tid = this->getNewTaskID();

  this->taskLookupTable[tid] =
      std::make_unique<Task>(Task(tid, type, std::move(functor), interval));

  return tid;
}

void Scheduler::addDependency(TaskID target, TaskID dependency) {
  // add circular dependency detection
  this->taskDependencies[target].insert(dependency);
}
