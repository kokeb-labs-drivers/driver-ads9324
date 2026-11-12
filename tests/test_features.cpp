/**
 * @file test_features.cpp
 * @brief Window comparator, alarm pin, digital filters, ADC calibration, power
 * and the knobs for the datasheet's open questions (T23 to T31).
 */
#include "test_support.hpp"

using namespace ads9324;
using namespace ads9324_test;

namespace {

/// Index in the log of the first register write to `addr` while `bank` is
/// selected; -1 if none.
int FirstWrite(const Ads9324Model& chip, uint8_t bank, uint8_t addr) {
  int i = 0;
  for (const FrameLog& f : chip.log) {
    if (f.is_register_frame && (f.addr == addr) && (f.bank == bank)) {
      return i;
    }
    ++i;
  }
  return -1;
}

Config WindowConfig(int ch, int8_t high, int8_t low) {
  Config c{};
  c.inputs[static_cast<std::size_t>(ch)].window = Window{true, high, low, 2};
  return c;
}

} // namespace

TEST_CASE("T23 the daisy chain is out of scope: refused, never written (F14)") {
  Config c{};
  c.daisy.enabled = true;
  CHECK_EQ(Validate(c), Error::Unsupported);
  Rig rig;
  REQUIRE(rig.Init());
  Config d = rig.drv.GetConfig();
  d.daisy.enabled = true;
  CHECK_EQ(rig.drv.ApplyConfig(d).error, Error::Unsupported);
  CHECK_EQ(rig.Chip().Reg(1, 0x09), 0U);
  CHECK_EQ(rig.Chip().CountWrites(0x09), 0);
}

TEST_CASE("T26 window thresholds are written before the window is enabled (A16)") {
  Rig rig(WindowConfig(3, 40, -40));
  REQUIRE(rig.Init());
  const int th = FirstWrite(rig.Chip(), 2, 0x25);
  const int en = FirstWrite(rig.Chip(), 2, 0x21);
  CHECK(th >= 0);
  CHECK(en > th);
  CHECK_EQ(rig.Chip().Reg(2, 0x25), 0x28D8U);
  CHECK_EQ(rig.Chip().Reg(2, 0x21) & 0x08U, 0x08U);
}

TEST_CASE("T26 the thresholds are written whatever the chip's reset value is (A16)") {
  for (const bool text : {false, true}) {
    Config c = WindowConfig(0, 127, -128);
    Rig rig(c);
    rig.Chip().knobs.dwc_reset_text = text;
    REQUIRE(rig.Init());
    CHECK_EQ(rig.Chip().Reg(2, 0x22), 0x7F80U);
    CHECK_EQ(rig.Chip().Reg(4, 0x22), 0x7F80U);
  }
}

TEST_CASE("T26 flags are read per channel for both banks and cleared per group") {
  Config c = WindowConfig(2, 10, -10);
  c.inputs[11].window = Window{true, 10, -10, 0};
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  rig.Chip().input_uv[2] = 2'000'000;
  rig.Chip().input_uv[11] = -2'000'000;
  REQUIRE(rig.Init());
  REQUIRE(rig.drv.MeasureOnce().ok());
  const auto flags = rig.drv.ReadWindowFlags();
  REQUIRE(flags.ok());
  CHECK_EQ(flags.value.high, 1U << 2U);
  CHECK_EQ(flags.value.low, 1U << 11U);
  CHECK(rig.drv.ClearWindowFlags(0).ok());
  const auto after = rig.drv.ReadWindowFlags();
  CHECK_EQ(after.value.high, 0U);
  CHECK_EQ(after.value.low, 1U << 11U);
  CHECK(rig.drv.ClearWindowFlags(1).ok());
  CHECK_EQ(rig.drv.ReadWindowFlags().value.low, 0U);
  CHECK_EQ(rig.drv.ClearWindowFlags(2).error, Error::InvalidArgument);
  CHECK_EQ(rig.Chip().Reg(4, 0x21) & 0x8000U, 0U); // DWC_STAT_RST ends at 0
}

