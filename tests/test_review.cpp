/**
 * @file test_review.cpp
 * @brief Tests that pin the reply position, reply validity, unseen resets, result pulses and
 * conversion time, plus the failure paths a mutation run showed untested (R01 to R24).
 */
#include <cstdint>

#include "mock_ads9324.hpp"
#include "test_harness.hpp"
#include "test_support.hpp"

using namespace ads9324;
using namespace ads9324_test;

namespace {

/// The write frames to a register of a bank, counted from the model's log.
int Writes(Rig& rig, uint8_t bank, uint8_t addr) {
  return WritesTo(rig.Chip(), bank, addr);
}

} // namespace

TEST_CASE("R01 a register read decodes SDOUT[23:8] and the frame tail is 0x00 (Table 7-17)") {
  Rig rig;
  REQUIRE(rig.Init());
  const auto id = rig.drv.ReadRegister(reg::Bank::Common, reg::kDeviceId);
  REQUIRE(id.ok());
  CHECK_EQ(id.value, 0x0004U);
  Rig shifted; // a chip that answers one byte late would read DEVICE_ID as 0x0400
  shifted.Chip().knobs.reply_shifted_by_8 = true;
  CHECK_EQ(shifted.drv.EnsureInitialized().error, Error::Bus);
  Rig junk;
  junk.Chip().knobs.reply_low_byte = 0x5A;
  CHECK_EQ(junk.drv.EnsureInitialized().error, Error::Bus);
}

TEST_CASE("R02 a start-up with a wrong DEVICE_ID or a wrong BANK_SEL is refused (A11)") {
  Rig wrong_id; // the second check, BANK_SEL, would pass
  wrong_id.Chip().knobs.device_id = 5;
  CHECK_EQ(wrong_id.drv.EnsureInitialized().error, Error::DeviceIdMismatch);
  Rig wrong_bank; // the first check, DEVICE_ID, passes
  wrong_bank.Chip().knobs.bank_sel_reset = 2;
  CHECK_EQ(wrong_bank.drv.EnsureInitialized().error, Error::DeviceIdMismatch);
}

