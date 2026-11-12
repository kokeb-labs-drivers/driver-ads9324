/**
 * @file ads9324.hpp
 * @brief ADS9324 driver: the one header to include.
 * @copyright Copyright (c) 2026 The driver-ads9324 authors.
 * SPDX-License-Identifier: MIT
 *
 * @details
 * `ads9324::ADS9324<Port>` drives one ADS9324 16-channel, 16-bit, 1 MSPS
 * simultaneous-sampling ADC through a platform port you write (see
 * `ads9324_spi_interface.hpp`). The driver has no platform code, allocates
 * nothing, throws nothing, takes no lock and is not thread-safe: give each
 * object one owner, or guard it with one mutex.
 *
 * Channels are counted from 0: channel 0 is AIN1 and channel 15 is AIN16.
 * Results are signed codes and integer microvolts; there is no floating point.
 */
#ifndef ADS9324_HPP
#define ADS9324_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "ads9324_detail.hpp"
#include "ads9324_encode.hpp"
#include "ads9324_registers.hpp"
#include "ads9324_spi_interface.hpp"
#include "ads9324_types.hpp"
#include "ads9324_version.h"

namespace ads9324 {

namespace detail {

// Optional hooks: a `Declares*` concept only tests the name, the `Has*` concept
// the exact signature, `noexcept` and the result type; the driver rejects a
// hook that is declared but does not match (KB-CPP20-007).

/// The port declares something called `GpioSet`.
template <typename Port>
concept DeclaresGpioSet =
    requires(Port& port) { port.GpioSet(CtrlPin::Reset, GpioSignal::ACTIVE); };

/// The port drives control pins: `bool GpioSet(CtrlPin, GpioSignal) noexcept`.
template <typename Port>
concept HasGpioSet =
    requires { ExactHook<Port, bool(CtrlPin, GpioSignal) noexcept>::Take(&Port::GpioSet); };

/// The port declares something called `GpioGet`.
template <typename Port>
concept DeclaresGpioGet =
    requires(Port& port, bool& level) { port.GpioGet(CtrlPin::DrdyAlarm, level); };

/// The port reads control pins: `bool GpioGet(CtrlPin, bool& high) noexcept`.
template <typename Port>
concept HasGpioGet =
    requires { ExactHook<Port, bool(CtrlPin, bool&) noexcept>::Take(&Port::GpioGet); };

/// The port declares something called `CaptureLanes`.
template <typename Port>
concept DeclaresCaptureLanes =
    requires(Port& port, uint8_t* rx) { port.CaptureLanes(uint8_t{8}, rx, std::size_t{2}); };

/// The port captures the parallel data lanes:
/// `bool CaptureLanes(uint8_t lanes, uint8_t* rx, std::size_t len) noexcept`.
template <typename Port>
concept HasCaptureLanes = requires {
  ExactHook<Port, bool(uint8_t, uint8_t*, std::size_t) noexcept>::Take(&Port::CaptureLanes);
};

/// The port declares something called `DelayUs`.
template <typename Port>
concept DeclaresDelayUs = requires(Port& port) { port.DelayUs(1_u32); };

/// The port waits: `void DelayUs(uint32_t us) noexcept`. A narrower parameter
/// would truncate long waits.
template <typename Port>
concept HasDelayUs = requires { ExactHook<Port, void(uint32_t) noexcept>::Take(&Port::DelayUs); };

/// The port declares something called `Log`.
template <typename Port>
concept DeclaresLog = requires(Port& port) { port.Log(LogLevel::Info, ""); };

/// The port takes log lines: `void Log(LogLevel, const char* message)
/// noexcept`.
template <typename Port>
concept HasLog =
    requires { ExactHook<Port, void(LogLevel, const char*) noexcept>::Take(&Port::Log); };

/// Wait through the port. A port without `DelayUs()` fails to compile at the
/// first call that needs a wait: the chip's reset and conversion times are
/// requirements, never skipped.
template <typename Port>
void DelayUs(Port& port, uint32_t us) noexcept {
  if constexpr (HasDelayUs<Port>) {
    port.DelayUs(us);
  } else {
    static_assert(kAlwaysFalse<Port>, "ads9324: the port must implement `void "
                                      "DelayUs(uint32_t us) noexcept`");
  }
}

/// Log through the port, or nothing when the port has no `Log()`. The driver
/// never formats text.
template <typename Port>
void Log(Port& port, [[maybe_unused]] LogLevel level,
         [[maybe_unused]] const char* message) noexcept {
  if constexpr (HasLog<Port>) {
    port.Log(level, message);
  } else {
    (void)port;
  }
}

/// Data lanes the port captures for a lane setting.
constexpr uint8_t LaneCount(Lanes lanes) noexcept {
  switch (lanes) {
  case Lanes::Four:
    return 4U;
  case Lanes::Two:
    return 2U;
  case Lanes::One:
    return 1U;
  case Lanes::Eight:
  default:
    break;
  }
  return 8U;
}

/// Order in which `flushImage()` writes: GEN_CFG5 before the filters, DWC_CFG
/// after the thresholds.
constexpr uint32_t FlushPhase(reg::Bank bank, uint8_t address) noexcept {
  if ((bank == reg::Bank::Common) && (address == reg::kGenCfg5)) {
    return 0U;
  }
  return ((bank != reg::Bank::Common) && (address == reg::kDwcCfg)) ? 2U : 1U;
}

/// A register whose write restarts the digital filter chain (needs the SYSREF
/// pulse).
constexpr bool IsFilterRegister(reg::Bank bank, uint8_t address) noexcept {
  if (bank == reg::Bank::Common) {
    return (address == reg::kDigFilter) || (address == reg::kGenCfg5);
  }
  return (address >= reg::kPhaseDelay) && (address < (reg::kPhaseDelay + 4U));
}

static_assert(FlushPhase(reg::Bank::Common, reg::kGenCfg5) == 0U);
static_assert(FlushPhase(reg::Bank::Ain9To16, reg::kDwcCfg) == 2U);
static_assert(FlushPhase(reg::Bank::Ain1To8, reg::kDwcTh) == 1U);
static_assert(IsFilterRegister(reg::Bank::Ain9To16, reg::kPhaseDelay + 3U));
static_assert(!IsFilterRegister(reg::Bank::Ain9To16, reg::kOffset));

} // namespace detail

/**
 * @defgroup ads9324_driver ADS9324 driver
 * @brief The driver class.
 * @{
 */

// clang-format off: the table below must keep one row per line.
/**
 * @brief Driver for one ADS9324 on a platform port.
 *
 * Lifecycle: construct with the configuration (no I/O), then `EnsureInitialized()`, the one way to
 * start (bus up, power-up wait, reset, identity check, configuration), use, optionally
 * `Deinitialize()`. Every call that touches the chip returns `Error::NotInitialized` until
 * `EnsureInitialized()` has succeeded; no call starts the chip as a side effect.
 *
 * Configuration is written through a write-through shadow of every register: a call writes only the
 * registers that differ from what the chip holds, and a setter stores the new configuration only
 * after the chip accepted it. Setters take a trailing `Verify` that reads every register back.
 *
 * Optional port hooks, found at compile time:
 *
 * - `bool GpioSet(CtrlPin, GpioSignal) noexcept` drives the RESET pin and the CONVST pulse. Without it
 *   `Convert()` and a hardware reset return `Unsupported`, and so does single-shot calibration.
 * - `bool GpioGet(CtrlPin, bool& high) noexcept` reads the DRDY/ALARM pin. Without it `ReadySource::Pin`
 *   waits and `ReadAlarmPin()` return `Unsupported`.
 * - `bool CaptureLanes(uint8_t lanes, uint8_t* rx, std::size_t len) noexcept` reads data on D7 to D0.
 *   Without it frames on D7 to D0 return `Unsupported`; one lane on SDOUT works.
 * - `void Log(LogLevel, const char* message) noexcept` takes one line per lifecycle event or failure.
 *   Without it nothing is logged.
 *
 * `void DelayUs(uint32_t us) noexcept` is required. A hook declared with the wrong signature is a
 * compile error.
 *
 * @tparam Port Your class derived from `SpiInterface<Port>`.
 */
// clang-format on
template <SpiPort Port>
class ADS9324 {
  static_assert(!detail::DeclaresGpioSet<Port> || detail::HasGpioSet<Port>,
                "ads9324: the optional GpioSet hook must be `bool "
                "GpioSet(CtrlPin, GpioSignal) "
                "noexcept`, so that a failed pin write is reported");
  static_assert(!detail::DeclaresGpioGet<Port> || detail::HasGpioGet<Port>,
                "ads9324: the optional GpioGet hook must be `bool "
                "GpioGet(CtrlPin, bool& high) "
                "noexcept`");
  static_assert(!detail::DeclaresCaptureLanes<Port> || detail::HasCaptureLanes<Port>,
                "ads9324: the optional CaptureLanes hook must be `bool "
                "CaptureLanes(uint8_t, "
                "uint8_t*, std::size_t) noexcept`");
  static_assert(!detail::DeclaresDelayUs<Port> || detail::HasDelayUs<Port>,
                "ads9324: the DelayUs hook must be `void DelayUs(uint32_t us) "
                "noexcept`, so that no "
                "wait is truncated");
  static_assert(!detail::DeclaresLog<Port> || detail::HasLog<Port>,
                "ads9324: the optional Log hook must be `void Log(LogLevel, "
                "const char* message) "
                "noexcept`");

public:
  /**
   * @brief Bind the driver to a port and store the configuration. No I/O,
   * cannot fail.
   * @param port The port; it must outlive the driver.
   * @param config What `EnsureInitialized()` will apply; checked there, before
   * any I/O.
   */
  explicit ADS9324(Port& port, const Config& config = {}) noexcept;