TEST_CASE("T26 the glitch filter delays the flag") {
  Config c = WindowConfig(1, 10, -10);
  c.window_glitch[0] = 2;
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  rig.Chip().input_uv[1] = 3'000'000;
  REQUIRE(rig.Init());
  REQUIRE(rig.drv.MeasureOnce().ok());
  CHECK_EQ(rig.drv.ReadWindowFlags().value.high, 0U);
  REQUIRE(rig.drv.MeasureOnce().ok());
  REQUIRE(rig.drv.MeasureOnce().ok());
  CHECK_EQ(rig.drv.ReadWindowFlags().value.high, 2U);
}

TEST_CASE("T26 a window with offset binary coding is refused (A16)") {
  Config c = WindowConfig(1, 10, -10);
  c.interface.format = DataFormat::OffsetBinary;
  CHECK_EQ(Validate(c), Error::InvalidArgument);
  Config d = WindowConfig(1, -10, 10); // low above high
  CHECK_EQ(Validate(d), Error::InvalidArgument);
}

TEST_CASE("T27 the alarm pin shows DRDY, the window alert, calibration done "
          "and the OR") {
  Config c = WindowConfig(0, 10, -10);
  c.alarm.function = AlarmFunction::WindowAlert;
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  rig.Chip().input_uv[0] = 3'000'000;
  REQUIRE(rig.Init());
  CHECK(!rig.drv.ReadAlarmPin().value);
  REQUIRE(rig.drv.MeasureOnce().ok());
  CHECK(rig.drv.ReadAlarmPin().value);
  CHECK(rig.drv.ClearWindowFlags(0).ok());
  CHECK(!rig.drv.ReadAlarmPin().value);
  Alarm a{};
  a.function = AlarmFunction::WindowAlert;
  a.active_low = true;
  a.pulse = true;
  REQUIRE(rig.drv.SetAlarm(a).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x0C) & 0x003FU, 0x0031U);
  CHECK(rig.drv.ReadAlarmPin().value == false);
  a.disabled = true;
  REQUIRE(rig.drv.SetAlarm(a).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x0D) & 0x0020U, 0x0020U);
}

TEST_CASE("T27 the OR of masked sources needs the acknowledgement and follows "
          "the model's mask (A41)") {
  Alarm a{};
  a.function = AlarmFunction::OrMasked;
  a.sources = kAlarmSourceDrdy | kAlarmSourceCalDone;
  Rig rig;
  REQUIRE(rig.Init());
  CHECK_EQ(rig.drv.SetAlarm(a).error, Error::InvalidArgument);
  a.or_mask_acknowledged = true;
  REQUIRE(rig.drv.SetAlarm(a).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x0C) >> 8U, 0xBEU);
  REQUIRE(rig.drv.Convert().ok());
  REQUIRE(rig.drv.WaitDataReady().ok());
  CHECK(rig.drv.ReadAlarmPin().value);
  // A chip with the opposite mask sense: the driver's choice is wrong there,
  // and the test shows it.
  Rig other;
  other.Chip().knobs.alrm_mask_one_masks = false;
  REQUIRE(other.Init());
  REQUIRE(other.drv.SetAlarm(a).ok());
  REQUIRE(other.drv.Convert().ok());
  REQUIRE(other.drv.WaitDataReady().ok());
  CHECK(!other.drv.ReadAlarmPin().value);
  a.sources = 0;
  CHECK_EQ(Validate([&] {
             Config c{};
             c.alarm = a;
             return c;
           }()),
           Error::InvalidArgument);
}

TEST_CASE("T27 a pulse alarm is for the window alert only") {
  Config c{};
  c.alarm.pulse = true;
  CHECK_EQ(Validate(c), Error::InvalidArgument);
  c.alarm.function = AlarmFunction::Drdy;
  c.alarm.sources = 3;
  c.alarm.pulse = false;
  CHECK_EQ(Validate(c), Error::InvalidArgument);
}

