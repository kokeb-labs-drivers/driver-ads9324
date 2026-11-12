/**
 * @file ads9324.ipp
 * @brief ADS9324 driver implementation, included by `ads9324.hpp`.
 * @copyright Copyright (c) 2026 The driver-ads9324 authors.
 * SPDX-License-Identifier: MIT
 *
 * Section numbers refer to the ADS9324 datasheet (SBASB22, December 2025).
 */
#ifndef ADS9324_IPP
#define ADS9324_IPP

namespace ads9324 {

namespace detail {
/// Poll step of the status waits, in microseconds.
inline constexpr uint32_t kPollUs = 1000;
/// Settle time after a reference change and after leaving standby, in
/// microseconds (section 5.5).
inline constexpr uint32_t kSettleUs = 30'000;
/// One DRDY poll per microsecond.
inline constexpr uint32_t kDrdyPollUs = 1;
/// Wait between ADC_CAL_TRIG 0 to 1 and 1 to 0, in microseconds
/// (section 7.3.10, step 4).
inline constexpr uint32_t kCalTrigUs = 10;
/// Pin hold time of CONVST pulses in microseconds (at least 50 ns,
/// section 5.6).
inline constexpr uint32_t kConvstHoldUs = 1;
/// Time a CONVST pulse takes, in microseconds: two holds.
inline constexpr uint32_t kPulseSpanUs = 2U * kConvstHoldUs;
/// First constant test pattern value of the self test.
inline constexpr uint16_t kSelfTestBase = 0x0300;
/// Step between the self test pattern values of two channels.
inline constexpr uint16_t kSelfTestStep = 0x0011;
/// A zero register value.
inline constexpr uint16_t kZero16 = 0;

/// ADC_CAL_MODE bits of a calibration mode.
constexpr uint16_t Code16(CalibrationMode mode) noexcept {
  return static_cast<uint16_t>(Code(mode));
}
} // namespace detail

template <SpiPort Port>
ADS9324<Port>::ADS9324(Port& port, const Config& config) noexcept : bus_(port), config_(config) {
  resetShadow();
}

// -------------------------------------------------------------------------------------------------
// Lifecycle
// -------------------------------------------------------------------------------------------------

template <SpiPort Port>
Result<void> ADS9324<Port>::EnsureInitialized() noexcept {
  using detail::operator""_u32;
  if (initialized_) {
    return Result<void>::Success();
  }
  const Config config = config_;
  if (const Error e = Validate(config); e != Error::Ok) {
    return fail(e, "ads9324: invalid Config");
  }
  if (!bus_.GetPort().EnsureInitialized()) {
    return fail(Error::Bus, "ads9324: the port could not bring the bus up");
  }
  // tPU: supplies and reference settle before the first access (section 5.7,
  // assumption A3).
  detail::DelayUs(bus_.GetPort(), kPowerUpMs * 1000_u32);
  if (auto r = startChip(config, config.hardware_reset); !r) {
    return r;
  }
  initialized_ = true;
  powered_down_ = false;
  detail::Log(bus_.GetPort(), LogLevel::Info, "ads9324: initialised");
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::startChip(const Config& config, bool hardware) noexcept {
  chip_live_ = true;
  if (auto r = resetChip(hardware); !r) {
    return r;
  }
  if (auto r = checkIdentity(); !r) {
    return r;
  }
  if (auto r = waitCalibrationIdle(kSingleShotCalMs + kCalMarginMs, false); !r) {
    return r;
  }
  // RESET_DETECT_FLAG reads 1 after a reset; the user writes it to 0
  // (section 8.1.3).
  if (auto r = writeRegister(reg::Bank::Common, reg::kDiagCtrl, detail::kZero16); !r) {
    return fail(r.error, "ads9324: DIAG_CTRL write failed");
  }
  if (auto r = applyImage(config, Verify::None); !r) {
    return fail(r.error, "ads9324: configuration write failed");
  }
  config_ = config;
  if (config.calibration.run_at_start) {
    return calibrate(detail::Code16(config.calibration.mode),
                     config.calibration.trigger == CalibrationTrigger::SingleShot);
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::resetChip(bool hardware) noexcept {
  using detail::operator""_u32;
  Port& port = bus_.GetPort();
  if (hardware) {
    if constexpr (detail::HasGpioSet<Port>) {
      // RESET is asserted for at least the pulse the driver chooses (assumption
      // A4).
      if (!port.GpioSet(CtrlPin::Reset, GpioSignal::ACTIVE)) {
        return fail(Error::Bus, "ads9324: RESET could not be asserted");
      }
      detail::DelayUs(port, kResetPulseUs);
      if (!port.GpioSet(CtrlPin::Reset, GpioSignal::INACTIVE)) {
        return fail(Error::Bus, "ads9324: RESET could not be released");
      }
    } else {
      return fail(Error::Unsupported, "ads9324: hardware reset needs GpioSet()");
    }
  } else {
    if (auto r = selectBank(reg::Bank::Common); !r) {
      return fail(r.error, "ads9324: bank select failed");
    }
    if (const Error e = bus_.Write(reg::kGenCfg1, reg::kSwRst); e != Error::Ok) {
      bank_valid_ = false;
      return fail(e, "ads9324: software reset failed");
    }
  }
  detail::DelayUs(port, kResetMs * 1000_u32);
  resetShadow();
  cal_mode_ = 0U;
  // BANK_SEL resets to 0x0001 (section 8.1.2): the common bank is selected without a write, so the
  // start-up reads really check the reset state instead of overwriting it first.
  bank_ = reg::Bank::Common;
  bank_valid_ = true;
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::checkIdentity() noexcept {
  const auto id = readRegister(reg::Bank::Common, reg::kDeviceId);
  if (!id) {
    return fail(id.error, "ads9324: DEVICE_ID read failed");
  }
  if (id.value != reg::kDeviceIdValue) {
    return fail(Error::DeviceIdMismatch, "ads9324: DEVICE_ID is not 0x0004");
  }
  const auto bank = readRegister(reg::Bank::Common, reg::kBankSel);
  if (!bank) {
    return fail(bank.error, "ads9324: BANK_SEL read failed");
  }
  if (bank.value != reg::BankSelCode(reg::Bank::Common)) {
    return fail(Error::DeviceIdMismatch, "ads9324: BANK_SEL is not 1 after reset");
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::waitCalibrationIdle(uint32_t timeout_ms, bool confirm) noexcept {
  // A calibration may be running after power-up (assumption A38); the bound is
  // the longest one.
  for (uint32_t i = 0U; i <= timeout_ms; ++i) {
    const auto status = readRegister(reg::Bank::Common, reg::kDeviceStatus);
    if (!status) {
      return fail(status.error, "ads9324: DEVICE_STATUS read failed");
    }
    if ((static_cast<uint32_t>(status.value) & reg::kCalibBusyFlag) == 0U) {
      // A stuck-low SDOUT also reads "not busy": confirm the line with DEVICE_ID.
      return confirm ? confirmIdentity() : Result<void>::Success();
    }
    detail::DelayUs(bus_.GetPort(), detail::kPollUs);
  }
  return fail(Error::Timeout, "ads9324: CALIB_BUSY did not clear");
}

template <SpiPort Port>
Result<void> ADS9324<Port>::Deinitialize() noexcept {
  const bool live = chip_live_;
  initialized_ = false;
  powered_down_ = false;
  chip_live_ = false;
  layout_ok_ = false;
  if (!live) {
    return Result<void>::Success(); // no frame was ever sent: nothing to put to sleep
  }
  bank_valid_ = false; // the chip may have reset: do not trust the cache
  return writeRegister(reg::Bank::Common, reg::kPdnCtl, reg::kDevicePdn);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::Reset(ResetKind kind) noexcept {
  if (!initialized_) {
    return Result<void>::Failure(Error::NotInitialized);
  }
  if (!detail::IsEnumerator(kind)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if ((kind == ResetKind::Hardware) && !detail::HasGpioSet<Port>) {
    return Result<void>::Failure(Error::Unsupported);
  }
  initialized_ = false;
  powered_down_ = false;
  if (auto r = startChip(config_, kind == ResetKind::Hardware); !r) {
    return r;
  }
  initialized_ = true;
  return Result<void>::Success();
}

template <SpiPort Port>
Result<uint16_t> ADS9324<Port>::Identify() noexcept {
  if (!initialized_) {
    return Result<uint16_t>::Failure(Error::NotInitialized);
  }
  return readRegister(reg::Bank::Common, reg::kDeviceId);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::checkReady() const noexcept {
  if (!initialized_) {
    return Result<void>::Failure(Error::NotInitialized);
  }
  return powered_down_ ? Result<void>::Failure(Error::Refused) : Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::confirmIdentity() noexcept {
  const auto id = readRegister(reg::Bank::Common, reg::kDeviceId);
  if (!id) {
    return Result<void>::Failure(id.error);
  }
  if (id.value != reg::kDeviceIdValue) {
    // A stuck or floating SDOUT line: no status read can be trusted.
    return fail(Error::Bus, "ads9324: DEVICE_ID reads back wrong, SDOUT is not trustworthy");
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::probeReset() noexcept {
  // A reset between two calls puts the chip back in the common bank with reset registers; the bank
  // cache and the shadow are then wrong. Re-send BANK_SEL (always accessible) and look at
  // RESET_DETECT_FLAG before any configuration byte goes out (KB-HW-012).
  bank_valid_ = false;
  if (auto r = confirmIdentity(); !r) {
    return r;
  }
  const auto diag = readRegister(reg::Bank::Common, reg::kDiagCtrl);
  if (!diag) {
    return Result<void>::Failure(diag.error);
  }
  if ((static_cast<uint32_t>(diag.value) & reg::kResetDetectFlag) != 0U) {
    initialized_ = false;
    layout_ok_ = false;
    return fail(Error::UnexpectedReset, "ads9324: the chip reset between calls");
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::checkAlive() noexcept {
  if (auto r = checkReady(); !r) {
    return r;
  }
  return probeReset();
}

// -------------------------------------------------------------------------------------------------
// Configuration
// -------------------------------------------------------------------------------------------------

template <SpiPort Port>
Result<void> ADS9324<Port>::ApplyConfig(const Config& config, Verify verify) noexcept {
  if (const Error e = Validate(config); e != Error::Ok) {
    return Result<void>::Failure(e);
  }
  if (!detail::IsEnumerator(verify)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (!initialized_ && !powered_down_) {
    if (chip_live_) {
      // A reset was reported: Ok would read as "applied". EnsureInitialized() starts the chip
      // again.
      return Result<void>::Failure(Error::NotInitialized);
    }
    config_ = config; // applied by the first EnsureInitialized()
    return Result<void>::Success();
  }
  if (auto r = checkAlive(); !r) {
    return r;
  }
  if (auto r = applyImage(config, verify); !r) {
    return r;
  }
  config_ = config;
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::SetInput(uint8_t channel, const Input& input, Verify verify) noexcept {
  if (channel >= reg::kChannelCount) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  Config next = config_;
  next.inputs[channel] = input;
  return ApplyConfig(next, verify);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::SetInterface(const Interface& interface, Verify verify) noexcept {
  Config next = config_;
  next.interface = interface;
  return ApplyConfig(next, verify);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::SetFilter(const Filter& filter, Verify verify) noexcept {
  Config next = config_;
  next.filter = filter;
  return ApplyConfig(next, verify);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::SetChannels(const ChannelSelect& select, Verify verify) noexcept {
  Config next = config_;
  next.channels = select;
  return ApplyConfig(next, verify);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::SetAlarm(const Alarm& alarm, Verify verify) noexcept {
  Config next = config_;
  next.alarm = alarm;
  return ApplyConfig(next, verify);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::SetReference(ReferenceSource source, Verify verify) noexcept {
  Config next = config_;
  next.reference = source;
  return ApplyConfig(next, verify);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::SetTestPattern(uint8_t group, const TestPattern& pattern,
                                           Verify verify) noexcept {
  if (group >= 2U) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  Config next = config_;
  next.test_pattern[group] = pattern;
  return ApplyConfig(next, verify);
}

// -------------------------------------------------------------------------------------------------
// Conversion and readout
// -------------------------------------------------------------------------------------------------

template <SpiPort Port>
Result<void> ADS9324<Port>::setConvst(GpioSignal level) noexcept {
  if constexpr (detail::HasGpioSet<Port>) {
    if (!bus_.GetPort().GpioSet(CtrlPin::Convst, level)) {
      return fail(Error::Bus, "ads9324: CONVST write failed");
    }
    return Result<void>::Success();
  } else {
    (void)level;
    return Result<void>::Failure(Error::Unsupported);
  }
}

template <SpiPort Port>
Result<void> ADS9324<Port>::pulseConvst() noexcept {
  // The falling edge converts (section 7.4.2). CONVST ends high: with the internal oversampling
  // clock it must be high before DRDY rises (Figure 5-7, note 1; tsu_CV_hi_DRDY_hi).
  if (auto r = setConvst(GpioSignal::ACTIVE); !r) {
    return r;
  }
  detail::DelayUs(bus_.GetPort(), detail::kConvstHoldUs);
  if (auto r = setConvst(GpioSignal::INACTIVE); !r) {
    return r;
  }
  // The DRDY of the previous result falls 350 ns after the falling edge: wait
  // it out (A45).
  detail::DelayUs(bus_.GetPort(), detail::kConvstHoldUs);
  return setConvst(GpioSignal::ACTIVE);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::Convert() noexcept {
  if (auto r = checkReady(); !r) {
    return r;
  }
  return pulseConvst();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::WaitDataReady() noexcept {
  if (auto r = checkReady(); !r) {
    return r;
  }
  if (config_.sampling.ready == ReadySource::Time) {
    detail::DelayUs(bus_.GetPort(), ConversionTimeUs(config_));
    return Result<void>::Success();
  }
  if constexpr (detail::HasGpioGet<Port>) {
    if (config_.alarm.function != AlarmFunction::Drdy) {
      return Result<void>::Failure(Error::Unsupported);
    }
    // One poll per microsecond, bounded by the validated timeout
    // (KB-CPP20-058).
    const uint32_t limit = (config_.sampling.ready_timeout_us < kReadyTimeoutMaxUs)
                               ? config_.sampling.ready_timeout_us
                               : kReadyTimeoutMaxUs;
    for (uint32_t i = 0U; i < limit; ++i) {
      bool high = false;
      if (!bus_.GetPort().GpioGet(CtrlPin::DrdyAlarm, high)) {
        return fail(Error::Bus, "ads9324: DRDY read failed");
      }
      if (high != config_.alarm.active_low) {
        return Result<void>::Success();
      }
      detail::DelayUs(bus_.GetPort(), detail::kDrdyPollUs);
    }
    return fail(Error::Timeout, "ads9324: no DRDY before the timeout");
  } else {
    return Result<void>::Failure(Error::Unsupported);
  }
}

template <SpiPort Port>
Result<void> ADS9324<Port>::readFrameBytes(std::span<uint8_t> bytes) noexcept {
  const Interface& ifc = config_.interface;
  if (ifc.data_on_sdout) {
    const Error e = bus_.ReadStream(bytes);
    return (e == Error::Ok) ? Result<void>::Success() : fail(e, "ads9324: frame read failed");
  }
  if constexpr (detail::HasCaptureLanes<Port>) {
    if (!bus_.GetPort().CaptureLanes(detail::LaneCount(ifc.lanes), bytes.data(), bytes.size())) {
      return fail(Error::Bus, "ads9324: lane capture failed");
    }
    return Result<void>::Success();
  } else {
    return Result<void>::Failure(Error::Unsupported);
  }
}

template <SpiPort Port>
Result<Frame> ADS9324<Port>::ReadFrame() noexcept {
  if (auto r = checkReady(); !r) {
    return Result<Frame>::Failure(r.error);
  }
  if (!layout_ok_) {
    // A failed configuration call left the layout registers half written: the frame would be
    // decoded with the wrong lane count or word length.
    return Result<Frame>::Failure(Error::Refused);
  }
  const std::size_t count = FrameBytes(config_.interface, config_.channels);
  const std::span<uint8_t> bytes{frame_buffer_.data(), count};
  if (auto r = readFrameBytes(bytes); !r) {
    return Result<Frame>::Failure(r.error);
  }
  const bool pattern = config_.test_pattern[0].enabled || config_.test_pattern[1].enabled;
  if (config_.sampling.reject_stuck_frames && !pattern && IsStuckFrame(bytes)) {
    (void)fail(Error::DataMismatch, "ads9324: frame is stuck at 0x00 or 0xFF");
    return Result<Frame>::Failure(Error::DataMismatch);
  }
  const auto frame = detail::DecodeFrame(config_, bytes);
  if (frame && frame.value.reset_detected) {
    initialized_ = false;
    (void)fail(Error::UnexpectedReset, "ads9324: the chip reset during operation");
    return Result<Frame>::Failure(Error::UnexpectedReset);
  }
  return frame;
}

template <SpiPort Port>
Result<Frame> ADS9324<Port>::MeasureOnce() noexcept {
  // A FIR filter or an external-clock block average needs several falling edges for one result;
  // reading after one pulse would clock out stale data (KB-HW-014).
  if (auto r = checkReady(); !r) { // before any use of the configuration: it is not validated yet
    return Result<Frame>::Failure(r.error);
  }
  const uint32_t pulses = PulsesPerResult(config_.filter);
  const uint32_t period_us = 1'000'000U / config_.sampling.convst_hz;
  for (uint32_t i = 0U; i < pulses; ++i) {
    if (auto r = Convert(); !r) {
      return Result<Frame>::Failure(r.error);
    }
    if (((i + 1U) < pulses) && (period_us > detail::kPulseSpanUs)) {
      detail::DelayUs(bus_.GetPort(), period_us - detail::kPulseSpanUs);
    }
  }
  if (auto r = WaitDataReady(); !r) {
    return Result<Frame>::Failure(r.error);
  }
  return ReadFrame();
}

template <SpiPort Port>
Result<bool> ADS9324<Port>::ReadAlarmPin() noexcept {
  if (!initialized_) {
    return Result<bool>::Failure(Error::NotInitialized);
  }
  if constexpr (detail::HasGpioGet<Port>) {
    bool high = false;
    if (!bus_.GetPort().GpioGet(CtrlPin::DrdyAlarm, high)) {
      return Result<bool>::Failure(Error::Bus);
    }
    return Result<bool>::Success(high != config_.alarm.active_low);
  } else {
    return Result<bool>::Failure(Error::Unsupported);
  }
}

// -------------------------------------------------------------------------------------------------
// Window comparator and calibration
// -------------------------------------------------------------------------------------------------

template <SpiPort Port>
Result<WindowFlags> ADS9324<Port>::ReadWindowFlags() noexcept {
  if (auto r = checkAlive(); !r) {
    return Result<WindowFlags>::Failure(r.error);
  }
  WindowFlags flags{};
  for (uint32_t b = 0U; b < 2U; ++b) {
    const reg::Bank bank = (b == 0U) ? reg::Bank::Ain1To8 : reg::Bank::Ain9To16;
    const auto v = readRegister(bank, reg::kDwcFlag);
    if (!v) {
      return Result<WindowFlags>::Failure(v.error);
    }
    for (uint8_t local = 0U; local < reg::kChannelsPerBank; ++local) {
      const uint32_t ch = reg::ChannelAt(bank, local);
      const uint32_t high = (static_cast<uint32_t>(v.value) >> (8U + local)) & 1U;
      const uint32_t low = (static_cast<uint32_t>(v.value) >> local) & 1U;
      flags.high = static_cast<uint16_t>(static_cast<uint32_t>(flags.high) | (high << ch));
      flags.low = static_cast<uint16_t>(static_cast<uint32_t>(flags.low) | (low << ch));
    }
  }
  return Result<WindowFlags>::Success(flags);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::ClearWindowFlags(uint8_t group) noexcept {
  if (group >= 2U) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (auto r = checkAlive(); !r) {
    return r;
  }
  const reg::Bank bank = (group == 0U) ? reg::Bank::Ain1To8 : reg::Bank::Ain9To16;
  const uint16_t base = shadow_[detail::BankIndex(bank)][reg::kDwcCfg];
  // A rising edge on DWC_STAT_RST clears the flags: write 1, then 0 (assumption
  // A18).
  if (auto r = writeRegister(bank, reg::kDwcCfg, detail::Or(base, reg::kDwcStatRst)); !r) {
    return r;
  }
  return writeRegister(bank, reg::kDwcCfg, base);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::beginCalibration(uint16_t base, uint16_t mode) noexcept {
  using reg::Bank;
  const uint16_t armed = detail::Or(base, mode);
  if (auto r = writeRegister(Bank::Common, reg::kAdcCal, armed); !r) {
    return r;
  }
  if (auto r = writeRegister(Bank::Common, reg::kAdcCal, detail::Or(armed, reg::kAdcCalTrig)); !r) {
    return r;
  }
  detail::DelayUs(bus_.GetPort(), detail::kCalTrigUs);
  return writeRegister(Bank::Common, reg::kAdcCal, armed);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::finishCalibration(bool single_shot) noexcept {
  using detail::operator""_u32;
  Port& port = bus_.GetPort();
  if (single_shot) {
    // At least one falling edge, ending high (section 7.3.10, step 6,
    // assumption A39).
    for (const GpioSignal level : {GpioSignal::ACTIVE, GpioSignal::INACTIVE, GpioSignal::ACTIVE}) {
      if (auto r = setConvst(level); !r) {
        return r;
      }
      detail::DelayUs(port, detail::kConvstHoldUs);
    }
    detail::DelayUs(port, kSingleShotCalMs * 1000_u32);
  } else {
    const uint32_t period_us = 1'000'000_u32 / config_.sampling.convst_hz;
    detail::DelayUs(port, kContinuousCalConvst * period_us);
  }
  return waitCalibrationIdle(kCalMarginMs, true);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::calibrate(uint16_t mode, bool single_shot) noexcept {
  using reg::Bank;
  if (single_shot && !detail::HasGpioSet<Port>) {
    return Result<void>::Failure(Error::Unsupported);
  }
  const uint16_t image = detail::Get(detail::Encode(config_, 0U), Bank::Common, reg::kAdcCal);
  const auto base = static_cast<uint16_t>(static_cast<uint32_t>(image) &
                                          ~static_cast<uint32_t>(reg::AdcCalMode::kMask));
  const uint16_t filter = shadow_[detail::BankIndex(Bank::Common)][reg::kDigFilter];
  // INT_TRIG_MODE belongs to the single-shot flow and is restored afterwards
  // (assumption A35).
  if (single_shot) {
    if (auto r =
            writeRegister(Bank::Common, reg::kDigFilter, detail::Or(filter, reg::kIntTrigMode));
        !r) {
      return r;
    }
  }
  Result<void> outcome = beginCalibration(base, mode);
  if (outcome) {
    outcome = finishCalibration(single_shot);
  }
  if (single_shot) {
    (void)setConvst(GpioSignal::INACTIVE);
    const Result<void> restored = writeRegister(Bank::Common, reg::kDigFilter, filter);
    outcome = outcome ? restored : outcome;
  }
  if (outcome) {
    cal_mode_ = mode;
  }
  return outcome;
}

template <SpiPort Port>
Result<void> ADS9324<Port>::RunAdcCalibration() noexcept {
  if (auto r = checkAlive(); !r) {
    return r;
  }
  if (config_.filter.kind != FilterKind::None) {
    return Result<void>::Failure(Error::Refused);
  }
  return calibrate(detail::Code16(config_.calibration.mode),
                   config_.calibration.trigger == CalibrationTrigger::SingleShot);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::DisableAdcCalibration() noexcept {
  if (auto r = checkAlive(); !r) {
    return r;
  }
  const uint16_t image = detail::Get(detail::Encode(config_, 0U), reg::Bank::Common, reg::kAdcCal);
  if (auto r = writeRegister(reg::Bank::Common, reg::kAdcCal, image); !r) {
    return r;
  }
  cal_mode_ = 0U;
  return Result<void>::Success();
}

template <SpiPort Port>
Result<bool> ADS9324<Port>::IsCalibrationBusy() noexcept {
  if (!initialized_) {
    return Result<bool>::Failure(Error::NotInitialized);
  }
  const auto status = readRegister(reg::Bank::Common, reg::kDeviceStatus);
  if (!status) {
    return Result<bool>::Failure(status.error);
  }
  const bool busy = (static_cast<uint32_t>(status.value) & reg::kCalibBusyFlag) != 0U;
  if (!busy) {
    if (auto r = confirmIdentity(); !r) { // a stuck-low line also reads "idle"
      return Result<bool>::Failure(r.error);
    }
  }
  return Result<bool>::Success(busy);
}

// -------------------------------------------------------------------------------------------------
// Power and diagnostics
// -------------------------------------------------------------------------------------------------

template <SpiPort Port>
Result<void> ADS9324<Port>::PowerDown() noexcept {
  if (!initialized_ && !chip_live_) {
    return Result<void>::Failure(
        Error::NotInitialized); // nothing was ever sent: nothing to switch off
  }
  // No doubt about the chip refuses the switch-off: look for a reset or a line fault, then write
  // the standby bit whatever the look found, and report what it found (hardware lessons 12c).
  Result<void> probe = Result<void>::Success();
  if (initialized_) {
    probe = probeReset();
  }
  bank_valid_ = false; // BANK_SEL is sent again with the write
  if (auto r = writeRegister(reg::Bank::Common, reg::kPdnCtl, reg::kDevicePdn); !r) {
    return r;
  }
  powered_down_ = initialized_;
  return probe;
}

template <SpiPort Port>
Result<void> ADS9324<Port>::Wake() noexcept {
  if (!initialized_) {
    return Result<void>::Failure(Error::NotInitialized);
  }
  if (!powered_down_) {
    return Result<void>::Failure(Error::Refused);
  }
  if (auto r = writeRegister(reg::Bank::Common, reg::kPdnCtl, detail::kZero16); !r) {
    return r;
  }
  powered_down_ = false;
  // Retention in standby is not stated (assumption A40): look for a reset, then
  // for lost settings.
  detail::DelayUs(bus_.GetPort(), detail::kSettleUs);
  if (auto r = confirmIdentity(); !r) {
    return r;
  }
  const auto diag = readRegister(reg::Bank::Common, reg::kDiagCtrl);
  if (!diag) {
    return Result<void>::Failure(diag.error);
  }
  if ((static_cast<uint32_t>(diag.value) & reg::kResetDetectFlag) != 0U) {
    initialized_ = false;
    layout_ok_ = false;
    return fail(Error::UnexpectedReset, "ads9324: the chip reset in standby");
  }
  const auto gc3 = readRegister(reg::Bank::Common, reg::kGenCfg3);
  if (!gc3) {
    return Result<void>::Failure(gc3.error);
  }
  if (gc3.value != shadow_[detail::BankIndex(reg::Bank::Common)][reg::kGenCfg3]) {
    return fail(Error::VerifyFailed, "ads9324: GEN_CFG3 changed in standby");
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<uint16_t> ADS9324<Port>::ReadRegister(reg::Bank bank, uint8_t address) noexcept {
  if (!initialized_) {
    return Result<uint16_t>::Failure(Error::NotInitialized);
  }
  if (!detail::IsEnumerator(bank) || !reg::IsDocumented(bank, address)) {
    return Result<uint16_t>::Failure(Error::InvalidArgument);
  }
  return readRegister(bank, address);
}

template <SpiPort Port>
uint16_t ADS9324<Port>::GetShadow(reg::Bank bank, uint8_t address) const noexcept {
  if (!detail::IsEnumerator(bank) || !reg::IsDocumented(bank, address)) {
    return detail::kZero16;
  }
  return shadow_[detail::BankIndex(bank)][address];
}

template <SpiPort Port>
Result<void> ADS9324<Port>::runPatternFrame(const Config& test, Frame& frame) noexcept {
  if (auto r = applyImage(test, Verify::None); !r) {
    return r;
  }
  config_ = test;
  if constexpr (detail::HasGpioSet<Port>) {
    if (auto r = Convert(); !r) {
      return r;
    }
  }
  if (auto r = WaitDataReady(); !r) {
    return r;
  }
  const auto read = ReadFrame();
  if (!read) {
    return Result<void>::Failure(read.error);
  }
  frame = read.value;
  for (uint8_t ch = 0U; ch < reg::kChannelCount; ++ch) {
    const auto want = static_cast<uint16_t>(detail::kSelfTestBase + (detail::kSelfTestStep * ch));
    if (frame.raw[ch] != want) {
      return fail(Error::DataMismatch, "ads9324: a channel returned another channel's pattern");
    }
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::restoreConfig(const Config& saved, Result<void> outcome) noexcept {
  const Result<void> restored = applyImage(saved, Verify::None);
  config_ = saved;
  return outcome ? restored : outcome;
}

template <SpiPort Port>
Result<void> ADS9324<Port>::RunDataPathSelfTest() noexcept {
  if (auto r = checkAlive(); !r) {
    return r;
  }
  const Config saved = config_;
  Config test = saved;
  test.channels = ChannelSelect{};
  test.filter = Filter{};
  for (uint32_t g = 0U; g < 2U; ++g) {
    TestPattern& t = test.test_pattern[g];
    t.enabled = true;
    t.kind = PatternKind::Constant;
    t.index_insertion = false;
    for (uint32_t i = 0U; i < 8U; ++i) {
      t.value[i] =
          static_cast<uint16_t>(detail::kSelfTestBase + (detail::kSelfTestStep * ((8U * g) + i)));
    }
  }
  Frame frame{};
  return restoreConfig(saved, runPatternFrame(test, frame));
}

// -------------------------------------------------------------------------------------------------
// Register engine
// -------------------------------------------------------------------------------------------------

template <SpiPort Port>
void ADS9324<Port>::resetShadow() noexcept {
  for (uint32_t b = 0U; b < reg::kBankCount; ++b) {
    const auto bank = static_cast<reg::Bank>(b);
    dirty_[b] = 0U;
    for (uint32_t a = 0U; a < reg::kAddressSpace; ++a) {
      shadow_[b][a] = reg::ResetValue(bank, static_cast<uint8_t>(a));
    }
  }
  // The threshold and hysteresis reset values are contradictory in the
  // datasheet (assumption A16): write them whatever the shadow says.
  for (uint8_t i = 0U; i < reg::kChannelsPerBank; ++i) {
    for (const reg::Bank bank : {reg::Bank::Ain1To8, reg::Bank::Ain9To16}) {
      dirty_[detail::BankIndex(bank)] |= detail::AddressBit(reg::DwcThAddress(i));
      dirty_[detail::BankIndex(bank)] |= detail::AddressBit(reg::DwcHysAddress(i));
    }
  }
}

template <SpiPort Port>
Result<void> ADS9324<Port>::selectBank(reg::Bank bank) noexcept {
  if (bank_valid_ && (bank_ == bank)) {
    return Result<void>::Success();
  }
  if (const Error e = bus_.Write(reg::kBankSel, reg::BankSelCode(bank)); e != Error::Ok) {
    bank_valid_ = false;
    return Result<void>::Failure(e);
  }
  bank_ = bank;
  bank_valid_ = true;
  return Result<void>::Success();
}

template <SpiPort Port>
Result<uint16_t> ADS9324<Port>::readRegister(reg::Bank bank, uint8_t address) noexcept {
  if (auto r = selectBank(bank); !r) {
    return Result<uint16_t>::Failure(r.error);
  }
  const auto value = bus_.Read(address);
  if (!value) {
    bank_valid_ = false;
  } else {
    last_reply_ = value.value;
  }
  return value;
}

template <SpiPort Port>
Result<void> ADS9324<Port>::writeRegister(reg::Bank bank, uint8_t address,
                                          uint16_t value) noexcept {
  if (!reg::IsDocumented(bank, address) || reg::IsReadOnly(bank, address)) {
    return Result<void>::Failure(Error::InvalidArgument);
  }
  if (auto r = selectBank(bank); !r) {
    return r;
  }
  const std::size_t b = detail::BankIndex(bank);
  dirty_[b] |= detail::AddressBit(address); // unknown until the chip took it
  if (const Error e = bus_.Write(address, value); e != Error::Ok) {
    bank_valid_ = false;
    return Result<void>::Failure(e);
  }
  shadow_[b][address] = value;
  dirty_[b] &= ~detail::AddressBit(address);
  written_[b] |= detail::AddressBit(address);
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::flushRegister(reg::Bank bank, uint8_t address,
                                          uint16_t value) noexcept {
  using detail::operator""_u32;
  const bool common = bank == reg::Bank::Common;
  if (common && (address == reg::kGenCfg5) && (value != 0U)) {
    // Set AVG_MODE_OVR_EN first, then AVG_MODE (section 7.3.11.2).
    if (auto r = writeRegister(bank, address, reg::kAvgModeOvrEn); !r) {
      return r;
    }
  }
  if (auto r = writeRegister(bank, address, value); !r) {
    return r;
  }
  if (common && (address == reg::kAnaCfg1)) {
    detail::DelayUs(bus_.GetPort(),
                    detail::kSettleUs); // reference turn-on (assumption A44)
  }
  filter_touched_ = filter_touched_ || detail::IsFilterRegister(bank, address);
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::flushPhase(const detail::RegisterImage& image,
                                       uint32_t phase) noexcept {
  for (uint32_t b = 0U; b < reg::kBankCount; ++b) {
    const auto bank = static_cast<reg::Bank>(b);
    for (uint32_t a = 0U; a < reg::kAddressSpace; ++a) {
      const auto address = static_cast<uint8_t>(a);
      if (!detail::IsTouched(image, bank, address) ||
          (detail::FlushPhase(bank, address) != phase)) {
        continue;
      }
      const uint16_t value = detail::Get(image, bank, address);
      const bool known = (dirty_[b] & detail::AddressBit(address)) == 0U;
      if (known && (shadow_[b][address] == value)) {
        continue;
      }
      if (auto r = flushRegister(bank, address, value); !r) {
        return r;
      }
    }
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::flushImage(const detail::RegisterImage& image) noexcept {
  for (uint32_t phase = 0U; phase < 3U; ++phase) {
    if (auto r = flushPhase(image, phase); !r) {
      return r;
    }
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::pulseSysref(uint32_t convst_hz) noexcept {
  // 0, 1, 0 with at least one CONVST period between the edges
  // (section 7.3.11.1).
  const uint16_t base = shadow_[detail::BankIndex(reg::Bank::Common)][reg::kDigFilter];
  const uint32_t period_us = (1'000'000U / convst_hz) + 1U;
  if (auto r =
          writeRegister(reg::Bank::Common, reg::kDigFilter, detail::Or(base, reg::kDigFiltSysref));
      !r) {
    return r;
  }
  detail::DelayUs(bus_.GetPort(), period_us);
  return writeRegister(reg::Bank::Common, reg::kDigFilter, base);
}

template <SpiPort Port>
Result<void> ADS9324<Port>::verifyWritten() noexcept {
  for (uint32_t b = 0U; b < reg::kBankCount; ++b) {
    const auto bank = static_cast<reg::Bank>(b);
    for (uint32_t a = 0U; a < reg::kAddressSpace; ++a) {
      const auto address = static_cast<uint8_t>(a);
      if ((written_[b] & detail::AddressBit(address)) == 0U) {
        continue;
      }
      const auto v = readRegister(bank, address);
      if (!v) {
        return Result<void>::Failure(v.error);
      }
      if (v.value != shadow_[b][address]) {
        return fail(Error::VerifyFailed, "ads9324: read-back differs from what was written");
      }
    }
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::applyImage(const Config& config, Verify verify) noexcept {
  written_ = {};
  filter_touched_ = false;
  layout_ok_ = false; // true again once every register of this image is written
  const detail::RegisterImage image = detail::Encode(config, cal_mode_);
  if (auto r = flushImage(image); !r) {
    return r;
  }
  layout_ok_ = true;
  const bool restart = (config.filter.kind != FilterKind::None) || detail::AnyPhaseDelay(config);
  if (filter_touched_ && restart) {
    if (auto r = pulseSysref(config.sampling.convst_hz); !r) {
      return r;
    }
  }
  if (verify == Verify::ReadBack) {
    return verifyWritten();
  }
  return Result<void>::Success();
}

template <SpiPort Port>
Result<void> ADS9324<Port>::fail(Error error, const char* message) noexcept {
  detail::Log(bus_.GetPort(), LogLevel::Error, message);
  return Result<void>::Failure(error);
}

} // namespace ads9324

#endif // ADS9324_IPP