TEST_CASE("R03 a reset between calls is seen before any setter writes (rule 12a)") {
  Rig rig;
  REQUIRE(rig.Init());
  Input in{};
  in.range = InputRange::V10;
  REQUIRE(rig.drv.SetInput(0, in).ok()); // bank 2 is now the cached bank
  rig.Chip().BrownOut();
  rig.Chip().Delay(31'000); // tPU
  const int pdn_writes = Writes(rig, 1, 0x08);
  in.range = InputRange::V12_5;
  const auto r = rig.drv.SetInput(0, in);
  CHECK_EQ(r.error, Error::UnexpectedReset);
  CHECK(!rig.drv.IsInitialized());
  CHECK_EQ(Writes(rig, 1, 0x08), pdn_writes); // nothing reached PDN_CTL (standby)
  CHECK(rig.Chip().bad_writes.empty());
  CHECK(rig.drv.EnsureInitialized().ok()); // the recovery re-applies everything
  CHECK_EQ(rig.Chip().Reg(2, 0x08), 0x0004U);
}

TEST_CASE("R04 every configuration call looks for a reset first, with the DIAG byte off") {
  Config c{};
  c.interface.append_diag = false;
  const auto call = [](Rig& rig, int which) {
    Input in{};
    switch (which) {
    case 0:
      return rig.drv.SetInput(1, in).error;
    case 1:
      return rig.drv.ClearWindowFlags(0).error;
    case 2:
      return rig.drv.RunAdcCalibration().error;
    case 3:
      return rig.drv.DisableAdcCalibration().error;
    case 4:
      return rig.drv.PowerDown().error;
    case 5:
      return rig.drv.RunDataPathSelfTest().error;
    default:
      return rig.drv.ReadWindowFlags().error;
    }
  };
  for (int which = 0; which < 7; ++which) {
    Rig rig(c);
    REQUIRE(rig.Init());
    rig.Chip().BrownOut();
    rig.Chip().Delay(31'000);
    const std::size_t before = rig.Chip().log.size();
    CHECK_EQ(call(rig, which), Error::UnexpectedReset);
    if (which == 4) {
      CHECK(rig.Chip().Standby()); // a switch-off is never refused for doubt (lessons 12c)
      continue;
    }
    for (std::size_t i = before; i < rig.Chip().log.size(); ++i) {
      const auto& f = rig.Chip().log[i];
      CHECK((f.addr == 0x00U) || (f.addr == 0x01U) || (f.addr == 0x02U)); // probe frames only
    }
  }
}

TEST_CASE("R05 a stuck, floating or malformed SDOUT is a Bus error under every status read") {
  for (int fault = 0; fault < 3; ++fault) {
    Rig rig;
    REQUIRE(rig.Init());
    Ads9324Model& chip = rig.Chip();
    chip.knobs.stuck_sdout = (fault == 0);
    chip.knobs.float_sdout = (fault == 1);
    chip.knobs.reply_low_byte = (fault == 2) ? 0x01U : 0U;
    CHECK_EQ(rig.drv.IsCalibrationBusy().error, Error::Bus); // never "idle"
    Input in{};
    CHECK_EQ(rig.drv.SetInput(0, in).error, Error::Bus);
    CHECK(rig.drv.IsInitialized()); // a line fault is not a reset
    CHECK_EQ(rig.drv.PowerDown().error, Error::Bus);
    CHECK(rig.Chip().Standby()); // the error is reported, the standby write still went out
  }
  Rig rig;
  REQUIRE(rig.Init());
  REQUIRE(rig.drv.PowerDown().ok());
  rig.Chip().knobs.stuck_sdout = true;
  CHECK_EQ(rig.drv.Wake().error, Error::Bus); // DIAG would read 0: RESET_DETECT is not proof
}

TEST_CASE("R06 MeasureOnce pulses CONVST for every result: block average and FIR") {
  struct Row {
    Filter filter{};
    uint32_t pulses{0};
  };
  const Row rows[] = {
      {{FilterKind::BlockAverage, AverageLength::X4, FirFilter::Fir1, false}, 4U},
      {{FilterKind::BlockAverage, AverageLength::X20, FirFilter::Fir1, false}, 20U},
      {{FilterKind::Fir, AverageLength::None, FirFilter::Fir3, false}, 4U},
      {{FilterKind::Fir, AverageLength::None, FirFilter::Fir7, false}, 8U},
      {{FilterKind::BlockAverage, AverageLength::X4, FirFilter::Fir1, true}, 1U},
      {{FilterKind::MovingAverage, AverageLength::X8, FirFilter::Fir1, false}, 1U},
  };
  for (const Row& row : rows) {
    Config c{};
    c.filter = row.filter;
    Rig rig(c);
    rig.Chip().knobs.stale_when_not_ready = true; // a read without a result gives the old frame
    REQUIRE(rig.Init());
    rig.Chip().input_uv[0] = 1'000'000;
    const uint32_t before = rig.Chip().conversions;
    const auto f = rig.drv.MeasureOnce();
    REQUIRE(f.ok());
    CHECK_EQ(rig.Chip().conversions - before, row.pulses);
    CHECK(f.value.codes[0] > 0); // a real result, not the idle line
    CHECK_EQ(PulsesPerResult(row.filter), row.pulses);
  }
}

TEST_CASE("R07 the conversion time is tCONV = (N - 1) x tCYC + 1.25 us (section 5.7)") {
  Config c{};
  c.sampling.ready = ReadySource::Time;
  c.sampling.convst_hz = 100'000;
  c.filter = {FilterKind::BlockAverage, AverageLength::X128, FirFilter::Fir1, true};
  Rig rig(c);
  rig.Chip().knobs.convst_hz = 100'000;
  REQUIRE(rig.Init());
  rig.Chip().input_uv[0] = 1'000'000;
  const uint64_t before = rig.port.state.delays_us;
  const auto f = rig.drv.MeasureOnce();
  REQUIRE(f.ok());
  CHECK(rig.port.state.delays_us - before >= 1271U); // 127 x 10 us + 1.25 us
  CHECK(f.value.codes[0] > 0);
}

TEST_CASE("R08 CONVST ends high before DRDY rises and every level lasts at least 50 ns (5.6)") {
  Config c{};
  c.filter = {FilterKind::BlockAverage, AverageLength::X8, FirFilter::Fir1, true};
  Rig rig(c);
  REQUIRE(rig.Init());
  rig.Chip().input_uv[0] = 1'000'000;
  rig.Chip().convst_edges.clear();
  REQUIRE(rig.drv.MeasureOnce().ok()); // lost without the rise (Figure 5-7, note 1)
  const auto& e = rig.Chip().convst_edges;
  REQUIRE(e.size() >= 3U);
  CHECK(e.back().second); // high at the end
  for (std::size_t i = 1; i < e.size(); ++i) {
    CHECK(e[i].first - e[i - 1].first >= 1U); // the port delay unit is 1 us, above 50 ns
  }
}

TEST_CASE("R09 Deinitialize after an unseen reset still puts the chip to sleep") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().BrownOut();
  rig.Chip().Delay(31'000);
  Input in{};
  REQUIRE(rig.drv.SetInput(0, in).error == Error::UnexpectedReset);
  rig.Chip().log.clear();
  CHECK(rig.drv.Deinitialize().ok());
  CHECK(rig.Chip().Standby());
  CHECK_EQ(Writes(rig, 1, 0x08), 1);
  Rig never; // nothing was ever sent: nothing to do
  CHECK(never.drv.Deinitialize().ok());
  CHECK(never.Chip().log.empty());
}

TEST_CASE("R10 a half-applied configuration refuses frames until a call succeeds") {
  Rig rig;
  REQUIRE(rig.Init());
  Interface ifc{};
  ifc.length = WordLength::Bits24;
  rig.Chip().knobs.fail_after_delivery = true; // the chip took 24-bit words, the driver heard "no"
  rig.Chip().knobs.fail_transfer = rig.Chip().transfers + 5; // the GEN_CFG3 write, after the probe
  CHECK_EQ(rig.drv.SetInterface(ifc).error, Error::Bus);
  rig.Chip().knobs.fail_transfer = -1;
  rig.Chip().knobs.fail_after_delivery = false;
  CHECK_EQ(rig.drv.ReadFrame().error, Error::Refused);
  CHECK(rig.drv.SetInterface(ifc).ok());
  CHECK(rig.drv.MeasureOnce().ok());
}

TEST_CASE("R11 a failed read or write re-sends BANK_SEL on the next call (KB-CPP-118)") {
  Rig rig;
  REQUIRE(rig.Init());
  REQUIRE(rig.drv.ReadRegister(reg::Bank::Common, reg::kDeviceId).ok());
  rig.Chip().knobs.fail_transfer = rig.Chip().transfers; // the read command
  CHECK_EQ(rig.drv.ReadRegister(reg::Bank::Common, reg::kDeviceId).error, Error::Bus);
  rig.Chip().knobs.fail_transfer = -1;
  rig.Chip().log.clear();
  REQUIRE(rig.drv.ReadRegister(reg::Bank::Common, reg::kDeviceId).ok());
  CHECK_EQ(rig.Chip().log[0].addr, 0x02U); // BANK_SEL first, the cache was dropped
  CHECK_EQ(rig.Chip().log[0].data, 0x0001U);
  // The write path: the PDN_CTL write fails after the probe.
  rig.Chip().knobs.fail_transfer = rig.Chip().transfers + 5;
  CHECK_EQ(rig.drv.PowerDown().error, Error::Bus);
  rig.Chip().knobs.fail_transfer = -1;
  rig.Chip().log.clear();
  REQUIRE(rig.drv.ReadRegister(reg::Bank::Common, reg::kDeviceId).ok());
  CHECK_EQ(rig.Chip().log[0].addr, 0x02U);
}

TEST_CASE("R12 a write that reached the chip but reported failure is sent again (dirty bit)") {
  Rig rig;
  REQUIRE(rig.Init());
  Input in{};
  in.range = InputRange::V10;
  REQUIRE(rig.drv.SetInput(0, in).ok()); // shadow: V10
  in.range = InputRange::V12_5;
  rig.Chip().knobs.fail_after_delivery = true;
  rig.Chip().knobs.fail_transfer = rig.Chip().transfers + 6; // the PGA_CONFIG write
  CHECK_EQ(rig.drv.SetInput(0, in).error, Error::Bus);
  rig.Chip().knobs.fail_transfer = -1;
  CHECK(rig.Chip().Reg(2, 0x08) != 0x0004U); // the chip did take V12_5
  in.range = InputRange::V10;
  REQUIRE(rig.drv.SetInput(0, in).ok());
  CHECK_EQ(rig.Chip().Reg(2, 0x08), 0x0004U); // the driver did not trust its shadow
}

TEST_CASE("R13 the RESET pulse is long enough for a chip that needs 1 ms (A4)") {
  Config c{};
  c.hardware_reset = true;
  Rig rig(c);
  rig.Chip().knobs.reset_min_us = 1000;
  const uint32_t resets = rig.Chip().resets;
  REQUIRE(rig.Init());
  CHECK(rig.Chip().resets > resets);
  CHECK(rig.drv.IsInitialized());
}

TEST_CASE("R14 the CALIB_BUSY wait is bounded by the single-shot time plus its margin (A36)") {
  Rig fast;
  fast.Chip().knobs.cal_busy_us = 149'000;
  REQUIRE(fast.Init());
  CHECK(fast.drv.RunAdcCalibration().ok());
  Rig slow;
  slow.Chip().knobs.cal_busy_us = 151'000;
  REQUIRE(slow.Init());
  CHECK_EQ(slow.drv.RunAdcCalibration().error, Error::Timeout);
}

TEST_CASE("R16 hysteresis lands in the right half of the right register on both banks") {
  Config c{};
  for (std::size_t ch = 0; ch < 16; ++ch) {
    c.inputs[ch].window = Window{true, 10, -10, static_cast<uint8_t>(ch + 1U)};
  }
  Rig rig(c);
  REQUIRE(rig.Init());
  for (int ch = 0; ch < 16; ++ch) {
    const uint32_t word =
        rig.Chip().Reg(Ads9324Model::ChannelBank(ch), rig.Chip().ChannelAddr(ch, 0x2AU, 2));
    const uint32_t half = ((rig.Chip().LocalIndex(ch) % 2) != 0) ? (word >> 8U) : (word & 0xFFU);
    CHECK_EQ(half, static_cast<uint32_t>(ch + 1));
  }
}

TEST_CASE("R17 switching the gain stage off sets GAN_CORR_DIS in both banks") {
  Config c{};
  c.correction.gain_enabled = false;
  Rig rig(c);
  REQUIRE(rig.Init());
  CHECK_EQ(rig.Chip().Reg(2, 0x37) & 1U, 1U);
  CHECK_EQ(rig.Chip().Reg(4, 0x37) & 1U, 1U);
  CHECK_EQ(rig.Chip().Reg(2, 0x37) & 2U, 0U);
}

TEST_CASE("R18 AVG_MODE_OVR_EN is written before AVG_MODE (section 7.3.11.2)") {
  Config c{};
  c.filter = {FilterKind::BlockAverage, AverageLength::X4, FirFilter::Fir1, false};
  Rig rig(c);
  REQUIRE(rig.Init());
  std::vector<uint16_t> seen;
  for (const auto& f : rig.Chip().log) {
    if (f.is_register_frame && (f.bank == 1U) && (f.addr == 0x15U)) {
      seen.push_back(f.data);
    }
  }
  REQUIRE(seen.size() >= 2U);
  CHECK_EQ(seen[seen.size() - 2U], 0x0002U);
  CHECK_EQ(seen[seen.size() - 1U], 0x0003U);
}

TEST_CASE("R15 a switch-off after a reset report writes standby and is not refused for doubt") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().BrownOut();
  rig.Chip().Delay(31'000);
  CHECK_EQ(rig.drv.ReadWindowFlags().error, Error::UnexpectedReset);
  CHECK(!rig.drv.IsInitialized());
  CHECK(rig.drv.PowerDown().ok());
  CHECK(rig.Chip().Standby());
}

TEST_CASE("R19 no call before the start divides by zero or sends a frame, whatever the Config") {
  Config c{};
  c.sampling.convst_hz = 0;
  Rig rig(c);
  CHECK_EQ(rig.drv.MeasureOnce().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.Convert().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.WaitDataReady().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.ReadFrame().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.EnsureInitialized().error, Error::InvalidArgument);
  CHECK_EQ(rig.Chip().log.size(), 0U);
}

TEST_CASE("R20 a setter after a reset report is NotInitialized, not Ok (lessons 12a)") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().BrownOut();
  rig.Chip().Delay(31'000);
  CHECK_EQ(rig.drv.ReadWindowFlags().error, Error::UnexpectedReset);
  Input in{};
  CHECK_EQ(rig.drv.SetInput(1, in).error, Error::NotInitialized);
  CHECK_EQ(rig.drv.ApplyConfig(Config{}).error, Error::NotInitialized);
  REQUIRE(rig.Init());
  CHECK(rig.drv.SetInput(1, in).ok());
}

TEST_CASE("R21 after a reset the data is on D7 to D0: a readout on SDOUT is not returned as Ok") {
  Config c{};
  c.interface.append_diag = false;
  Rig rig(c);
  REQUIRE(rig.Init());
  rig.Chip().BrownOut();
  rig.Chip().Delay(31'000);
  const auto f = rig.drv.MeasureOnce();
  CHECK(!f.ok());
}

TEST_CASE("R22 a continuous-mode calibration is over when RunAdcCalibration returns (fuzz crash)") {
  Config c{};
  c.calibration.trigger = CalibrationTrigger::Continuous;
  c.calibration.mode = CalibrationMode::Gain;
  c.test_pattern[1].enabled = true; // the stuck-frame check is off, an idle frame would pass
  Rig rig(c);
  REQUIRE(rig.Init());
  const auto cal = rig.drv.RunAdcCalibration();
  CHECK_EQ(cal.error, Error::Ok);
  REQUIRE(cal.ok());
  CHECK(!rig.Chip().Calibrating());
  CHECK_EQ(rig.Chip().calibrations, 1U); // finished, not started by the next CONVST pulse
  const auto f = rig.drv.MeasureOnce();
  REQUIRE(f.ok());
  CHECK_EQ(f.value.raw[0], rig.Chip().sent_raw[0]);
  CHECK(!rig.Chip().Calibrating());
}
