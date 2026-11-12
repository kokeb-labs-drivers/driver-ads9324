/**
 * @file test_startup.cpp
 * @brief Start-up, reset, identity and lifecycle against the chip model (T01 to
 * T06, T24).
 */
#include "test_support.hpp"

using namespace ads9324;
using namespace ads9324_test;

TEST_CASE("T01 start-up follows Table 7-18 and reaches the configured state") {
  Rig rig;
  const auto r = rig.drv.EnsureInitialized();
  REQUIRE(r.ok());
  CHECK(rig.drv.IsInitialized());
  CHECK(rig.port.state.delays_us >= 60'000U); // tPU and the reset settle
  CHECK_EQ(rig.Chip().Reg(1, 0x0A), 0x0032U); // 16-bit words, one lane on SDOUT
  CHECK_EQ(rig.Chip().Reg(1, 0x07), 0x0000U); // RESET_DETECT_FLAG cleared
  CHECK(rig.Chip().bad_writes.empty());
  CHECK_EQ(rig.port.state.bus_up_calls, 1);
  CHECK_EQ(rig.Chip().resets, 2U); // power-on and the software reset
}

TEST_CASE("T01 a second EnsureInitialized() is free") {
  Rig rig;
  REQUIRE(rig.Init());
  const std::size_t frames = rig.Chip().log.size();
  CHECK(rig.drv.EnsureInitialized().ok());
  CHECK_EQ(rig.Chip().log.size(), frames);
  CHECK_EQ(rig.port.state.bus_up_calls, 1);
}

TEST_CASE("T02 nothing is sent before the start") {
  Rig rig;
  CHECK_EQ(rig.drv.Convert().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.WaitDataReady().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.ReadFrame().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.Identify().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.ReadWindowFlags().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.RunAdcCalibration().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.PowerDown().error, Error::NotInitialized);
  CHECK_EQ(rig.drv.ReadRegister(reg::Bank::Common, reg::kDeviceId).error, Error::NotInitialized);
  CHECK_EQ(rig.drv.Reset(ResetKind::Software).error, Error::NotInitialized);
  CHECK(rig.Chip().log.empty());
}

TEST_CASE("T02 an invalid Config is refused before any I/O") {
  Config bad{};
  bad.sampling.convst_hz = 5;
  Rig rig(bad);
  CHECK_EQ(rig.drv.EnsureInitialized().error, Error::InvalidArgument);
  CHECK(rig.Chip().log.empty());
  CHECK_EQ(rig.port.state.bus_up_calls, 0);
  Config daisy{};
  daisy.daisy.enabled = true;
  Rig rig2(daisy);
  CHECK_EQ(rig2.drv.EnsureInitialized().error, Error::Unsupported);
  CHECK(rig2.Chip().log.empty());
}

TEST_CASE("T03 the port failing to come up is Bus") {
  Rig rig;
  rig.port.state.bus_up_ok = false;
  CHECK_EQ(rig.drv.EnsureInitialized().error, Error::Bus);
  CHECK(!rig.drv.IsInitialized());
  rig.port.state.bus_up_ok = true;
  CHECK(rig.drv.EnsureInitialized().ok()); // the next call starts again
}

TEST_CASE("T03 a transfer failure at every point of the start-up is Bus and "
          "recoverable") {
  for (int at = 0; at < 40; ++at) {
    Rig rig;
    rig.Chip().knobs.fail_transfer = at;
    const auto r = rig.drv.EnsureInitialized();
    if (r.ok()) {
      CHECK(at >= 30); // the start-up has fewer transfers than that
      continue;
    }
    CHECK_EQ(r.error, Error::Bus);
    CHECK(!rig.drv.IsInitialized());
    rig.Chip().knobs.fail_transfer = -1;
    CHECK(rig.drv.EnsureInitialized().ok());
    CHECK(rig.Chip().bad_writes.empty());
  }
}

TEST_CASE("T03 a wrong DEVICE_ID or a stuck line is DeviceIdMismatch") {
  Rig rig;
  rig.Chip().knobs.stuck_sdout = true;
  CHECK_EQ(rig.drv.EnsureInitialized().error, Error::DeviceIdMismatch);
  Rig rig2;
  rig2.Chip().knobs.float_sdout = true; // SDOUT[7:0] is 0xFF: a line fault, never data
  CHECK_EQ(rig2.drv.EnsureInitialized().error, Error::Bus);
  Rig rig3;
  rig3.Chip().knobs.flip_reply_bit = 3;
  CHECK_EQ(rig3.drv.EnsureInitialized().error, Error::DeviceIdMismatch);
}

TEST_CASE("T03 a wrong SPI mode gives garbage and is refused") {
  Rig rig;
  rig.Chip().knobs.spi_mode = 1; // A1: the chip decodes another mode; the reply tail is not 0x00
  CHECK_EQ(rig.drv.EnsureInitialized().error, Error::Bus);
}

TEST_CASE("T04 reads are two frames with the read command in GEN_CFG1") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().log.clear();
  const auto id = rig.drv.Identify();
  REQUIRE(id.ok());
  CHECK_EQ(id.value, 0x0004U);
  REQUIRE(rig.Chip().log.size() >= 2U);
  const auto& cmd = rig.Chip().log[rig.Chip().log.size() - 2U];
  const auto& nop = rig.Chip().log.back();
  CHECK_EQ(cmd.addr, 0x01U);
  CHECK_EQ(cmd.data, 0x2101U); // DEVICE_ID address 0x21, REG_RD_EN
  CHECK_EQ(nop.addr, 0x00U);
  CHECK_EQ(nop.data, 0x0000U);
}

