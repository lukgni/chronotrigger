#ifndef CHRONOTRIGGER_EXCEPTIONS_H
#define CHRONOTRIGGER_EXCEPTIONS_H

#include <exception>

#include "./types.h"

namespace chronotrigger {

class TaskCycleException : public std::runtime_error {
 public:
  explicit TaskCycleException(const std::string& message)
      : std::runtime_error(message) {}
};

class TaskNotFoundException : public std::logic_error {
 public:
  explicit TaskNotFoundException(const std::string& message)
      : std::logic_error(message) {}
};

}  // namespace chronotrigger

#endif  // CHRONOTRIGGER_EXCEPTIONS_H
