/**
 * @file not_a_port.cpp
 * @brief Must NOT compile: the driver is constrained by the port concept.
 *
 * EXPECT: Port
 */
#include "ads9324.hpp"

struct NotAPort {
  bool EnsureInitialized() noexcept {
    return true;
  }
};

int main() {
  NotAPort port;
  ads9324::ADS9324<NotAPort> sensor(port);
  return static_cast<int>(sensor.EnsureInitialized().error);
}
