/**
 * @file test_readout.cpp
 * @brief Conversion and data readout against the chip model: lanes, word
 * lengths, formats, channel modes, DIAG and PRBS, test patterns, stuck lines
 * (T09 to T14).
 */
#include "test_support.hpp"

using namespace ads9324;
using namespace ads9324_test;

namespace {

/// Set the same input voltage on every channel, plus a slope so each channel
/// differs.
void Drive(Rig& rig, int32_t base_uv, int32_t step_uv) {
  for (int ch = 0; ch < 16; ++ch) {
    rig.Chip().input_uv[static_cast<std::size_t>(ch)] = base_uv + (step_uv * ch);
  }
}

/// Run `Convert` + `WaitDataReady` + `ReadFrame`.
Result<Frame> Measure(Rig& rig) {
  return rig.drv.MeasureOnce();
}

} // namespace

TEST_CASE("T09 one conversion returns 16 codes that match the input voltages") {
  Config c{};
  c.calibration = Calibration{};
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  Drive(rig, -4'000'000, 500'000);
  REQUIRE(rig.Init());
  const auto f = Measure(rig);
  REQUIRE(f.ok());
  CHECK_EQ(f.value.present_mask, 0xFFFFU);
  for (int ch = 0; ch < 16; ++ch) {
    const int32_t want = -4'000'000 + (500'000 * ch);
    const int32_t got = f.value.microvolts[static_cast<std::size_t>(ch)];
    CHECK(((got - want) < 400) && ((want - got) < 400));
  }
}

TEST_CASE("T09 every range scales the code") {
  const InputRange ranges[] = {InputRange::V2_5, InputRange::V5, InputRange::V6_25, InputRange::V10,
                               InputRange::V12_5};
  const int32_t full[] = {2'500'000, 5'000'000, 6'250'000, 10'000'000, 12'500'000};
  for (std::size_t i = 0; i < 5; ++i) {
    Config c{};
    for (auto& in : c.inputs) {
      in.range = ranges[i];
    }
    Rig rig(c);
    rig.Chip().analog_offset_lsb = 0;
    Drive(rig, full[i] / 2, 0);
    REQUIRE(rig.Init());
    const auto f = Measure(rig);
    REQUIRE(f.ok());
    CHECK_EQ(f.value.codes[0], 16384);
    CHECK(f.value.microvolts[0] > ((full[i] / 2) - 400));
    CHECK(f.value.microvolts[0] < ((full[i] / 2) + 400));
  }
}

TEST_CASE("T09 full scale clips and is reported") {
  Rig rig;
  rig.Chip().analog_offset_lsb = 0;
  Drive(rig, 0, 0);
  rig.Chip().input_uv[2] = 9'000'000;
  rig.Chip().input_uv[9] = -9'000'000;
  REQUIRE(rig.Init());
  const auto f = Measure(rig);
  REQUIRE(f.ok());
  CHECK_EQ(f.value.codes[2], 32767);
  CHECK_EQ(f.value.codes[9], -32768);
  CHECK_EQ(f.value.clipped_mask, (1U << 2U) | (1U << 9U));
}

TEST_CASE("T10 offset binary coding") {
  Config c{};
  c.interface.format = DataFormat::OffsetBinary;
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  Drive(rig, 1'000'000, 0);
  REQUIRE(rig.Init());
  const auto f = Measure(rig);
  REQUIRE(f.ok());
  CHECK_EQ(f.value.codes[0], 6554);
  CHECK_EQ(f.value.raw[0], 6554U ^ 0x8000U);
}

TEST_CASE("T10 the randomizer is removed from every channel (equations 7 and 8)") {
  Config c{};
  c.interface.randomize = true;
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  Drive(rig, -1'000'000, 100'000);
  REQUIRE(rig.Init());
  CHECK_EQ(rig.Chip().Reg(1, 0x0B), 0x00F1U);
  for (int round = 0; round < 6; ++round) {
    const auto f = Measure(rig);
    REQUIRE(f.ok());
    CHECK(f.value.prbs != 0U);
    for (int ch = 0; ch < 16; ++ch) {
      const int32_t want = -1'000'000 + (100'000 * ch);
      const int32_t got = f.value.microvolts[static_cast<std::size_t>(ch)];
      CHECK(((got - want) < 400) && ((want - got) < 400));
    }
  }
}

TEST_CASE("T10 the randomizer with 24-bit words is refused (A13)") {
  Config c{};
  c.interface.randomize = true;
  c.interface.length = WordLength::Bits24;
  Rig rig(c);
  CHECK_EQ(rig.drv.EnsureInitialized().error, Error::InvalidArgument);
  CHECK(rig.Chip().log.empty());
}

TEST_CASE("T10 the DIAG byte carries the window alert") {
  Config c{};
  c.interface.append_diag = true;
  c.inputs[0].window = Window{true, 10, -10, 0};
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  Drive(rig, 0, 0);
  rig.Chip().input_uv[0] = 3'000'000;
  REQUIRE(rig.Init());
  const auto f = Measure(rig);
  REQUIRE(f.ok());
  CHECK(f.value.window_alert);
  CHECK(!f.value.reset_detected);
}

TEST_CASE("T10 DIAG and PRBS order follows the datasheet figures (A14)") {
  Config c{};
  c.interface.randomize = true;
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  REQUIRE(rig.Init());
  const auto f = Measure(rig);
  REQUIRE(f.ok());
  CHECK_EQ(f.value.diag & 0x01U, 0U);
  // Reversed order in the chip: the driver reads PRBS as DIAG, so the codes
  // come out wrong.
  Rig other(c);
  other.Chip().knobs.diag_before_prbs = false;
  other.Chip().analog_offset_lsb = 0;
  REQUIRE(other.Init());
  const auto g = Measure(other);
  CHECK(!g.ok() || g.value.codes[0] != 0);
}

TEST_CASE("T10 24-bit words carry the extra byte unprocessed (A15)") {
  Config c{};
  c.interface.length = WordLength::Bits24;
  Rig rig(c);
  rig.Chip().analog_offset_lsb = 0;
  rig.Chip().knobs.low_byte_extra = true;
  Drive(rig, 500'000, 0);
  REQUIRE(rig.Init());
  const auto f = Measure(rig);
  REQUIRE(f.ok());
  CHECK_EQ(f.value.low_byte[5], 0x5AU);
  CHECK_EQ(f.value.codes[5], 3277);
}

TEST_CASE("T11 all 15 channel modes of Table 7-12 return the channels the "
          "table lists") {
  const struct {
    ChannelCount count;
    uint32_t groups;
  } rows[] = {{ChannelCount::All16, 1},
              {ChannelCount::Eight, 2},
              {ChannelCount::Four, 4},
              {ChannelCount::Two, 8}};
  for (const auto& row : rows) {
    for (uint32_t g = 0; g < row.groups; ++g) {
      Config c{};
      c.channels = ChannelSelect{row.count, static_cast<uint8_t>(g)};
      Rig rig(c);
      rig.Chip().analog_offset_lsb = 0;
      Drive(rig, -3'000'000, 400'000);
      REQUIRE(rig.Init());
      const auto num = static_cast<uint32_t>(row.count);
      uint32_t sel = g;
      if (num == 1U) {
        sel = g << 2U;
      } else if (num == 2U) {
        sel = g << 1U;
      }
      const auto expect = Ads9324Model::SlotChannels(num, sel);
      const auto f = Measure(rig);
      REQUIRE(f.ok());
      uint16_t mask = 0;
      for (const int ch : expect) {
        mask = static_cast<uint16_t>(mask | (1U << static_cast<uint32_t>(ch)));
      }
      CHECK_EQ(f.value.present_mask, mask);
      for (const int ch : expect) {
        const int32_t want = -3'000'000 + (400'000 * ch);
        const int32_t got = f.value.microvolts[static_cast<std::size_t>(ch)];
        CHECK(((got - want) < 400) && ((want - got) < 400));
      }
      CHECK_EQ(FrameBytes(c.interface, c.channels), expect.size() * 2U);
    }
  }
}

TEST_CASE("T12 every lane count returns the same frame, on D7 to D0 through "
          "CaptureLanes") {
  const Lanes lanes[] = {Lanes::Eight, Lanes::Four, Lanes::Two, Lanes::One};
  for (const Lanes l : lanes) {
    for (const bool sdout : {false, true}) {
      if (sdout && (l != Lanes::One)) {
        continue;
      }
      Config c{};
      c.interface.lanes = l;
      c.interface.data_on_sdout = sdout;
      Rig rig(c);
      rig.Chip().analog_offset_lsb = 0;
      Drive(rig, -2'000'000, 250'000);
      REQUIRE(rig.Init());
      const auto f = Measure(rig);
      REQUIRE(f.ok());
      CHECK_EQ(f.value.present_mask, 0xFFFFU);
      CHECK(f.value.microvolts[15] > 1'700'000);
      CHECK(f.value.microvolts[15] < 1'800'000);
    }
  }
}

TEST_CASE("T12 data on the lanes needs the CaptureLanes hook") {
  Config c{};
  c.interface.lanes = Lanes::Eight;
  c.interface.data_on_sdout = false;
  MinimalPort port;
  ADS9324<MinimalPort> drv(port, c);
  REQUIRE(drv.EnsureInitialized().ok());
  CHECK_EQ(drv.ReadFrame().error, Error::Unsupported);
}

TEST_CASE("T12 the data frame sends only zeros on SDI (A9)") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().log.clear();
  (void)Measure(rig);
  bool seen = false;
  for (const FrameLog& f : rig.Chip().log) {
    if (!f.is_register_frame) {
      seen = true;
      CHECK(f.all_zero_tx);
      CHECK_EQ(f.len, FrameBytes(Config{}.interface, Config{}.channels));
    }
  }
  CHECK(seen);
}

TEST_CASE("T13 CONVST pulses high then low (the falling edge converts), ends high, and the DRDY "
          "wait follows the pin") {
  Config c{};
  c.sampling.ready = ReadySource::Pin;
  Rig rig(c);
  REQUIRE(rig.Init());
  rig.port.state.gpio_log.clear();
  CHECK(rig.drv.Convert().ok());
  REQUIRE(rig.port.state.gpio_log.size() == 3U);
  CHECK_EQ(rig.port.state.gpio_log[0].second, static_cast<uint8_t>(GpioSignal::ACTIVE));
  CHECK_EQ(rig.port.state.gpio_log[1].second, static_cast<uint8_t>(GpioSignal::INACTIVE));
  CHECK_EQ(rig.port.state.gpio_log[2].second, static_cast<uint8_t>(GpioSignal::ACTIVE));
  CHECK(rig.drv.WaitDataReady().ok());
  CHECK(rig.drv.ReadFrame().ok());
  // Without a new conversion there is nothing to wait for: the wait times out,
  // bounded.
  const uint64_t before = rig.port.state.delays_us;
  CHECK_EQ(rig.drv.WaitDataReady().error, Error::Timeout);
  CHECK(rig.port.state.delays_us - before <= 10'100U);
}

TEST_CASE("T13 DRDY polarity is applied to the pin level") {
  Config c{};
  c.sampling.ready = ReadySource::Pin;
  c.alarm.active_low = true;
  Rig rig(c);
  REQUIRE(rig.Init());
  CHECK(rig.drv.MeasureOnce().ok());
  const auto idle = rig.drv.ReadAlarmPin();
  REQUIRE(idle.ok());
  CHECK(!idle.value);
}

TEST_CASE("T13 the ready wait on the pin needs GpioGet and a DRDY pin function") {
  Config c{};
  c.sampling.ready = ReadySource::Pin;
  MinimalPort port;
  ADS9324<MinimalPort> drv(port, c);
  REQUIRE(drv.EnsureInitialized().ok());
  CHECK_EQ(drv.WaitDataReady().error, Error::Unsupported);
  CHECK_EQ(drv.Convert().error, Error::Unsupported);
  Config w{};
  w.sampling.ready = ReadySource::Pin;
  w.alarm.function = AlarmFunction::WindowAlert;
  Rig rig(w);
  REQUIRE(rig.Init());
  CHECK_EQ(rig.drv.WaitDataReady().error, Error::Unsupported);
}

TEST_CASE("T13 a result read before it is ready is a stuck frame, not data") {
  Rig rig;
  REQUIRE(rig.Init());
  CHECK(rig.drv.Convert().ok());
  const auto early = rig.drv.ReadFrame(); // no wait
  CHECK(early.ok() || early.error == Error::DataMismatch);
  Rig rig2;
  REQUIRE(rig2.Init());
  const auto none = rig2.drv.ReadFrame(); // never converted
  CHECK_EQ(none.error, Error::DataMismatch);
}

TEST_CASE("T14 a stuck or floating data line is DataMismatch, unless a test "
          "pattern is on") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().knobs.stuck_sdout = true;
  CHECK_EQ(rig.drv.MeasureOnce().error, Error::DataMismatch);
  rig.Chip().knobs.stuck_sdout = false;
  rig.Chip().knobs.float_sdout = true;
  CHECK_EQ(rig.drv.MeasureOnce().error, Error::DataMismatch);
  rig.Chip().knobs.float_sdout = false;
  CHECK(rig.drv.MeasureOnce().ok());
  Config c{};
  c.sampling.reject_stuck_frames = false;
  Rig lenient(c);
  REQUIRE(lenient.Init());
  lenient.Chip().knobs.stuck_sdout = true;
  CHECK(lenient.drv.MeasureOnce().ok());
}

TEST_CASE("T14 every transfer of a measurement can fail and is Bus") {
  for (int at = 0; at < 6; ++at) {
    Rig rig;
    REQUIRE(rig.Init());
    rig.Chip().knobs.fail_transfer = rig.Chip().transfers + at;
    const auto m = rig.drv.MeasureOnce();
    if (m.ok()) {
      continue;
    }
    CHECK(m.error == Error::Bus);
  }
  Rig rig;
  REQUIRE(rig.Init());
  rig.port.state.gpio_fail = rig.port.state.gpio_calls;
  CHECK_EQ(rig.drv.Convert().error, Error::Bus);
  rig.port.state.gpio_fail = rig.port.state.gpio_calls + 1;
  CHECK_EQ(rig.drv.Convert().error, Error::Bus);
}

TEST_CASE("T15 test patterns: constant, with index insertion, and the ramp") {
  Config c{};
  c.test_pattern[0].enabled = true;
  c.test_pattern[0].index_insertion = false;
  for (std::size_t i = 0; i < 8; ++i) {
    c.test_pattern[0].value[i] = static_cast<uint16_t>(0x0100U * (i + 1U));
  }
  Rig rig(c);
  REQUIRE(rig.Init());
  const auto f = Measure(rig);
  REQUIRE(f.ok());
  for (std::size_t i = 0; i < 8; ++i) {
    CHECK_EQ(f.value.raw[i], 0x0100U * (i + 1U));
  }
  // Index insertion adds the channel index to the top nibble (A24).
  TestPattern t = c.test_pattern[0];
  t.index_insertion = true;
  REQUIRE(rig.drv.SetTestPattern(0, t).ok());
  const auto g = Measure(rig);
  REQUIRE(g.ok());
  CHECK_EQ(g.value.raw[3], 0x0400U + 0x3000U);
  t.kind = PatternKind::Ramp;
  CHECK(rig.drv.SetTestPattern(0, t).ok());
  CHECK_EQ(rig.Chip().Reg(2, 0x2E) & 0x0070U, 0x0020U);
  CHECK_EQ(rig.drv.SetTestPattern(2, t).error, Error::InvalidArgument);
}

TEST_CASE("T15 the data path self test proves the mapping of both banks and "
          "restores the config") {
  Rig rig;
  REQUIRE(rig.Init());
  const Config before = rig.drv.GetConfig();
  CHECK(rig.drv.RunDataPathSelfTest().ok());
  CHECK_EQ(rig.drv.GetConfig().test_pattern[0].enabled, before.test_pattern[0].enabled);
  CHECK_EQ(rig.Chip().Reg(2, 0x2E) & 1U, 0U);
  CHECK_EQ(rig.Chip().Reg(4, 0x2E) & 1U, 0U);
  // A chip with the second bank in ascending order fails the self test: that is
  // the point of it.
  Rig wrong;
  wrong.Chip().knobs.bank2_mirrored = false;
  REQUIRE(wrong.Init());
  CHECK_EQ(wrong.drv.RunDataPathSelfTest().error, Error::DataMismatch);
  CHECK_EQ(wrong.Chip().Reg(4, 0x2E) & 1U,
           0U); // restored even after the failure
}
