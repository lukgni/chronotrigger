#include "../include/chronotrigger/scheduler.h"

using namespace chronotrigger;

int main() {
    auto p = Scheduler(2, chronotrigger::Scheduler::TickIntervalsE::Interval_040ms);

    p.addFixedRateTask([]{}, std::chrono::milliseconds (500));
    p.addFixedRateTask([]{}, std::chrono::milliseconds (2000));
    p.execute();
    return 0;
}