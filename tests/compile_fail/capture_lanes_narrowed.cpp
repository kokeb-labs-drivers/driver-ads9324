/**
 * @file capture_lanes_narrowed.cpp
 * @brief Must NOT compile: a `CaptureLanes()` with a 16-bit length would
 * truncate long frames.
 *
 * EXPECT: CaptureLanes hook must be
 */
#include "ads9324.hpp"
#include "mock_ads9324.hpp"

class NarrowCapturePort : public ads9324_test::MockPortBase<NarrowCapturePort> {
public:
  void DelayUs(uint32_t us) noexcept {
    (void)us;
  }
  bool CaptureLanes(uint8_t lanes, uint8_t* rx, uint16_t len) noexcept {
    (void)lanes;
    (void)rx;
    return len > 0U;
  }
};

int main() {
  NarrowCapturePort port;
  ads9324::ADS9324<NarrowCapturePort> sensor(port);
  return static_cast<int>(sensor.EnsureInitialized().error);
}
