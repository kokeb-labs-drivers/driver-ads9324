/**
 * @file ads9324_types.hpp
 * @brief Public types of the ADS9324 driver: errors, `Result<T>`, enumerations,
 * configuration, the data frame and the pure helpers that decode it.
 * @copyright Copyright (c) 2026 The driver-ads9324 authors.
 * SPDX-License-Identifier: MIT
 *
 * @details
 * Everything here is plain data and `constexpr` code with no platform
 * dependency, so tests and tools can include it without the driver template.
 * Channels are counted from 0: channel 0 is AIN1 and channel 15 is AIN16.
 * Voltages are integer microvolts; the driver has no floating point.
 */
#ifndef ADS9324_TYPES_HPP
#define ADS9324_TYPES_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "ads9324_detail.hpp"
#include "ads9324_registers.hpp"
#include "ads9324_version.h"

/** @brief The ADS9324 driver: everything it declares lives here; internals in
 * `detail`. */
namespace ads9324 {

/**
 * @defgroup ads9324_types Driver types
 * @brief Errors, results, enumerations, configuration and frame decoding.
 * @{
 */

/**
 * @brief Why a call failed. Every call of the driver returns one of these in
 * its `Result`.
 *
 * Values 0 to 9 are the shared base set; chip-specific values start at 16.
 * `Crc` and `Busy` are in the set but this chip never produces them (it has no
 * CRC and no busy reply).
 */
enum class [[nodiscard]] Error : uint8_t {
  Ok = 0,               ///< The call succeeded.
  NotInitialized = 1,   ///< `EnsureInitialized()` has not succeeded, or the driver was reset.
  InvalidArgument = 2,  ///< An argument or a `Config` field is out of range; nothing was sent.
  Bus = 3,              ///< A port hook returned `false`.
  Timeout = 4,          ///< A flag did not change in the allowed time.
  DeviceIdMismatch = 5, ///< DEVICE_ID or BANK_SEL did not read as a ADS9324 after a reset.
  Crc = 6,              ///< Never returned: the chip has no CRC.
  Busy = 7,             ///< Never returned: the chip has no busy reply.
  DeviceFault = 8,      ///< Reserved for a chip fault flag; the ADS9324 reports none.
  Unsupported = 9,      ///< The port hook or chip feature this call needs is absent.
  UnexpectedReset = 16, ///< RESET_DETECT_FLAG is set: the chip reset since the start.
  VerifyFailed = 17,    ///< A read-back differs from what was written.
  Refused = 18,         ///< The call is not allowed in the current state; nothing was sent.
  DataMismatch = 19,    ///< A data frame is not what the chip can produce (stuck
                        ///< line, self-test).
};

/**
 * @brief The name of an error, for logs.
 * @param error The error.
 * @return A static string such as `"Timeout"`; `"Unknown"` for a value that is
 * not an enumerator.
 */
constexpr const char* ToString(Error error) noexcept {
  switch (error) {
  case Error::Ok:
    return "Ok";
  case Error::NotInitialized:
    return "NotInitialized";
  case Error::InvalidArgument:
    return "InvalidArgument";
  case Error::Bus:
    return "Bus";
  case Error::Timeout:
    return "Timeout";
  case Error::DeviceIdMismatch:
    return "DeviceIdMismatch";
  case Error::Crc:
    return "Crc";
  case Error::Busy:
    return "Busy";
  case Error::DeviceFault:
    return "DeviceFault";
  case Error::Unsupported:
    return "Unsupported";
  case Error::UnexpectedReset:
    return "UnexpectedReset";
  case Error::VerifyFailed:
    return "VerifyFailed";
  case Error::Refused:
    return "Refused";
  case Error::DataMismatch:
    return "DataMismatch";
  default:
    break;
  }
  return "Unknown";
}

/**
 * @brief The value of a call, or why there is none.
 *
 * Check `ok()` (or the `bool` conversion) before reading `value`; on failure
 * `value` is `T{}`.
 *
 * @tparam T The value type. `Result<void>` carries only the error.
 */
template <typename T>
struct [[nodiscard]] Result {
  T value{};              ///< The value; `T{}` when the call failed.
  Error error{Error::Ok}; ///< `Error::Ok` on success.

  /** @return true when the call succeeded. */
  [[nodiscard]] constexpr bool ok() const noexcept {
    return error == Error::Ok;
  }
  /** @return true when the call succeeded. */
  constexpr explicit operator bool() const noexcept {
    return ok();
  }
  /**
   * @brief A successful result.
   * @param v The value.
   * @return `{v, Error::Ok}`.
   */
  static constexpr Result Success(T v) noexcept {
    return {v, Error::Ok};
  }
  /**
   * @brief A failed result.
   * @param e Why it failed; never `Error::Ok`.
   * @return `{T{}, e}`.
   */
  static constexpr Result Failure(Error e) noexcept {
    return {T{}, e};
  }
};

/** @brief The result of a call that returns no value. */
template <>
struct [[nodiscard]] Result<void> {
  Error error{Error::Ok}; ///< `Error::Ok` on success.

