/**
 * @file ports.cpp
 * @brief The port with every optional hook, as shown on the Porting page.
 * Compiled, never run.
 */
#include <cstddef>
#include <cstdint>

#include "ads9324.hpp"

bool platform_spi_init() noexcept;
bool platform_spi_transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept;
bool platform_lanes_capture(uint8_t lanes, uint8_t* rx, std::size_t len) noexcept;
void platform_delay_us(uint32_t us) noexcept;
bool platform_gpio_write(int pin, bool high) noexcept;
bool platform_gpio_read(int pin) noexcept;
void platform_log(const char* line) noexcept;

// --8<-- [start:full-port]
class FullPort : public ads9324::SpiInterface<FullPort> {
public:
  bool EnsureInitialized() noexcept {
    return platform_spi_init();
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    return platform_spi_transfer(tx, rx, len);
  }

  // Required by every call that waits (power-up, reset, conversions,
  // calibration).
  void DelayUs(uint32_t us) noexcept {
    platform_delay_us(us);
  }

  // Optional: RESET and CONVST on GPIOs. ACTIVE means "asserted": RESET is
  // active low on the pin, so ACTIVE is a low level there; CONVST is a high
  // pulse, so ACTIVE is a high level.
  bool GpioSet(ads9324::CtrlPin pin, ads9324::GpioSignal signal) noexcept {
    const bool active = signal == ads9324::GpioSignal::ACTIVE;
    switch (pin) {
    case ads9324::CtrlPin::Reset:
      return platform_gpio_write(kResetGpio, !active);
    case ads9324::CtrlPin::Convst:
      return platform_gpio_write(kConvstGpio, active);
    default:
      return false;
    }
  }

  // Optional: the DRDY/ALARM pin as an input. Report the raw level; the driver
  // applies the polarity.
  bool GpioGet(ads9324::CtrlPin pin, bool& high) noexcept {
    if (pin != ads9324::CtrlPin::DrdyAlarm) {
      return false;
    }
    high = platform_gpio_read(kDrdyGpio);
    return true;
  }

  // Optional: only for 2, 4 or 8 data lanes. Clock the lanes for `len` bytes of
  // the frame and pack the bits as the Porting page describes.
  bool CaptureLanes(uint8_t lanes, uint8_t* rx, std::size_t len) noexcept {
    return platform_lanes_capture(lanes, rx, len);
  }

  // Optional: one line per lifecycle event or failure. The driver never
  // formats; the port does.
  void Log(ads9324::LogLevel level, const char* message) noexcept {
    if (level > ads9324::LogLevel::Warning) {
      return;
    }
    platform_log(message); // plain text: never use it as a format string
  }

private:
  static constexpr int kResetGpio = 11;
  static constexpr int kConvstGpio = 4;
  static constexpr int kDrdyGpio = 5;
};
// --8<-- [end:full-port]

// --8<-- [start:hardware-reset]
ads9324::Error RecoverWithResetPin(ads9324::ADS9324<FullPort>& adc) {
  // With GpioSet() this pulses RESET and writes the stored configuration again;
  // a port without it gets Error::Unsupported.
  return adc.Reset(ads9324::ResetKind::Hardware).error;
}
// --8<-- [end:hardware-reset]
