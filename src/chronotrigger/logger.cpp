#include "chronotrigger/logger.h"

#include <iostream>
using namespace chronotrigger;

Logger& Logger::getInstance() {
  static Logger instance;

  return instance;
}

void Logger::print(const std::string& message) {
  std::lock_guard<std::mutex> lock(getInstance().mtx);

  std::cout << message;
}