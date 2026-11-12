/**
 * @file esp32_ads9324_port.hpp
 * @brief ESP-IDF port for the ADS9324 driver: spi_master for the bus and the data on SDOUT, GPIO
 * for RESET, CONVST and DRDY/ALARM, esp_rom delays and ESP_LOG for the log hook.
 *
 * Start from this file for any ESP-IDF target. The pins are in Esp32Ads9324Pins; the defaults match
 * the wiring table in examples/esp32/README.md (ESP32-C6 DevKitC-1). The port has no
 * `CaptureLanes()`: the data goes on one lane over SDOUT (`Config{}`), which the SPI peripheral
 * reads like a register frame.
 */
#ifndef ADS9324_ESP32_ADS9324_PORT_HPP
#define ADS9324_ESP32_ADS9324_PORT_HPP

#include <cstddef>
#include <cstdint>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#include "ads9324.hpp"

/// The port's default SPI clock: 10 MHz for jumper wires (the chip allows 60 MHz, see the docs).
inline constexpr int kEsp32Ads9324ClockHz = 10'000'000;
/// The port's SPI mode.
inline constexpr int kEsp32Ads9324SpiMode = 0;
// The bus requirements of the driver (handbook KB-CPP-123), checked at compile time.
static_assert(kEsp32Ads9324ClockHz <= static_cast<int>(ads9324::kMaxSdoutClockHz),
              "the default clock is above the ADS9324's f_SCLK max");
static_assert(kEsp32Ads9324SpiMode == ads9324::kSpiMode, "the driver is tested in SPI mode 0");
static_assert(ads9324::kMsbFirst, "this port sends MSB first (no SPI_DEVICE_BIT_LSBFIRST)");

/// Pins and bus settings of the ESP-IDF port.
struct Esp32Ads9324Pins {
  spi_host_device_t host = SPI2_HOST;
  int sclk = 6;
  int mosi = 7;
  int miso = 2;
  int cs = 10;
  int reset = 11; ///< -1 when RESET is tied high.
  int convst = 4; ///< -1 when CONVST comes from a timer or another source.
  int drdy = 5;   ///< DRDY/ALARM input; -1 when not wired.
  int clock_hz =
      kEsp32Ads9324ClockHz; ///< Up to `ads9324::kMaxSdoutClockHz`; checked at build time.
};

/// The ADS9324 port on ESP-IDF, with every optional hook except `CaptureLanes()`.
class Esp32Ads9324Port : public ads9324::SpiInterface<Esp32Ads9324Port> {
public:
  explicit Esp32Ads9324Port(const Esp32Ads9324Pins& pins = {}) noexcept : pins_(pins) {}

  bool EnsureInitialized() noexcept {
    if (device_ != nullptr) {
      return true;
    }
    if (pins_.clock_hz <= 0 || pins_.clock_hz > static_cast<int>(ads9324::kMaxClockHz)) {
      ESP_LOGE(kTag, "clock outside the ADS9324's bus requirements");
      return false;
    }
    spi_bus_config_t bus{};
    bus.mosi_io_num = pins_.mosi;
    bus.miso_io_num = pins_.miso;
    bus.sclk_io_num = pins_.sclk;
    bus.quadwp_io_num = -1;
    bus.quadhd_io_num = -1;
    bus.max_transfer_sz = 64; // the longest data frame is 50 bytes
    const esp_err_t bus_err = spi_bus_initialize(pins_.host, &bus, SPI_DMA_DISABLED);
    if (bus_err != ESP_OK && bus_err != ESP_ERR_INVALID_STATE) { // INVALID_STATE: bus already up
      ESP_LOGE(kTag, "spi_bus_initialize: %s", esp_err_to_name(bus_err));
      return false;
    }
    spi_device_interface_config_t dev{};
    dev.clock_speed_hz = pins_.clock_hz;
    dev.mode = static_cast<uint8_t>(kEsp32Ads9324SpiMode);
    dev.spics_io_num = pins_.cs;
    dev.queue_size = 1;
    if (spi_bus_add_device(pins_.host, &dev, &device_) != ESP_OK) {
      device_ = nullptr;
      return false;
    }
    // RESET and CONVST idle high (the driver leaves CONVST high after each pulse): each is driven
    // to its idle level before its output is enabled, so configuring the pins never pulses a reset
    // or starts a conversion (KB-HW-018).
    return setupOutput(pins_.reset, 1) && setupOutput(pins_.convst, 1) && setupInput(pins_.drdy);
  }

  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    if (device_ == nullptr || len == 0) {
      return false;
    }
    spi_transaction_t t{};
    t.length = len * 8;
    t.tx_buffer = tx;
    t.rx_buffer = rx; // nullptr for writes: MISO is discarded
    return spi_device_polling_transmit(device_, &t) == ESP_OK;
  }

  void DelayUs(uint32_t us) noexcept {
    esp_rom_delay_us(us);
  }

  bool GpioSet(ads9324::CtrlPin pin, ads9324::GpioSignal signal) noexcept {
    const bool active = signal == ads9324::GpioSignal::ACTIVE;
    if (pin == ads9324::CtrlPin::Reset && pins_.reset >= 0) {
      // RESET is active low: ACTIVE (in reset) is level 0.
      return gpio_set_level(static_cast<gpio_num_t>(pins_.reset), active ? 0 : 1) == ESP_OK;
    }
    if (pin == ads9324::CtrlPin::Convst && pins_.convst >= 0) {
      return gpio_set_level(static_cast<gpio_num_t>(pins_.convst), active ? 1 : 0) == ESP_OK;
    }
    return false;
  }

  bool GpioGet(ads9324::CtrlPin pin, bool& high) noexcept {
    if (pin != ads9324::CtrlPin::DrdyAlarm || pins_.drdy < 0) {
      return false;
    }
    high = gpio_get_level(static_cast<gpio_num_t>(pins_.drdy)) != 0;
    return true;
  }

  void Log(ads9324::LogLevel level, const char* message) noexcept {
    const esp_log_level_t esp_level = level == ads9324::LogLevel::Error     ? ESP_LOG_ERROR
                                      : level == ads9324::LogLevel::Warning ? ESP_LOG_WARN
                                      : level == ads9324::LogLevel::Info    ? ESP_LOG_INFO
                                                                            : ESP_LOG_DEBUG;
    esp_log_write(esp_level, kTag, "%s\n", message); // plain text, never a format
  }

private:
  static bool setupOutput(int pin, uint32_t idle_level) noexcept {
    if (pin < 0) {
      return true;
    }
    gpio_config_t io{};
    io.pin_bit_mask = 1ULL << static_cast<unsigned>(pin);
    io.mode = GPIO_MODE_OUTPUT;
    return gpio_set_level(static_cast<gpio_num_t>(pin), idle_level) == ESP_OK &&
           gpio_config(&io) == ESP_OK;
  }

  static bool setupInput(int pin) noexcept {
    if (pin < 0) {
      return true;
    }
    gpio_config_t io{};
    io.pin_bit_mask = 1ULL << static_cast<unsigned>(pin);
    io.mode = GPIO_MODE_INPUT;
    return gpio_config(&io) == ESP_OK;
  }

  static constexpr const char* kTag = "ads9324";
  Esp32Ads9324Pins pins_;
  spi_device_handle_t device_ = nullptr;
};

#endif // ADS9324_ESP32_ADS9324_PORT_HPP
