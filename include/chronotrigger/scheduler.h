#ifndef CHRONOTRIGGER_SCHEDULER_H
#define CHRONOTRIGGER_SCHEDULER_H
#include "./types.h"
#include "./task.h"
#include "./scheduledTask.h"

#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <forward_list>
#include <functional>
#include <tuple>

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

        TaskID addFixedRateTask(std::function<void()> functor, std::chrono::milliseconds interval);

        TaskID addFixedDelayTask(std::function<void()> functor, std::chrono::milliseconds interval);

        void addDependency(TaskID target, TaskID dependency);

        void addDependency(TaskID target, std::vector<TaskID> dependencies);

        void execute();

    private:
        TaskID addTask(Task::TypeT type, std::function<void()> functor, std::chrono::milliseconds interval);

        void calculateExecutionPlan();

        TaskID getNewTaskID();

        int threadPoolSize;
        std::chrono::milliseconds timerInterval;

        // consider changing to unordered_set if TaskID will be not used in Task
       std::unordered_map<TaskID, std::unique_ptr<Task>> taskLookupTable;
       std::unordered_map<TaskID, std::unordered_set<TaskID>> taskDependencies;

       std::priority_queue<ScheduledTask> plannedTasksQueue; // oddzielny mutex

       using TaskExecutionStatus = std::tuple<TaskID, Task::StatusT, TimePoint>;
       std::queue<TaskExecutionStatus>  executionStatusQueue; // oddzielny mutex
    };
}
#endif //CHRONOTRIGGER_SCHEDULER_H
