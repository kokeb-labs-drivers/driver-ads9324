/**
 * @file host_ads9324_port.hpp
 * @brief The host port: the driver's hooks over a simulated ADS9324, so the
 * example runs on any PC.
 *
 * `ads9324_test::MockPortBase` (tests/mock_ads9324.hpp) supplies the required
 * bus hooks over the chip model the host tests use. This class adds what a real
 * port adds: `DelayUs()` (here it advances the simulated chip's clock instead
 * of waiting), the control pins, the lane capture and `Log()` to the console. A
 * port for a real bus on a PC (Linux spidev with GPIO, a USB bridge) implements
 * the same hooks: see docs/porting.md.
 */
#ifndef ADS9324_HOST_ADS9324_PORT_HPP
#define ADS9324_HOST_ADS9324_PORT_HPP

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "mock_ads9324.hpp"

/// The host port: simulated chip, real console.
class HostAds9324Port : public ads9324_test::MockPortBase<HostAds9324Port> {
public:
  /// The bus settings a real port would use; checked against the driver's bus
  /// requirements (handbook KB-CPP-123) like any port's.
  static constexpr uint32_t kClockHz = 20'000'000;
  static constexpr uint8_t kSpiMode = 0;
  static_assert(kClockHz <= ads9324::kMaxClockHz, "clock above the ADS9324's f_SCLK max");
  static_assert(kSpiMode == ads9324::kSpiMode, "the ADS9324 is tested in SPI mode 0");

  /// Time passes in the simulated chip; nothing sleeps.
  void DelayUs(uint32_t us) noexcept {
    state.chip.Delay(us);
  }

  /// RESET and CONVST drive the simulated chip's pins.
  bool GpioSet(ads9324::CtrlPin pin, ads9324::GpioSignal level) noexcept {
    const bool active = level == ads9324::GpioSignal::ACTIVE;
    if (pin == ads9324::CtrlPin::Reset) {
      state.chip.SetResetPin(active);
      return true;
    }
    if (pin == ads9324::CtrlPin::Convst) {
      state.chip.SetConvst(active);
      return true;
    }
    return false;
  }

  /// The DRDY/ALARM pin of the simulated chip.
  bool GpioGet(ads9324::CtrlPin pin, bool& high) noexcept {
    if (pin != ads9324::CtrlPin::DrdyAlarm) {
      return false;
    }
    high = state.chip.AlarmPin();
    return true;
  }

  /// Data on D7 to D0: the simulated chip returns the same byte stream as on
  /// SDOUT.
  bool CaptureLanes(uint8_t lanes, uint8_t* rx, std::size_t len) noexcept {
    return state.chip.CaptureLanes(lanes, rx, len);
  }

  /// Driver messages (they carry their own `ads9324:` prefix) go to the
  /// console, one line each.
  void Log(ads9324::LogLevel level, const char* message) noexcept {
    (void)level;
    (void)std::puts(message); // plain text: never pass it as a format string
  }
};

#endif // ADS9324_HOST_ADS9324_PORT_HPP