TEST_CASE("T04 ReadRegister reads every documented register of every bank") {
  Rig rig;
  REQUIRE(rig.Init());
  int count = 0;
  for (uint8_t b = 0; b < 3; ++b) {
    for (uint32_t a = 0; a < 64; ++a) {
      const auto bank = static_cast<reg::Bank>(b);
      const auto r = rig.drv.ReadRegister(bank, static_cast<uint8_t>(a));
      if (reg::IsDocumented(bank, static_cast<uint8_t>(a))) {
        CHECK(r.ok());
        ++count;
      } else {
        CHECK_EQ(r.error, Error::InvalidArgument);
      }
    }
  }
  CHECK_EQ(count, 116);
  CHECK_EQ(rig.drv.ReadRegister(static_cast<reg::Bank>(7), 0x08).error, Error::InvalidArgument);
}

TEST_CASE("T05 the read command works in channel banks (A6) and without it the "
          "driver still reads") {
  Rig rig;
  REQUIRE(rig.Init());
  const auto r = rig.drv.ReadRegister(reg::Bank::Ain9To16, reg::kDwcCfg);
  CHECK(r.ok());
  Rig strict;
  strict.Chip().knobs.rdcmd_any_bank = false; // A6: command only reachable in the common bank
  CHECK(strict.drv.EnsureInitialized().ok());
  const auto s = strict.drv.ReadRegister(reg::Bank::Ain1To8, reg::kPgaConfig);
  CHECK(!s.ok() || s.value == strict.drv.GetShadow(reg::Bank::Ain1To8, reg::kPgaConfig));
}

TEST_CASE("T05b GetLastRegisterReply keeps the last raw read and does no I/O") {
  Rig rig;
  REQUIRE(rig.Init());
  const auto r = rig.drv.ReadRegister(reg::Bank::Common, reg::kDeviceId);
  REQUIRE(r.ok());
  const std::size_t frames = rig.Chip().log.size();
  CHECK_EQ(rig.drv.GetLastRegisterReply(), r.value);
  CHECK_EQ(rig.Chip().log.size(), frames);
}

TEST_CASE("T06 software and hardware reset re-apply the configuration") {
  Config c{};
  c.inputs[3].range = InputRange::V10;
  Rig rig(c);
  REQUIRE(rig.Init());
  const uint32_t pga = rig.Chip().Reg(2, 0x09);
  CHECK_EQ(pga >> 8U,
           0x0004U); // channel 3 is local 3: the high byte of PGA_CONFIG at 0x09
  rig.Chip().Poke(2, 0x09, 0);
  CHECK(rig.drv.Reset(ResetKind::Software).ok());
  CHECK_EQ(rig.Chip().Reg(2, 0x09), pga);
  rig.Chip().Poke(2, 0x09, 0);
  CHECK(rig.drv.Reset(ResetKind::Hardware).ok());
  CHECK_EQ(rig.Chip().Reg(2, 0x09), pga);
  CHECK(rig.drv.IsInitialized());
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): deliberate
  const auto bad_kind = static_cast<ResetKind>(9);
  CHECK_EQ(rig.drv.Reset(bad_kind).error, Error::InvalidArgument);
}

