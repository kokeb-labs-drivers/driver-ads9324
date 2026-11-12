/**
 * @file gpio_get_returns_void.cpp
 * @brief Must NOT compile: `GpioGet()` must return `bool`, so that a failed pin
 * read is reported.
 *
 * EXPECT: GpioGet hook must be
 */
#include "ads9324.hpp"
#include "mock_ads9324.hpp"

class VoidGetPort : public ads9324_test::MockPortBase<VoidGetPort> {
public:
  void DelayUs(uint32_t us) noexcept {
    (void)us;
  }
  void GpioGet(ads9324::CtrlPin pin, bool& high) noexcept {
    (void)pin;
    high = true;
  }
};

int main() {
  VoidGetPort port;
  ads9324::ADS9324<VoidGetPort> sensor(port);
  return static_cast<int>(sensor.EnsureInitialized().error);
}