  /** @brief One driver object per chip. */
  ADS9324(const ADS9324&) = delete;
  /** @brief One driver object per chip. */
  ADS9324& operator=(const ADS9324&) = delete;

  /// @name Lifecycle
  /// @{

  /**
   * @brief Start the chip, once: the only way to initialise the driver.
   *
   * Brings the port up, waits the 30 ms power-up time, resets the chip (the
   * RESET pin when `Config::hardware_reset` is set, else SW_RST), reads
   * DEVICE_ID and BANK_SEL, waits for CALIB_BUSY to clear, clears
   * RESET_DETECT_FLAG, writes the configuration and optionally runs the ADC
   * calibration. After it succeeds, later calls return `Ok` at once with no
   * I/O. After it fails, the next call starts again from the beginning.
   *
   * @return `Ok`, `InvalidArgument` (the configuration fails `Validate()`; no
   * I/O), `Unsupported` (daisy chain, or a hardware reset without `GpioSet()`),
   * `Bus`, `Timeout`, `DeviceIdMismatch`, `VerifyFailed` or the calibration
   * errors.
   */
  Result<void> EnsureInitialized() noexcept;

  /** @return true after a successful `EnsureInitialized()` and before
   * `Deinitialize()`. No I/O. */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  /** @return true between `PowerDown()` and `Wake()`. No I/O. */
  [[nodiscard]] bool IsPoweredDown() const noexcept {
    return powered_down_;
  }

