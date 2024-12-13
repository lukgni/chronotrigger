#include "../include/chronotrigger/scheduler.h"

using namespace chronotrigger;

int main() {
  auto p = Scheduler(2);

  p.addFixedRateTask([] {}, std::chrono::milliseconds(500));
  p.addFixedRateTask([] {}, std::chrono::milliseconds(2000));

  p.executeInLoop();
}