#include "../include/chronotrigger/scheduler.h"

using namespace chronotrigger;

int main() {
  auto p = Scheduler(6);

  auto tid0 = p.addFixedRateTask([] {}, std::chrono::milliseconds(50));
  auto tid1 = p.addFixedRateTask([] {}, std::chrono::milliseconds(50));
  auto tid2 = p.addFixedRateTask([] {}, std::chrono::milliseconds(50));
  auto tid3 = p.addFixedRateTask([] {}, std::chrono::milliseconds(50));
  auto tid4 = p.addFixedRateTask([] {}, std::chrono::milliseconds(25));

  p.addDependency(tid0, tid1);
  p.addDependency(tid1, tid2);
  p.addDependency(tid2, tid3);

  p.executeInLoop();

  return 0;
}