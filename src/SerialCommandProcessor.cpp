#include "SerialCommandProcessor.h"

#include <cstring>

namespace vibhearing {

void SerialCommandProcessor::update() {
  while (Serial.available() > 0) {
    const char value = static_cast<char>(Serial.read());
    lastByteReceivedMs_ = millis();

    if (value == '\r' || value == '\n') {
      if (commandLength_ > 0) {
        executeCommand();
      }
      continue;
    }

    if (value == ' ' || value == '\t') {
      continue;
    }

    if (commandLength_ >= kMaximumCommandLength) {
      Serial.println("[WARN] Serial command is too long");
      clearCommand();
      continue;
    }

    command_[commandLength_++] = value;
    command_[commandLength_] = '\0';
  }

  if (commandLength_ > 0 &&
      millis() - lastByteReceivedMs_ >= kCommandIdleTimeoutMs) {
    executeCommand();
  }
}

void SerialCommandProcessor::executeCommand() {
  if (std::strcmp(command_, "r") == 0 ||
      std::strcmp(command_, "reset") == 0) {
    Serial.println("[INFO] Serial command: restarting");
    Serial.flush();
    delay(20);
    ESP.restart();
  }

  Serial.printf("[WARN] Unknown serial command: %s\n", command_);
  clearCommand();
}

void SerialCommandProcessor::clearCommand() {
  commandLength_ = 0;
  command_[0] = '\0';
}

}  // namespace vibhearing