  /**
   * @brief Put the chip in standby and mark the driver uninitialised, even if
   * the write failed.
   * @return `Ok` or `Bus`; `Ok` with no I/O on an uninitialised driver.
   */
  Result<void> Deinitialize() noexcept;

  /**
   * @brief Reset the chip and write the stored configuration again.
   * @param kind `Software` (SW_RST) or `Hardware` (a pulse on the RESET pin).
   * @return `Ok`, `NotInitialized`, `InvalidArgument` (`kind` is not an
   * enumerator), `Unsupported` (no `GpioSet()` for `Hardware`), `Bus`,
   * `Timeout`, `DeviceIdMismatch`.
   */
  Result<void> Reset(ResetKind kind) noexcept;

  /**
   * @brief Read DEVICE_ID.
   * @return 0x0004, or `NotInitialized` / `Bus`.
   */
  Result<uint16_t> Identify() noexcept;

  /// @}
  /// @name Configuration
  /// @{

  /**
   * @brief Change the whole configuration. Before the start it is only stored,
   * with no I/O.
   * @param config Checked with `Validate()` before any I/O.
   * @param verify `ReadBack` reads every register written and compares (one
   * two-frame read each).
   * @return `Ok`, `InvalidArgument`, `Unsupported` (daisy chain), `Refused`
   * (powered down), `Bus` or `VerifyFailed`. The stored configuration changes
   * only on `Ok`.
   */
  Result<void> ApplyConfig(const Config& config, Verify verify = Verify::None) noexcept;

