/**
 * @file api_tasks.cpp
 * @brief The API guide's examples, compiled by the host build (never run).
 */
#include <cstdio>

#include "ads9324.hpp"
#include "my_port.hpp"

using Adc = ads9324::ADS9324<MyPort>;

// --8<-- [start:lifecycle]
ads9324::Error BringUp(Adc& adc) {
  // The one entry point: starts the chip on the first call, then returns Ok
  // with no I/O. After a failure the next call starts again from the beginning.
  const auto r = adc.EnsureInitialized();
  if (!r) {
    std::printf("ADS9324 init: %s\n", ads9324::ToString(r.error));
  }
  return r.error;
}
// --8<-- [end:lifecycle]

// --8<-- [start:measure]
ads9324::Result<int32_t> MeasureAin5(Adc& adc) {
  // CONVST pulse, wait for the result, read and decode one frame (needs
  // GpioSet() for CONVST).
  const auto frame = adc.MeasureOnce();
  if (!frame) {
    return ads9324::Result<int32_t>::Failure(frame.error);
  }
  return ads9324::Result<int32_t>::Success(frame.value.microvolts[4]); // AIN5, in microvolts
}
// --8<-- [end:measure]

// --8<-- [start:input]
ads9324::Error UseTenVoltRange(Adc& adc) {
  ads9324::Input in =
      adc.GetConfig().inputs[8];       // AIN9: the register layout is mirrored, the driver knows
  in.range = ads9324::InputRange::V10; // +-10 V full scale
  in.bandwidth = ads9324::Bandwidth::Wide; // 280 kHz to 350 kHz low-pass
  return adc.SetInput(8, in).error;        // only the registers that change are written
}
// --8<-- [end:input]

// --8<-- [start:window]
ads9324::Error WatchAin3(Adc& adc) {
  ads9324::Input in = adc.GetConfig().inputs[2];
  in.window =
      ads9324::Window{true, 40, -40, 2}; // alert outside -40..+40 on the top byte, hysteresis 2
  if (auto r = adc.SetInput(2, in); !r) {
    return r.error;
  }
  ads9324::Alarm alarm = adc.GetConfig().alarm;
  alarm.function = ads9324::AlarmFunction::WindowAlert; // DRDY/ALARM now shows
                                                        // the window alert
  return adc.SetAlarm(alarm).error;
}

ads9324::Error AcknowledgeAlerts(Adc& adc) {
  const auto flags = adc.ReadWindowFlags(); // which channels went high (bit n =
                                            // AIN n+1) or low
  if (!flags) {
    return flags.error;
  }
  std::printf("high 0x%04X low 0x%04X\n", static_cast<unsigned>(flags.value.high),
              static_cast<unsigned>(flags.value.low));
  const auto a = adc.ClearWindowFlags(0);             // AIN1 to AIN8
  return a ? adc.ClearWindowFlags(1).error : a.error; // AIN9 to AIN16
}
// --8<-- [end:window]

// --8<-- [start:filter]
ads9324::Error AverageSixteen(Adc& adc) {
  ads9324::Filter f{};
  f.kind = ads9324::FilterKind::MovingAverage;
  f.length = ads9324::AverageLength::X16;
  return adc.SetFilter(f).error; // the driver pulses SYSREF after the write, as the chip needs
}
// --8<-- [end:filter]

// --8<-- [start:calibrate]
ads9324::Error Calibrate(Adc& adc) {
  // Needs no digital filter. Single shot: about 125 ms, then the chip keeps the
  // result.
  return adc.RunAdcCalibration()
      .error; // Refused when a filter is on, Timeout when CALIB_BUSY stays set
}
// --8<-- [end:calibrate]

// --8<-- [start:errors]
const char* Describe(ads9324::Error e) {
  switch (e) {
  case ads9324::Error::Bus:
  case ads9324::Error::DeviceIdMismatch:
    return "check the SPI wiring, the mode and the chip select";
  case ads9324::Error::UnexpectedReset:
    return "the chip lost its configuration: call EnsureInitialized() again";
  case ads9324::Error::DataMismatch:
    return "all zeros or all ones on the data line: check SDOUT and the "
           "supplies";
  case ads9324::Error::Refused:
    return "the chip is in standby or a filter is on: Wake(), or SetFilter() "
           "with None first";
  default:
    return ads9324::ToString(e);
  }
}
// --8<-- [end:errors]

// --8<-- [start:power]
void Standby(Adc& adc) {
  if (auto r = adc.PowerDown(); !r) {
    std::printf("ADS9324 standby failed: %s\n", ads9324::ToString(r.error));
    return;
  }
  // ... later ...
  const auto w = adc.Wake(); // waits 30 ms and checks that the registers survived
  if (w.error == ads9324::Error::UnexpectedReset) {
    (void)adc.EnsureInitialized(); // they did not: start again
  }
}
// --8<-- [end:power]