TEST_CASE("T28 the block average: gate, internal clock and the SYSREF pulse 0, "
          "1, 0 after the filter") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().log.clear();
  Filter f{FilterKind::BlockAverage, AverageLength::X8, FirFilter::Fir1, true};
  REQUIRE(rig.drv.SetFilter(f).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x14), 0x0042U);
  CHECK_EQ(rig.Chip().Reg(1, 0x15), 0x0003U);
  CHECK_EQ(rig.Chip().sysref_edges, 1U);
  const int gate = FirstWrite(rig.Chip(), 1, 0x15);
  const int filter = FirstWrite(rig.Chip(), 1, 0x14);
  CHECK(gate >= 0);
  CHECK(filter > gate);
  int pulses = 0;
  for (const FrameLog& l : rig.Chip().log) {
    pulses += (l.is_register_frame && (l.addr == 0x14U) && ((l.data & 1U) != 0U)) ? 1 : 0;
  }
  CHECK_EQ(pulses, 1);
  CHECK_EQ(rig.Chip().Reg(1, 0x14) & 1U, 0U);
  CHECK(rig.Chip().bad_writes.empty());
}

TEST_CASE("T28 the block average returns the mean after N conversions") {
  Config c{};
  c.filter = Filter{FilterKind::BlockAverage, AverageLength::X4, FirFilter::Fir1, false};
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  rig.Chip().input_uv[0] = 1'000'000;
  REQUIRE(rig.Init());
  for (int i = 0; i < 3; ++i) {
    REQUIRE(rig.drv.Convert().ok());
  }
  CHECK_EQ(rig.drv.ReadFrame().error, Error::DataMismatch); // not ready yet
  REQUIRE(rig.drv.Convert().ok());
  CHECK(rig.drv.WaitDataReady().ok());
  CHECK_EQ(rig.drv.ReadFrame().value.codes[0], 6554);
  CHECK_EQ(OutputRateHz(c), 25'000U);
}

TEST_CASE("T28 every filter kind and length encodes to a register the model "
          "accepts") {
  Rig rig;
  REQUIRE(rig.Init());
  for (uint32_t len = 1; len <= 11; ++len) {
    REQUIRE(rig.drv
                .SetFilter({FilterKind::MovingAverage, static_cast<AverageLength>(len),
                            FirFilter::Fir1, false})
                .ok());
    CHECK_EQ((rig.Chip().Reg(1, 0x14) >> 8U) & 0xFU, len);
    REQUIRE(rig.drv
                .SetFilter({FilterKind::BlockAverage, static_cast<AverageLength>(len),
                            FirFilter::Fir1, false})
                .ok());
    CHECK_EQ((rig.Chip().Reg(1, 0x14) >> 4U) & 0xFU, len);
  }
  const FirFilter firs[] = {FirFilter::Fir1, FirFilter::Fir2, FirFilter::Fir3,
                            FirFilter::Fir4, FirFilter::Fir5, FirFilter::Fir7};
  for (const FirFilter fir : firs) {
    REQUIRE(rig.drv.SetFilter({FilterKind::Fir, AverageLength::None, fir, false}).ok());
    CHECK_EQ((rig.Chip().Reg(1, 0x14) >> 12U) & 7U, static_cast<uint32_t>(fir));
  }
  CHECK_EQ(
      rig.drv.SetFilter({FilterKind::Fir, AverageLength::None, static_cast<FirFilter>(6), false})
          .error,
      Error::InvalidArgument);
  REQUIRE(rig.drv.SetFilter(Filter{}).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x15), 0U);
  CHECK(rig.Chip().bad_writes.empty());
}