  /** @return The configuration last applied or stored. No I/O. */
  [[nodiscard]] const Config& GetConfig() const& noexcept {
    return config_;
  }
  /** @brief Not on a temporary driver: the reference would dangle
   * (KB-CPP20-044). */
  void GetConfig() const&& = delete;

  /**
   * @brief Change one channel's input settings (range, type, filter, offset,
   * gain, phase, window).
   * @param channel 0 (AIN1) to 15 (AIN16).
   * @param input The new settings.
   * @param verify Read back what was written.
   * @return As `ApplyConfig()`; `InvalidArgument` for a channel above 15.
   */
  Result<void> SetInput(uint8_t channel, const Input& input, Verify verify = Verify::None) noexcept;

  /** @brief Change the data interface. @param interface The new settings.
   * @param verify Read back.
   *  @return As `ApplyConfig()`. */
  Result<void> SetInterface(const Interface& interface, Verify verify = Verify::None) noexcept;

  /** @brief Change the digital filter. @param filter The new filter. @param
   * verify Read back.
   *  @return As `ApplyConfig()`. A changed filter is followed by the SYSREF
   * pulse. */
  Result<void> SetFilter(const Filter& filter, Verify verify = Verify::None) noexcept;

  /** @brief Choose the channels in the data frame. @param select Count and
   * group.
   *  @param verify Read back. @return As `ApplyConfig()`. */
  Result<void> SetChannels(const ChannelSelect& select, Verify verify = Verify::None) noexcept;

  /** @brief Change the DRDY/ALARM pin. @param alarm The new settings. @param
   * verify Read back.
   *  @return As `ApplyConfig()`. */
  Result<void> SetAlarm(const Alarm& alarm, Verify verify = Verify::None) noexcept;

  /** @brief Change the reference source (30 ms settle follows). @param source
   * The source.
   *  @param verify Read back. @return As `ApplyConfig()`. */
  Result<void> SetReference(ReferenceSource source, Verify verify = Verify::None) noexcept;

  /**
   * @brief Change the test pattern of one channel group.
   * @param group 0 (AIN1 to AIN8) or 1 (AIN9 to AIN16).
   * @param pattern The pattern.
   * @param verify Read back.
   * @return As `ApplyConfig()`; `InvalidArgument` for a group above 1.
   */
  Result<void> SetTestPattern(uint8_t group, const TestPattern& pattern,
                              Verify verify = Verify::None) noexcept;

  /// @}
  /// @name Conversion and readout
  /// @{

  /**
   * @brief Start one conversion with a pulse on CONVST (high, low, high: the
   * falling edge samples, and the line is high again before DRDY rises).
   * With a FIR filter or an external-clock block average one pulse is not
   * a result: see `PulsesPerResult()`; `MeasureOnce()` sends them all.
   * @return `Ok`, `NotInitialized`, `Refused` (powered down), `Unsupported` (no
   * `GpioSet()`; drive CONVST yourself) or `Bus`.
   */
  Result<void> Convert() noexcept;

  /**
   * @brief Wait until a result is ready: the DRDY/ALARM pin
   * (`ReadySource::Pin`), or the conversion time.
   * @return `Ok`, `NotInitialized`, `Refused`, `Unsupported` (pin wait without
   * `GpioGet()`, or the pin is not set to DRDY), `Timeout`
   * (`Sampling::ready_timeout_us`) or `Bus`.
   */
  Result<void> WaitDataReady() noexcept;

