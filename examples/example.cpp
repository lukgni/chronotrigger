#include "../include/chronotrigger/scheduler.h"

using namespace chronotrigger;

int main() {
  auto p = Scheduler(5);

  auto tid0 = p.addFixedRateTask([] {}, std::chrono::milliseconds(50));
  auto tid1 = p.addFixedRateTask([] {}, std::chrono::milliseconds(50));
  auto tid2 = p.addFixedRateTask([] {}, std::chrono::milliseconds(50));
  auto tid3 = p.addFixedRateTask([] {}, std::chrono::milliseconds(50));

  p.addDependency(tid0, tid1);
  p.addDependency(tid1, tid3);
  p.addDependency(tid2, tid3);

  p.executeInLoop();

  return 0;
}