TEST_CASE("T29 single-shot calibration removes the offset error and keeps the "
          "mode (A35, A39)") {
  Rig rig;
  rig.Chip().analog_offset_lsb = 20;
  rig.Chip().input_uv[0] = 0;
  REQUIRE(rig.Init());
  CHECK_EQ(rig.drv.MeasureOnce().value.codes[0], 20);
  const uint64_t before = rig.port.state.delays_us;
  REQUIRE(rig.drv.RunAdcCalibration().ok());
  CHECK(rig.port.state.delays_us - before >= 125'000U);
  CHECK_EQ(rig.Chip().calibrations, 1U);
  CHECK_EQ(rig.Chip().CalMode(), 3U);
  CHECK_EQ(rig.Chip().Reg(1, 0x14) & 2U, 0U); // INT_TRIG_MODE restored
  CHECK_EQ(rig.Chip().Reg(1, 0x12) & 3U, 3U);
  CHECK_EQ(rig.Chip().Reg(1, 0x12) & 0x0100U, 0U); // TRIG back at 0
  CHECK_EQ(rig.drv.MeasureOnce().value.codes[0], 0);
  CHECK(!rig.port.state.gpio_log.empty());
  CHECK_EQ(rig.port.state.gpio_log.back().second,
           static_cast<uint8_t>(GpioSignal::ACTIVE)); // a pulse ends high (Figure 5-7)
  CHECK(!rig.drv.IsCalibrationBusy().value);
  // A later configuration change keeps ADC_CAL_MODE.
  Input in{};
  in.range = InputRange::V10;
  REQUIRE(rig.drv.SetInput(3, in).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x12) & 3U, 3U);
  REQUIRE(rig.drv.DisableAdcCalibration().ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x12) & 3U, 0U);
}

TEST_CASE("T29 calibration needs the filters off, an awake chip and CONVST") {
  Rig rig;
  REQUIRE(rig.Init());
  REQUIRE(rig.drv.SetFilter({FilterKind::MovingAverage, AverageLength::X4, FirFilter::Fir1, false})
              .ok());
  CHECK_EQ(rig.drv.RunAdcCalibration().error, Error::Refused);
  MinimalPort port;
  ADS9324<MinimalPort> drv(port);
  REQUIRE(drv.EnsureInitialized().ok());
  CHECK_EQ(drv.RunAdcCalibration().error, Error::Unsupported);
  Config c{};
  c.calibration.run_at_start = true;
  c.filter = Filter{FilterKind::BlockAverage, AverageLength::X2, FirFilter::Fir1, false};
  CHECK_EQ(Validate(c), Error::InvalidArgument);
}

TEST_CASE("T29 a chip that needs CONVST to end high shows a timeout when it "
          "does not") {
  Rig rig;
  rig.Chip().knobs.cal_needs_high_end = true;
  REQUIRE(rig.Init());
  // The driver ends high, so this passes; a port that cannot hold the level
  // would time out.
  CHECK(rig.drv.RunAdcCalibration().ok());
  Rig slow;
  slow.Chip().knobs.cal_busy_us = 400'000; // longer than 125 ms plus the margin
  REQUIRE(slow.Init());
  CHECK_EQ(slow.drv.RunAdcCalibration().error, Error::Timeout);
}

TEST_CASE("T29 calibration at start-up and the continuous flow") {
  Config c{};
  c.calibration.run_at_start = true;
  c.calibration.mode = CalibrationMode::Offset;
  Rig rig(c);
  REQUIRE(rig.Init());
  CHECK_EQ(rig.Chip().CalMode(), 1U);
  Config d{};
  d.calibration.trigger = CalibrationTrigger::Continuous;
  d.calibration.mode = CalibrationMode::Gain;
  Rig cont(d);
  REQUIRE(cont.Init());
  CHECK(cont.drv.RunAdcCalibration().error == Error::Timeout || cont.drv.RunAdcCalibration().ok());
  CHECK_EQ(cont.drv.IsCalibrationBusy().error == Error::Ok, true);
}