  /**
   * @brief Read and decode one data frame.
   *
   * The frame is read on SDOUT (one lane) or on the lanes through
   * `CaptureLanes()`; the transmit side is all zeros, so no register is written
   * by accident. A DIAG byte with RESET_DETECT_FLAG makes the driver
   * uninitialised and returns `UnexpectedReset`.
   *
   * @return The frame, or `NotInitialized`, `Refused`, `Unsupported`, `Bus`,
   * `UnexpectedReset`, `DataMismatch` (an all-0x00 or all-0xFF frame, unless a
   * test pattern is on).
   */
  Result<Frame> ReadFrame() noexcept;

  /**
   * @brief `Convert()`, `WaitDataReady()` and `ReadFrame()`.
   * @return The frame, or the errors of those calls.
   */
  Result<Frame> MeasureOnce() noexcept;

  /**
   * @brief The level of the DRDY/ALARM pin as asserted or not, with
   * `Alarm::active_low` applied.
   * @return true when the pin function is asserted, or `NotInitialized`,
   * `Unsupported`, `Bus`.
   */
  Result<bool> ReadAlarmPin() noexcept;

  /// @}
  /// @name Window comparator and calibration
  /// @{

  /**
   * @brief Read the latched window comparator flags of both banks.
   * @return The flags per channel, or `NotInitialized`, `Refused`, `Bus`.
   */
  Result<WindowFlags> ReadWindowFlags() noexcept;

  /**
   * @brief Clear the latched flags of a group (DWC_STAT_RST written 1, then 0).
   * @param group 0 (AIN1 to AIN8) or 1 (AIN9 to AIN16).
   * @return `Ok`, `InvalidArgument`, `NotInitialized`, `Refused` or `Bus`.
   */
  Result<void> ClearWindowFlags(uint8_t group) noexcept;

  /**
   * @brief Run the ADC calibration with `Config::calibration`.
   *
   * Single shot: INT_TRIG_MODE, ADC_CAL_MODE, TRIG 0 to 1 and back, one CONVST
   * pulse ending high, then at most 125 ms plus 25 ms of CALIB_BUSY polling.
   * Continuous: expects a free-running CONVST and waits 100,000 periods.
   * ADC_CAL_MODE stays set afterwards (mode 0 would remove the calibration).
   *
   * @return `Ok`, `NotInitialized`, `Refused` (powered down, or a digital
   * filter is on), `Unsupported` (single shot without `GpioSet()`), `Timeout`
   * or `Bus`.
   */
  Result<void> RunAdcCalibration() noexcept;

  /**
   * @brief Write ADC_CAL_MODE 0: the calibration values are no longer applied.
   * @return `Ok`, `NotInitialized`, `Refused` or `Bus`.
   */
  Result<void> DisableAdcCalibration() noexcept;

  /**
   * @brief Read CALIB_BUSY_FLAG.
   * @return true while a calibration runs, or `NotInitialized`, `Bus`.
   */
  Result<bool> IsCalibrationBusy() noexcept;

  /// @}
  /// @name Power and diagnostics
  /// @{

  /**
   * @brief Enter standby (PDN_CTL.DEVICE_PDN). Other calls return `Refused`
   * until `Wake()`. A switch-off is never refused for doubt: after a reset report or a line fault
   * the standby write is still sent, and the error found is returned.
   * @return `Ok`; `NotInitialized` only when no frame was ever sent; `Bus`,
   * `DeviceIdMismatch` or `UnexpectedReset` when the check before the write failed (standby was
   * still written).
   */
  Result<void> PowerDown() noexcept;

  /**
   * @brief Leave standby, wait 30 ms and check that the chip kept its
   * registers.
   * @return `Ok`; `UnexpectedReset` (RESET_DETECT_FLAG: the registers were
   * lost; the driver is uninitialised); `VerifyFailed` (GEN_CFG3 differs from
   * the shadow); `NotInitialized`, `Refused` (not powered down) or `Bus`.
   */
  Result<void> Wake() noexcept;

  /**
   * @brief Read one documented register once: bank select, command and data
   * frame.
   * @param bank The bank.
   * @param address A documented address of that bank.
   * @return The value, or `NotInitialized`, `InvalidArgument` (nothing sent) or
   * `Bus`.
   */
  Result<uint16_t> ReadRegister(reg::Bank bank, uint8_t address) noexcept;

