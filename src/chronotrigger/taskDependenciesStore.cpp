#include "chronotrigger/taskDependenciesStore.h"

#include <queue>

using namespace chronotrigger;

void TaskDependenciesStore::registerTask(TaskID tid) {
  auto it = blocking.find(tid);
  if (it == blocking.end()) {
    blocking[tid] = std::unordered_set<TaskID>();
  }

  it = blockedBy.find(tid);
  if (it == blockedBy.end()) {
    blockedBy[tid] = std::unordered_set<TaskID>();
  }

  auto itd = inDegreesTracker.find(tid);
  if (itd == inDegreesTracker.end()) {
    inDegreesTracker[tid] = 0;
  }
}

void TaskDependenciesStore::markTaskAsRunning(TaskID tid) {
  for (auto tid : blockedBy[tid]) {
    inDegreesTracker[tid]++;
  }
}

void TaskDependenciesStore::markTaskAsFinished(TaskID tid) {
  for (auto tid : blockedBy[tid]) {
    inDegreesTracker[tid]--;
  }
}

bool TaskDependenciesStore::isTaskBlocked(TaskID tid) {
  return inDegreesTracker[tid] != 0;
}

bool TaskDependenciesStore::isTaskBlocking(TaskID tid) {
  auto res = false;
  for (auto& task : blockedBy[tid]) {
    if (isTaskBlocked(task)) {
      res = true;
      break;
    }
  }

  return res;
}

std::optional<std::vector<TaskID>>
TaskDependenciesStore::tryAddDependenciesOrReturnCycle(
    TaskID dependent,
    const std::vector<TaskID>& dependencies) {
  auto blockingCpy = blocking;
  auto blockedByCpy = blockedBy;
  auto newDepsCounter = 0;

  for (auto tid : dependencies) {
    blockedByCpy[tid].insert(dependent);
    auto [it, newDep] = blockingCpy[dependent].insert(tid);
    if (newDep) {
      newDepsCounter++;
    }
  }

  auto resPair =
      TaskDependenciesStore::sortTaskIDsTopologicallyOrReturnCycleIfDetected(
          blockingCpy, blockedByCpy);

  if (resPair.second.size() > 0) {
    return std::optional<std::vector<TaskID>>{resPair.second};
  }

  blocking = blockingCpy;
  blockedBy = blockedByCpy;
  topollogicalySorted = resPair.first;
  inDegreesTracker[dependent] += newDepsCounter;

  return std::nullopt;
}

std::vector<TaskID> TaskDependenciesStore::getTaskIDsBlockedBy(
    TaskID tid) const {
  std::vector<TaskID> result;

  auto it = blockedBy.find(tid);
  if (it != blockedBy.end()) {
    result.insert(result.end(), it->second.begin(), it->second.end());
  }

  return result;
}

std::vector<TaskID> TaskDependenciesStore::getTaskIDsBlocking(
    TaskID tid) const {
  std::vector<TaskID> result;

  auto it = blocking.find(tid);
  if (it != blocking.end()) {
    result.insert(result.end(), it->second.begin(), it->second.end());
  }

  return result;
}

std::vector<TaskID> TaskDependenciesStore::getTaskIdsTopollogicalySorted()
    const {
  return topollogicalySorted;
}

std::pair<std::vector<TaskID>, std::vector<TaskID>>
TaskDependenciesStore::sortTaskIDsTopologicallyOrReturnCycleIfDetected(
    const std::unordered_map<TaskID, std::unordered_set<TaskID>>& blocking,
    const std::unordered_map<TaskID, std::unordered_set<TaskID>>& blockedBy) {
  std::queue<TaskID> q;
  std::unordered_map<TaskID, int> inDegrees;
  std::vector<TaskID> idsTopoSorted, cycledIds;

  for (auto [tid, deps] : blocking) {
    if (inDegrees.find(tid) == inDegrees.end()) {
      inDegrees[tid] = 0;
    }
    inDegrees[tid] += deps.size();
  }

  for (auto& [tid, indegree] : inDegrees) {
    if (indegree == 0) {
      q.push(tid);
    }
  }

  while (!q.empty()) {
    auto v = q.front();
    q.pop();
    idsTopoSorted.push_back(v);

    auto it = blockedBy.find(v);
    if (it != blockedBy.end()) {
      for (auto taskBlocked : it->second) {
        inDegrees[taskBlocked]--;
        if (inDegrees[taskBlocked] == 0) {
          q.push(taskBlocked);
        }
      }
    }
  }

  for (auto [d, degree] : inDegrees) {
    if (degree > 0) {
      cycledIds.push_back(d);
    }
  }

  return std::make_pair(idsTopoSorted, cycledIds);
}