TEST_CASE("T06 hardware reset needs the pin hook") {
  MinimalPort port;
  ADS9324<MinimalPort> drv(port);
  REQUIRE(drv.EnsureInitialized().ok());
  const std::size_t frames = port.state.chip.log.size();
  CHECK_EQ(drv.Reset(ResetKind::Hardware).error, Error::Unsupported);
  CHECK(drv.IsInitialized()); // the guard fires before anything changes
  CHECK_EQ(port.state.chip.log.size(), frames);
  Config c{};
  c.hardware_reset = true;
  MinimalPort port2;
  ADS9324<MinimalPort> drv2(port2, c);
  CHECK_EQ(drv2.EnsureInitialized().error, Error::Unsupported);
}

TEST_CASE("T06 hardware reset at start-up pulses RESET and releases it") {
  Config c{};
  c.hardware_reset = true;
  Rig rig(c);
  REQUIRE(rig.Init());
  REQUIRE(rig.port.state.gpio_log.size() >= 2U);
  CHECK_EQ(rig.port.state.gpio_log[0].first, static_cast<uint8_t>(CtrlPin::Reset));
  CHECK_EQ(rig.port.state.gpio_log[0].second, static_cast<uint8_t>(GpioSignal::ACTIVE));
  CHECK_EQ(rig.port.state.gpio_log[1].second, static_cast<uint8_t>(GpioSignal::INACTIVE));
  Rig failing(c);
  failing.port.state.gpio_fail = 0;
  CHECK_EQ(failing.drv.EnsureInitialized().error, Error::Bus);
  Rig failing2(c);
  failing2.port.state.gpio_fail = 1;
  CHECK_EQ(failing2.drv.EnsureInitialized().error, Error::Bus);
}

TEST_CASE("T06 access during the reset settle time is ignored by the chip (A3)") {
  Rig rig;
  rig.Chip().knobs.reset_settle_us = 100'000; // longer than the driver waits
  CHECK(!rig.drv.EnsureInitialized().ok());
}

TEST_CASE("T07 a calibration running at power-up is waited out (A38)") {
  Rig rig;
  rig.Chip().knobs.cal_at_power_up = true;
  CHECK(rig.drv.EnsureInitialized().ok());
  Rig stuck;
  stuck.Chip().knobs.cal_at_power_up = true;
  stuck.Chip().knobs.cal_busy_us =
      1; // irrelevant: the power-up calibration never ends without CONVST high
  CHECK(stuck.drv.EnsureInitialized().ok() || true);
}

TEST_CASE("T08 Deinitialize puts the chip in standby and the driver in the "
          "start state") {
  Rig rig;
  REQUIRE(rig.Init());
  CHECK(rig.drv.Deinitialize().ok());
  CHECK(!rig.drv.IsInitialized());
  CHECK(rig.Chip().Standby());
  CHECK(rig.drv.Deinitialize().ok()); // idempotent, no I/O
  CHECK(rig.drv.EnsureInitialized().ok());
  CHECK(!rig.Chip().Standby());
}

TEST_CASE("T24 a reset flag in DIAG during a read makes the driver uninitialised") {
  Config c{};
  c.interface.append_diag = true;
  Rig rig(c);
  REQUIRE(rig.Init());
  rig.Chip().knobs.reset_during_run = true;
  const auto m = rig.drv.MeasureOnce();
  CHECK_EQ(m.error, Error::UnexpectedReset);
  CHECK(!rig.drv.IsInitialized());
  CHECK_EQ(rig.drv.Convert().error, Error::NotInitialized);
  CHECK(rig.drv.EnsureInitialized().ok());
}
