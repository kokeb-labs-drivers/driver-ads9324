/**
 * @file configuration.cpp
 * @brief The Configuration page's examples, compiled by the host build (never
 * run).
 */
#include "ads9324.hpp"
#include "ads9324_kconfig.hpp"
#include "my_port.hpp"

// --8<-- [start:custom]
ads9324::Config SixteenChannelTenVolt() {
  ads9324::Config c{}; // Table 7-18: 16 channels, 16 bits, one lane on SDOUT
  for (auto& in : c.inputs) {
    in.range = ads9324::InputRange::V10; // every channel +-10 V
  }
  c.sampling.convst_hz = 200'000; // 100 kHz to 1 MHz; checked by Validate()
  c.interface.append_diag = true; // one DIAG byte per frame: reset and window flags
  return c;                       // ads9324::Validate(c) checks it without the chip
}

ads9324::Error StartTenVolt(MyPort& port) {
  static ads9324::ADS9324<MyPort> adc(port, SixteenChannelTenVolt()); // stored, no I/O
  return adc.EnsureInitialized().error; // InvalidArgument if the config is out of range
}
// --8<-- [end:custom]

// --8<-- [start:checked]
// A configuration known at compile time can be proved valid at compile time:
// CheckedConfig is consteval and refuses to compile an invalid one.
constexpr ads9324::CheckedConfig kFixed{ads9324::Config{}};

ads9324::Error StartFixed(MyPort& port) {
  static ads9324::ADS9324<MyPort> adc(port, kFixed.Get());
  return adc.EnsureInitialized().error;
}
// --8<-- [end:checked]

// --8<-- [start:kconfig]
ads9324::Error StartFromMenuconfig(MyPort& port) {
  static ads9324::ADS9324<MyPort> adc(port, ads9324::KconfigDefaults());
  return adc.EnsureInitialized().error;
}
// --8<-- [end:kconfig]
