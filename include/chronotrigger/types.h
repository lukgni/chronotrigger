#ifndef CHRONOTRIGGER_TYPES_H
#define CHRONOTRIGGER_TYPES_H

#include <chrono>

namespace chronotrigger {
    using TimeClock = std::chrono::system_clock;
    using TimePoint = TimeClock::time_point;

    using TaskID = int;
}
#endif //CHRONOTRIGGER_TYPES_H
