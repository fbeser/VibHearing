#include "Logger.h"

namespace vibhearing {

void Logger::begin(const uint32_t baudRate) {
  Serial.begin(baudRate);
}

void Logger::info(const char* message) {
  Serial.printf("[INFO] %s\n", message);
}

void Logger::error(const char* message, const esp_err_t error) {
  Serial.printf("[ERROR] %s (%d)\n", message, static_cast<int>(error));
}

}  // namespace vibhearing