  /**
   * @brief The raw value the last register read returned. No bus traffic.
   * @return The last value `ReadRegister` or a verify read-back received; 0
   * before any read.
   */
  [[nodiscard]] uint16_t GetLastRegisterReply() const noexcept {
    return last_reply_;
  }

  /**
   * @brief Check the data path with constant test patterns on all 16 channels:
   * every channel must return its own pattern, which also proves the
   * channel-to-register mapping of both banks. The configuration is restored
   * afterwards.
   * @return `Ok`, `DataMismatch` (a channel returned another value), or the
   * errors of the calls it makes.
   */
  Result<void> RunDataPathSelfTest() noexcept;

  /**
   * @brief The register value the driver believes the chip holds. No I/O.
   * @param bank The bank.
   * @param address The address.
   * @return The shadow value; 0 for an undocumented address.
   */
  [[nodiscard]] uint16_t GetShadow(reg::Bank bank, uint8_t address) const noexcept;

  /// @}

private:
  Result<void> startChip(const Config& config, bool hardware) noexcept;
  Result<void> resetChip(bool hardware) noexcept;
  Result<void> checkIdentity() noexcept;
  Result<void> waitCalibrationIdle(uint32_t timeout_ms, bool confirm) noexcept;
  Result<void> checkReady() const noexcept;
  Result<void> probeReset() noexcept;
  Result<void> checkAlive() noexcept;
  Result<void> confirmIdentity() noexcept;
  Result<void> applyImage(const Config& config, Verify verify) noexcept;
  Result<void> flushImage(const detail::RegisterImage& image) noexcept;
  Result<void> flushPhase(const detail::RegisterImage& image, uint32_t phase) noexcept;
  Result<void> flushRegister(reg::Bank bank, uint8_t address, uint16_t value) noexcept;
  Result<void> pulseSysref(uint32_t convst_hz) noexcept;
  Result<void> verifyWritten() noexcept;
  Result<void> writeRegister(reg::Bank bank, uint8_t address, uint16_t value) noexcept;
  Result<uint16_t> readRegister(reg::Bank bank, uint8_t address) noexcept;
  Result<void> selectBank(reg::Bank bank) noexcept;
  Result<void> setConvst(GpioSignal level) noexcept;
  Result<void> pulseConvst() noexcept;
  Result<void> readFrameBytes(std::span<uint8_t> bytes) noexcept;
  Result<void> beginCalibration(uint16_t base, uint16_t mode) noexcept;
  Result<void> finishCalibration(bool single_shot) noexcept;
  Result<void> calibrate(uint16_t mode, bool single_shot) noexcept;
  Result<void> runPatternFrame(const Config& test, Frame& frame) noexcept;
  Result<void> restoreConfig(const Config& saved, Result<void> outcome) noexcept;
  void resetShadow() noexcept;
  Result<void> fail(Error error, const char* message) noexcept;

  detail::RegisterBus<Port> bus_;
  Config config_{};
  std::array<std::array<uint16_t, reg::kAddressSpace>, reg::kBankCount> shadow_{};
  std::array<uint64_t, reg::kBankCount> dirty_{};   ///< Bit set: the chip's value is unknown.
  std::array<uint64_t, reg::kBankCount> written_{}; ///< Written by the current call.
  uint16_t cal_mode_{0};                            ///< ADC_CAL_MODE bits kept in the image.
  reg::Bank bank_{reg::Bank::Common}; ///< The bank BANK_SEL holds, when `bank_valid_`.
  bool bank_valid_{false};
  bool filter_touched_{false}; ///< A filter or phase register was written by the current call.
  bool initialized_{false};
  bool powered_down_{false};
  bool chip_live_{false};  ///< Frames were sent since the last `Deinitialize()`.
  bool layout_ok_{false};  ///< `config_` is what the chip's layout registers hold.
  uint16_t last_reply_{0}; ///< The last raw register reply.
  std::array<uint8_t, kMaxFrameBytes>
      frame_buffer_{}; ///< Receive buffer of `ReadFrame()` (DMA reach).
};

/** @} */

} // namespace ads9324

// codeql::misra_deviation_next_line(MD-CPP-009)
#include "../src/ads9324.ipp" // IWYU pragma: export

#endif // ADS9324_HPP
