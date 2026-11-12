/**
 * @file log_returns_int.cpp
 * @brief Must NOT compile: the optional `Log()` hook is `void Log(LogLevel,
 * const char*) noexcept`.
 *
 * EXPECT: Log hook must be
 */
#include "ads9324.hpp"
#include "mock_ads9324.hpp"

class IntLogPort : public ads9324_test::MockPortBase<IntLogPort> {
public:
  void DelayUs(uint32_t us) noexcept {
    (void)us;
  }
  int Log(ads9324::LogLevel level, const char* message) noexcept {
    (void)level;
    (void)message;
    return 0;
  }
};

int main() {
  IntLogPort port;
  ads9324::ADS9324<IntLogPort> sensor(port);
  return static_cast<int>(sensor.EnsureInitialized().error);
}