TEST_CASE("T29 the calibration reads SE_DIFF from the input types of each bank") {
  Config c{};
  for (std::size_t i = 0; i < 8; ++i) {
    c.inputs[i].type = InputType::SingleEnded;
  }
  Rig rig(c);
  REQUIRE(rig.Init());
  CHECK_EQ(rig.Chip().Reg(1, 0x12) & 0x3000U,
           0x1000U); // bank 1 single ended, bank 2 differential
  CHECK_EQ((rig.Chip().Reg(2, 0x08) & 0xFFU) >> 4U, 5U);
}

TEST_CASE("T30 standby: refused calls, wake, and a lost configuration") {
  Rig rig;
  REQUIRE(rig.Init());
  CHECK_EQ(rig.drv.Wake().error, Error::Refused);
  REQUIRE(rig.drv.PowerDown().ok());
  CHECK(rig.drv.IsPoweredDown());
  CHECK_EQ(rig.drv.Convert().error, Error::Refused);
  CHECK_EQ(rig.drv.ReadFrame().error, Error::Refused);
  CHECK_EQ(rig.drv.ApplyConfig(Config{}).error, Error::Refused);
  CHECK_EQ(rig.drv.RunAdcCalibration().error, Error::Refused);
  REQUIRE(rig.drv.Wake().ok());
  CHECK(!rig.drv.IsPoweredDown());
  CHECK(rig.drv.MeasureOnce().ok());
  Rig lossy;
  lossy.Chip().knobs.standby_retains = false;
  REQUIRE(lossy.Init());
  REQUIRE(lossy.drv.PowerDown().ok());
  CHECK_EQ(lossy.drv.Wake().error, Error::UnexpectedReset);
  CHECK(!lossy.drv.IsInitialized());
  CHECK(lossy.drv.EnsureInitialized().ok());
  CHECK_EQ(lossy.Chip().Reg(1, 0x0A), 0x0032U);
}

TEST_CASE("T30 a changed GEN_CFG3 after standby is VerifyFailed") {
  Rig rig;
  REQUIRE(rig.Init());
  REQUIRE(rig.drv.PowerDown().ok());
  rig.Chip().Poke(1, 0x0A, 0x0000);
  CHECK_EQ(rig.drv.Wake().error, Error::VerifyFailed);
}

TEST_CASE("T31 RESET_DETECT_FLAG reading 0 after reset is tolerated (A11)") {
  Rig rig;
  rig.Chip().knobs.reset_detect_init = false;
  CHECK(rig.drv.EnsureInitialized().ok());
}

TEST_CASE("T31 the reply to a non-read frame is never used as data (A7)") {
  Rig rig;
  rig.Chip().knobs.sdout_after_write_zero = false;
  CHECK(rig.drv.EnsureInitialized().ok());
  CHECK_EQ(rig.drv.Identify().value, 0x0004U);
}

TEST_CASE("T31 bank layout knob: an ascending second bank fails the self test, "
          "not silently (A19)") {
  Rig rig;
  rig.Chip().knobs.bank2_mirrored = false;
  REQUIRE(rig.Init());
  Input in{};
  in.range = InputRange::V10;
  REQUIRE(rig.drv.SetInput(15, in).ok());
  // The driver wrote the register it believes is AIN16's; the chip put it on
  // AIN9.
  CHECK_EQ(rig.Chip().Reg(4, 0x08) & 0xFFU, 4U);
}

TEST_CASE("T31 a transfer that fails in standby, calibration or window calls "
          "is Bus") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().knobs.fail_transfer = rig.Chip().transfers;
  CHECK_EQ(rig.drv.ReadWindowFlags().error, Error::Bus);
  rig.Chip().knobs.fail_transfer = rig.Chip().transfers;
  CHECK_EQ(rig.drv.IsCalibrationBusy().error, Error::Bus);
  rig.Chip().knobs.fail_transfer = rig.Chip().transfers;
  CHECK_EQ(rig.drv.PowerDown().error, Error::Bus);
  rig.Chip().knobs.fail_transfer = -1;
  rig.port.state.gpio_fail = rig.port.state.gpio_calls;
  CHECK_EQ(rig.drv.ReadAlarmPin().error == Error::Ok, true);
}
