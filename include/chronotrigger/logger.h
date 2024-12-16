#ifndef CHRONOTRIGGER_LOGGER_H
#define CHRONOTRIGGER_LOGGER_H

#include <mutex>

namespace chronotrigger {

class Logger {
 public:
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  static void print(const std::string& message);

 private:
  Logger() = default;

  static Logger& getInstance();
  static std::shared_ptr<Logger> loggerInst;

  std::mutex mtx;
};
}  // namespace chronotrigger

#endif  // CHRONOTRIGGER_LOGGER_H
