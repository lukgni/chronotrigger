#ifndef CHRONOTRIGGER_TASK_H
#define CHRONOTRIGGER_TASK_H

#include "./types.h"

#include <functional>

namespace chronotrigger {
    class Task {
    public:
        enum class TypeT : int {
            FixedRate,
            FixedDelay,
        };

        enum class StatusT : int {
            Scheduled,
            Started,
            Blocked,
            Finished,
        };

        Task(TaskID tid, TypeT type, const std::function<void()> &functor, std::chrono::milliseconds interval);

        ~Task() = default;

        std::function<void()> getFunctor() const;

        StatusT getStatus() const;

        void setStatus(StatusT status, TimePoint time);

        TimePoint getStartedAt() const;

        TimePoint getFinishedAt() const;

        TimePoint getDesiredStartingTime() const;

    private:
        TaskID tid;
        StatusT status = StatusT::Finished;

        TypeT type;
        std::chrono::milliseconds interval;

        TimePoint startedAt;
        TimePoint finishedAt;
        TimePoint changedStatusAt;

        std::function<void()> functor;
    };
}

#endif //CHRONOTRIGGER_TASK_H
