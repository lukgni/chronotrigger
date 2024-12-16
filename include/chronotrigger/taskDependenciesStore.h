#ifndef CHRONOTRIGGER_TASKDEPENDENCIES_H
#define CHRONOTRIGGER_TASKDEPENDENCIES_H

#include <optional>
#include <unordered_set>

#include "./types.h"

namespace chronotrigger {

class TaskDependenciesStore {
 public:
  std::optional<std::vector<TaskID>> tryAddDependenciesOrReturnCycle(
      TaskID dependent,
      const std::vector<TaskID>& dependencies);

  void registerTask(TaskID);

  std::vector<TaskID> getTaskIDsBlockedBy(TaskID tid) const;
  std::vector<TaskID> getTaskIDsBlocking(TaskID tid) const;

  std::vector<TaskID> getTaskIdsTopollogicalySorted() const;

 private:
  static std::pair<std::vector<TaskID>, std::vector<TaskID>>
  sortTaskIDsTopologicallyOrReturnCycleIfDetected(
      const std::unordered_map<TaskID, std::unordered_set<TaskID>>& blocking,
      const std::unordered_map<TaskID, std::unordered_set<TaskID>>& blockedBy);

  std::unordered_map<TaskID, std::unordered_set<TaskID>> blocking;
  std::unordered_map<TaskID, std::unordered_set<TaskID>> blockedBy;

  std::vector<TaskID> topollogicalySorted;
};

}  // namespace chronotrigger

#endif  // CHRONOTRIGGER_TASKDEPENDENCIES_H
