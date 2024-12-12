#include "../include/chronotrigger/scheduler.h"

#include <iostream>

using namespace chronotrigger;

int main() {
    auto p = Scheduler(2, chronotrigger::Scheduler::TickIntervalsE::Interval_100ms);

    std:: cout << p.addFixedRateTask([]{std::cout << "1\n";}, std::chrono::milliseconds (200)) << std::endl;
    std:: cout << p.addFixedRateTask([]{std::cout << "2\n";}, std::chrono::milliseconds (2000)) << std::endl;
    p.execute();
    return 0;
}