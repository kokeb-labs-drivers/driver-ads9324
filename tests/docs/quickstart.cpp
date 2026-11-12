/**
 * @file quickstart.cpp
 * @brief The Quickstart page's example, compiled by the host build (never run).
 */
#include "my_port.hpp"

// --8<-- [start:quickstart]
#include "ads9324.hpp"

// This minimal port has no CONVST pin hook, so the example drives CONVST itself
// (see "Drive CONVST yourself" on the API guide); ReadFrame() reads the latest
// conversion.
int read_ain1_microvolts(MyPort& port, int32_t& microvolts) {
  ads9324::ADS9324<MyPort> adc(port); // no I/O yet

  if (auto r = adc.EnsureInitialized(); !r) { // 1. bus up, reset, DEVICE_ID, Config{}
    return static_cast<int>(r.error);         //    Bus, Timeout or DeviceIdMismatch: check wiring
  }
  // ... pulse CONVST here, wait 1.25 us ...
  const auto frame = adc.ReadFrame(); // 2. one frame: all 16 channels, 16 bits each
  if (!frame) {
    return static_cast<int>(frame.error); //    DataMismatch: SDOUT stuck low or high
  }
  microvolts = frame.value.microvolts[0]; // 3. AIN1, +-5 V range, in microvolts
  return 0;
}
// --8<-- [end:quickstart]
