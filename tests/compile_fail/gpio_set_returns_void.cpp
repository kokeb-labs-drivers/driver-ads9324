/**
 * @file gpio_set_returns_void.cpp
 * @brief Must NOT compile: an optional hook with the wrong signature is
 * rejected, not ignored.
 *
 * EXPECT: GpioSet hook must be
 */
#include "ads9324.hpp"
#include "mock_ads9324.hpp"

class VoidGpioPort : public ads9324_test::MockPortBase<VoidGpioPort> {
public:
  void DelayUs(uint32_t us) noexcept {
    (void)us;
  }
  void GpioSet(ads9324::CtrlPin pin, ads9324::GpioSignal signal) noexcept {
    (void)pin;
    (void)signal;
  }
};

int main() {
  VoidGpioPort port;
  ads9324::ADS9324<VoidGpioPort> sensor(port);
  return static_cast<int>(sensor.EnsureInitialized().error);
}
