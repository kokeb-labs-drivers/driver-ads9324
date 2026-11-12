/**
 * @file main.cpp
 * @brief The ADS9324 driver on your PC: start a simulated chip, read all 16
 * channels, use the window comparator and the ADC calibration.
 *
 * No hardware and no SDK: the host port (host_ads9324_port.hpp) connects the
 * real driver to the chip model the host tests use. The same calls run on an
 * MCU with that MCU's port (examples/esp32/ for ESP-IDF). Exit code 0 when
 * every call succeeded.
 */
#include <cstdint>
#include <cstdio>

#include "ads9324.hpp"
#include "host_ads9324_port.hpp"

namespace {

using Driver = ads9324::ADS9324<HostAds9324Port>;

/// Print the voltage of every channel in the frame, in millivolts.
void Print(const ads9324::Frame& frame) {
  for (uint8_t ch = 0; ch < 16; ++ch) {
    if (((static_cast<uint32_t>(frame.present_mask) >> ch) & 1U) != 0U) {
      std::printf("  AIN%-2u %7d mV\n", static_cast<unsigned>(ch) + 1U,
                  static_cast<int>(frame.microvolts[ch] / 1000));
    }
  }
}

bool Report(const char* what, ads9324::Error error) {
  if (error != ads9324::Error::Ok) {
    std::printf("%s failed: %s\n", what, ads9324::ToString(error));
    return false;
  }
  return true;
}

} // namespace

int main() {
  HostAds9324Port port;
  for (int ch = 0; ch < 16; ++ch) {
    port.state.chip.input_uv[static_cast<std::size_t>(ch)] = -4'000'000 + (500'000 * ch);
  }
  Driver adc(port); // no I/O yet; Config{} reads 16 channels of 16 bits on one
                    // SDOUT lane

  std::printf("ADS9324 driver %s on the host\n", ads9324::GetDriverVersion());
  if (!Report("EnsureInitialized", adc.EnsureInitialized().error)) {
    return 1;
  }

  // 1. One conversion of every channel.
  const auto frame = adc.MeasureOnce();
  if (!Report("MeasureOnce", frame.error)) {
    return 1;
  }
  Print(frame.value);

  // 2. A window on AIN3: an alert when its code leaves -40 to +40 (top byte).
  ads9324::Input in3{};
  in3.window = ads9324::Window{.enabled = true, .high_code = 40, .low_code = -40, .hysteresis = 2};
  ads9324::Alarm alarm{};
  alarm.function = ads9324::AlarmFunction::WindowAlert;
  if (!Report("SetInput", adc.SetInput(2, in3).error) ||
      !Report("SetAlarm", adc.SetAlarm(alarm).error)) {
    return 1;
  }
  (void)adc.MeasureOnce();
  const auto pin = adc.ReadAlarmPin();
  const auto flags = adc.ReadWindowFlags();
  if (!Report("ReadWindowFlags", flags.error) || !Report("ReadAlarmPin", pin.error)) {
    return 1;
  }
  std::printf("window alert pin %s, low flags 0x%04X\n", pin.value ? "asserted" : "idle",
              static_cast<unsigned>(flags.value.low));

  // 3. ADC calibration removes the offset error of the converter.
  if (!Report("RunAdcCalibration", adc.RunAdcCalibration().error)) {
    return 1;
  }
  std::printf("calibration done, mode kept: %s\n",
              adc.IsCalibrationBusy().value ? "still busy" : "idle");
  return 0;
}
