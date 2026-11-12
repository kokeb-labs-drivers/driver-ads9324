/**
 * @file test_support.hpp
 * @brief Helpers shared by the host tests: a driver on the full mock port,
 * ready to use.
 */
#ifndef ADS9324_TEST_SUPPORT_HPP
#define ADS9324_TEST_SUPPORT_HPP

#include "ads9324.hpp"
#include "mock_ads9324.hpp"
#include "test_harness.hpp"

namespace ads9324_test {

/// A driver on a full mock port.
struct Rig {
  FullPort port;
  ads9324::ADS9324<FullPort> drv;

  explicit Rig(const ads9324::Config& config = {}) : drv(port, config) {}

  Ads9324Model& Chip() {
    return port.state.chip;
  }
  bool Init() {
    return drv.EnsureInitialized().ok();
  }
};

/// The config of a one-lane SDOUT run with every channel at a range and an
/// input voltage.
inline ads9324::Config BasicConfig() {
  return ads9324::Config{};
}

/// Count the frames of the log that wrote a register of the given address while
/// `bank` was selected.
inline int WritesTo(const Ads9324Model& chip, uint8_t bank, uint8_t addr) {
  int n = 0;
  for (const FrameLog& f : chip.log) {
    n += (f.is_register_frame && (f.addr == addr) && (f.bank == bank) && (addr != 0x01U) &&
          (addr != 0x02U))
             ? 1
             : 0;
  }
  return n;
}

} // namespace ads9324_test

#endif // ADS9324_TEST_SUPPORT_HPP
