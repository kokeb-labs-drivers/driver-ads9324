/**
 * @file hook_not_noexcept.cpp
 * @brief Must NOT compile: a Transfer hook that is not noexcept breaks the port
 * contract, so the port concept rejects it (handbook KB-CPP20-006).
 *
 * EXPECT: SpiPort
 */
#include <cstddef>
#include <cstdint>

#include "ads9324.hpp"

class ThrowingPort : public ads9324::SpiInterface<ThrowingPort> {
public:
  bool EnsureInitialized() noexcept {
    return true;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) {
    (void)tx;
    (void)rx;
    return len > 0U;
  }
  void DelayUs(uint32_t us) noexcept {
    (void)us;
  }
};

int main() {
  ThrowingPort port;
  ads9324::ADS9324<ThrowingPort> sensor(port);
  return static_cast<int>(sensor.EnsureInitialized().error);
}