  /** @return true when the call succeeded. */
  [[nodiscard]] constexpr bool ok() const noexcept {
    return error == Error::Ok;
  }
  /** @return true when the call succeeded. */
  constexpr explicit operator bool() const noexcept {
    return ok();
  }
  /** @return A successful result. */
  static constexpr Result Success() noexcept {
    return {Error::Ok};
  }
  /**
   * @brief A failed result.
   * @param e Why it failed; never `Error::Ok`.
   * @return `{e}`.
   */
  static constexpr Result Failure(Error e) noexcept {
    return {e};
  }
};

/** @brief Severity passed to the optional `Log()` hook. */
enum class LogLevel : uint8_t {
  Error = 0, ///< A call failed.
  Warning,   ///< Something unexpected that the driver recovered from.
  Info,      ///< Lifecycle events.
  Debug,     ///< Detail for bring-up.
};

/** @brief Whether a setter reads back what it wrote (costs one two-frame read
 * per register). */
enum class Verify : uint8_t {
  None = 0,     ///< No read-back.
  ReadBack = 1, ///< Read every register the call touched and compare with the shadow.
};

/** @brief How `Reset()` resets the chip. */
enum class ResetKind : uint8_t {
  Software = 0, ///< SW_RST in GEN_CFG1.
  Hardware = 1, ///< A pulse on the RESET pin through `GpioSet()`.
};

/** @brief Input type of a channel (CM_RANGE_AINn, Table 7-1). */
enum class InputType : uint8_t {
  Differential = 0,            ///< Differential, common mode up to +-12.5 V.
  SingleEnded = 5,             ///< AINnM tied to ground.
  SingleEndedOpenWireSafe = 6, ///< Single ended with the least code change on an open wire.
};

/** @brief Input range of a channel (INPUT_RANGE_AINn, Table 7-1). */
enum class InputRange : uint8_t {
  V5 = 0,    ///< +-5 V.
  V2_5 = 2,  ///< +-2.5 V.
  V6_25 = 3, ///< +-6.25 V.
  V10 = 4,   ///< +-10 V.
  V12_5 = 5, ///< +-12.5 V.
};

/** @brief Analog low-pass filter of a channel (PGA_BW_SEL_AINn, Table 7-2). */
enum class Bandwidth : uint8_t {
  Low = 0,  ///< 25.5 kHz for every range.
  Wide = 1, ///< 280 kHz to 350 kHz, by range.
};

/** @brief Number of data lanes (DOUT_LANE_SEL, Table 7-11). */
enum class Lanes : uint8_t {
  Eight = 0, ///< D7 to D0. Reset value.
  Four = 1,  ///< D7 to D4.
  Two = 2,   ///< D7 and D6.
  One = 3,   ///< D7, or SDOUT with `Interface::data_on_sdout`.
};

/** @brief Bits per conversion result on the data interface (DOUT_LENGTH). */
enum class WordLength : uint8_t {
  Bits16 = 0, ///< 16 bits. Reset value.
  Bits24 = 2, ///< 24 bits: the result and an extra byte.
};

/** @brief Coding of the conversion result (EN_OFS_BINARY). */
enum class DataFormat : uint8_t {
  TwosComplement = 0, ///< Binary two's complement. Reset value.
  OffsetBinary = 1,   ///< Straight offset binary.
};

/** @brief Drive strength of the digital outputs (DRIVE_STRENGTH, Table 7-15).
 */
enum class DriveStrength : uint8_t {
  Normal = 0,     ///< 1 x. Reset value.
  Half = 1,       ///< 0.5 x.
  Double = 2,     ///< 2 x.
  OneAndHalf = 3, ///< 1.5 x.
};

/** @brief Where the ADC reference comes from (Table 7-3). */
enum class ReferenceSource : uint8_t {
  Pin = 0,      ///< The REFSEL pin chooses. Reset value.
  Internal = 1, ///< Internal 4.096 V reference, REFSEL ignored.
  External = 2, ///< External reference on REFIO, REFSEL ignored. No reference:
                ///< the chip powers down.
};

/** @brief How many channels the data frame carries (ADC_NUM_SEL, Table 7-12).
 */
enum class ChannelCount : uint8_t {
  All16 = 0, ///< 16 channels. Reset value.
  Eight = 1, ///< 8 channels; `group` 0 or 1.
  Four = 2,  ///< 4 channels; `group` 0 to 3.
  Two = 3,   ///< 2 channels; `group` 0 to 7.
};

/** @brief What the DRDY/ALARM pin shows (DRDY_ALRM_SEL, Table 7-10). */
enum class AlarmFunction : uint8_t {
  Drdy = 0,        ///< ADC data ready. Reset value.
  WindowAlert = 1, ///< Digital window comparator alert.
  CalDone = 6,     ///< ADC calibration done.
  OrMasked = 8,    ///< The OR of the sources selected by `Alarm::sources` (bit map
                   ///< unverified).
};

/** @brief Sources of the OR output; bit n of `Alarm::sources` is the source
 * with code n. */
inline constexpr uint8_t kAlarmSourceDrdy = 0x01;    ///< Source code 0, DRDY.
inline constexpr uint8_t kAlarmSourceWindow = 0x02;  ///< Source code 1, window alert.
inline constexpr uint8_t kAlarmSourceCalDone = 0x40; ///< Source code 6, ADC calibration done.

/** @brief Length of a block or moving average (BLK_AVG_OSR, MVG_AVG_LEN). */
enum class AverageLength : uint8_t {
  None = 0,
  X2,
  X4,
  X6,
  X8,
  X10,
  X12,
  X16,
  X20,
  X32,
  X64,
  X128,
};

/** @brief The FIR low-pass filters (FIR_FILT_SEL, Table 7-9). FIR6 is reserved
 * and absent. */
enum class FirFilter : uint8_t {
  Fir1 = 1, ///< Decimation 2.
  Fir2 = 2, ///< Decimation 2.
  Fir3 = 3, ///< Decimation 4.
  Fir4 = 4, ///< Decimation 2.
  Fir5 = 5, ///< Decimation 4.
  Fir7 = 7, ///< Decimation 8.
};

/** @brief Which one digital filter is in the data path (section 7.3.11). */
enum class FilterKind : uint8_t {
  None = 0,          ///< No filter.
  BlockAverage = 1,  ///< Block average over `Filter::length` conversions.
  MovingAverage = 2, ///< Moving average over `Filter::length` conversions.
  Fir = 3,           ///< A FIR filter, `Filter::fir`.
};

/** @brief Kind of a test pattern (TP_MODE, Table 7-14). */
enum class PatternKind : uint8_t {
  Constant = 0, ///< A fixed 16-bit value per channel.
  Ramp = 2,     ///< A ramp whose step is the value of the group's first channel
                ///< plus one.
};

/** @brief How `WaitDataReady()` learns that a result is ready. */
enum class ReadySource : uint8_t {
  Time = 0, ///< Wait the conversion time (no pin needed).
  Pin = 1,  ///< Poll the DRDY/ALARM pin through `GpioGet()`.
};

/** @brief Which errors ADC_CAL removes (ADC_CAL_MODE). */
enum class CalibrationMode : uint8_t {
  Offset = 1,        ///< Offset error.
  Gain = 2,          ///< Gain error.
  OffsetAndGain = 3, ///< Both.
};

/** @brief How the CONVST clock behaves during ADC calibration (section 7.3.10).
 */
enum class CalibrationTrigger : uint8_t {
  SingleShot = 0, ///< INT_TRIG_MODE = 1: one CONVST pulse, 125 ms.
  Continuous = 1, ///< A free-running CONVST: 100,000 periods.
};

namespace detail {
/// Last enumerator of `Verify` (detail::IsEnumerator).
template <>
struct EnumLast<Verify> {
  static constexpr Verify kValue = Verify::ReadBack; ///< The last enumerator.
};
/// Last enumerator of `ResetKind`.
template <>
struct EnumLast<ResetKind> {
  static constexpr ResetKind kValue = ResetKind::Hardware; ///< The last enumerator.
};
/// Last enumerator of `Bandwidth`.
template <>
struct EnumLast<Bandwidth> {
  static constexpr Bandwidth kValue = Bandwidth::Wide; ///< The last enumerator.
};
/// Last enumerator of `Lanes`.
template <>
struct EnumLast<Lanes> {
  static constexpr Lanes kValue = Lanes::One; ///< The last enumerator.
};
/// Last enumerator of `DataFormat`.
template <>
struct EnumLast<DataFormat> {
  static constexpr DataFormat kValue = DataFormat::OffsetBinary; ///< The last enumerator.
};
/// Last enumerator of `DriveStrength`.
template <>
struct EnumLast<DriveStrength> {
  static constexpr DriveStrength kValue = DriveStrength::OneAndHalf; ///< The last enumerator.
};
/// Last enumerator of `ReferenceSource`.
template <>
struct EnumLast<ReferenceSource> {
  static constexpr ReferenceSource kValue = ReferenceSource::External; ///< The last enumerator.
};
/// Last enumerator of `ChannelCount`.
template <>
struct EnumLast<ChannelCount> {
  static constexpr ChannelCount kValue = ChannelCount::Two; ///< The last enumerator.
};
/// Last enumerator of `AverageLength`.
template <>
struct EnumLast<AverageLength> {
  static constexpr AverageLength kValue = AverageLength::X128; ///< The last enumerator.
};
/// Last enumerator of `FilterKind`.
template <>
struct EnumLast<FilterKind> {
  static constexpr FilterKind kValue = FilterKind::Fir; ///< The last enumerator.
};
/// Last enumerator of `ReadySource`.
template <>
struct EnumLast<ReadySource> {
  static constexpr ReadySource kValue = ReadySource::Pin; ///< The last enumerator.
};
/// Last enumerator of `CalibrationTrigger`.
template <>
struct EnumLast<CalibrationTrigger> {
  static constexpr CalibrationTrigger kValue = CalibrationTrigger::Continuous; ///< Last.
};
} // namespace detail

/**
 * @brief Whether an input type holds a documented code.
 * @param v The value to check.
 * @return true when it is an enumerator the chip documents.
 */
constexpr bool IsValid(InputType v) noexcept {
  switch (v) {
  case InputType::Differential:
  case InputType::SingleEnded:
  case InputType::SingleEndedOpenWireSafe:
    return true;
  default:
    break;
  }
  return false;
}
/**
 * @brief Whether an input range holds a documented code.
 * @param v The value to check.
 * @return true when it is an enumerator the chip documents.
 */
constexpr bool IsValid(InputRange v) noexcept {
  switch (v) {
  case InputRange::V5:
  case InputRange::V2_5:
  case InputRange::V6_25:
  case InputRange::V10:
  case InputRange::V12_5:
    return true;
  default:
    break;
  }
  return false;
}
/**
 * @brief Whether a word length holds a documented code (01b is reserved).
 * @param v The value to check.
 * @return true when it is an enumerator the chip documents.
 */
constexpr bool IsValid(WordLength v) noexcept {
  return (v == WordLength::Bits16) || (v == WordLength::Bits24);
}
/**
 * @brief Whether an alarm function holds a documented code.
 * @param v The value to check.
 * @return true when it is an enumerator the chip documents.
 */
constexpr bool IsValid(AlarmFunction v) noexcept {
  switch (v) {
  case AlarmFunction::Drdy:
  case AlarmFunction::WindowAlert:
  case AlarmFunction::CalDone:
  case AlarmFunction::OrMasked:
    return true;
  default:
    break;
  }
  return false;
}
/**
 * @brief Whether a FIR filter holds a documented code (110b is reserved).
 * @param v The value to check.
 * @return true when it is an enumerator the chip documents.
 */
constexpr bool IsValid(FirFilter v) noexcept {
  const auto code = static_cast<uint32_t>(detail::ToUnderlying(v));
  return (code >= 1U) && (code <= 7U) && (code != 6U);
}
/**
 * @brief Whether a test pattern kind holds a documented code (only 0 and 2 are
 * reachable).
 * @param v The value to check.
 * @return true when it is an enumerator the chip documents.
 */
constexpr bool IsValid(PatternKind v) noexcept {
  return (v == PatternKind::Constant) || (v == PatternKind::Ramp);
}
/**
 * @brief Whether a calibration mode holds a documented code.
 * @param v The value to check.
 * @return true when it is an enumerator the chip documents.
 */
constexpr bool IsValid(CalibrationMode v) noexcept {
  const auto code = static_cast<uint32_t>(detail::ToUnderlying(v));
  return (code >= 1U) && (code <= 3U);
}

/** @brief The digital window comparator of one channel (section 7.3.12). */
struct Window {
  bool enabled{false};   ///< DWC_EN: the digital window comparator watches this channel.
  int8_t high_code{127}; ///< High threshold on the top byte of the code (two's complement).
  int8_t low_code{-128}; ///< Low threshold on the top byte of the code.
  uint8_t hysteresis{0}; ///< Hysteresis on both thresholds, in top-byte counts.
};

/** @brief The configuration of one input channel. */
struct Input {
  InputType type{InputType::Differential}; ///< Differential, single ended, open-wire safe.
  InputRange range{InputRange::V5};        ///< Full-scale range.
  Bandwidth bandwidth{Bandwidth::Low};     ///< Analog low-pass filter.
  bool cm_correction{false};               ///< Common mode error correction (CME_CORR_EN).
  int16_t offset_quarter_lsb{0};           ///< OFS: -512 to 511 quarter LSBs, added first.
  int16_t gain{0};                         ///< GAN: -8192 to 8191, result x (1 + gain / 65536).
  uint8_t phase_delay{0};                  ///< Sample delay in CONVST periods, 0 to 255.
  Window window{};                         ///< The window comparator of this channel.
};

/** @brief The data interface (GEN_CFG3, XOR_BITS_CTL, GEN_CFG4,
 * DIG_DELAY_CFG1/2). */
struct Interface {
  Lanes lanes{Lanes::One};                       ///< Lane count; `Config{}` uses one lane.
  bool data_on_sdout{true};                      ///< One lane on SDOUT (true) or D7 (false).
  WordLength length{WordLength::Bits16};         ///< 16 or 24 bits per channel.
  DataFormat format{DataFormat::TwosComplement}; ///< Coding.
  bool append_diag{false};                       ///< Append the DIAG byte (EN_DIAG_FLAG).
  bool randomize{false};                         ///< XOR the data with PRBS, append the PRBS byte.
  DriveStrength drive{DriveStrength::Normal};    ///< Output drive strength.
  std::array<uint8_t, 9> delay_ns{};             ///< Output delay 0 to 5 ns: SDOUT, then D0 to D7.
};

/** @brief Which channels the data frame carries (ANA_CFG2). */
struct ChannelSelect {
  ChannelCount count{ChannelCount::All16}; ///< How many channels.
  uint8_t group{0};                        ///< Which group of that size (ADC_CH_SEL).
};

/** @brief The digital filter in the data path (DIG_FILTER, GEN_CFG5). */
struct Filter {
  FilterKind kind{FilterKind::None};         ///< Which filter.
  AverageLength length{AverageLength::None}; ///< For the two average kinds.
  FirFilter fir{FirFilter::Fir1};            ///< For `FilterKind::Fir`.
  bool internal_clock{false};                ///< Block average: internal oversampling clock.
};

/** @brief The DRDY/ALARM pin (DRDY_ALARM_SEL, GEN_CFG4). */
struct Alarm {
  AlarmFunction function{AlarmFunction::Drdy}; ///< What the pin shows.
  bool active_low{false};                      ///< ALRM_POL, applies to every function.
  bool pulse{false};                           ///< ALRM_TYPE: pulse instead of level (window).
  bool disabled{false};                        ///< ALRM_DIS: the alarm function is off.
  uint8_t sources{0};                          ///< For `OrMasked`: the sources in the OR.
  bool or_mask_acknowledged{false}; ///< The caller accepts that the mask sense is unverified.
};

/** @brief CONVST timing and the ready wait (section 5.6). */
struct Sampling {
  uint32_t convst_hz{100'000};          ///< CONVST frequency, 100 kHz to 1 MHz.
  ReadySource ready{ReadySource::Time}; ///< How a result is known to be ready.
  uint32_t ready_timeout_us{10'000};    ///< Longest pin wait, 1 to `kReadyTimeoutMaxUs`.
  bool reject_stuck_frames{true};       ///< `ReadFrame()` refuses an all-0x00 or all-0xFF frame.
};

/** @brief System offset and gain correction stages (GEN_CFG5 of the channel
 * banks). */
struct Correction {
  bool offset_enabled{true}; ///< OFS stage in the data path.
  bool gain_enabled{true};   ///< GAN stage in the data path.
};

/** @brief Calibration at start-up and the mode `RunAdcCalibration()` uses. */
struct Calibration {
  bool run_at_start{false};                             ///< Run ADC_CAL in `EnsureInitialized()`.
  CalibrationMode mode{CalibrationMode::OffsetAndGain}; ///< Which errors.
  CalibrationTrigger trigger{CalibrationTrigger::SingleShot}; ///< CONVST behaviour.
};

/** @brief A test pattern of one channel group (TP_CFG, TP_AIN). */
struct TestPattern {
  bool enabled{false};                     ///< TP_EN.
  PatternKind kind{PatternKind::Constant}; ///< Constant or ramp.
  bool index_insertion{true};      ///< Add the channel index in the top nibble (TP_DIS_IDX = 0).
  bool step_every_convst{false};   ///< Ramp: step at every CONVST (TP_UPD_MODE).
  std::array<uint16_t, 8> value{}; ///< Pattern per channel; the first is the ramp step.
};

/** @brief Daisy chain settings: out of scope in this version, `Validate()`
 * refuses `enabled`. */
struct DaisyChain {
  bool enabled{false}; ///< Refused with `Unsupported`.
  uint8_t location{0}; ///< Position in the chain (unused).
  uint8_t devices{0};  ///< Devices in the chain (unused).
};

/** @brief Supplies in millivolts for the range checks; 0 means "not stated". */
struct Supply {
  uint32_t avdd_5v_mv{0};  ///< AVDD_5V, 4750 to 5250.
  uint32_t avdd_1v8_mv{0}; ///< AVDD_1V8, 1700 to 1900.
  uint32_t iovdd_mv{0};    ///< IOVDD, 1700 to 3600; the timing is specified at
                           ///< 1700 to 1900 only.
};

/**
 * @brief The default settings of the 16 inputs, spelled out because GCC skips
 * nested member initialisers in `inputs{}`.
 * @return Sixteen default `Input` values.
 */
constexpr std::array<Input, 16> DefaultInputs() noexcept {
  std::array<Input, 16> inputs{};
  for (Input& in : inputs) {
    in = Input{};
  }
  return inputs;
}

/**
 * @brief Everything `EnsureInitialized()` and `ApplyConfig()` write to the
 * chip.
 *
 * `Config{}` is the datasheet's initialisation example (Table 7-18: 16-bit
 * words on SDOUT, one lane) over the reset values of every other register;
 * `ChipResetConfig()` is the chip's reset state, eight lanes on D7 to D0.
 */
struct Config {
  Interface interface{};                           ///< Data interface.
  ReferenceSource reference{ReferenceSource::Pin}; ///< Reference selection.
  std::array<Input, 16> inputs{DefaultInputs()};   ///< AIN1 to AIN16.
  Filter filter{};                                 ///< Digital filter.
  ChannelSelect channels{};                        ///< Channels in the frame.
  bool low_speed_crosstalk{false}; ///< CH_XTALK_LOW_SPEED, for under 100 kSPS per channel.
  Alarm alarm{};                   ///< DRDY/ALARM pin.
  Sampling sampling{};             ///< CONVST and ready wait.
  Correction correction{};         ///< Offset and gain stages.
  Calibration calibration{};       ///< ADC calibration.
  std::array<TestPattern, 2> test_pattern{}; ///< Group 0 is AIN1 to AIN8, group 1 AIN9 to AIN16.
  std::array<uint8_t, 2> window_glitch{};    ///< DWC glitch filter per group, 0 to 15.
  DaisyChain daisy{};                        ///< Out of scope.
  Supply supply{};                           ///< Supply voltages for the range check.
  bool hardware_reset{false};                ///< Use the RESET pin at start-up (needs `GpioSet()`).
};

/** @brief The latched digital window comparator flags, one bit per channel (bit
 * 0 is AIN1). */
struct WindowFlags {
  uint16_t high{0}; ///< Channel n went above its high threshold.
  uint16_t low{0};  ///< Channel n went below its low threshold.
  /**
   * @brief Equal when both masks are.
   * @param lhs The left operand.
   * @param rhs The right operand.
   * @return true when `high` and `low` are equal.
   */
  friend constexpr bool operator==(const WindowFlags& lhs,
                                   const WindowFlags& rhs) noexcept = default;
};

/** @brief DEVICE_ID value of every ADS9324. */
inline constexpr uint16_t kDeviceId = 0x0004;
/** @brief Power-up wait before the first access, tPU (section 5.7). */
inline constexpr uint32_t kPowerUpMs = 30;
/** @brief Wait after a reset before the first access (tPU, A3). */
inline constexpr uint32_t kResetMs = 30;
/** @brief Shortest RESET low pulse the driver drives (a design choice,
 * section 7.4.1). */
inline constexpr uint32_t kResetPulseUs = 1000;
/** @brief Lowest CONVST frequency, fCONVST (section 5.6). */
inline constexpr uint32_t kConvstMinHz = 100'000;
/** @brief Highest CONVST frequency, fCONVST. */
inline constexpr uint32_t kConvstMaxHz = 1'000'000;
/** @brief Shortest CONVST high time. */
inline constexpr uint32_t kConvstHighMinNs = 50;
/** @brief Shortest CONVST low time. */
inline constexpr uint32_t kConvstLowMinNs = 50;
/** @brief Delay from the CONVST falling edge to the DRDY falling edge. */
inline constexpr uint32_t kDrdyLowDelayNs = 350;
/** @brief Conversion time without oversampling, tCONV. */
inline constexpr uint32_t kConversionNs = 1250;
/** @brief Highest SCLK frequency (section 5.6). */
inline constexpr uint32_t kSclkMaxHz = 60'000'000;
/** @brief The single-shot ADC calibration time, TWAIT (Table 7-7). */
inline constexpr uint32_t kSingleShotCalMs = 125;
/** @brief Extra time the driver polls CALIB_BUSY after the single-shot wait. */
inline constexpr uint32_t kCalMarginMs = 25;
/** @brief The free-running ADC calibration time in CONVST periods (Table 7-7).
 */
inline constexpr uint32_t kContinuousCalConvst = 100'000;
/** @brief Longest `Sampling::ready_timeout_us`. */
inline constexpr uint32_t kReadyTimeoutMaxUs = 200'000;
/** @brief Internal reference voltage in millivolts (section 5.5). */
inline constexpr uint32_t kInternalVrefMv = 4096;
/** @brief Input resistance of each AINnP and AINnM pin in ohms (section 7.3.3).
 */
inline constexpr uint32_t kInputResistanceOhm = 1'000'000;
/** @brief Longest data frame in bytes: 16 channels of 24 bits, DIAG and PRBS.
 */
inline constexpr std::size_t kMaxFrameBytes = 50;

/**
 * @brief The chip's reset state as a `Config`: eight lanes on D7 to D0,
 * everything else as `Config{}`.
 * @return The configuration.
 */
constexpr Config ChipResetConfig() noexcept {
  Config c{};
  c.interface.lanes = Lanes::Eight;
  c.interface.data_on_sdout = false;
  return c;
}

/** @brief Highest DWC glitch filter count (DWC_GLITCH_FILT, 4 bits). */
inline constexpr uint8_t kGlitchMax = 15;
/** @brief Largest output delay in nanoseconds (3 bits, section 8.1.10). */
inline constexpr uint8_t kDelayMaxNs = 5;
/** @brief OFS range in quarter LSBs (10 bits, two's complement). */
inline constexpr int32_t kOffsetMin = -512;
/** @brief OFS range in quarter LSBs. */
inline constexpr int32_t kOffsetMax = 511;
/** @brief GAN range (14 bits, two's complement). */
inline constexpr int32_t kGainMin = -8192;
/** @brief GAN range. */
inline constexpr int32_t kGainMax = 8191;

namespace detail {

/// Whether `v` lies in `[lo, hi]`.
constexpr bool InRange(int32_t v, int32_t lo, int32_t hi) noexcept {
  return (v >= lo) && (v <= hi);
}

/// `Ok`, or `InvalidArgument` when `condition` is false.
constexpr Error Check(bool condition) noexcept {
  return condition ? Error::Ok : Error::InvalidArgument;
}

/// Groups in a channel count: how many `group` values are valid.
constexpr uint8_t GroupCount(ChannelCount count) noexcept {
  switch (count) {
  case ChannelCount::All16:
    return 1U;
  case ChannelCount::Eight:
    return 2U;
  case ChannelCount::Four:
    return 4U;
  case ChannelCount::Two:
    return 8U;
  default:
    break;
  }
  return 0U;
}

constexpr Error ValidateInterface(const Interface& i) noexcept {
  bool ok =
      IsEnumerator(i.lanes) && IsEnumerator(i.format) && IsEnumerator(i.drive) && IsValid(i.length);
  // One data lane on SDOUT is the only lane layout that uses SDOUT (Table
  // 7-11).
  ok = ok && (!i.data_on_sdout || (i.lanes == Lanes::One));
  // The randomizer is not specified for 24-bit words (section 7.3.14.4).
  ok = ok && (!i.randomize || (i.length == WordLength::Bits16));
  for (const uint8_t d : i.delay_ns) {
    ok = ok && (d <= kDelayMaxNs);
  }
  return Check(ok);
}

constexpr Error ValidateWindow(const Window& w) noexcept {
  return Check(!w.enabled || (w.low_code <= w.high_code));
}

constexpr Error ValidateInput(const Input& in) noexcept {
  bool ok = IsValid(in.type) && IsValid(in.range) && IsEnumerator(in.bandwidth);
  ok = ok && InRange(in.offset_quarter_lsb, kOffsetMin, kOffsetMax);
  ok = ok && InRange(in.gain, kGainMin, kGainMax);
  return Check(ok);
}

constexpr bool IsDifferential(const Input& in) noexcept {
  return in.type == InputType::Differential;
}

/// Every channel in one bank has the same SE_DIFF class (ADC_CAL has one bit
/// per bank).
constexpr bool BankClassUniform(const std::array<Input, 16>& inputs, uint32_t first) noexcept {
  const bool first_class = IsDifferential(inputs[first]);
  for (uint32_t i = first; i < (first + 8U); ++i) {
    if (IsDifferential(inputs[i]) != first_class) {
      return false;
    }
  }
  return true;
}

constexpr Error ValidateInputs(const std::array<Input, 16>& inputs) noexcept {
  for (const Input& in : inputs) {
    const Error e = ValidateInput(in);
    if (e != Error::Ok) {
      return e;
    }
    const Error w = ValidateWindow(in.window);
    if (w != Error::Ok) {
      return w;
    }
  }
  return Check(BankClassUniform(inputs, 0U) && BankClassUniform(inputs, 8U));
}

constexpr Error ValidateFilter(const Filter& f) noexcept {
  bool ok = IsEnumerator(f.kind) && IsEnumerator(f.length) && IsValid(f.fir);
  const bool average =
      (f.kind == FilterKind::BlockAverage) || (f.kind == FilterKind::MovingAverage);
  ok = ok && (!average || (f.length != AverageLength::None));
  ok = ok && (!f.internal_clock || (f.kind == FilterKind::BlockAverage));
  return Check(ok);
}

constexpr Error ValidateAlarm(const Alarm& a) noexcept {
  bool ok = IsValid(a.function);
  const bool or_masked = a.function == AlarmFunction::OrMasked;
  ok = ok && (or_masked || (a.sources == 0U));
  ok = ok && (!or_masked || (a.or_mask_acknowledged && (a.sources != 0U)));
  ok = ok && (!a.pulse || (a.function == AlarmFunction::WindowAlert));
  return Check(ok);
}

constexpr Error ValidateSampling(const Sampling& s) noexcept {
  bool ok = IsEnumerator(s.ready);
  ok = ok && (s.convst_hz >= kConvstMinHz) && (s.convst_hz <= kConvstMaxHz);
  ok = ok && (s.ready_timeout_us >= 1U) && (s.ready_timeout_us <= kReadyTimeoutMaxUs);
  return Check(ok);
}

constexpr Error ValidateSupply(const Supply& s) noexcept {
  const auto in = [](uint32_t v, uint32_t lo, uint32_t hi) {
    return (v == 0U) || ((v >= lo) && (v <= hi));
  };
  return Check(in(s.avdd_5v_mv, 4750U, 5250U) && in(s.avdd_1v8_mv, 1700U, 1900U) &&
               in(s.iovdd_mv, 1700U, 3600U));
}

constexpr Error ValidateExtras(const Config& c) noexcept {
  bool ok = IsEnumerator(c.reference) && IsEnumerator(c.channels.count);
  ok = ok && (c.channels.group < GroupCount(c.channels.count));
  ok = ok && IsValid(c.calibration.mode) && IsEnumerator(c.calibration.trigger);
  // ADC calibration needs the digital filters off (section 7.3.10).
  ok = ok && (!c.calibration.run_at_start || (c.filter.kind == FilterKind::None));
  for (const TestPattern& t : c.test_pattern) {
    ok = ok && IsValid(t.kind);
  }
  for (const uint8_t g : c.window_glitch) {
    ok = ok && (g <= kGlitchMax);
  }
  return Check(ok);
}

/// A window compares signed codes: offset binary would turn the thresholds
/// around (A16).
constexpr bool WindowWithOffsetBinary(const Config& c) noexcept {
  if (c.interface.format != DataFormat::OffsetBinary) {
    return false;
  }
  return std::ranges::any_of(c.inputs, [](const Input& in) { return in.window.enabled; });
}

} // namespace detail

/**
 * @brief Check a configuration against the documented ranges and combinations,
 * with no I/O.
 * @param c The configuration.
 * @return `Ok`; `Unsupported` when the daisy chain is asked for (out of scope
 * in this version); otherwise `InvalidArgument`.
 */
constexpr Error Validate(const Config& c) noexcept {
  if (c.daisy.enabled) {
    return Error::Unsupported;
  }
  const std::array<Error, 8> steps{detail::ValidateInterface(c.interface),
                                   detail::ValidateInputs(c.inputs),
                                   detail::ValidateFilter(c.filter),
                                   detail::ValidateAlarm(c.alarm),
                                   detail::ValidateSampling(c.sampling),
                                   detail::ValidateSupply(c.supply),
                                   detail::ValidateExtras(c),
                                   detail::Check(!detail::WindowWithOffsetBinary(c))};
  for (const Error e : steps) {
    if (e != Error::Ok) {
      return e;
    }
  }
  return Error::Ok;
}

static_assert(Validate(Config{}) == Error::Ok);
static_assert(Validate(ChipResetConfig()) == Error::Ok);

namespace detail {
/**
 * @brief Reached only when a `CheckedConfig` holds an invalid configuration:
 * never defined and not `constexpr`, so the constant evaluation fails and the
 * build names it.
 */
void ConfigIsInvalid() noexcept;
} // namespace detail

/**
 * @brief A configuration proven valid when the program is built. An invalid one
 * does not compile.
 *
 * `constexpr CheckedConfig kBoard{MyConfig()};` runs `Validate()` at build
 * time; `Get()` returns the proven configuration.
 */
class CheckedConfig {
public:
  /** @param config The configuration; not valid per `Validate()` fails the
   * build. */
  consteval explicit CheckedConfig(const Config& config) : config_(config) {
    if (Validate(config) != Error::Ok) {
      detail::ConfigIsInvalid();
    }
  }
  /** @return The configuration. */
  [[nodiscard]] constexpr const Config& Get() const& noexcept {
    return config_;
  }
  /** @brief Not on a temporary: the reference would dangle (KB-CPP20-044). */
  void Get() const&& = delete;

private:
  Config config_;
};

/** @return The major version of the driver, from the generated version header.
 */
constexpr int GetDriverVersionMajor() noexcept {
  return ADS9324_VERSION_MAJOR;
}
/** @return The minor version of the driver. */
constexpr int GetDriverVersionMinor() noexcept {
  return ADS9324_VERSION_MINOR;
}
/** @return The patch version of the driver. */
constexpr int GetDriverVersionPatch() noexcept {
  return ADS9324_VERSION_PATCH;
}
/** @return The version as text, for example "0.1.0". */
constexpr const char* GetDriverVersion() noexcept {
  return ADS9324_VERSION_STRING;
}

/**
 * @brief The averaging factor of a length code: 1, 2, 4, 6, 8, 10, 12, 16, 20,
 * 32, 64, 128.
 * @param length The length.
 * @return Samples averaged; 1 for `None`.
 */
constexpr uint32_t AverageFactor(AverageLength length) noexcept {
  constexpr std::array<uint8_t, 12> kFactor{1, 2, 4, 6, 8, 10, 12, 16, 20, 32, 64, 128};
  const auto code = static_cast<uint32_t>(detail::ToUnderlying(length));
  return (code < kFactor.size()) ? static_cast<uint32_t>(kFactor[code]) : 1U;
}

/**
 * @brief The decimation of a FIR filter (Table 7-9).
 * @param fir The filter.
 * @return 2, 4 or 8.
 */
constexpr uint32_t FirDecimation(FirFilter fir) noexcept {
  switch (fir) {
  case FirFilter::Fir3:
  case FirFilter::Fir5:
    return 4U;
  case FirFilter::Fir7:
    return 8U;
  case FirFilter::Fir1:
  case FirFilter::Fir2:
  case FirFilter::Fir4:
  default:
    break;
  }
  return 2U;
}

/**
 * @brief Samples that make one result of the digital filter in the data path.
 * @param filter The filter.
 * @return The block average length, the FIR decimation, or 1.
 */
constexpr uint32_t ResultsPerOutput(const Filter& filter) noexcept {
  switch (filter.kind) {
  case FilterKind::BlockAverage:
    return AverageFactor(filter.length);
  case FilterKind::Fir:
    return FirDecimation(filter.fir);
  case FilterKind::None:
  case FilterKind::MovingAverage:
  default:
    break;
  }
  return 1U;
}

/**
 * @brief CONVST falling edges the chip needs for one result.
 * @param filter The digital filter.
 * @return The FIR decimation, the block average length with the external
 * clock, and 1 otherwise (including the internal oversampling clock, where
 * one falling edge starts the whole block).
 */
constexpr uint32_t PulsesPerResult(const Filter& filter) noexcept {
  if (filter.kind == FilterKind::Fir) {
    return FirDecimation(filter.fir);
  }
  if ((filter.kind == FilterKind::BlockAverage) && !filter.internal_clock) {
    return AverageFactor(filter.length);
  }
  return 1U;
}

/**
 * @brief Result rate of the data interface in hertz.
 * @param config The configuration.
 * @return `convst_hz` divided by the block average length or the FIR
 * decimation.
 */
constexpr uint32_t OutputRateHz(const Config& config) noexcept {
  return config.sampling.convst_hz / ResultsPerOutput(config.filter);
}

/**
 * @brief Time from the start of a conversion to a ready result, rounded up to
 * microseconds, at least 2.
 *
 * Section 5.7: tCONV = (N - 1) x tCYC + 1.25 us, with N the block average
 * length or the FIR decimation and tCYC = 1 / fCONVST; 1.25 us without a
 * digital filter that combines results.
 *
 * @param config The configuration.
 * @return The time in microseconds.
 */
constexpr uint32_t ConversionTimeUs(const Config& config) noexcept {
  const uint32_t n = ResultsPerOutput(config.filter);
  uint64_t ns = kConversionNs;
  if (n > 1U) {
    // Section 5.7: tCONV = (N - 1) x tCYC + 1.25 us, tCYC = 1 / fCONVST.
    ns += static_cast<uint64_t>(n - 1U) * (1'000'000'000ULL / config.sampling.convst_hz);
  }
  const uint64_t us = (ns + 999ULL) / 1000ULL;
  return static_cast<uint32_t>((us < 2ULL) ? 2ULL : us);
}

/**
 * @brief The OFS value that removes a measured offset (equation 1,
 * section 7.3.9).
 * @param expected_code The code the input should give.
 * @param measured_code The code measured.
 * @return Quarter LSBs, `4 * (expected - measured)`, clamped to the register
 * range.
 */
constexpr int16_t OffsetCorrectionFor(int32_t expected_code, int32_t measured_code) noexcept {
  const int32_t v = 4 * (expected_code - measured_code);
  const int32_t clamped = std::clamp(v, kOffsetMin, kOffsetMax);
  return static_cast<int16_t>(clamped);
}

/**
 * @brief The GAN value that removes a measured gain error (equations 2 to 4,
 * section 7.3.9).
 * @param expected_code The code a full-scale input should give.
 * @param measured_code The code measured; 0 gives 0.
 * @return `(expected - measured) * 65536 / measured`, rounded half away from
 * zero, clamped.
 */
constexpr int16_t GainCorrectionFor(int32_t expected_code, int32_t measured_code) noexcept {
  if (measured_code == 0) {
    return 0;
  }
  const int64_t num = static_cast<int64_t>(expected_code - measured_code) * 65536;
  const int64_t den = measured_code;
  const int64_t half = ((den < 0) ? -den : den) / 2;
  const bool same_sign = (num >= 0) == (den > 0);
  const int64_t v = same_sign ? ((((num < 0) ? -num : num) + half) / ((den < 0) ? -den : den))
                              : -((((num < 0) ? -num : num) + half) / ((den < 0) ? -den : den));
  const int64_t clamped = std::clamp<int64_t>(v, kGainMin, kGainMax);
  return static_cast<int16_t>(clamped);
}

static_assert((AverageFactor(AverageLength::X128) == 128U) &&
              (AverageFactor(AverageLength::None) == 1U));
static_assert(ConversionTimeUs(Config{}) == 2U);
static_assert((OffsetCorrectionFor(0, 3) == -12) && (OffsetCorrectionFor(0, 1000) == kOffsetMin));
static_assert((GainCorrectionFor(32000, 32000) == 0) && (GainCorrectionFor(32768, 32000) == 1573));

/**
 * @brief Full-scale voltage of a range in millivolts.
 * @param range The range.
 * @return 5000, 2500, 6250, 10000 or 12500.
 */
constexpr int32_t RangeMillivolts(InputRange range) noexcept {
  switch (range) {
  case InputRange::V5:
    return 5000;
  case InputRange::V2_5:
    return 2500;
  case InputRange::V6_25:
    return 6250;
  case InputRange::V10:
    return 10000;
  case InputRange::V12_5:
    return 12500;
  default:
    break;
  }
  return 5000;
}

/**
 * @brief Microvolts of a signed code: `code x full scale / 32768`, rounded half
 * away from zero.
 * @param code The code, -32768 to 32767, in two's complement terms.
 * @param range The channel range.
 * @return The input voltage in microvolts.
 */
constexpr int32_t CodeToMicrovolts(int32_t code, InputRange range) noexcept {
  const int64_t full_scale_uv = static_cast<int64_t>(RangeMillivolts(range)) * 1000;
  const int64_t scaled = static_cast<int64_t>(code) * full_scale_uv;
  const int64_t half = 16384;
  const int64_t rounded = (scaled >= 0) ? ((scaled + half) / 32768) : ((scaled - half) / 32768);
  return static_cast<int32_t>(rounded);
}

/**
 * @brief Reinterpret a 16-bit wire value as a signed code (two's complement).
 * @param raw The wire value.
 * @return The signed code.
 */
constexpr int16_t ToSigned16(uint16_t raw) noexcept {
  return static_cast<int16_t>(raw);
}

/**
 * @brief The signed code of a wire value.
 * @param raw The 16 bits as received, after derandomizing.
 * @param format The coding in use.
 * @return -32768 to 32767.
 */
constexpr int16_t WireToCode(uint16_t raw, DataFormat format) noexcept {
  if (format == DataFormat::OffsetBinary) {
    return ToSigned16(static_cast<uint16_t>(static_cast<uint32_t>(raw) ^ 0x8000U));
  }
  return ToSigned16(raw);
}

/**
 * @brief The XOR mask of the randomizer (equations 7 and 8, section 7.3.14.4).
 * @param prbs The PRBS byte appended to the frame.
 * @return `prbs x 256 + (prbs bit 0) x 255`.
 */
constexpr uint16_t DerandomizeMask(uint8_t prbs) noexcept {
  const auto value = static_cast<uint32_t>(prbs);
  const uint32_t low = ((value & 1U) != 0U) ? 255U : 0U;
  return static_cast<uint16_t>((value << 8U) + low);
}

/**
 * @brief Number of channels in a frame.
 * @param select The channel selection.
 * @return 16, 8, 4 or 2.
 */
constexpr uint32_t FrameChannels(const ChannelSelect& select) noexcept {
  switch (select.count) {
  case ChannelCount::All16:
    return 16U;
  case ChannelCount::Eight:
    return 8U;
  case ChannelCount::Four:
    return 4U;
  case ChannelCount::Two:
    return 2U;
  default:
    break;
  }
  return 16U;
}

/**
 * @brief Which channel a slot of the frame carries (Table 7-12).
 * @param select The channel selection; the group must be valid.
 * @param slot The slot, counted from 0, below `FrameChannels()`.
 * @return The channel counted from 0 (AIN1 is 0).
 */
constexpr uint8_t FrameSlotChannel(const ChannelSelect& select, uint32_t slot) noexcept {
  const auto group = static_cast<uint32_t>(select.group);
  uint32_t channel = slot;
  switch (select.count) {
  case ChannelCount::Eight:
    if (group != 0U) {
      channel = slot + 4U;
    } else if (slot >= 4U) {
      channel = slot + 8U;
    } else {
      channel = slot; // group 0, slots 0 to 3: AIN1 to AIN4
    }
    break;
  case ChannelCount::Four:
    channel = (slot < 2U) ? (slot + (2U * group)) : (slot + 12U - (2U * group));
    break;
  case ChannelCount::Two:
    channel = (slot == 0U) ? group : (15U - group);
    break;
  case ChannelCount::All16:
  default:
    break;
  }
  return static_cast<uint8_t>(channel);
}

/**
 * @brief Whether the frame ends with a DIAG byte: asked for, or implied by the
 * randomizer (the datasheet shows the PRBS byte only together with the DIAG
 * byte, assumption A14).
 * @param interface The data interface.
 * @return true when the DIAG byte is appended.
 */
constexpr bool HasDiagByte(const Interface& interface) noexcept {
  return interface.append_diag || interface.randomize;
}

/**
 * @brief Bytes of one data frame (section 7.3.14).
 * @param interface The data interface.
 * @param select The channel selection.
 * @return Channels x 2 or 3 bytes, plus the DIAG byte and the PRBS byte when
 * enabled.
 */
constexpr std::size_t FrameBytes(const Interface& interface, const ChannelSelect& select) noexcept {
  const uint32_t per_channel = (interface.length == WordLength::Bits24) ? 3U : 2U;
  uint32_t bytes = FrameChannels(select) * per_channel;
  bytes += HasDiagByte(interface) ? 1U : 0U;
  bytes += interface.randomize ? 1U : 0U;
  return static_cast<std::size_t>(bytes);
}

/** @brief One decoded data frame. */
struct Frame {
  std::array<uint16_t, 16> raw{};       ///< Wire values after derandomizing; 0 for absent channels.
  std::array<int16_t, 16> codes{};      ///< Signed codes.
  std::array<uint8_t, 16> low_byte{};   ///< The extra byte of a 24-bit word, unprocessed.
  std::array<int32_t, 16> microvolts{}; ///< Input voltage per channel.
  uint16_t present_mask{0};             ///< Bit n: channel n is in this frame.
  uint16_t clipped_mask{0};             ///< Bit n: channel n is at a code limit.
  uint8_t diag{0};                      ///< The DIAG byte, 0 when not appended.
  uint8_t prbs{0};                      ///< The PRBS byte, 0 when not appended.
  bool window_alert{false};             ///< DIAG bit 6: some window comparator alerts.
  bool reset_detected{false};           ///< DIAG bit 0: the chip reset.
};

/** @brief DIAG bit 6, DWC_ALERT_FLAG (Table 7-13). */
inline constexpr uint8_t kDiagWindowAlert = 0x40;
/** @brief DIAG bit 0, RESET_DETECT_FLAG. */
inline constexpr uint8_t kDiagResetDetect = 0x01;

/**
 * @brief Whether a data frame could come from a stuck or floating line: every
 * byte 0x00 or every byte 0xFF (hardware lessons, KB-CPP-116).
 * @param bytes The frame.
 * @return true for four or more identical 0x00 or 0xFF bytes.
 */
constexpr bool IsStuckFrame(std::span<const uint8_t> bytes) noexcept {
  if (bytes.size() < 4U) {
    return false;
  }
  const uint8_t first = bytes[0];
  if ((first != 0x00U) && (first != 0xFFU)) {
    return false;
  }
  return std::ranges::all_of(bytes, [first](uint8_t b) { return b == first; });
}

} // namespace ads9324

#endif // ADS9324_TYPES_HPP
