/**
 * @file checked_config_invalid.cpp
 * @brief Must NOT compile: a `CheckedConfig` runs `Validate()` at build time.
 *
 * EXPECT: ConfigIsInvalid
 */
#include "ads9324_types.hpp"

int main() {
  // 5 Hz is below the 100 kHz the chip's CONVST allows: Validate() rejects it.
  constexpr ads9324::CheckedConfig kBoard{[] {
    ads9324::Config c{};
    c.sampling.convst_hz = 5;
    return c;
  }()};
  return static_cast<int>(kBoard.Get().sampling.convst_hz);
}
